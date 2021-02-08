#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <mpi.h>

#include "individual.h"

#define POPULATION_SIZE 10;

int main(int argc, char const *argv[])
{

  // MPI_Init(NULL, NULL);

  // int my_rank, world_size;
  // MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
  // MPI_Comm_size(MPI_COMM_WORLD, &world_size

  Individual newInd = {1, true, true, 3.33, 10, 55};
  printIndividualData(newInd);
  Individual newInd2 = {2, false, true, 56.1, 146, 588};
  Individual test = {3, false, true, 23, 88, 12};
  printIndividualData(newInd2);
  printIndividualData(test);
  updatePosition(&newInd, 99, 99);
  printIndividualData(newInd);
  //MPI_Finalize();
}
