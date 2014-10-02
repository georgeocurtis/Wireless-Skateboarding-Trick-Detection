package geushmond.funfacts;

import android.graphics.Color;

import java.util.Random;

/**
 * Created by ryan on 10/2/14.
 */
public class ColorWheel {
    private int [] colors = {Color.RED,
                             Color.GREEN,
                             Color.BLUE,
                             Color.BLACK,
                             Color.CYAN};

    public int getColor() {
        // Change to new fact
        int color = 0;

        Random randomGen = new Random();

        int rand = randomGen.nextInt(colors.length);
        color = colors[rand];
        return color;
    }
}
