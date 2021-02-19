#include <mpi.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

//#include "cell_list.h"
#include "country_stats.h"
//#include "individual.h"
#include "parameters.h"
//#include "utils.h"

int main(int argc, char const *argv[]) {
  double time_spent = 0.0;
  clock_t begin = clock();
  srand(0);
  MPI_Init(NULL, NULL);
  // to store execution time of code

  MPI_Datatype individual_type = serializeIndividualStruct();
  MPI_Datatype country_stats_type = serializeCountryStatsStruct();
  MPI_Op country_stats_op;
  MPI_Op_create(&country_stats_sum, 1, &country_stats_op);

  int my_rank, world_size;
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);
  Cell grid[GRID_HEIGHT][GRID_WIDTH];
  Individual individuals[POPULATION_SIZE];

  int num_elements_per_proc = POPULATION_SIZE / world_size;

  //TODO: Fix displacement bug
  // Scatterv and Gatherv setup
  int *displs, *scounts;

  displs = (int *)malloc(world_size * sizeof(int));
  scounts = (int *)malloc(world_size * sizeof(int));
  for (int i = 0; i < world_size; ++i) {
    displs[i] = i * num_elements_per_proc;
    if (i != world_size - 1) {
      scounts[i] = num_elements_per_proc;
    } else {
      scounts[i] = POPULATION_SIZE - (world_size - 1) * num_elements_per_proc;
    }
    //printf("%d) scounts: %d, displs: %d\n", i, scounts[i], displs[i]);
  }

  // Every process initialize a grid
  for (int i = 0; i < GRID_HEIGHT; i++) {
    for (int j = 0; j < GRID_WIDTH; j++) {
      grid[i][j].head = NULL;
    }
  }

  int countriesCount = assignCountries(grid);

  // if (my_rank == 0) {
  //   for (int row = 0; row < GRID_HEIGHT; row++) {
  //     for (int columns = 0; columns < GRID_WIDTH; columns++) {
  //       printf("%d\t", grid[row][columns].countryID);
  //     }
  //     printf("\n");
  //   }
  // }

  if (my_rank == 0) {
    printf("// INITIAL POPULATION // \n");
    int assignedInfected = 0;
    for (int i = 0; i < POPULATION_SIZE; i++) {
      Individual ind = {i,
                        (assignedInfected < INITITAL_INFECTED) ? true : false,
                        false,
                        0,
                        0,
                        0,
                        rand_int(0, (GRID_HEIGHT - 1)),
                        rand_int(0, (GRID_WIDTH - 1)),
                        rand_int(1, MAX_SPEED),
                        0};
      individuals[i] = ind;
      push(&grid[ind.row][ind.column].head, ind.ID);
      //printIndividualData(ind, grid[ind.row][ind.column].countryID);
      assignedInfected++;
      //printList(grid[ind.row][ind.column].head, ind.row, ind.column);
    }
  }

  // Initialization, should only be called once.
  Individual *local_arr = (Individual *)malloc(sizeof(Individual) * scounts[my_rank]);
  Individual *gather_array = (Individual *)malloc(sizeof(Individual) * POPULATION_SIZE);
  Individual *final_gather_array;

  if (my_rank == 0) final_gather_array = (Individual *)malloc(sizeof(Individual) * POPULATION_SIZE);

  for (int t = 0; t < END_TIME; t += TIME_STEP) {
    if (my_rank == 0 && t % DAY == 0) printf("(R: %d) SIMULATION DAY: %d \n", my_rank, t / DAY);
    clearGrid(grid);

    if (my_rank == 0) {
      for (int i = 0; i < POPULATION_SIZE; i++) {
        Direction dir = (Direction)rand_int(0, 3);
        individuals[i].direction = dir;
      }
    }

    // MPI_Scatter(individuals, num_elements_per_proc, individual_type, local_arr, num_elements_per_proc, individual_type, 0, MPI_COMM_WORLD);
    MPI_Scatterv(individuals, scounts, displs, individual_type, local_arr, scounts[my_rank], individual_type, 0, MPI_COMM_WORLD);

    for (int i = 0; i < scounts[my_rank]; i++) {
      updatePosition(&local_arr[i]);
      // printf("(R: %d, t: %d) ", my_rank, t);
      // printIndividualData(local_arr[i], grid[local_arr[i].row][local_arr[i].column].countryID);
    }

    // Every process receives all the updated indiduals
    // MPI_Allgather(local_arr, num_elements_per_proc, individual_type, gather_array, num_elements_per_proc, individual_type, MPI_COMM_WORLD);
    MPI_Allgatherv(local_arr, scounts[my_rank], individual_type, gather_array, scounts, displs, individual_type, MPI_COMM_WORLD);

    for (int i = 0; i < POPULATION_SIZE; i++) {
      push(&grid[gather_array[i].row][gather_array[i].column].head, gather_array[i].ID);
    }

    CountryStats localStats[countriesCount];
    memset(localStats, 0, sizeof(localStats));

    for (int i = 0; i < scounts[my_rank]; i++) {
      updateIndividualCounters(&local_arr[i], grid, gather_array, SPREAD_DISTANCE, VERBOSE);
      updateCountryStats(local_arr[i], grid, localStats, my_rank, t, VERBOSE);
    }

    // MPI_Gather(local_arr, num_elements_per_proc, individual_type, final_gather_array, num_elements_per_proc, individual_type, 0, MPI_COMM_WORLD);
    MPI_Gatherv(local_arr, scounts[my_rank], individual_type, final_gather_array, scounts, displs, individual_type, 0, MPI_COMM_WORLD);

    // Reduce all of the local sums into the global sum
    CountryStats globalStats[countriesCount];
    if (my_rank == 0) memset(globalStats, 0, sizeof(globalStats));
    MPI_Reduce(localStats, globalStats, countriesCount, country_stats_type, country_stats_op, 0, MPI_COMM_WORLD);

    bool noInfectedLeft = false;
    if (my_rank == 0) {
      for (int i = 0; i < POPULATION_SIZE; i++) {
        individuals[i] = final_gather_array[i];
        // if (t % DAY == 0) {
        //   printf("FINAL (R: %d, t: %d) ", my_rank, t);
        //   Individual ind = individuals[i];
        //   printIndividualData(ind, grid[ind.row][ind.column].countryID);
        // }
      }

      if (t % DAY == 0) {
        for (int i = 0; i < countriesCount; i++) {
          printf("FINAL (R: %d, DAY: %d) COUNTRY STATS %d) infected: %d, immune: %d, susceptible: %d\n", my_rank, t / DAY, i, globalStats[i].infected, globalStats[i].immune, globalStats[i].susceptible);
        }
        noInfectedLeft = !anyInfected(globalStats, countriesCount);
        if (noInfectedLeft) printf("(R: %d, DAY: %d) No infected left, sending termination signal \n", my_rank, t / DAY);
      }
    }

    if (t % DAY == 0) {
      bool exitSimulation;
      MPI_Allreduce(&noInfectedLeft, &exitSimulation, 1, MPI_C_BOOL, MPI_LOR, MPI_COMM_WORLD);
      if (exitSimulation) {
        printf("(R: %d, DAY: %d) EXITING SIMULATION: No infected left \n", my_rank, t / DAY);
        break;
      }
    }

    MPI_Barrier(MPI_COMM_WORLD);
  }

  //Completely free the memory

  if (my_rank == 0) free(final_gather_array);

  free(gather_array);
  clearGrid(grid);
  free(local_arr);
  free(displs);
  free(scounts);

  MPI_Type_free(&individual_type);
  MPI_Type_free(&country_stats_type);
  MPI_Op_free(&country_stats_op);
  MPI_Finalize();

  clock_t end = clock();

  time_spent += (double)(end - begin) / CLOCKS_PER_SEC;
  printf("Time elpased is %f seconds\n", time_spent);

  return 0;
}
