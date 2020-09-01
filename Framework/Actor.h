#ifndef ACTOR_FRAMEWORK_ACTOR_H
#define ACTOR_FRAMEWORK_ACTOR_H

#include <vector>

#include "ExecutionUnit.h"
#include "Message.h"

#define MAX_NUM_MESSAGES_PER_ACTOR 25

class ExecutionUnit;
class Actor {
   public:
    /**
     * @brief OVERRIDE: INTERNAL: Function call to process a new communication
     * message between different actors.
     *
     * @param messageSize Size in bytes of the message
     * @param messageData Data send by the message
     */
    virtual void processMessage(const int messageSize, const Message::MessageData *messageData) = 0;
    /**
     * @brief OVERRIDE: Function called to update one actor.
     *
     */
    virtual void update() = 0;
    /**
     * @brief OVERRIDE: Create a New Actor object based on the called actor class
     *
     * @param actorId_ Actor Id of the new actor
     * @param messageData Data used to create the new actor. Is sent across
     * multiple execution units.
     * @return Actor* New created actor.
     */
    virtual Actor *createNewActor(const unsigned int actorId_, const char *messageData) = 0;
    /**
   * @brief Gets if the actor is active or paused
   *
   * @return true The actor is active (update will be called)
   * @return false The actor is paused (update is not called)
   */
    bool getActive() const { return isActive; };
    /**
   * @brief INTERNAL: Checks if the actor has finished. If it is finished checks
   * if all messages have been sent
   *
   * @return true The actor is still working or message are waiting to be sent.
   * @return false The actor is finished and does not have more messages
   * waiting. It can be deleted
   */
    bool checkKill();
    /**
     * @brief Creates a sent Message.
     * Stores it on a free position of the buffer to wait until is copied.
     * executionUnit->communicateWithActor(message) MUST be called to send the message.
     *
     * @param destActorId Actor id of the destination.
     * @param type Message type.
     * @param messageSize Message size in bytes.
     * @return Message& Message data to be set by the developer before its send.
     */
    Message &createMessage(unsigned int destActorId, unsigned int type = 0, int messageSize = 0);

   protected:
    ExecutionUnit *executionUnit;
    const unsigned int actorId;
    bool messagesWaiting;
    Message sendMessages[MAX_NUM_MESSAGES_PER_ACTOR];

    bool isActive;
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

#endif