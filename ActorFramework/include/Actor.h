#ifndef ACTOR_FRAMEWORK_ACTOR_H
#define ACTOR_FRAMEWORK_ACTOR_H

#include <vector>

#include "ExecutionUnit.h"
#include "Message.h"

#ifndef MAX_NUM_MESSAGES_PER_ACTOR
/**
 * @brief Can be changed to control the number of sent messages that an actor can buffered.
 *
 */
#define MAX_NUM_MESSAGES_PER_ACTOR 25
#endif//MAX_NUM_MESSAGES_PER_ACTOR

namespace ActorFramework {
    class ExecutionUnit;

    /**
     * @brief This is the main class of the program.
     * Actors will be in charge of executing the simulation.
     * Communication between actors is done using messages.
     */
    class Actor {
    public:
        /**
        * @brief OVERRIDE: This function is in used to process new communications between different actors.
        * Each actor should override this function in order to handle its corresponding messages.
        * This 
        *
        * @param messageSize Size in bytes of the message (including header)
        * @param messageData Data send by the message, the header will include the origin actor, the origin destiny, and the type of message.
        */
        virtual void processMessage(const int messageSize, const Message::MessageData *messageData) = 0;

        /**
        * @brief OVERRIDE: Function called to update one actor.
        * If the actor is active this function will be called continuously.
        * Actors waiting for messages will be non-active and skip the update.
        *
        */
        virtual void update() = 0;

        /**
        * @brief OVERRIDE: Creates a New Actor object.
        * An usual implementation will call the normal constructor of the corresponding actor.
        * messageData its sent across multiple execution units.
        * We need a generic function capable of initialise each actor type .
        *
        * @param actorId_ Actor Id of the new actor
        * @param messageData Data used to create the new actor. An usual implementation will reinterpret it as the correct type.
        * @return Actor* New created actor.
        */
        virtual Actor *createNewActor(const unsigned int actorId_, const char *messageData) = 0;

        /**
        * @brief Gets if the actor is active or paused
        *
        * @return true The actor is active (update will be called)
        * @return false The actor is paused (update is not called) usually waiting for data
        */
        bool getActive() const { return isActive; };

        /**
        * @brief INTERNAL: Checks if the actor has finished. If it is finished checks
        * if all messages have been sent
        *
        * @return true The actor is still working or message are waiting to be sent.
        * @return false The actor is finished and does not have more messages waiting. It can be deleted
        */
        bool checkKill();

        /**
        * @brief Creates a sent Message.
        * Stores it on a free position of the buffer to wait until is copied.
        * executionUnit->communicateWithActor(message) MUST be called to send the message.
        * Creating and storing the message before sending it allows more complex patterns
        *
        * @param destActorId Actor id of the destination.
        * @param type Message type.
        * @param messageSize Message size in bytes.
        * @return Message& Message data to be set by the developer before its send.
        */
        Message &createMessage(unsigned int destActorId, unsigned int type = 0, int messageSize = 0);

    protected:

        /**
         * @brief Execution unit in charge of executing this actor.
         */
        ExecutionUnit *executionUnit;
        /**
         * @brief Id of the actor. Is unique and identical across all execution units.
         */
        const unsigned int actorId;
        /**
         * @brief Is waiting due to messages.
         */
        bool messagesWaiting;
        /**
         * @brief Buffer with all messages to be sent.
         */
        Message sendMessages[MAX_NUM_MESSAGES_PER_ACTOR];

        /**
         * @brief If true the actor is still active.
         */
        bool isActive;

        /**
         * @brief If true the actor has already finish.
         */
        bool isFinished;

        /**
        * @brief Prints a log message.
        * Calls to Logger::printMessage with the corresponding parameters.
        *
        * @param logLevel Level of the printed text
        * @param text Text to be printed
        */

        void logPrint(int logLevel, const std::string &text);

        /**
        * @brief INTERNAL: Check if there are any message waiting to be sent.
        * Updates messagesWaiting.
        *
        */
        void checkSendMessages();

        /**
        * @brief INTERNAL: Construct a new Actor
        *
        * @param actorId_ Id of the new actor.
        * @param executionUnit_ Execution unit which stores the actor.
        */
        Actor(const unsigned int actorId_, ExecutionUnit *executionUnit_);
    };
}//ActorFramework

#endif//ACTOR_FRAMEWORK_ACTOR_H