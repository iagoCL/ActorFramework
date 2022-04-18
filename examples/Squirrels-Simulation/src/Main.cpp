#include <iostream>

#include "../../../ActorFramework/include/ExecutionUnit.h"
#include "../../../ActorFramework/include/Logger.h"
#include "../../../ActorFramework/include/Message.h"

#include "../include/Main.h"
#include "../include/Cell.h"
#include "../include/Clock.h"
#include "../include/RandomGenerator.h"
#include "../include/Squirrel.h"
#include "../include/Tracker.h"

int main() {
    ActorFramework::Logger::setLogLevel(ActorFramework::Logger::EXHAUSTIVE);

    // First create one execution unit each rank
    ActorFramework::ExecutionUnit *executionUnit = new ActorFramework::ExecutionUnit();

    // Initialise the random number generator
    RandomGenerator::initialiseRNG(executionUnit->getRank());

    ActorFramework::Message createClockMessage((1 + NUM_CELLS) * sizeof(unsigned int), ActorFramework::Message::CREATE_NEW_ACTOR);
    auto actorsId = reinterpret_cast<unsigned int *>(createClockMessage.getMessageData()->data);

    // Creates prototype of a tracker
    Tracker trackerPrototype(0, executionUnit);
    // Registers protype of a tracker
    auto actorType = executionUnit->registerActorType(&trackerPrototype);
    if (executionUnit->isMaster()) {
        // Creates message to create a tracker
        ActorFramework::Message createTrackerMessage(0, ActorFramework::Message::CREATE_NEW_ACTOR);
        createTrackerMessage.getMessageData()->type = actorType;
        // Creates a tracker actor
        executionUnit->createNewActor(createTrackerMessage, &actorsId[NUM_CELLS]);
    }

    // Creates prototype of a cell
    ActorFramework::Message createActorMessage(sizeof(Cell::CreateNewCell), ActorFramework::Message::CREATE_NEW_ACTOR);
    auto cellData = reinterpret_cast<Cell::CreateNewCell *>(createActorMessage.getMessageData()->data);
    Cell cellPrototype(cellData, 0, executionUnit);
    // Registers protype of a cell
    actorType = executionUnit->registerActorType(&cellPrototype);
    if (executionUnit->isMaster()) {
        // Message to create a cell
        // Change next line to start a cell with an initial infection level
        cellData->initialInfections = cellData->initialPopulations = 0.0f;
        createActorMessage.getMessageData()->type = actorType;
        cellData->trackerId = actorsId[NUM_CELLS];

        for (cellData->cellId = 0; cellData->cellId < NUM_CELLS; ++cellData->cellId) {
            // Creates a cell actor
            executionUnit->createNewActor(createActorMessage, &actorsId[cellData->cellId]);
        }
    }

    // Creates prototype of a clock
    Clock clockPrototype(actorsId, 0, executionUnit);
    // Registers protype of a clock
    actorType = executionUnit->registerActorType(&clockPrototype);
    if (executionUnit->isMaster()) {
        // Creates message to create a clock
        createClockMessage.getMessageData()->type = actorType;
        // Creates a clock actor
        executionUnit->createNewActor(createClockMessage);
    }
    // Creates prototype of a squirrel
    auto squirrelParams = reinterpret_cast<Squirrel::SquirrelBasicData *>(createActorMessage.getMessageData()->data);
    Squirrel squirrelsPrototype(squirrelParams, 0, executionUnit);
    // Registers protype of a squirrel
    squirrelParams->squirrelActorType = executionUnit->registerActorType(&squirrelsPrototype);
    if (executionUnit->isMaster()) {
        // Creates message to create a squirrel
        createActorMessage.getMessageData()->type = squirrelParams->squirrelActorType;
        createActorMessage.messageSize = ActorFramework::Message::MESSAGE_DATA_HEADER_SIZE + sizeof(Squirrel::SquirrelBasicData);
        for (int i = 0; i <= NUM_CELLS; ++i) {
            squirrelParams->actorsId[i] = actorsId[i];
        }
        squirrelParams->isInfected = false;
        // Creates a squirrel actor
        for (int i = 0; i < (INITIAL_HEALTHY_SQUIRRELS + INITIAL_INFECTED_SQUIRRELS); ++i) {
            if (i == INITIAL_HEALTHY_SQUIRRELS) {
                squirrelParams->isInfected = !squirrelParams->isInfected;
            }
            squirrelParams->positionY = squirrelParams->positionX = 0.0f;
            squirrelParams->squirrelRandomPoss();
            executionUnit->createNewActor(createActorMessage);
        }
    }

    executionUnit->mainLoop();
    return 0;
}