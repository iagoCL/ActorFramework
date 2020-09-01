#include "ActorA.h"

#include <iostream>

#include "../Framework/Logger.h"

ActorA::ActorA(const unsigned int data_, unsigned int actorBId_, const unsigned int actorId_, ExecutionUnit *executionUnit_)
    : Actor(actorId_, executionUnit_),
      data(data_),
      count(0),
      actorBId(actorBId_) {
    if (actorId_ > 0) {
        logPrint(Logger::LogLevel::EXHAUSTIVE, " Created Actor A: data: " + Logger::numToString(data + count));
    } else {
        logPrint(Logger::LogLevel::EXHAUSTIVE, " Created prototype Actor A");
    }
}

void ActorA::processMessage(const int messageSize, const Message::MessageData *messageData) {
    logPrint(Logger::LogLevel::EXHAUSTIVE, "ERROR: Actor A: Should not receive messages");
}
void ActorA::update() {
    if (++count == countUntil) {
        auto &sendMessage = createMessage(actorBId, 0, 2 * sizeof(int));
        auto sendData = reinterpret_cast<unsigned int *>(sendMessage.getMessageData()->data);
        sendData[0] = data;
        sendData[1] = count;
        executionUnit->communicateWithActor(sendMessage);
        logPrint(Logger::LogLevel::NORMAL, " Actor A completed with data: " + Logger::numToString(data + count));
        isActive = false;
        isFinished = true;
    } else {
        if ((count % printFreq) == 0) {
            logPrint(Logger::LogLevel::EXHAUSTIVE, " Updating actor A: " + Logger::numToString(data + count));
        }
        isActive = true;
    }
};
Actor *ActorA::createNewActor(const unsigned int actorId_, const char *messageData) {
    auto data_ = reinterpret_cast<const unsigned int *>(messageData);
    return new ActorA(data_[0], data_[1], actorId_, executionUnit);
}