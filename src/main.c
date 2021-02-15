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
  ListPointer grid[MAX_HEIGHT][MAX_WIDTH];
  Individual individuals[POPULATION_SIZE];

  int num_elements_per_proc = POPULATION_SIZE / world_size;

  // Every process initialize a grid
  for (int i = 0; i < MAX_HEIGHT; i++) {
    for (int j = 0; j < MAX_WIDTH; j++) {
      grid[i][j].head = NULL;
    }
  }
  
  if (my_rank == 0) {
    printf("// INITIAL POPULATION // \n");
    for (int i = 0; i < POPULATION_SIZE; i++) {
      Individual ind = {i,
                        true,
                        true,
                        0,
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
  Individual *local_arr = (Individual *)malloc(sizeof(Individual) * num_elements_per_proc);

  Individual *gather_array = (Individual *)malloc(sizeof(Individual) * POPULATION_SIZE);
  

  for (int t = 0; t < END_TIME; t += TIME_STEP) {
    printf("My rank %d: ", my_rank);
    printf("SIMULATION TIME: %d \n", t);
    clearGrid(grid);

    MPI_Scatter(individuals, num_elements_per_proc, individual_type, local_arr, num_elements_per_proc, individual_type, 0, MPI_COMM_WORLD);

    for (int i = 0; i < num_elements_per_proc; i++) {
      updatePosition(&local_arr[i], SPEED);
      printf("(%d) My rank %d: ", t, my_rank);
      printIndividualData(local_arr[i]);
    }

    // Every process receives all the updated indiduals
    MPI_Allgather(local_arr, num_elements_per_proc, individual_type, gather_array, num_elements_per_proc, individual_type, MPI_COMM_WORLD);

    for (int i = 0; i < POPULATION_SIZE; i++) {
      individuals[i] = gather_array[i];
      Individual ind = individuals[i];

      /* if( my_rank == 0) {
        printf("(%d) GATHERED: ", t);
        printf("My rank %d: ", my_rank);
        printIndividualData(ind);
      } */
      push(&grid[ind.row][ind.column].head, ind.ID);
      
      printf("(%d) My rank %d ", t, my_rank);
      printList(grid[ind.row][ind.column].head, ind.row, ind.column);
    }

    // Scatter updated individuals to compute neighbors
    MPI_Scatter(individuals, num_elements_per_proc, individual_type, local_arr, num_elements_per_proc, individual_type, 0, MPI_COMM_WORLD);

    int* neighbours = NULL; 
    for (int i = 0; i < num_elements_per_proc; i++) {
      printf("(%d) My rank %d: ", t, my_rank);
      if (local_arr[i].isInfected) {
        int neighboursLen = 0;
        //TODO need to concatenate all neighbours in a big list and need constant frees
        neighbours = findNeighbours(local_arr[i], grid, SPREAD_DISTANCE, &neighboursLen, VERBOSE);
      }
    }
    //TODO need to gather all neighbours in one place and then scatter again to compute infections
    // MPI_Gather has a fixed num_elements_per_proc ... maybe probe + send?

    MPI_Gather(local_arr, num_elements_per_proc, individual_type, gather_array, num_elements_per_proc, individual_type, 0,  MPI_COMM_WORLD);




    MPI_Barrier(MPI_COMM_WORLD);

    // for (int i = 0; i < POPULATION_SIZE; i++) {
    //   updatePosition(&individuals[i], SPEED);
    //   Individual ind = individuals[i];
    //   printIndividualData(ind);
    //   push(&grid[ind.row][ind.column].head, ind.ID);
    //   printList(grid[ind.row][ind.column].head, ind.row, ind.column);
    // }

  }

  //Completely free the memory
  printf("// END OF SIMULATION // \n");

  free(gather_array);
  clearGrid(grid);
  free(local_arr);

  MPI_Type_free(&individual_type);
  MPI_Finalize();
}
