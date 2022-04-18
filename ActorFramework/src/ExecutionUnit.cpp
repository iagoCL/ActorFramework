#include <algorithm>
#include <iostream>

#include "../include/ExecutionUnit.h"
#include "../include/Logger.h"

namespace ActorFramework {
    ExecutionUnit::ExecutionUnit() : newActorId(0) {
        MPI_Init(NULL, NULL);
        MPI_Comm_rank(comm, &rank);
        Logger::printMessage(0, rank, Logger::LogLevel::ALL_STATUS, "Created execution unit.");
        masterExecutioner = (rank == MASTER_RANK);
        MPI_Comm_size(comm, &size);
        if (masterExecutioner) {
            // Only the master stores the units load balance
            executionUnitsLoadBalance.reserve(size);
            for (int i = 0; i < size; ++i) {
                // Sets the initial load balance to 0 in all units.
                executionUnitsLoadBalance.push_back(0);
            }
        }
    }

    void ExecutionUnit::sendMessage(const int destExecutionUnit, Message &message, MPI_Request *req) const {
        if (message.messageSize > Message::MAX_MESSAGE_DATA || message.messageSize < Message::MESSAGE_DATA_HEADER_SIZE) {
            Logger::printMessage(message.getMessageData()->origActorId, rank, Logger::LogLevel::WARNING, "WARNING: Send message has incorrect size: " + std::string(message));
        }
        MPI_Isend(message.getMessageData(), message.messageSize, MPI_CHAR,
                destExecutionUnit, message.tag, comm, req);
        Logger::printMessage(0, rank, Logger::LogLevel::COMMUNICATIONS, "Send to: " + Logger::numToString(destExecutionUnit) + ": " + std::string(message));
    }

