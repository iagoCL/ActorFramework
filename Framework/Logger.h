#ifndef ACTOR_FRAMEWORK_LOGGER_H
#define ACTOR_FRAMEWORK_LOGGER_H

#include "Actor.h"
#include "ExecutionUnit.h"

class Logger {
   public:
    enum LogLevel { ALL_STATUS,
                    MAJOR_STATUS,
                    ALL_COMMUNICATIONS,
                    COMMUNICATIONS,
                    COMMUNICATION_REDIRECTION,
                    EXHAUSTIVE,
                    NORMAL,
                    WARNING,
                    ERROR };
    /**
     * @brief Sets the minimum log Level of the Logger
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
     * @brief Transforms a integer to a string.
     * It will have at least 3 digits, uses left 0.
     * 
     * @param number Number to transform to string.
     * @return std::string Number as string with at least 3 digits.
     */
    static std::string numToString(const int number);

   private:
    static int currentLogLevel;
};

#endif