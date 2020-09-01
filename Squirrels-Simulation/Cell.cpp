#include "Cell.h"

#include "../Framework/Logger.h"
#include "Main.h"

Cell::Cell(const Cell::CreateNewCell *parameters, const unsigned int actorId_, ExecutionUnit *executionUnit_)
    : Actor(actorId_, executionUnit_),
      cellId(parameters->cellId),
      trackerId(parameters->trackerId) {
    if (actorId != 0) {
        monthsInfection[MONTHS_INFECTION - 1] = infectionData.total = parameters->initialInfections;
        monthsPopulation[MONTHS_POPULATION - 1] = populationData.total = parameters->initialPopulations;
        infectionData.thisMonth = populationData.thisMonth = 0.0f;
        avgData.compute(parameters->initialInfections, parameters->initialPopulations, 1);
        for (int i = 0; i < (MONTHS_INFECTION - 1); ++i) {
            monthsInfection[i] = 0.0f;
        }
        for (int i = 0; i < (MONTHS_POPULATION - 1); ++i) {
            monthsPopulation[i] = 0.0f;
        }
        isActive = false;
    }
}
void Cell::processMessage(const int messageSize, const Message::MessageData *messageData) {
    switch (messageData->type) {
        case SQUIRREL_SIMULATION_MESSAGES::SQUIRREL_HOP: {
            const auto receivedFromSquirrel = messageData->origActorId;
            // Send message with infectionLevel
            //std::cout << "ID: " + std::to_string(actorId) + "-" + std::to_string(receivedFromSquirrel) << std::endl;
            auto &sendMessage = createMessage(receivedFromSquirrel, SQUIRREL_SIMULATION_MESSAGES::CELL_INFECTION, sizeof(float));
            auto sendData = reinterpret_cast<CellAverageLevel *>(sendMessage.getMessageData()->data);
            *sendData = avgData;
            executionUnit->communicateWithActor(sendMessage);

            // Update current month infection levels and population influx
            const auto squirrelIsInfected = reinterpret_cast<const bool *>(messageData->data)[0];
            if (squirrelIsInfected) {
                ++infectionData.thisMonth;
            }
            ++populationData.thisMonth;
        };
            break;
        case SQUIRREL_SIMULATION_MESSAGES::NEW_MONTH: {
            // Update population Influx and infection level
            const auto monthNums = reinterpret_cast<const unsigned int *>(messageData->data);
            const auto monthNum = monthNums[0];
            auto monthId = monthNum % MONTHS_POPULATION;
            populationData.updateTotal(monthsPopulation[monthId]);
            monthsPopulation[monthId] = populationData.thisMonth;
            monthId = monthNum % MONTHS_INFECTION;
            infectionData.updateTotal(monthsInfection[monthId]);
            monthsInfection[monthId] = infectionData.thisMonth;
            avgData.compute(infectionData.total, populationData.total, monthNum);

            // Send message with infectionLevel and population influx to the cell
            auto &sendMessage = createMessage(trackerId, SQUIRREL_SIMULATION_MESSAGES::CELL_SUMARY, sizeof(CellSumary));
            auto sendData = reinterpret_cast<CellSumary *>(sendMessage.getMessageData()->data);
            sendData->cellId = cellId;
            sendData->monthId = monthNum;
            sendData->cellData.avgValues = avgData;
            sendData->cellData.populationHops = populationData;
            sendData->cellData.infectionHops = infectionData;
            executionUnit->communicateWithActor(sendMessage);

            // Reset this month levels
            populationData.thisMonth = infectionData.thisMonth = 0.0f;
        };
            break;
        default:
            logPrint(Logger::LogLevel::ERROR, "ERROR: Cells: Should not receive messages of type: " + Logger::numToString(messageData->type));
            break;
    }
}
void Cell::update() { logPrint(Logger::LogLevel::ERROR, "ERROR: Cells: Should not be updated"); }

Actor *Cell::createNewActor(const unsigned int actorId_, const char *messageData) {
    return new Cell(reinterpret_cast<const CreateNewCell *>(messageData), actorId_, executionUnit);
}

Cell::CellSumaryData::operator std::string() const {
    return " Avg. Population Influx: " + std::to_string(avgValues.populationInflux) +
           " [" + std::to_string(static_cast<int>(populationHops.total)) +
           "(" + std::to_string(static_cast<int>(populationHops.thisMonth)) +
           ")] Avg. Infection Level: " + std::to_string(avgValues.infectionLevel) +
           " [" + std::to_string(static_cast<int>(infectionHops.total)) +
           "(" + std::to_string(static_cast<int>(infectionHops.thisMonth)) + ")]";
}
void Cell::CellLevel::set(const float value) {
    total = thisMonth = value;
}
void Cell::CellAverageLevel::set(const float value) {
    populationInflux = infectionLevel = value;
}
void Cell::CellAverageLevel::compute(const float infection, const float population, const int months) {
    const auto monthsF = std::max(static_cast<float>(months), 1.0f);
    infectionLevel = infection / std::max(monthsF, static_cast<float>(MONTHS_INFECTION));
    populationInflux = population / std::max(monthsF, static_cast<float>(MONTHS_POPULATION));
}
void Cell::CellSumaryData::set(const float value) {
    avgValues.set(value);
    populationHops.set(value);
    infectionHops.set(value);
}
void Cell::CellLevel::updateTotal(const float minus) {
    total += thisMonth - minus;
}