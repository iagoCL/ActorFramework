#include "Tracker.h"

#include "../Framework/Logger.h"

Tracker::Tracker(const unsigned int actorId_, ExecutionUnit *executionUnit_)
    : numSquirrels(0), numInfectedSquirrels(0), numDeathSquirrels(0), Actor(actorId_, executionUnit_) {
    if (actorId != 0) {
        for (int i = 0; i < NUM_MONTHS; ++i) {
            steps[i].receivedMessages = 0;
            /**/
            steps[i].numDeathSquirrels = steps[i].numDeathSquirrels = steps[i].numSquirrels = -1;
            for (int j = 0; j < NUM_CELLS; ++j) {
                steps[i].avgCellInfection[j].set(-1.0f);
            }
            //*/
        }
        isActive = false;
    }
}

void Tracker::processMessage(const int messageSize, const Message::MessageData *messageData) {
    switch (messageData->type) {
        case SQUIRREL_SIMULATION_MESSAGES::CELL_SUMARY: {
            const auto cellInfo = reinterpret_cast<const Cell::CellSumary *>(messageData->data);
            const auto monthNum = cellInfo->monthId;
            const auto cellId = cellInfo->cellId;
            steps[monthNum].avgCellInfection[cellId] = cellInfo->cellData;
            checkStepCompleted(monthNum, steps[monthNum]);

        };
            break;
        case SQUIRREL_SIMULATION_MESSAGES::NEW_MONTH: {
            const auto monthNum = (reinterpret_cast<const unsigned int *>(messageData->data))[0];
            steps[monthNum].numDeathSquirrels = numDeathSquirrels;
            steps[monthNum].numInfectedSquirrels = numInfectedSquirrels;
            steps[monthNum].numSquirrels = numSquirrels;
            checkStepCompleted(monthNum, steps[monthNum]);

        };
            break;
        case SQUIRREL_SIMULATION_MESSAGES::SQUIRREL_STATUS_BORN: {
            ++numSquirrels;
            //std::cout << "Born squirrel: " + std::to_string(numSquirrels) + "-" + std::to_string(numDeathSquirrels) + "-" + std::to_string(numInfectedSquirrels) + "\n"<< std::endl;
            if (numSquirrels > MAX_NUM_SQUIRRELS) {
                logPrint(Logger::NORMAL, "Limit of alive Squirrels surpassed: " +
                                             std::to_string(numSquirrels) +
                                             "/" + std::to_string(MAX_NUM_SQUIRRELS) + " ending simulation.");
                executionUnit->finishProgram();
            }
        };
            break;
        case SQUIRREL_SIMULATION_MESSAGES::SQUIRREL_STATUS_INFECTED: {
            ++numInfectedSquirrels;
            //std::cout << "Infected squirrel: " + std::to_string(numSquirrels) + "-" + std::to_string(numDeathSquirrels) + "-" + std::to_string(numInfectedSquirrels) + "\n" << std::endl;
        };
            break;
        case SQUIRREL_SIMULATION_MESSAGES::SQUIRREL_STATUS_BORN_INFECTED: {
            ++numInfectedSquirrels;
            ++numSquirrels;
            //std::cout << "Born infected: " + std::to_string(numSquirrels) + "-" + std::to_string(numDeathSquirrels) + "-" + std::to_string(numInfectedSquirrels) + "\n"<< std::endl;
        };
            break;
        case SQUIRREL_SIMULATION_MESSAGES::SQUIRREL_STATUS_KILLED: {
            --numInfectedSquirrels;
            --numSquirrels;
            ++numDeathSquirrels;
            //std::cout << "killed squirrel: " + std::to_string(numSquirrels) + "-" + std::to_string(numDeathSquirrels) + "-" + std::to_string(numInfectedSquirrels) + "\n"<< std::endl;
        };
            break;
        default:
            logPrint(Logger::LogLevel::ERROR, "ERROR: Tracker: Should not receive messages of type: " + Logger::numToString(messageData->type));
            break;
    }
}

void Tracker::update() { logPrint(Logger::LogLevel::ERROR, "ERROR: Tracker: Should not be updated"); }

Actor *Tracker::createNewActor(const unsigned int actorId_, const char *messageData) {
    return new Tracker(actorId_, executionUnit);
}

std::string Tracker::stepInfoToString(const unsigned int stepNumber, StepInfo &stepInfo) {
    std::string stepString =
        "\nStep: " + Logger::numToString(stepNumber) +
        "\n  Number of Cells:    " + Logger::numToString(stepInfo.receivedMessages - 1) +
        "\n  Alive Squirrels:    " + Logger::numToString(stepInfo.numSquirrels) +
        "\n  Infected Squirrels: " + Logger::numToString(stepInfo.numInfectedSquirrels) +
        "\n  Death Squirrels:    " + Logger::numToString(stepInfo.numDeathSquirrels) + "\n";
    for (int i = 0; i < NUM_CELLS; ++i) {
        stepString += "\n     Cell:  " + Logger::numToString(i + 1) + std::string(stepInfo.avgCellInfection[i]);
    }
    return stepString + "\n\n";
}
void Tracker::checkStepCompleted(const unsigned int stepNumber, StepInfo &stepInfo) {
    if (++stepInfo.receivedMessages == (NUM_CELLS + 1)) {
        logPrint(Logger::LogLevel::EXHAUSTIVE, "\nCompleted:{\n" + stepInfoToString(stepNumber, stepInfo) + "}\n");
        if (stepNumber == (NUM_MONTHS - 1)) {
            std::string text =
                "\n\nSimulation Completed:\n";
            /**/
            for (int i = 0; i < NUM_MONTHS; ++i) {
                text += stepInfoToString(i, steps[i]);
            }  //*/
            logPrint(Logger::LogLevel::NORMAL,
                     text + "Squirrels created: " + Logger::numToString(numDeathSquirrels + numSquirrels) +
                         "\nSquirrels Killed: " + Logger::numToString(numDeathSquirrels) +
                         "\nSquirrels Alive: " + Logger::numToString(numSquirrels) +
                         "\nSquirrels Healthy: " + Logger::numToString(numSquirrels - numInfectedSquirrels) +
                         "\nSquirrels infected (total): " + Logger::numToString(numDeathSquirrels + numInfectedSquirrels) +
                         "\nSquirrels infected (alive): " + Logger::numToString(numInfectedSquirrels) +
                         "\nTotal months: " + Logger::numToString(NUM_MONTHS) + "\n\n");
            executionUnit->finishProgram();
        }
    }
}