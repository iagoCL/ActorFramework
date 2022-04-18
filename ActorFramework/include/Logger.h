#ifndef ACTOR_FRAMEWORK_LOGGER_H
#define ACTOR_FRAMEWORK_LOGGER_H

#include "Actor.h"
#include "ExecutionUnit.h"

namespace ActorFramework {
    /**
     * @brief This class handles different logging levels, allowing to have different messages for debug and execution.
     */
    class Logger {
    public:
        /**
         * @brief Log level of the logger
         * 
         */
        enum LogLevel {
            /**All status updates*/
            ALL_STATUS,
            /**Major status updates*/
            MAJOR_STATUS,
            /**All communications*/
            ALL_COMMUNICATIONS,
            /**Resumed communication*/
            COMMUNICATIONS,
            /**Communication redirection due to failures*/
            COMMUNICATION_REDIRECTION,
            /**Exhaustive logging*/
            EXHAUSTIVE,
            /**Normal*/
            NORMAL,
            /**Warnings*/
            WARNING,
            /**Errors*/
            ERROR
            };

        /**
         * @brief Sets the minimum log Level of the Logger
         * Messages of a lower level will be ignored.
         * 
         * @param logLevel_ New Log level
         */
        static void setLogLevel(int logLevel_);

        /**
         * @brief Prints a message (using the standar output)
         * 
         * @param actorId Actor id to show
         * @param executionUnit Rank of the execution unit
         * @param logLevel Log level, the message is only print if the log level is equal or greater than the current log level
         * @param text text to be print.
         */
        static void printMessage(const unsigned int actorId, const int executionUnit, int logLevel, std::string text);

        /**
         * @brief Transforms an integer to a string.
         * It will have at least 3 digits, uses left 0.
         * 
         * @param number Number to transform to string.
         * @return std::string Number as string with at least 3 digits.
         */
        static std::string numToString(const int number);

    private:
        static int currentLogLevel;
    };
}// ActorFramework

#endif//ACTOR_FRAMEWORK_LOGGER_H