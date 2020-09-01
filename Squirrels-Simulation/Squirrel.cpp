#include "Squirrel.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "../Framework/Logger.h"
#include "RandomGenerator.h"
#include "Squirrel.h"
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define BIRTH_PROBABILITY 100.0f        // Decrease this to make more likely, increase less likely
#define DISEASE_PROBABILITY 1000.0f     // Decrease this to make more likely, increase less likely
#define DEATH_PROBABILITY 0.166666666f  // Decrease this to make less likely, increase more likely

#ifdef USE_ORIGINAL_FUNCTIONS
const bool Squirrel::willCatchDisease(const float avg_inf_level) {
    return (RandomGenerator::getRandomFloat() < (atan(((avg_inf_level < 40000 ? avg_inf_level : 40000)) / DISEASE_PROBABILITY) / M_PI));
}

void Squirrel::squirrelStep(const float x, const float y, float* x_new, float* y_new) {
    float diff = RandomGenerator::getRandomFloat();
    *x_new = (x + diff) - (int)(x + diff);

    diff = RandomGenerator::getRandomFloat();
    *y_new = (y + diff) - (int)(y + diff);
}

const bool Squirrel::willGiveBirth(const float avg_pop) {
    const float tmp = avg_pop / BIRTH_PROBABILITY;

    return (RandomGenerator::getRandomFloat() < (atan(tmp * tmp) / (4 * tmp)));
}

const int Squirrel::getCellFromPosition(const float x, const float y) {
    return ((int)(x * 4) + 4 * (int)(y * 4));
}
#endif

void Squirrel::SquirrelBasicData::squirrelRandomPoss() {
#ifdef USE_ORIGINAL_FUNCTIONS
    Squirrel::squirrelStep(squirrelBasicData.positionX, squirrelBasicData.positionY, &squirrelBasicData.positionX, &squirrelBasicData.positionY);
#else
    float diff = RandomGenerator::getRandomFloat();
    positionX = (positionX + diff) - static_cast<float>(static_cast<int>(positionX + diff));

    diff = RandomGenerator::getRandomFloat();
    positionY = (positionY + diff) - static_cast<float>(static_cast<int>(positionY + diff));
#endif
}

const bool Squirrel::willGiveBirth() const {
#ifdef USE_ORIGINAL_FUNCTIONS
    willGiveBirth(lastVisitedCellsAvg.populationInflux);
#else
    const float tmp = lastVisitedCellsAvg.populationInflux / BIRTH_PROBABILITY;
    const float value = atan(tmp * tmp) / (4 * tmp);

    return RandomGenerator::getRandomBool(value);
#endif
}

const bool Squirrel::willCatchDisease() const {
#ifdef USE_ORIGINAL_FUNCTIONS
    willCatchDisease(lastVisitedCellsAvg.infectionLevel);
#else
    const float value = atan(std::min(lastVisitedCellsAvg.infectionLevel, 40000.0f) / DISEASE_PROBABILITY) / M_PI;
    return RandomGenerator::getRandomBool(value);
#endif
}

const bool Squirrel::willDie() {
#ifdef USE_ORIGINAL_FUNCTIONS
    return RandomGenerator::getRandomFloat() < DEATH_PROBABILITY;
#else
    return RandomGenerator::getRandomBool(DEATH_PROBABILITY);
#endif
}

const int Squirrel::SquirrelBasicData::getCellFromPosition() const {
#ifdef USE_ORIGINAL_FUNCTIONS
    getCellFromPosition(positionX, positionY);
#else
    return (static_cast<int>(positionX * 4) + 4 * static_cast<int>(positionY * 4));
#endif
}

Squirrel::Squirrel(const Squirrel::SquirrelBasicData* squirrelBasicData_,
                   const unsigned int actorId_, ExecutionUnit* executionUnit_)
    : squirrelBasicData(*squirrelBasicData_),
      Actor(actorId_, executionUnit_),
      stepsAlive(0),
      stepsInfected(0),
      numChildren(0) {
    if (actorId_ != 0) {
        lastVisitedCellsAvg.set(0.0f);
        lastVisitedCellsTotal.set(0.0f);
        if (squirrelBasicData.isInfected) {
            sendTrackerMessage(SQUIRREL_SIMULATION_MESSAGES::SQUIRREL_STATUS_BORN_INFECTED);
        } else {
            sendTrackerMessage(SQUIRREL_SIMULATION_MESSAGES::SQUIRREL_STATUS_BORN);
        }
        for (int i = 0; i < STEPS_STORED_DATA; ++i) {
            visitedCellsData[i].set(0.0f);
        }

        logPrint(Logger::LogLevel::EXHAUSTIVE, "Squirrel born at: [ " + std::to_string(squirrelBasicData.positionX) + ", " + std::to_string(squirrelBasicData.positionY) + " ] " + std::string(*this));
    }
}

