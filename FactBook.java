package geushmond.funfacts;

import java.util.Random;

public class FactBook {

    private String [] facts = {"The moon is big",
                               "The earth is big too"};

    public String getFact() {
        // Change to new fact
        String fact = "";

        Random randomGen = new Random();

        int rand = randomGen.nextInt(facts.length);
        fact = facts[rand];
        return fact;
    }
}
