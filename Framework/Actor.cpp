#include "Actor.h"

#include <algorithm>

#include "Logger.h"

Actor::Actor(const unsigned int actorId_, ExecutionUnit *executionUnit_)
    : actorId(actorId_),
      executionUnit(executionUnit_),
      isFinished(false),
      messagesWaiting(false),
      isActive(true) {
    if (actorId > 0) {
        for (int i = 0; i < MAX_NUM_MESSAGES_PER_ACTOR; ++i) {
            // Sets all origin actor to this Id
            sendMessages[i].getMessageData()->origActorId = actorId;
            // Send all tags to the correct one.
            sendMessages[i].tag = Message::ACTORS_MESSAGE;
        }
    }
}
void Actor::checkSendMessages() {
    if (messagesWaiting) {
        for (int i = 0; i < MAX_NUM_MESSAGES_PER_ACTOR; ++i) {
            if (sendMessages[i].messageSize >= 0) {
                const bool completed = sendMessages[i].isCompleted();
                if (completed) {
                    sendMessages[i].messageSize = -1;
                } else {
                    messagesWaiting = true;
                    return;
                }
            }
        }
    }
    messagesWaiting = false;
}
bool Actor::checkKill() {
    if (isFinished) {
        checkSendMessages();
        return !messagesWaiting;
    } else {
        return false;
    }
}
Message &Actor::createMessage(unsigned int destActorId, unsigned int type, int messageSize) {
    int i;
    bool found = false;
    for (i = 0;; i = (i + 1) % MAX_NUM_MESSAGES_PER_ACTOR) {
        // Iterates until finding a  place to store the message
        if (sendMessages[i].messageSize < 0) {
            found = true;
        } else {
            found = sendMessages[i].isCompleted();
        }
        if (found) {
            // Stores the message
            sendMessages[i].getMessageData()->destActorId = destActorId;
            sendMessages[i].getMessageData()->type = type;
            sendMessages[i].messageSize = messageSize + Message::MESSAGE_DATA_MIN_SIZE;
            messagesWaiting = true;
            // Return the message to be edited.
            return sendMessages[i];
        }
    }
}

void Actor::logPrint(int logLevel, const std::string &text) {
    Logger::printMessage(actorId, executionUnit->getRank(), logLevel, text);
}
