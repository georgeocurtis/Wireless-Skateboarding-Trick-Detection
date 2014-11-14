/* 
 * File:   main.c
 * Author: Ryan
 *
 * Created on September 17, 2014, 5:39 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#define _PLIB_DISABLE_LEGACY
#define MAG 0x3C
#define I2CMOD I2C1
#define ACL 0xA6
#define GYRO 0xD0

#define ACL_DATA_START 0x32
#define GYRO_DATA_START 0x1D
#define MAG_DATA_START 0x03

#include <plib.h>
#include "PmodOLED.h"
#include "OledChar.h"
#include "OledGrph.h"
#include "delay.h"

// Digilent board configuration
#pragma config ICESEL       = ICS_PGx1  // ICE/ICD Comm Channel Select
#pragma config DEBUG        = OFF       // Debugger Disabled for Starter Kit

#pragma config FNOSC        = PRIPLL	// Oscillator selection
#pragma config POSCMOD      = XT	// Primary oscillator mode
#pragma config FPLLIDIV     = DIV_2	// PLL input divider
#pragma config FPLLMUL      = MUL_20	// PLL multiplier
#pragma config FPLLODIV     = DIV_1	// PLL output divider
#pragma config FPBDIV       = DIV_8	// Peripheral bus clock divider
#pragma config FSOSCEN      = OFF	// Secondary oscillator enable

unsigned sec1000;

BYTE initI2C(BYTE id);
void startI2C(BYTE id);
void stopI2C(BYTE id);
BYTE sendByteI2C(BYTE id, BYTE data);
BYTE readByteI2C(BYTE id, BYTE ack);
int16_t getRegI2C(BYTE id, BYTE devID, BYTE reg);
BYTE writeI2C(BYTE id, BYTE devID, BYTE reg, BYTE data);
void initSPI(BYTE chan);
BYTE initI2C(BYTE id);
BYTE initACL(BYTE id);
BYTE initMAG(BYTE id);

// Interrupt handler - respond to timer-generated interrupt
#pragma interrupt InterruptHandler_2534 ipl1 vector 0
void InterruptHandler_2534( void )
{
   if( INT_SOURCE_TIMER(2) )            // Verify source of interrupt
   {
      sec1000++;                        // Update global variable
      INTClearFlag(INT_T2);             // Acknowledge interrupt
   }
}

int main(int argc, char** argv) {

    DelayInit();
    OledInit();

    ODCACLR = 0x01;
    ODCGCLR = 0x80;
    TRISGSET = 0x80;     // For BTN1 and 2: configure PortG bit for input
    TRISASET = 0x01;
    DDPCONbits.JTAGEN = 0;

    // Set up timer 2 to roll over every ms
   OpenTimer2(T2_ON         |
             T2_IDLE_CON    |
             T2_SOURCE_INT  |
             T2_PS_1_16     |
             T2_GATE_OFF,
             625);  // freq = 10MHz/16/625 = 1 kHz

   // Set up CPU to respond to interrupts from Timer2
   INTSetVectorPriority(INT_TIMER_2_VECTOR, INT_PRIORITY_LEVEL_1);
   INTClearFlag(INT_T2);
   INTEnable(INT_T2, INT_ENABLED);
   INTConfigureSystem(INT_SYSTEM_CONFIG_SINGLE_VECTOR);
   INTEnableInterrupts();

   OledClear();
   OledSetCursor(0, 0);

   initI2C(I2CMOD);

   if(!initACL(I2CMOD))
       OledPutString("ERRORACL");
   else
       OledPutString("Initted");
   
   if(!initMAG(I2CMOD))
       OledPutString("ERRORMAG");
   else
       OledPutString("Initted");

   while(1)
   {
       char numstring[10];
       sprintf(numstring, "%d", getRegI2C(I2CMOD, ACL, ACL_DATA_START));
       OledClear();
       OledSetCursor(0, 0);
       OledPutString(numstring);
   }

   return(EXIT_SUCCESS);
}

/* initSPI
 *
 * Inputs: none
 * Outputs: none
 *
 * This initializes the SPI to work fo the ACL
 */
