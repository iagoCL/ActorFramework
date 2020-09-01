#ifndef ACTOR_FRAMEWORK_MESSAGE_H
#define ACTOR_FRAMEWORK_MESSAGE_H

#include <iostream>

#include "mpi.h"

class Message {
   public:
    static const int MAX_MESSAGE_DATA = 100;
    static const int MESSAGE_DATA_MIN_SIZE = 3 * sizeof(int);

    enum MessagesTags {
        ASK_CREATE_NEW_ACTOR,
        CREATE_NEW_ACTOR,
        DELETE_ACTOR,
        STOP_ALL_ACTORS,
        ACTORS_MESSAGE,
        REDIRECTED_ACTORS_MESSAGE,
        REDIRECTED_ACTORS_MESSAGE_MASTER
    };

    typedef struct {
        unsigned int origActorId;
        unsigned int destActorId;
        unsigned int type;
        char data[MAX_MESSAGE_DATA];
    } MessageData;
    static const int MAX_MESSAGE_SIZE = sizeof(MessageData);

    int messageSize;
    int tag;
    MPI_Request req;

   private:
    MessageData messageData;

   public:
    /**
     * @brief Construct a new Message object
     *
     */
    Message();
    /**
     * @brief Construct a new Message object
     *
     * @param messageSize_ Size of the message in bytes
     * @param tag_ Tag of the message. Some functions may set the correct tag
     */
    Message(int messageSize_, int tag_);
    /**
     * @brief Construct a new Message object
     *
     * @param origActorId Id of the actor origin
     * @param destActorId Id of the actor destination
     * @param type Type of the message
     * @param messageSize_ Size of the message in bytes
     * @param tag_ Tag of the message. Some functions may set the correct tag
     */
    Message(unsigned int origActorId, unsigned int destActorId, unsigned int type = 0, int messageSize_ = 0, int tag_ = ACTORS_MESSAGE);
    /**
     * @brief Get the Message Data object
     *
     * @return MessageData* Data send by the message
     */
    MessageData *getMessageData() { return &messageData; }
    /**
     * @brief Checks if the message has been send.
     * Uses the internal request not set by all send functions.
     *
     * @return true The message has been copied and the message can be erased or reused.
     * @return false The message is still been copied and cannot be reused.
     */
    bool isCompleted();
    /**
     * @brief Transforms the message to a string. Used to debug information.
     *
     * @return std::string Message as a text to easy debug.
     */
    operator std::string() const;
};

#endif