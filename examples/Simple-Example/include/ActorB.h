#ifndef SIMPLE_EXAMPLE_ACTOR_B_H
#define SIMPLE_EXAMPLE_ACTOR_B_H

#include "../../../ActorFramework/include/Actor.h"

class ActorB : public ActorFramework::Actor {
   private:
    const unsigned int expectedData;
    const unsigned int expectedCount;
    const unsigned int totalNumProcess;
    int unsigned localData;
    int unsigned localCount;
    unsigned int localProcess;

   public:
    ActorB(const unsigned int expectedData_, const unsigned int expectedCount_, const unsigned int totalNumProcess_, const unsigned int actorId_, ActorFramework::ExecutionUnit *executionUnit_);
    void processMessage(const int messageSize, const ActorFramework::Message::MessageData *messageData) override;
    void update() override;
    ActorFramework::Actor *createNewActor(const unsigned int actorId_, const char *messageData) override;

    operator std::string() const;
};

#endif