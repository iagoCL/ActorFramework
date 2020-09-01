#ifndef SQUIRRELS_SIMULATION_SQUIRREL_H
#define SQUIRRELS_SIMULATION_SQUIRREL_H

#include "../Framework/Actor.h"
#include "Cell.h"
#include "Main.h"

#define STEPS_STORED_DATA 50u

class Squirrel : public Actor {
   public:
    typedef struct {
        unsigned int squirrelActorType;
        unsigned int actorsId[NUM_CELLS + 1];
        float positionX;
        float positionY;
        bool isInfected;
        void squirrelRandomPoss();
        const int getCellFromPosition() const;
    } SquirrelBasicData;

    Squirrel(const Squirrel::SquirrelBasicData* squirrelBasicData_,
             const unsigned int actorId_, ExecutionUnit* executionUnit_);
    void processMessage(const int messageSize, const Message::MessageData* messageData) override;
    void update() override;
    Actor* createNewActor(const unsigned int actorId_, const char* messageData) override;

   private:
    unsigned int stepsAlive;
    unsigned int stepsInfected;
    unsigned int numChildren;

    Cell::CellAverageLevel visitedCellsData[STEPS_STORED_DATA];
    Cell::CellAverageLevel lastVisitedCellsTotal;
    Cell::CellAverageLevel lastVisitedCellsAvg;

    SquirrelBasicData squirrelBasicData;

    int moveSquirrel();
    void sendJumpMessage(const int cellId);
    void sendTrackerMessage(const unsigned int status);
    void checkToInfect();
    bool checkToKill();
    void checkGiveBirth();

    operator std::string() const;

    /**
     * Determines whether a squirrel will give birth or not based upon the average population.
     */
    const bool willGiveBirth() const;
    /**
     * Determines whether a squirrel will catch the disease or not based upon the average infection level.
     */
    const bool willCatchDisease() const;
    /**
     * Determines if a squirrel will die or not.
     */
    static const bool willDie();

#ifdef USE_ORIGINAL_FUNCTIONS
    static const int getCellFromPosition(const float x, const float y);
    static const bool willCatchDisease(const float avg_inf_level);
    static const bool willGiveBirth(const float avg_pop);
    static void squirrelStep(const float x, const float y, float* x_new, float* y_new);
#endif
};

#endif