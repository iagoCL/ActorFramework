#include <iostream>

#include "../include/ActorA.h"

#include "../../../ActorFramework/include/Logger.h"

ActorA::ActorA(const unsigned int data_, unsigned int actorBId_, const unsigned int actorId_, ActorFramework::ExecutionUnit *executionUnit_)
    : Actor(actorId_, executionUnit_),
      data(data_),
      count(0),
      actorBId(actorBId_) {
    if (actorId_ > 0) {
        logPrint(ActorFramework::Logger::LogLevel::EXHAUSTIVE, " Created Actor A: data: " + ActorFramework::Logger::numToString(data + count));
    } else {
        logPrint(ActorFramework::Logger::LogLevel::EXHAUSTIVE, " Created prototype Actor A");
    }
}

void ActorA::processMessage(const int messageSize, const ActorFramework::Message::MessageData *messageData) {
    logPrint(ActorFramework::Logger::LogLevel::EXHAUSTIVE, "ERROR: Actor A: Should not receive messages");
}
void ActorA::update() {
    if (++count == countUntil) {
        auto &sendMessage = createMessage(actorBId, 0, 2 * sizeof(int));
        auto sendData = reinterpret_cast<unsigned int *>(sendMessage.getMessageData()->data);
        sendData[0] = data;
        sendData[1] = count;
        executionUnit->communicateWithActor(sendMessage);
        logPrint(ActorFramework::Logger::LogLevel::NORMAL, " Actor A completed with data: " + ActorFramework::Logger::numToString(data + count));
        isActive = false;
        isFinished = true;
    } else {
        if ((count % printFreq) == 0) {
            logPrint(ActorFramework::Logger::LogLevel::EXHAUSTIVE, " Updating actor A: " + ActorFramework::Logger::numToString(data + count));
        }
        isActive = true;
    }
};
ActorFramework::Actor *ActorA::createNewActor(const unsigned int actorId_, const char *messageData) {
    auto data_ = reinterpret_cast<const unsigned int *>(messageData);
    return new ActorA(data_[0], data_[1], actorId_, executionUnit);
}