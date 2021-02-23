#include <mpi.h>
#include <stdbool.h>

#include "cell_list.h"
#include "parameters.h"
#include "utils.h"

#ifndef INDIVIDUAL_H_
# define INDIVIDUAL_H_
typedef struct {
  int ID;
  bool isInfected;
  bool isImmune;
  int immunity_count;
  int infection_count;
  int susceptible_count;
  int row;
  int column;
  int speed;
  Direction direction;
} Individual;
#endif

void updatePosition(Individual *ind, Config config);
void searchAndUpdateOnSusceptibles(Individual *ind, int height, int width, Cell grid[height][width], Individual individuals[], int spreadDistance, Config config);
void searchSusceptibleOnInfected(Individual *ind, int height, int width, Cell grid[height][width], Individual individuals[], int spreadDistance, bool susceptibleFlags[], Config config);
void updateSuscpetibleFlags(CellList *head, Individual individuals[], bool susceptibleFlags[]);
void updateIndividualCounters(Individual *ind, bool updateInfectionCounter, Config config);
bool infectedInCell(CellList *head_ref, Individual individuals[]);
void printIndividualData(Individual ind, int countryID);
void printNeighbours(int id, int *neighbours, int len);
MPI_Datatype serializeIndividualStruct();
