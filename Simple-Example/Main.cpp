#include "Main.h"

#include <iostream>

#include "../Framework/ExecutionUnit.h"
#include "../Framework/Logger.h"
#include "../Framework/Message.h"
#include "ActorA.h"
#include "ActorB.h"

#define NUM_ACTORS_A 500

int main() {
    Logger::setLogLevel(Logger::EXHAUSTIVE);

    const auto countUntil = ActorA::countUntil;
    unsigned int data = 0;
    for (int i = 1; i <= NUM_ACTORS_A; ++i) {
        data += (countUntil + 1) * i;
    }

    // First create one execution unit each rank
    ExecutionUnit *executionUnit = new ExecutionUnit();
    // Create prototypes of all participating actors types
    ActorB actorB(data, NUM_ACTORS_A * countUntil, NUM_ACTORS_A, 0, executionUnit);
    ActorA actorA(0, 0, 0, executionUnit);

    // Register actor prototypes
    const auto actorBType = executionUnit->registerActorType(&actorB);
    const auto actorAType = executionUnit->registerActorType(&actorA);

    // Creates the actors from master ( they are distributed)
    if (executionUnit->isMaster()) {
        // Store parameters used to create each actor
        Message createActorMessage(0, Message::CREATE_NEW_ACTOR);
        createActorMessage.getMessageData()->type = actorBType;

        // Stores the actor id of each new actor
        unsigned int newActorsId[NUM_ACTORS_A + 1];

        // Creates a new actor
        executionUnit->createNewActor(createActorMessage, &newActorsId[0]);

        // Change actor creation parameters
        createActorMessage.getMessageData()->type = actorAType;
        createActorMessage.messageSize = Message::MESSAGE_DATA_MIN_SIZE + 3 * sizeof(unsigned int);
        auto params = reinterpret_cast<int *>(createActorMessage.getMessageData()->data);
        params[1] = newActorsId[0];

        for (int i = 1; i <= NUM_ACTORS_A; ++i) {
            params[0] = (countUntil + 1) * i;

            // Creates an actor of type A
            executionUnit->createNewActor(createActorMessage, &newActorsId[i]);
        }
    }

    // Enters infinite loop to update each actor.
    executionUnit->mainLoop();
    return 0;
}