#ifndef SQUIRRELS_SIMULATION_TRACKER_H
#define SQUIRRELS_SIMULATION_TRACKER_H

#include "../Framework/Actor.h"
#include "Cell.h"
#include "Main.h"

class Tracker : public Actor {
   public:
    Tracker(const unsigned int actorId_, ExecutionUnit *executionUnit_);
    void processMessage(const int messageSize, const Message::MessageData *messageData) override;
    void update() override;
    Actor *createNewActor(const unsigned int actorId_, const char *messageData) override;

   private:
    typedef struct {
        unsigned int receivedMessages;
        unsigned int numSquirrels;
        unsigned int numInfectedSquirrels;
        unsigned int numDeathSquirrels;
        Cell::CellSumaryData avgCellInfection[NUM_CELLS];
    } StepInfo;

    unsigned int numSquirrels;
    unsigned int numInfectedSquirrels;
    unsigned int numDeathSquirrels;
    StepInfo steps[NUM_MONTHS];

    void checkStepCompleted(const unsigned int stepNumber, StepInfo &stepInfo);
    static std::string stepInfoToString(const unsigned int stepNumber, StepInfo &stepInfo);
};

#endif