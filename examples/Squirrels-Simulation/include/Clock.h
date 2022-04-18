#ifndef SQUIRRELS_SIMULATION_CLOCK_H
#define SQUIRRELS_SIMULATION_CLOCK_H

#include "../../../ActorFramework/include/Actor.h"

#include "Main.h"

class Clock : public ActorFramework::Actor {
   public:
    Clock(const unsigned int communicateProcessesId_[NUM_CELLS + 1], const unsigned int actorId_, ActorFramework::ExecutionUnit *executionUnit_);
    void processMessage(const int messageSize, const ActorFramework::Message::MessageData *messageData) override;
    void update() override;
    ActorFramework::Actor *createNewActor(const unsigned int actorId_, const char *messageData) override;

   private:
    unsigned int month;
    int updatesToNextMonth;
    long seed;
    unsigned int communicateProcessesId[NUM_CELLS + 1];
};

#endif