void initSPI(BYTE chan)
{
    SpiChnOpen(chan, SPI_OPEN_MSTEN|SPI_OPEN_MSSEN|SPI_OPEN_CKP_HIGH|SPI_OPEN_MODE8|SPI_OPEN_ENHBUF, 5);
    SpiChnPutC(chan, 0x2D);
    SpiChnPutC(chan, 0x08);
    SpiChnGetC(chan);
    SpiChnGetC(chan);
}

int16_t getRegI2C(BYTE id, BYTE devID, BYTE reg)
{
    BYTE value1;
    BYTE value2;
    startI2C(id);
    if (!sendByteI2C(id, devID))
        return 11;
    if (!sendByteI2C(id, reg))
        return 22;
    I2CRepeatStart(id);
    while(!(I2CGetStatus(id) & I2C_START));
    if (!sendByteI2C(id, devID + 1))
        return 33;
    value1 = readByteI2C(id, 1);
    value2 = readByteI2C(id, 0);
    stopI2C(id);
    return value1 << 8 | value2;
}

/* readByteI2C
 *
 * Inputs: none
 * Outputs: BYTE        the data
 *
 * This reads a register from the accelerometer
 */
BYTE readByteI2C(BYTE id, BYTE ack)
{
    BYTE value;
    I2CReceiverEnable(id, 1);
    while (!I2CReceivedDataIsAvailable(id));
    value = I2CGetByte(id);
    I2CAcknowledgeByte(id, 0);
    while (!I2CAcknowledgeHasCompleted(id));
    return value;
}

/* sendByteI2C
 *
 * Inputs: BYTE data        The data to be transferred
 * Outputs: BYTE        1 if it was successful, 0 if not
 *
 * This sends a byte to the ACL and returns whether or not it was successful
 */
BYTE sendByteI2C(BYTE id, BYTE data)
{
    while(!I2CTransmitterIsReady(id));
    I2CSendByte(id, data);
    while (!I2CTransmissionHasCompleted(id));
    return I2CByteWasAcknowledged(id);
}

/* stopI2C
 *
 * Inputs: none
 * Outputs: none
 *
 * This stops to I2C bus
 */
void stopI2C(BYTE id)
{
    I2CStop(id);
    while (!(I2CGetStatus(id) & I2C_STOP));
}

/* startI2C
 *
 * Inputs: none
 * Outputs: none
 *
 * This starts the I2C bus
 */
void startI2C(BYTE id)
{
    while (!I2CBusIsIdle(id));
    I2CStart(id);
    while (!(I2CGetStatus(id) & I2C_START));
}

BYTE writeI2C(BYTE id, BYTE devID, BYTE reg, BYTE data)
{
    startI2C(id);
    if (!sendByteI2C(id, devID))
        return 0;
    if (!sendByteI2C(id, reg))
        return 0;
    if (!sendByteI2C(id, data))
        return 0;
    stopI2C(id);
    return 1;
}

BYTE initMAG(BYTE id)
{
    if (!writeI2C(id, MAG, 0x00, 0x70))
        return 0;
    if (!writeI2C(id, MAG, 0x01, 0x20))
        return 0;
    if (!writeI2C(id, MAG, 0x02, 0x00))
        return 0;
    return 1;
}

BYTE initACL(BYTE id)
{
    if (!writeI2C(id, ACL, 0x31, 0x01))
        return 0;
    if (!writeI2C(id, ACL, 0x2D, 0x08))
        return 0;
    return 1;
}

BYTE initI2C(BYTE id)
{
    I2CConfigure(id, 0x00);
    I2CSetFrequency(id, 10000000, 100000);
    I2CEnable(id, 1);
}
