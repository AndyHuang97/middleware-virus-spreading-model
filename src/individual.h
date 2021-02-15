#include <mpi.h>
#include <stdbool.h>
#include "cell_list.h"
#include "parameters.h"

typedef struct {
  int ID;
  bool isInfected;
  bool isImmune;
  int infection_count;
  int susceptible_count;
  int row;
  int column;
} Individual;

void updatePosition(Individual *ind, int speed);
int* findNeighbours(Individual ind, ListPointer grid[MAX_HEIGHT][MAX_WIDTH], int spreadDistance, int* neighboursLen, bool verbose);
void printIndividualData(Individual ind);
void printNeighbours(int id, int* neighbours, int len);
MPI_Datatype serializeStruct();
