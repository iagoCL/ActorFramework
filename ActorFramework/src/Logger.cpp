#include <iostream>

#include "../include/Logger.h"

namespace ActorFramework {
    int Logger::currentLogLevel = Logger::LogLevel::NORMAL;

    void Logger::setLogLevel(int logLevel_) {
        currentLogLevel = logLevel_;
    }
    void Logger::printMessage(const unsigned int actorId, const int executionUnit, int logLevel, std::string text) {
        if (currentLogLevel <= logLevel) {
            std::cout << "Actor: [ " + Logger::numToString(actorId) + ", " + Logger::numToString(executionUnit) + " ] " + text + "\n"
                    << std::endl;
        }
    }

    std::string Logger::numToString(const int number) {
        if (number < 10) {
            return "00" + std::to_string(number);
        } else if (number < 100) {
            return "0" + std::to_string(number);
        } else {
            return std::to_string(number);
        }
    }
}