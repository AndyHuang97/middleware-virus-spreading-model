#include <mpi.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "individual.h"
#include "parameters.h"
#include "utils.h"

int main(int argc, char const *argv[]) {
  Individual individuals[POPULATION_SIZE];
  // MPI_Init(NULL, NULL);

  // int my_rank, world_size;
  // MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
  // MPI_Comm_size(MPI_COMM_WORLD, &world_size

  srand(time(0));  // Initialization, should only be called once.
  printf("// INITIAL POPULATION // \n");
  for (int i = 0; i < POPULATION_SIZE; i++) {
    Individual ind = {i,
                      true,
                      true,
                      0,
                      0,
                      rand_int(0, MAX_WIDTH),
                      rand_int(0, MAX_HEIGHT)};
    individuals[i] = ind;
    printIndividualData(individuals[i]);
  }

  for (int t = 0; t < END_TIME; t += TIME_STEP) {
    printf("Simulation time: %d \n", t);
    for (int i = 0; i < POPULATION_SIZE; i++) {
      updatePosition(&individuals[i], SPEED);
      printIndividualData(individuals[i]);
    }
  }

  // MPI_Finalize();
}
