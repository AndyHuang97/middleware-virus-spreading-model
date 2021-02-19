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
void updateIndividualCounters(Individual *ind, Cell grid[GRID_HEIGHT][GRID_WIDTH], Individual individuals[], int spreadDistance, bool verbose);
bool infectedInCell(CellList *head_ref, Individual individuals[]);
void printIndividualData(Individual ind, int countryID);
void printNeighbours(int id, int *neighbours, int len);
MPI_Datatype serializeIndividualStruct();
