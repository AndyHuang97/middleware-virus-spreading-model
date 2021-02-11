#include <mpi.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "cell_list.h"
#include "individual.h"
#include "parameters.h"
#include "utils.h"

int main(int argc, char const *argv[]) {
  ListPointer grid[MAX_HEIGHT][MAX_WIDTH];
  for (int i = 0; i < MAX_HEIGHT; i++) {
    for (int j = 0; j < MAX_WIDTH; j++) {
      grid[i][j].head = NULL;
    }
  }

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
                      rand_int(0, (MAX_HEIGHT - 1)),
                      rand_int(0, (MAX_WIDTH - 1))};
    individuals[i] = ind;
    printIndividualData(individuals[i]);
    push(&grid[ind.row][ind.column].head, ind.ID);
    printList(grid[ind.row][ind.column].head, ind.row, ind.column);
  }

  for (int t = 0; t < END_TIME; t += TIME_STEP) {
    printf("SIMULATION TIME: %d \n", t);
    clearGrid(grid);
    for (int i = 0; i < POPULATION_SIZE; i++) {
      updatePosition(&individuals[i], SPEED);
      Individual ind = individuals[i];
      printIndividualData(ind);
      push(&grid[ind.row][ind.column].head, ind.ID);
      printList(grid[ind.row][ind.column].head, ind.row, ind.column);
    }
  }

  //Completely free the memory
  clearGrid(grid);
  printf("// END OF SIMULATION // \n");
  // MPI_Finalize();
}