void Squirrel::processMessage(const int messageSize, const Message::MessageData* messageData) {
    if (messageData->type == SQUIRREL_SIMULATION_MESSAGES::CELL_INFECTION) {
        // Gets the cell avg. data
        const auto avgCellData = (reinterpret_cast<const Cell::CellAverageLevel*>(messageData->data));
        const auto stepId = (stepsAlive - 1) % STEPS_STORED_DATA;
        // Updates the new total
        lastVisitedCellsTotal.infectionLevel += avgCellData->infectionLevel - visitedCellsData[stepId].infectionLevel;
        lastVisitedCellsTotal.populationInflux += avgCellData->populationInflux - visitedCellsData[stepId].populationInflux;
        // Stores the cell data to be able to remove it in the next cycle
        visitedCellsData[stepId] = *avgCellData;
        const auto stepF = static_cast<float>(std::min(STEPS_STORED_DATA, stepsAlive));
        // Updates the averages
        lastVisitedCellsAvg.infectionLevel = lastVisitedCellsTotal.infectionLevel / stepF;
        lastVisitedCellsAvg.populationInflux = lastVisitedCellsTotal.populationInflux / stepF;
        // Set active to execute its next update
        isActive = true;
    } else {
        logPrint(Logger::LogLevel::ERROR, "ERROR: Squirrels: Should not receive messages of type: " + Logger::numToString(messageData->type));
    }
}

void Squirrel::update() {
    checkToInfect();
    // Infects if necessary
    checkGiveBirth();
    // Reproduces if necessary
    if (!checkToKill()) {
        // It must not die
        // Updates is position, informs the cell and waits for answer
        sendJumpMessage(moveSquirrel());
        ++stepsAlive;
        // Increases steps alive, it completes this update before being paused
    } else {
        // Is infected and must die.
        // Marked as finished to be erased.
        isActive = false;
        isFinished = true;
    }
}

Actor* Squirrel::createNewActor(const unsigned int actorId_, const char* messageData) {
    return new Squirrel(reinterpret_cast<const SquirrelBasicData*>(messageData), actorId_, executionUnit);
}

int Squirrel::moveSquirrel() {
    squirrelBasicData.squirrelRandomPoss();
    return squirrelBasicData.getCellFromPosition();
}

void Squirrel::sendJumpMessage(const int cellId) {
    // Sends a message to the cell indicating if its infected
    auto& sendMessage = createMessage(squirrelBasicData.actorsId[cellId], SQUIRREL_SIMULATION_MESSAGES::SQUIRREL_HOP, sizeof(bool));
    auto sendIsInfected = reinterpret_cast<bool*>(sendMessage.getMessageData()->data);
    sendIsInfected[0] = squirrelBasicData.isInfected;
    executionUnit->communicateWithActor(sendMessage);
    // Waits until receives the data of the other cell
    isActive = false;
}

void Squirrel::checkGiveBirth() {
    if (stepsAlive > 0 && (stepsAlive % STEPS_STORED_DATA) == 0 && willGiveBirth()) {
        //Extra data to show more detailed information
        ++numChildren;
        //logPrint(Logger::LogLevel::EXHAUSTIVE, "Squirrel give birth: " + std::string(*this));
        // Creates a new squirrel actor.
        Message message(sizeof(SquirrelBasicData), Message::CREATE_NEW_ACTOR);
        message.getMessageData()->type = squirrelBasicData.squirrelActorType;
        auto newSquirrelParameters = reinterpret_cast<SquirrelBasicData*>(message.getMessageData()->data);
        *newSquirrelParameters = squirrelBasicData;
        newSquirrelParameters->isInfected = false;
        executionUnit->createNewActor(message);
    }
}

void Squirrel::sendTrackerMessage(const unsigned int status) {
    auto& sendMessage = createMessage(squirrelBasicData.actorsId[NUM_CELLS], status, 0);
    executionUnit->communicateWithActor(sendMessage);
}

Squirrel::operator std::string() const {
    return "{ Id: " + Logger::numToString(actorId) + (squirrelBasicData.isInfected ? " is infected" : " is healthy") +
           " Num. steps alive: " + Logger::numToString(stepsAlive) +
           " Num. steps infected: " + Logger::numToString(stepsInfected) +
           " Num children: " + Logger::numToString(numChildren) + " }";
}

void Squirrel::checkToInfect() {
    if (squirrelBasicData.isInfected) {
        ++stepsInfected;
    } else {
        // Checks if must be infected all the steps
        squirrelBasicData.isInfected = willCatchDisease();
        if (squirrelBasicData.isInfected) {
            // Shows an extra message to the user when infected.
            logPrint(Logger::LogLevel::EXHAUSTIVE, "Squirrel Infected: " + std::string(*this));
            // The tracker knows the total amount of infected squirrels
            sendTrackerMessage(SQUIRREL_SIMULATION_MESSAGES::SQUIRREL_STATUS_INFECTED);
        }
    }
}

bool Squirrel::checkToKill() {
    if (stepsInfected > STEPS_STORED_DATA && willDie()) {
        // Lives for a minimum of 50 steps before checking to die
        sendTrackerMessage(SQUIRREL_SIMULATION_MESSAGES::SQUIRREL_STATUS_KILLED);
        // The tracker must know the total number of squirrels
        logPrint(Logger::LogLevel::EXHAUSTIVE, "Squirrel Killed: " + std::string(*this));
        return true;
    } else {
        return false;
    }
}