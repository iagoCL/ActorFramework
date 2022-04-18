#include <iostream>

#include "../include/ActorB.h"

#include "../../../ActorFramework/include/Logger.h"

ActorB::ActorB(const unsigned int expectedData_, const unsigned int expectedCount_, const unsigned int totalNumProcess_, const unsigned int actorId_, ActorFramework::ExecutionUnit *executionUnit_)
    : expectedData(expectedData_),
      expectedCount(expectedCount_),
      totalNumProcess(totalNumProcess_),
      localData(0),
      localProcess(0),
      localCount(0),
      Actor(actorId_, executionUnit_) {
    if (actorId_ > 0) {
        logPrint(ActorFramework::Logger::LogLevel::EXHAUSTIVE, " Created Actor B: " + std::string(*this));
    } else {
        logPrint(ActorFramework::Logger::LogLevel::EXHAUSTIVE, " Created prototype Actor B: " + std::string(*this));
    }
    isActive = false;
}

void ActorB::processMessage(const int messageSize, const ActorFramework::Message::MessageData *messageData) {
    if (messageData->type == 0) {
        auto data = reinterpret_cast<const unsigned int *>(messageData->data);
        ++localProcess;
        localData += data[0];
        localCount += data[1];
        if (localProcess == totalNumProcess) {
            if (localData == expectedData && localCount == expectedCount) {
                logPrint(ActorFramework::Logger::LogLevel::NORMAL, "Actor B: Finished correctly" + std::string(*this));
            } else {
                logPrint(ActorFramework::Logger::LogLevel::ERROR, "ERROR finished with incorrect value Actor B: " + std::string(*this));
            }
            isFinished = true;
            executionUnit->finishProgram();
        } else {
            logPrint(ActorFramework::Logger::LogLevel::EXHAUSTIVE, "Actor B: Message received from: " + ActorFramework::Logger::numToString(messageData->origActorId) + ":" + std::string(*this));
        }
    } else {
        logPrint(ActorFramework::Logger::LogLevel::EXHAUSTIVE, "ERROR: Actor B: Should only receive messages of type 0");
    }
}
void ActorB::update() {
    logPrint(ActorFramework::Logger::LogLevel::ERROR, "ERROR: Actor B: Should not be updated");
};
ActorFramework::Actor *ActorB::createNewActor(const unsigned int actorId_, const char *messageData) {
    return new ActorB(expectedData, expectedCount, totalNumProcess, actorId_, executionUnit);
}
ActorB::operator std::string() const {
    return "{ process: " + ActorFramework::Logger::numToString(localProcess) + " (" + ActorFramework::Logger::numToString(totalNumProcess) 
    + ") data: " + ActorFramework::Logger::numToString(localData) + " (" + ActorFramework::Logger::numToString(expectedData) + " ) count: " 
    + ActorFramework::Logger::numToString(localCount) + " (" + ActorFramework::Logger::numToString(expectedCount) + " ) }";
}