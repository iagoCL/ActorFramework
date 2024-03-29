#ifndef ACTOR_FRAMEWORK_EXECUTION_UNIT_H
#define ACTOR_FRAMEWORK_EXECUTION_UNIT_H

#include <list>
#include <map>
#include <vector>

#include "Actor.h"
#include "Message.h"
#include "mpi.h"

namespace ActorFramework {
    class Actor;

   /**
     * @brief Execution units are in charge of processing all the actors.
     * When an actor is created it should be assigned to an execution unit which will execute it.
     * Execution units are in charge of controlling communication between actors calling the internal functions.
     * They are also in charge of distributing load to try to keep a more balanced execution.
     *
     * When using the MPI implementation they are MPI processes.
     *
     */
    class ExecutionUnit {
    public:
        /**
        * @brief Construct a new Execution Unit object
        *
        */
        ExecutionUnit();

        /**
        * @brief Creates a New Actor object on the less loaded execution unit.
        * First sends a message to the master which decides the less loaded unit.
        * Then the master send a message to all other execution units.
        * The corresponding execution unit creates the actor.
        * The other execution units update their maps to mantain communications.
        *
        * @param message Message used to create the actor.
        *                It must store the actor type, size and parameters.
        *                Tag, destActorId, origActorId will be correctly set.
        * @param createdActorId Returns the Actor id of the new actor.
        *                       It can only be used if called from the master execution unit.
        */
        void createNewActor(Message &message, unsigned int *createdActorId = nullptr);

        /**
        * @brief Send a termination message to all execution units completing the program.
        * This is similar to a force termination.
        * Can be used to finish the simulation as soon as the result is known.
        *
        */
        void finishProgram();

        /**
        * @brief Send a message to an actor.
        * It communicates with other execution units.
        * Error handle allows to redirect incorrect messages to master,
        * which should be able of redirect to the correct execution unit.
        *
        * @param message Message to send to the other actor.
        *                Dest actor Id is used to find the correct execution unit.
        *                Size and origActorId are expected to be correct.
        */
        void communicateWithActor(Message &message) const;
    
        /**
        * @brief Starts the program.
        * The execution unit will be on an infinite loop to update and receive actors messages.
        * finishProgram competes the loop in all execution units.
        *
        */
        void mainLoop();
    
        /**
        * @brief Register a new type of actors that can be created.
        * Must be called on all execution units.
        *
        * @param actor Actor of the class that it is being registered.
        * @return unsigned int Type used to create new actors of this class.
        */
        unsigned int registerActorType(Actor *actor);

        /**
        * @brief Destroys one actor
        * Depending of the warning level new messages send to this actor
        * will be ignored or create a warning.
        *
        * @param actorId_ Actor Id of the removed actor.
        */
        void destroyActor(const unsigned int actorId_);

        /**
        * @brief Gets the Rank or id of the execution unit.
        * When using the MPI implementation is the same as calling MPI_rank.
        *
        * @return int The execution unit rank
        */
        int getRank() const { return rank; }

        /**
        * @brief Gets if the execution unit is the master
        *
        * @return true Is the master
        * @return false Is not the master
        */
        bool isMaster() const { return masterExecutioner; }

    private:
        static const int MASTER_RANK = 0;
        static const MPI_Comm comm = MPI_COMM_WORLD;

        int rank;
        int size;
        unsigned int newActorId;
        bool masterExecutioner;

        std::map<unsigned int, Actor *> localActors;
        std::map<unsigned int, int> actorIdToExecutionRank;

        std::vector<unsigned int> executionUnitsLoadBalance;

        std::vector<Actor *> templateActors;

        /**
        * @brief INTERNAL: Checks if messages have been received, it also processes pending messages.
        *
        * @param activeActors If false waits until receive a message.
        */
        void checkAndProcessMessages(const bool activeActors);

        /**
        * @brief INTERNAL: Uses the correct function to process the message.
        * It process the message according to its tag
        * If its not an internal message it uses the message destActorId to assign the correct actor
        *
        * @param message Message to be processed
        */

        void processMessage(Message &message);

        /**
        * @brief INTERNAL: Processes messages received when a new actor is created.
        * It registers the new actor enabling to send new messages to it.
        * If its the correct execution unit it also creates the actor.
        * If it is not the correct execution unit it updates the communication map.
        *
        * @param messageData Data used to create the new actor.
        */
        void processMessageCreateNewActor(Message::MessageData *messageData);

        /**
        * @brief INTERNAL: Processes messages received when an actor is deleted.
        * It removes the deleted actor forbiding to send new messages to it.
        * If the execution unit contains the actor it also deletes the actor.
        *
        * @param messageData Data used to create the new actor.
        */
        void processMessageDeleteActor(Message::MessageData *messageData);

        /**
        * @brief INTERNAL: Processes a message to stop the program.
        *
        */
        void processMessageStop();

        /**
        * @brief INTERNAL: Processes a message communication between two actors.
        * If the execution unit is not correct it redirects the message.
        *
        * @param messageSize Size of the message in bytes.
        * @param message Message received from other actor.
        */
        void processMessageCommunication(const int messageSize, Message &message);

        /**
        * @brief INTERNAL: Calls update on each active actors.
        *
        * @return true Some active actors are still active.
        * @return false  No actors are still active.
        */
        bool updateActors();

        /**
        * @brief INTERNAL: Checks if there is any finished process without pending send messages.
        * If a finished actor has no longer messages to be send its eliminated.
        *
        */
        void cleanDeletedProcesses();

        /**
        * @brief INTERNAL: Send a message to an execution unit.
        * Asynchronous communication are used (The MPI implementation calls MPI_Isend).
        * The message cannot be free until the req is completed.
        *
        * @param destExecutionUnit Rank of the execution unit to send the message
        * @param message Message send to the execution unit.
        * @param req Req used to send the message. Some function used message.req but
        * for multiple send custom options are also enabled.
        */
        void sendMessage(const int destExecutionUnit, Message &message, MPI_Request *req) const;
    
        /**
        * @brief INTERNAL: Receives a message.
        * It expects to be able to obtain the correct message size from the status.
        * The framework correctly sets this using MPI_Probe or MPI_Iprobe.
        *
        * @param status Status used to received the new message. Must be adequately
        * set
        */
        void recvMessage(MPI_Status &status);
    };
}// ActorFramework

#endif//ACTOR_FRAMEWORK_EXECUTION_UNIT_H