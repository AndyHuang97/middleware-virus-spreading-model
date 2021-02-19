#include <mpi.h>
#include <stdbool.h>

#include "cell_list.h"
#include "parameters.h"
#include "utils.h"

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

void updatePosition(Individual *ind);
void searchAndUpdateOnSusceptibles(Individual *ind, Cell grid[GRID_HEIGHT][GRID_WIDTH], Individual individuals[], int spreadDistance);
void searchSusceptibleOnInfected(Individual *ind, Cell grid[GRID_HEIGHT][GRID_WIDTH], Individual individuals[], int spreadDistance, bool susceptibleFlags[]);
void updateSuscpetibleFlags(CellList *head, Individual individuals[], bool susceptibleFlags[]);
void updateIndividualCounters(Individual *ind, bool updateInfectionCounter);
bool infectedInCell(CellList *head_ref, Individual individuals[]);
void printIndividualData(Individual ind, int countryID);
void printNeighbours(int id, int *neighbours, int len);
MPI_Datatype serializeIndividualStruct();
