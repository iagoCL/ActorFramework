#ifndef ACTOR_FRAMEWORK_MESSAGE_H
#define ACTOR_FRAMEWORK_MESSAGE_H

#include <iostream>

#include "mpi.h"

#ifndef MAX_BYTES_PER_MESSAGE_DATA
/**
 * @brief Can be changed to increase the maximum number of bytes per message.
 *
 */
#define MAX_BYTES_PER_MESSAGE_DATA 100
#endif//MAX_BYTES_PER_MESSAGE_DATA

namespace ActorFramework {
/**
 * @brief Messages are shared between different execution units for internal and intra-actor communication.
 */
    class Message {
    public:
        /**
         * @brief Maximum number of bytes that can be send in a message between actors
         * Default value of 100.
         * Recompile using the define MAX_BYTES_PER_MESSAGE_DATA to change it.
         * Includes the internal header.
         *
         */
        static const int MAX_MESSAGE_DATA = MAX_BYTES_PER_MESSAGE_DATA;

        /**
         * @brief Size of the internal header used for messages between actors.
         *
         */
        static const int MESSAGE_DATA_HEADER_SIZE = 3 * sizeof(int);

        /**
         * @brief Types of messages used internally
         */
        enum MessagesTags {
            /**Check for the creation of a new actor*/
            ASK_CREATE_NEW_ACTOR,
            /**Creates a new actor*/
            CREATE_NEW_ACTOR,
            /**Deletes an actor*/
            DELETE_ACTOR,
            /**Stops all actors*/
            STOP_ALL_ACTORS,
            /**Communication between actors, usually created by the user*/
            ACTORS_MESSAGE,
            /**Resends a message, usually due to failure*/
            REDIRECTED_ACTORS_MESSAGE,
            /**Resends a message using the master*/
            REDIRECTED_ACTORS_MESSAGE_MASTER
        };

        /**
         * @brief Message send between actor
         */
        typedef struct {
            /** @brief Actor who sends the message */
            unsigned int origActorId;
            /** @brief Actor who will receive the message */
            unsigned int destActorId;
            /** 
             * @brief Can be used by the programmer to identify messages.
             * Similar to a MPI tag.
             */
            unsigned int type;
            /** 
             * @brief Custom data sent between actors.
             * We recommend re-interpret it to the correct type.
             */
            char data[MAX_MESSAGE_DATA];
        } MessageData;

        /** @brief Maximum size of message, including header */
        static const int MAX_MESSAGE_SIZE = sizeof(MessageData);

        /** @brief Number of bytes used by the message */
        int messageSize;
        /** @brief Usually the type of MessagesTags */
        int tag;
        /**
         * @brief MPI_Request that handles the message.
         * Can be used to check its status.
         */
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
        * @brief Constructs a new Message object
        *
        * @param messageSize_ Size of the message in bytes
        * @param tag_ Tag of the message.
                    The message tag is usually internal and usually of type MessagesTags
                    Some functions may set the correct tag
        */
        Message(int messageSize_, int tag_);

        /**
        * @brief Construct a new Message object
        *
        * @param origActorId Id of the actor origin
        * @param destActorId Id of the actor destination
        * @param type Type of the message
                    This can be used for the user to identify a type of message.
                    Works similar to an MPI tag
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
        * @return std::string Message as a text. Design to facilitate debugging.
        */
        operator std::string() const;
    };
}//ActorFramework

#endif//ACTOR_FRAMEWORK_MESSAGE_H