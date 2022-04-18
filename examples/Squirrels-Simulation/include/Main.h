#ifndef SQUIRRELS_SIMULATION_MAIN_H
#define SQUIRRELS_SIMULATION_MAIN_H

#include "../../../ActorFramework/include/Actor.h"

#define NUM_CELLS 16
#define NUM_MONTHS 24
#define MAX_NUM_SQUIRRELS 200
#define INITIAL_HEALTHY_SQUIRRELS 30
#define INITIAL_INFECTED_SQUIRRELS 4

enum SQUIRREL_SIMULATION_MESSAGES {
    SQUIRREL_STATUS_BORN,           // = 0
    SQUIRREL_STATUS_BORN_INFECTED,  // = 1
    SQUIRREL_STATUS_INFECTED,       // = 2
    SQUIRREL_STATUS_KILLED,         // = 3
    SQUIRREL_HOP,                   // = 4
    CELL_INFECTION,                 // = 5
    NEW_MONTH,                      // = 6
    CELL_SUMARY                     // = 7
};

int main();

#endif