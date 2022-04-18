#ifndef SIMPLE_EXAMPLE_ACTOR_A_H
#define SIMPLE_EXAMPLE_ACTOR_A_H

#include "../../../ActorFramework/include/Actor.h"
#include "../../../ActorFramework/include/Message.h"

class ActorA : public ActorFramework::Actor {
   private:
    const unsigned int data;
    const unsigned int actorBId;
    unsigned int count;

   public:
    static const unsigned int countUntil = 99;
    static const unsigned int printFreq = 10;

    ActorA(const unsigned int data_, const unsigned int actorBId_, const unsigned int actorId_, ActorFramework::ExecutionUnit *executionUnit_);
    void processMessage(const int messageSize, const ActorFramework::Message::MessageData *messageData) override;
    void update() override;
    ActorFramework::Actor *createNewActor(const unsigned int actorId_, const char *messageData) override;
};

#endif