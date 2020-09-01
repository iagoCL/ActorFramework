#ifndef SQUIRRELS_SIMULATION_CELL_H
#define SQUIRRELS_SIMULATION_CELL_H

#include "../Framework/Actor.h"

#define MONTHS_POPULATION 3
#define MONTHS_INFECTION 2

class Cell : public Actor {
   public:
    typedef struct {
        float total;
        float thisMonth;
        void set(const float value);
        void updateTotal(const float minus);
    } CellLevel;

    typedef struct {
        float populationInflux;
        float infectionLevel;
        void set(const float value);
        void compute(const float infection, const float population, const int months);
    } CellAverageLevel;

    typedef struct {
        CellAverageLevel avgValues;
        CellLevel infectionHops;
        CellLevel populationHops;
        operator std::string() const;
        void set(const float value);
    } CellSumaryData;

    typedef struct {
        CellSumaryData cellData;
        int cellId;
        unsigned int monthId;
    } CellSumary;

    typedef struct {
        float initialInfections;
        float initialPopulations;
        int cellId;
        unsigned int trackerId;
    } CreateNewCell;

    Cell(const CreateNewCell *parameters, const unsigned int actorId_, ExecutionUnit *executionUnit_);

    void processMessage(const int messageSize, const Message::MessageData *messageData) override;
    void update() override;
    Actor *createNewActor(const unsigned int actorId_, const char *messageData) override;

   private:
    const int cellId;
    const unsigned int trackerId;
    CellLevel infectionData;
    CellLevel populationData;
    CellAverageLevel avgData;
    float monthsInfection[MONTHS_INFECTION];
    float monthsPopulation[MONTHS_POPULATION];
};

#endif