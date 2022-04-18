#ifndef SQUIRRELS_SIMULATION_RANDOM_GENERATOR_H
#define SQUIRRELS_SIMULATION_RANDOM_GENERATOR_H
class RandomGenerator {
   public:
    static bool getRandomBool(float probability = 0.5f);
    static int getRandomInteger(int maxNumber, int minNumber = 0);
    static float getRandomFloat();
    /**
     * Initialises the random number generator, call it once at the start of the program on each process.
     * ran2 is a flawed RNG and must be used with particular care in parallel however it is perfectly sufficient
     * for this course work
     * The seed is modified and then passed to each other subsequent function that relies on the random generator,
     * which also modify the seed
     */
    static void initialiseRNG(int rankId);

   private:
    static float ran2(long* seed);
    static long seedValue;
};

#endif