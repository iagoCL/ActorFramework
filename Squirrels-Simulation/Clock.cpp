#include "Clock.h"

#include "../Framework/Logger.h"
#include "Main.h"
#include "RandomGenerator.h"

#define MINIMUM_UPDATE_FREQUENCY 425.0f
#define MAXIMUM_UPDATE_FREQUENCY 600.0f

Clock::Clock(const unsigned int communicateProcessesId_[NUM_CELLS + 1], const unsigned int actorId_, ExecutionUnit *executionUnit_)
    : Actor(actorId_, executionUnit_),
      month(0),
      updatesToNextMonth(RandomGenerator::getRandomInteger(MAXIMUM_UPDATE_FREQUENCY, MINIMUM_UPDATE_FREQUENCY)) {
    if (actorId_ != 0) {
        for (int i = 0; i <= NUM_CELLS; ++i) {
            communicateProcessesId[i] = communicateProcessesId_[i];
        }
    }
}
void Clock::processMessage(const int messageSize, const Message::MessageData *messageData) { logPrint(Logger::LogLevel::ERROR, "ERROR: Clock: Should not receive messages"); }
void Clock::update() {
    if (--updatesToNextMonth == 0) {
        // Checks if a new month has passed
        for (const auto sendActorId : communicateProcessesId) {
            // Sends a new month message to all needed actors
            auto &sendMessage = createMessage(sendActorId, SQUIRREL_SIMULATION_MESSAGES::NEW_MONTH, sizeof(unsigned int));
            auto sendData = reinterpret_cast<unsigned int *>(sendMessage.getMessageData()->data);
            sendData[0] = month;
            executionUnit->communicateWithActor(sendMessage);
        }
        if (++month == NUM_MONTHS) {
            // Checked if the simulation must end
            isActive = false;
            isFinished = true;
        } else {
            updatesToNextMonth = RandomGenerator::getRandomInteger(MAXIMUM_UPDATE_FREQUENCY, MINIMUM_UPDATE_FREQUENCY);  //Number of updates of the next month
        }
    }
}
Actor *Clock::createNewActor(const unsigned int actorId_, const char *messageData) {
    return new Clock(reinterpret_cast<const unsigned int *>(messageData), actorId_, executionUnit);
}