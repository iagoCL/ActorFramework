#include "Message.h"

#include "Logger.h"

Message::Message() : messageSize(-1) {}
Message::Message(int messageSize_, int tag_) : messageSize(messageSize_ + MESSAGE_DATA_MIN_SIZE), tag(tag_) {}
Message::Message(unsigned int origActorId, unsigned int destActorId, unsigned int type, int messageSize_, int tag_)
    : messageSize(messageSize_ + MESSAGE_DATA_MIN_SIZE), tag(tag_) {
    messageData.type = type;
    messageData.origActorId = origActorId;
    messageData.destActorId = destActorId;
}
Message::operator std::string() const {
    return "Message: { from: " + Logger::numToString(messageData.origActorId) + " to: " + Logger::numToString(messageData.destActorId) +
           " type: " + Logger::numToString(messageData.type) + " tag: " + Logger::numToString(tag) + " messageSize: " + Logger::numToString(messageSize) + /*" data: " + (messageData.data) +*/ " }";
}
bool Message::isCompleted() {
    int flag;
    MPI_Test(&req, &flag, MPI_STATUS_IGNORE);
    return flag;
}