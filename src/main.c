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
  srand(time(0));
  MPI_Init(NULL, NULL);
  MPI_Datatype individual_type = serializeStruct();

  int my_rank, world_size;
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);
  ListPointer grid[MAX_HEIGHT][MAX_WIDTH];
  Individual individuals[POPULATION_SIZE];

  int num_elements_per_proc = 12 / world_size;

  if (my_rank == 0) {
    for (int i = 0; i < MAX_HEIGHT; i++) {
      for (int j = 0; j < MAX_WIDTH; j++) {
        grid[i][j].head = NULL;
      }
    }

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
  }
  // Initialization, should only be called once.

  for (int t = 0; t < END_TIME; t += TIME_STEP) {
    printf("My rank %d: ", my_rank);
    printf("SIMULATION TIME: %d \n", t);
    if (my_rank == 0) clearGrid(grid);
    Individual *local_arr = (Individual *)malloc(sizeof(Individual) * num_elements_per_proc);
    MPI_Scatter(individuals, num_elements_per_proc, individual_type, local_arr, num_elements_per_proc, individual_type, 0, MPI_COMM_WORLD);

    for (int i = 0; i < num_elements_per_proc; i++) {
      updatePosition(&local_arr[i], SPEED);
      printf("(%d) My rank %d: ", t, my_rank);
      printIndividualData(local_arr[i]);
    }

    Individual *gather_array;
    if (my_rank == 0) {
      gather_array = (Individual *)malloc(sizeof(Individual) * POPULATION_SIZE);
    }

    MPI_Gather(local_arr, num_elements_per_proc, individual_type, gather_array, num_elements_per_proc, individual_type, 0, MPI_COMM_WORLD);

    if (my_rank == 0) {
      for (int i = 0; i < POPULATION_SIZE; i++) {
        individuals[i] = gather_array[i];
        printf("(%d) GATHERED: ", t);
        printIndividualData(individuals[i]);
      }

      free(local_arr);
      free(gather_array);
    }

    // for (int i = 0; i < POPULATION_SIZE; i++) {
    //   updatePosition(&individuals[i], SPEED);
    //   Individual ind = individuals[i];
    //   printIndividualData(ind);
    //   push(&grid[ind.row][ind.column].head, ind.ID);
    //   printList(grid[ind.row][ind.column].head, ind.row, ind.column);
    // }
  }

  //Completely free the memory
  clearGrid(grid);
  printf("// END OF SIMULATION // \n");
  MPI_Barrier(MPI_COMM_WORLD);
  MPI_Finalize();
}