    void ExecutionUnit::processMessage(Message &message) {
        Logger::printMessage(0, rank, Logger::LogLevel::COMMUNICATIONS, "Received " + std::string(message));
        switch (message.tag) {
            case Message::MessagesTags::CREATE_NEW_ACTOR:
                processMessageCreateNewActor(message.getMessageData());
                break;
            case Message::MessagesTags::ASK_CREATE_NEW_ACTOR:
                createNewActor(message);
                break;
            case Message::MessagesTags::DELETE_ACTOR:
                processMessageDeleteActor(message.getMessageData());
                break;
            case Message::MessagesTags::STOP_ALL_ACTORS:
                processMessageStop();
                break;
            case Message::MessagesTags::ACTORS_MESSAGE:
                processMessageCommunication(message.messageSize, message);
                break;
            case Message::MessagesTags::REDIRECTED_ACTORS_MESSAGE_MASTER:
                processMessageCommunication(message.messageSize, message);
                break;
            case Message::MessagesTags::REDIRECTED_ACTORS_MESSAGE:
                processMessageCommunication(message.messageSize, message);
                break;
            default:
                Logger::printMessage(0, rank, Logger::LogLevel::ERROR, "ERROR: unknown message tag in message: " + std::string(message));
                break;
        }
    }
    void ExecutionUnit::processMessageCreateNewActor(Message::MessageData *messageData) {
        Logger::printMessage(0, rank, Logger::LogLevel::ALL_STATUS, "Creating new actor");
        const auto actorExecutionUnitId = messageData->destActorId;
        const auto newActorId = messageData->origActorId;
        // Inserted in all units for direct communication.
        const auto pairInsertion = actorIdToExecutionRank.insert(std::make_pair(newActorId, actorExecutionUnitId));
        if (!pairInsertion.second) {
            Logger::printMessage(newActorId, rank, Logger::LogLevel::WARNING, "WARNING: Inserting duplicated actor id.");
        }
        if (actorExecutionUnitId == rank) {
            // Actor only created on the correct unit.
            const auto type = messageData->type;
            Actor *actorModel = templateActors.at(type);
            Actor *newActor = actorModel->createNewActor(newActorId, messageData->data);
            const auto localPairInsertion = localActors.insert(std::make_pair(newActorId, newActor));
            if (!localPairInsertion.second) {
                Logger::printMessage(newActorId, rank, Logger::LogLevel::WARNING, "WARNING: Inserting local duplicated actor id.");
            }
        }
    }
    void ExecutionUnit::createNewActor(Message &message, unsigned int *createdActorId) {
        Logger::printMessage(0, rank, Logger::LogLevel::ALL_STATUS, "Asking to create new actor.");
        if (masterExecutioner) {
            // New actors are only created from the master
            // First finds unit wit lesser load balance
            int actorExecutionUnit = std::distance(
                executionUnitsLoadBalance.begin(),
                std::min_element(executionUnitsLoadBalance.begin(), executionUnitsLoadBalance.end()));
            // Broadcast new id to all execution units, to enable direct communication.
            message.tag = Message::CREATE_NEW_ACTOR;
            // destActorId stores the rank of the execution unit.
            message.getMessageData()->destActorId = actorExecutionUnit;
            // origActorId stores the id of the new actor.
            message.getMessageData()->origActorId = ++newActorId;

            const auto messageSize = message.messageSize;
            message.messageSize = Message::MESSAGE_DATA_HEADER_SIZE;
            MPI_Request req[size - 1];

            for (int i = 1; i < size; ++i) {
                // Send message, only with basic information to all units
                // Excepts to myself and the destination
                if (i != actorExecutionUnit) {
                    sendMessage(i, message, &req[i - 1]);
                }
            }
            // Send message with complete information to correct execution unit
            message.messageSize = messageSize;
            if (actorExecutionUnit != MASTER_RANK) {
                sendMessage(actorExecutionUnit, message, &req[actorExecutionUnit - 1]);
            }
            // Master creates the new actor without using the network
            processMessageCreateNewActor(message.getMessageData());
            if (createdActorId) {
                // return the Id of the created actor.
                *createdActorId = newActorId;
            }
            // Updates the units load balance
            ++executionUnitsLoadBalance.at(actorExecutionUnit);
            // Waits to send all messages.
            MPI_Waitall(size - 1, req, MPI_STATUSES_IGNORE);
        } else {
            // If the execution unit is not the master sends a message to the master which creates the actor.
            message.tag = Message::ASK_CREATE_NEW_ACTOR;
            sendMessage(MASTER_RANK, message, &message.req);
            if (createdActorId) {
                Logger::printMessage(0, rank, Logger::LogLevel::ERROR, "ERROR: you can only receive new actors id if they are created from the master.");
                *createdActorId = 0;
            }
            MPI_Wait(&message.req, MPI_STATUS_IGNORE);
        }
    }
    void ExecutionUnit::processMessageDeleteActor(Message::MessageData *messageData) {
        const auto deletedActorId = messageData->origActorId;
        const auto executionUnitId = messageData->destActorId;
        // Removes actor since it cannot longer receive messages
        actorIdToExecutionRank.erase(deletedActorId);
        if (masterExecutioner) {
            // Updated load balances at the master
            --executionUnitsLoadBalance.at(executionUnitId);
        }
        if (rank == executionUnitId) {
            // Deletes the actor from the local execution unit.
            auto deletedActorMapIt = localActors.find(deletedActorId);
            if (deletedActorMapIt == localActors.end()) {
                Logger::printMessage(deletedActorId, rank, Logger::LogLevel::WARNING, "WARNING: deleted Actor Id:" + Logger::numToString(deletedActorId) + " not found.");
            } else {
                Actor *deletedActor = deletedActorMapIt->second;
                localActors.erase(deletedActorMapIt);
                delete deletedActor;
                Logger::printMessage(deletedActorId, rank, Logger::LogLevel::ALL_STATUS, "Deleted and free local Actor");
            }
        } else {
            Logger::printMessage(deletedActorId, rank, Logger::LogLevel::ALL_STATUS, "Deleted Actor");
        }
    }
    void ExecutionUnit::processMessageStop() {
        Logger::printMessage(0, rank, Logger::LogLevel::ALL_STATUS, "Stopping simulation");
        MPI_Finalize();
        exit(0);
    }
    void ExecutionUnit::processMessageCommunication(const int messageSize, Message &message) {
        Logger::printMessage(0, rank, Logger::LogLevel::ALL_STATUS, "Processing Communication ");
        const auto messageData = message.getMessageData();
        const auto destActorId = messageData->destActorId;
        const auto destActor = localActors.find(destActorId);
        if (destActor == localActors.end()) {
            // The actor of the received messages is not in this execution unit
            const auto destExecutionUnit = actorIdToExecutionRank.find(destActorId);
            switch (message.tag) {
                case Message::ACTORS_MESSAGE: {
                    // Message received without any previous  redirections
                    if (destExecutionUnit == actorIdToExecutionRank.end()) {
                        // The local execution unit does not know where redirect the message
                        if (masterExecutioner) {
                            // The master has probably received a message of a deleted or not created actor so it CANNOT be processed.
                            Logger::printMessage(destActorId, rank, Logger::LogLevel::WARNING,
                                                "WARNING: processing communication  dest Actor Id not found, the rank is already the master: " + std::string(message));
                        } else {
                            // The message is redirected to the master. Probably the actor has not yet been created.
                            message.tag = Message::REDIRECTED_ACTORS_MESSAGE;
                            sendMessage(MASTER_RANK, message, &message.req);
                            Logger::printMessage(destActorId, rank, Logger::LogLevel::COMMUNICATION_REDIRECTION,
                                                "Processing communication  dest Actor Id not found, redirecting to master: " + std::string(message));
                            MPI_Wait(&message.req, MPI_STATUS_IGNORE);
                        }
                    } else if (destExecutionUnit->second == rank) {
                        // The destination actor should be this execution unit.
                        if (masterExecutioner) {
                            // The master has probably received a message  of a deleted or not created actor so it CANNOT be processed.
                            Logger::printMessage(destActorId, rank, Logger::LogLevel::WARNING,
                                                "WARNING: processing communication  dest Actor Id should be local, the rank is already the master: " + std::string(message));
                        } else {
                            // The message is redirected to the master. Probably the actor has not yet been created.
                            message.tag = Message::REDIRECTED_ACTORS_MESSAGE;
                            sendMessage(MASTER_RANK, message, &message.req);
                            Logger::printMessage(destActorId, rank, Logger::LogLevel::COMMUNICATION_REDIRECTION,
                                                "Processing communication  dest Actor Id should be local, redirecting to master: " + std::string(message));
                            MPI_Wait(&message.req, MPI_STATUS_IGNORE);
                        }
                    } else {
                        // The message is redirected to the correct execution unit.
                        message.tag = masterExecutioner ? Message::REDIRECTED_ACTORS_MESSAGE_MASTER : Message::REDIRECTED_ACTORS_MESSAGE;
                        sendMessage(destExecutionUnit->second, message, &message.req);
                        Logger::printMessage(destActorId, rank, Logger::LogLevel::COMMUNICATION_REDIRECTION,
                                            "Processing communication dest Actor Id not found in local execution unit, sending to execution unit: " + std::to_string(destExecutionUnit->second) + " , " + std::string(message));
                        MPI_Wait(&message.req, MPI_STATUS_IGNORE);
                    }

                };
                    break;
                case Message::REDIRECTED_ACTORS_MESSAGE: {
                    // The message has already been redirected, but not from the master.
                    if (destExecutionUnit == actorIdToExecutionRank.end()) {
                        // The local execution unit does not know where redirect the message
                        if (masterExecutioner) {
                            // The master has probably received a message  of a deleted or not created actor so it CANNOT be processed.
                            Logger::printMessage(destActorId, rank, Logger::LogLevel::WARNING,
                                                "WARNING: processing redirected communication  dest Actor Id not found, the rank is already the master: " + std::string(message));
                        } else {
                            // The master is redirected to the master.
                            sendMessage(MASTER_RANK, message, &message.req);
                            Logger::printMessage(destActorId, rank, Logger::LogLevel::COMMUNICATION_REDIRECTION,
                                                "Processing redirected communication  dest Actor Id not found, redirecting to master: " + std::string(message));
                            MPI_Wait(&message.req, MPI_STATUS_IGNORE);
                        }
                    }
                };
                    break;
                case Message::REDIRECTED_ACTORS_MESSAGE_MASTER: {
                    // The message has already been redirected to the master unsuccessfully. Probably is a received a message of a deleted or not created actor so it CANNOT be processed.
                    Logger::printMessage(destActorId, rank, Logger::LogLevel::WARNING,
                                        "WARNING: processing communication redirected from master dest Actor Id not found: " + std::string(message));
                }; break;
                default:
                    // The received message has a tag that should not be processed by this function
                    Logger::printMessage(destActorId, rank, Logger::LogLevel::WARNING,
                                        "WARNING: processing communication, not valid tag: " + std::string(message));
                    break;
            }

        } else {
            // Message correctly received and proccesed by the corresponding actor.
            destActor->second->processMessage(messageSize, messageData);
            Logger::printMessage(destActorId, rank, Logger::LogLevel::COMMUNICATIONS, "Processing message between actors: " + Logger::numToString(messageData->origActorId) + " - " + Logger::numToString(destActorId));
        }
    }
    void ExecutionUnit::destroyActor(const unsigned int actorId_) {
        // Execution unit containing the deleted actor
        const auto executionUnitId = actorIdToExecutionRank.find(actorId_);
        // Sends to other execution units a message to delete the actor.
        MPI_Request req[size - 1];
        Message message(actorId_, executionUnitId->second, 0, 0, Message::DELETE_ACTOR);
        for (int i = 0; i < rank; ++i) {
            sendMessage(i, message, &req[i]);
        }
        for (int i = rank + 1; i < size; ++i) {
            sendMessage(i, message, &req[i - 1]);
        }
        // Deletes the actor on this unit.
        processMessageDeleteActor(message.getMessageData());
        // Waits until all messages have been sent.
        MPI_Waitall(size - 1, req, MPI_STATUSES_IGNORE);
    }

