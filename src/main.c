#include <mpi.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

//#include "cell_list.h"
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
  Cell grid[GRID_HEIGHT][GRID_WIDTH];
  Individual individuals[POPULATION_SIZE];

  int num_elements_per_proc = POPULATION_SIZE / world_size;

  // Every process initialize a grid
  for (int i = 0; i < GRID_HEIGHT; i++) {
    for (int j = 0; j < GRID_WIDTH; j++) {
      grid[i][j].head = NULL;
    }
  }

  assignCountries(grid);

  if (my_rank == 0) {
    for (int row = 0; row < GRID_HEIGHT; row++) {
      for (int columns = 0; columns < GRID_WIDTH; columns++) {
        printf("%d\t", grid[row][columns].countryID);
      }
      printf("\n");
    }
  }

  if (my_rank == 0) {
    printf("// INITIAL POPULATION // \n");
    for (int i = 0; i < POPULATION_SIZE; i++) {
      Individual ind = {i,
                        rand() % 3 == 0 ? true : false,
                        false,
                        0,
                        0,
                        0,
                        rand_int(0, (GRID_HEIGHT - 1)),
                        rand_int(0, (GRID_WIDTH - 1))};
      individuals[i] = ind;
      printIndividualData(individuals[i]);
      push(&grid[ind.row][ind.column].head, ind.ID);
      //printList(grid[ind.row][ind.column].head, ind.row, ind.column);
    }
  }

  // Initialization, should only be called once.
  Individual *local_arr = (Individual *)malloc(sizeof(Individual) * num_elements_per_proc);
  Individual *gather_array = (Individual *)malloc(sizeof(Individual) * POPULATION_SIZE);
  Individual *final_gather_array;

  if (my_rank == 0) final_gather_array = (Individual *)malloc(sizeof(Individual) * POPULATION_SIZE);

  for (int t = 0; t < END_TIME; t += TIME_STEP) {
    if (my_rank == 0 && t % (60 * 60 * 24 * 7) == 0) printf("(R: %d) SIMULATION TIME: %d \n", my_rank, t);
    clearGrid(grid);

    MPI_Scatter(individuals, num_elements_per_proc, individual_type, local_arr, num_elements_per_proc, individual_type, 0, MPI_COMM_WORLD);

    for (int i = 0; i < num_elements_per_proc; i++) {
      updatePosition(&local_arr[i], SPEED);
      // printf("(R: %d, t: %d) ", my_rank, t);
      // printIndividualData(local_arr[i]);
    }

    // Every process receives all the updated indiduals
    MPI_Allgather(local_arr, num_elements_per_proc, individual_type, gather_array, num_elements_per_proc, individual_type, MPI_COMM_WORLD);

    for (int i = 0; i < POPULATION_SIZE; i++) {
      push(&grid[gather_array[i].row][gather_array[i].column].head, gather_array[i].ID);

      if (my_rank == 1) {
        // printf("(R: %d, t: %d) ", my_rank, t);
        // printList(grid[gather_array[i].row][gather_array[i].column].head, gather_array[i].row, gather_array[i].column);
      }
    }

    for (int i = 0; i < num_elements_per_proc; i++) {
      updateIndividualCounters(&local_arr[i], grid, gather_array, SPREAD_DISTANCE, VERBOSE);
    }

    MPI_Gather(local_arr, num_elements_per_proc, individual_type, final_gather_array, num_elements_per_proc, individual_type, 0, MPI_COMM_WORLD);

    if (my_rank == 0) {
      for (int i = 0; i < POPULATION_SIZE; i++) {
        individuals[i] = final_gather_array[i];
        if (t % (60 * 60 * 24 * 7) == 0) {
          printf("FINAL (R: %d, t: %d) ", my_rank, t);
          printIndividualData(individuals[i]);
        }
      }
    }

    MPI_Barrier(MPI_COMM_WORLD);
  }

  //Completely free the memory

  if (my_rank == 0) free(final_gather_array);

  free(gather_array);
  clearGrid(grid);
  free(local_arr);

  MPI_Type_free(&individual_type);
  MPI_Finalize();
}