    void ExecutionUnit::mainLoop() {
        // Waits so that all execution units have finished creating actors.
        MPI_Barrier(comm);
        // Checks pending messages. Ensure all actors are created.
        checkAndProcessMessages(true);
        // Waits so that all execution units have created all their actors.
        // Otherwise an actor may receive a message before its created.
        MPI_Barrier(comm);
        Logger::printMessage(0, rank, Logger::LogLevel::ALL_STATUS,
                            "Simulation started, local actors: " + Logger::numToString(localActors.size()) + " global actors: " + Logger::numToString(actorIdToExecutionRank.size()));
        /**
    //Local debug text to check all units have correctly set up.
        std::string s;
        for (auto& [actorId, actor] : localActors) {
            s += std::to_string(rank) + " a " + std::to_string(actorId) + "\n";
        }
        s += "\n";
        for (auto& [actorId, actor] : actorIdToExecutionRank) {
            s += std::to_string(rank) + " b " + std::to_string(actorId) + " " + std::to_string(actor) + "\n";
        }
        s += "\n\n";
        std::cout << s << std::endl;  //*/

        for (;;) {
            // Infintie loop
            // Update active actors
            const bool activeActors = updateActors();
            // Process new messages.
            checkAndProcessMessages(activeActors);
        }
    }

    void ExecutionUnit::checkAndProcessMessages(const bool activeActors) {
        MPI_Status status;
        if (activeActors) {
            // Actors are still active.
            int messageReceived;
            MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, comm, &messageReceived, &status);  // Checks if a message is received and finds its size
            while (messageReceived) {
                // Proces all remaining messages
                // Receives and process a message
                recvMessage(status);
                // Checks if there are more messages.
                MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, comm, &messageReceived, &status);
            }
        } else {
            // No active actors. Wait until a message is received
            MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, comm, &status);
            recvMessage(status);
        }
    }

    void ExecutionUnit::recvMessage(MPI_Status &status) {
        Message message;
        MPI_Get_count(&status, MPI_CHAR, &message.messageSize);
        if (message.messageSize > Message::MAX_MESSAGE_DATA || message.messageSize < Message::MESSAGE_DATA_HEADER_SIZE) {
            Logger::printMessage(message.getMessageData()->origActorId, rank, Logger::LogLevel::WARNING, "WARNING: received message has incorrect size: " + std::string(message));
        }

        MPI_Recv(message.getMessageData(), message.messageSize, MPI_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, comm, &status);
        message.tag = status.MPI_TAG;
        processMessage(message);
    }
    bool ExecutionUnit::updateActors() {
        bool actorsActive = false;
        std::vector<unsigned int> deletedActorsId;
        // Iterates over all local actors.
        for (auto &[actorId, actor] : localActors) {
            // Check actors that must be killed.
            if (actor->checkKill()) {
                // Remove actors from a list that we are iterating is complicated and would require several code restructures.
                deletedActorsId.push_back(actorId);
                // Instead it Stores its id to be deleted after.
            } else if (actor->getActive()) {
                // Update active actors
                actor->update();
                // There are still active actors.
                actorsActive = true;
            }
        }
        for (const auto &deletedId : deletedActorsId) {
            // Deletes all actors mark to be deleted
            destroyActor(deletedId);
        }
        return actorsActive;
    }

    unsigned int ExecutionUnit::registerActorType(Actor *actor) {
        templateActors.push_back(actor);
        return templateActors.size() - 1;
    }

    void ExecutionUnit::communicateWithActor(Message &message) const {
        const auto data = message.getMessageData();
        const auto destActorId = data->destActorId;
        const auto origActorId = data->origActorId;
        const auto destExecutionUnit = actorIdToExecutionRank.find(destActorId);
        if (destExecutionUnit == actorIdToExecutionRank.end()) {
            if (masterExecutioner) {
                Logger::printMessage(origActorId, rank, Logger::LogLevel::ERROR,
                                    "WARNING: communicating  dest Actor Id not found, already master not sending message: " + std::string(message));
                message.messageSize = -1;
            } else {
                message.tag = Message::REDIRECTED_ACTORS_MESSAGE;
                sendMessage(MASTER_RANK, message, &message.req);
                Logger::printMessage(origActorId, rank, Logger::LogLevel::COMMUNICATION_REDIRECTION,
                                    "Communicating dest Actor Id not found, redirect to master: " + std::string(message));
            }
        } else {
            Logger::printMessage(origActorId, rank, Logger::LogLevel::COMMUNICATIONS, "Sent message between actors: " + std::string(message));
            message.tag = Message::MessagesTags::ACTORS_MESSAGE;
            sendMessage(destExecutionUnit->second, message, &message.req);
        }
    }

    void ExecutionUnit::finishProgram() {
        // Send termination message to all units.
        MPI_Request req[size - 1];
        Message message(0, Message::STOP_ALL_ACTORS);
        auto data = message.getMessageData();
        data->destActorId = data->origActorId = data->type = 0;
        for (int i = 0; i < rank; ++i) {
            sendMessage(i, message, &req[i]);
        }
        for (int i = rank + 1; i < size; ++i) {
            sendMessage(i, message, &req[i - 1]);
        }
        // Wait to all units to end
        MPI_Waitall(size - 1, req, MPI_STATUSES_IGNORE);
        // Ends this unit.
        processMessageStop();
    }
}