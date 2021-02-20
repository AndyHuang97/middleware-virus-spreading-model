#include <mpi.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "cell_list.h"
#include "country_stats.h"
#include "individual.h"
#include "parameters.h"
#include "utils.h"

int main(int argc, char const *argv[]) {
  Config config;
  if (argc > 15) {
    config.POPULATION_SIZE = atoi(argv[1]);
    config.INITITAL_INFECTED = atoi(argv[2]);
    config.GRID_HEIGHT = atoi(argv[3]);
    config.GRID_WIDTH = atoi(argv[4]);
    config.TIME_STEP = atoi(argv[5]);
    config.MAX_SPEED = atoi(argv[6]);
    config.SPREAD_DISTANCE = atoi(argv[7]);
    config.SUSCEPTIBILITY_THR = atoi(argv[8]);
    config.INFECTION_THR = atoi(argv[9]);
    config.IMMUNITY_THR = atoi(argv[10]);
    config.COUNTRY_HEIGHT = atoi(argv[11]);
    config.COUNTRY_WIDTH = atoi(argv[12]);
    config.DAY = atoi(argv[13]);
    config.END_TIME = atoi(argv[14]);
    config.DENSITY_THR = atof(argv[15]);
  } else {
    printf("You need to provide all parameters.");
    exit(1);
  }

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
  Cell grid[config.GRID_HEIGHT][config.GRID_WIDTH];
  Individual individuals[config.POPULATION_SIZE];

  int num_elements_per_proc = config.POPULATION_SIZE / world_size;

  // Scatterv and Gatherv setup
  int *displs, *scounts;

  displs = (int *)malloc(world_size * sizeof(int));
  scounts = (int *)malloc(world_size * sizeof(int));
  for (int i = 0; i < world_size; ++i) {
    displs[i] = i * num_elements_per_proc;
    if (i != world_size - 1) {
      scounts[i] = num_elements_per_proc;
    } else {
      scounts[i] = config.POPULATION_SIZE - (world_size - 1) * num_elements_per_proc;
    }
    //printf("%d) scounts: %d, displs: %d\n", i, scounts[i], displs[i]);
  }

  // Every process initialize a grid
  for (int i = 0; i < config.GRID_HEIGHT; i++) {
    for (int j = 0; j < config.GRID_WIDTH; j++) {
      grid[i][j].head = NULL;
    }
  }

  int countriesCount = assignCountries(config.GRID_HEIGHT, config.GRID_WIDTH, grid, config);

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
    for (int i = 0; i < config.POPULATION_SIZE; i++) {
      Individual ind = {i,
                        (assignedInfected < config.INITITAL_INFECTED) ? true : false,
                        false,
                        0,
                        0,
                        0,
                        rand_int(0, (config.GRID_HEIGHT - 1)),
                        rand_int(0, (config.GRID_WIDTH - 1)),
                        rand_int(1, config.MAX_SPEED),
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
  Individual *gather_array = (Individual *)malloc(sizeof(Individual) * config.POPULATION_SIZE);
  Individual *final_gather_array;
  bool searchOnInfected = (((float)(config.POPULATION_SIZE - config.INITITAL_INFECTED) / (float)config.POPULATION_SIZE) > config.DENSITY_THR) ? true : false;

  if (my_rank == 0) final_gather_array = (Individual *)malloc(sizeof(Individual) * config.POPULATION_SIZE);

  for (int t = 0; t < config.END_TIME; t += config.TIME_STEP) {
    if (my_rank == 0 && t % config.DAY == 0) printf("(R: %d) SIMULATION DAY: %d \n", my_rank, t / config.DAY);
    clearGrid(config.GRID_HEIGHT, config.GRID_WIDTH, grid, config);

    if (my_rank == 0) {
      for (int i = 0; i < config.POPULATION_SIZE; i++) {
        Direction dir = (Direction)rand_int(0, 3);
        individuals[i].direction = dir;
      }
    }

    // MPI_Scatter(individuals, num_elements_per_proc, individual_type, local_arr, num_elements_per_proc, individual_type, 0, MPI_COMM_WORLD);
    MPI_Scatterv(individuals, scounts, displs, individual_type, local_arr, scounts[my_rank], individual_type, 0, MPI_COMM_WORLD);

    for (int i = 0; i < scounts[my_rank]; i++) {
      updatePosition(&local_arr[i], config);
      // printf("(R: %d, t: %d) ", my_rank, t);
      // printIndividualData(local_arr[i], grid[local_arr[i].row][local_arr[i].column].countryID);
    }

    // Every process receives all the updated indiduals
    // MPI_Allgather(local_arr, num_elements_per_proc, individual_type, gather_array, num_elements_per_proc, individual_type, MPI_COMM_WORLD);
    MPI_Allgatherv(local_arr, scounts[my_rank], individual_type, gather_array, scounts, displs, individual_type, MPI_COMM_WORLD);

    for (int i = 0; i < config.POPULATION_SIZE; i++) {
      push(&grid[gather_array[i].row][gather_array[i].column].head, gather_array[i].ID);
    }
    CountryStats localStats[countriesCount];
    memset(localStats, 0, sizeof(localStats));

    if (searchOnInfected) {
      bool susceptibleFlags[config.POPULATION_SIZE];
      memset(susceptibleFlags, false, sizeof(susceptibleFlags));
      for (int i = 0; i < scounts[my_rank]; i++) {
        searchSusceptibleOnInfected(&local_arr[i], config.GRID_HEIGHT, config.GRID_WIDTH, grid, gather_array, config.SPREAD_DISTANCE, susceptibleFlags, config);
      }

      // printf("(R:%d, t:%d) FLAGS:", my_rank, t);
      // for (int i = 0; i < POPULATION_SIZE; i++) {
      //   printf("%d:%d ", i, susceptibleFlags[i]);
      // }
      // printf("\n");

      bool reducedSusceptibleFlags[config.POPULATION_SIZE];
      memset(reducedSusceptibleFlags, false, sizeof(reducedSusceptibleFlags));
      MPI_Allreduce(&susceptibleFlags, &reducedSusceptibleFlags, config.POPULATION_SIZE, MPI_C_BOOL, MPI_LOR, MPI_COMM_WORLD);

      // printf("(R:%d, t:%d) ", my_rank, t);
      // printf("REDUCED FLAGS:");
      // for (int i = 0; i < POPULATION_SIZE; i++) {
      //   printf("%d:%d ", i, reducedSusceptibleFlags[i]);
      // }
      // printf("\n");

      for (int i = 0; i < scounts[my_rank]; i++) {
        updateIndividualCounters(&local_arr[i], reducedSusceptibleFlags[local_arr[i].ID], config);
        updateCountryStats(local_arr[i], config.GRID_HEIGHT, config.GRID_WIDTH, grid, localStats, my_rank, t);
      }

    } else {
      for (int i = 0; i < scounts[my_rank]; i++) {
        searchAndUpdateOnSusceptibles(&local_arr[i], config.GRID_HEIGHT, config.GRID_WIDTH, grid, gather_array, config.SPREAD_DISTANCE, config);
        updateCountryStats(local_arr[i], config.GRID_HEIGHT, config.GRID_WIDTH, grid, localStats, my_rank, t);
      }
    }

    // MPI_Gather(local_arr, num_elements_per_proc, individual_type, final_gather_array, num_elements_per_proc, individual_type, 0, MPI_COMM_WORLD);
    MPI_Gatherv(local_arr, scounts[my_rank], individual_type, final_gather_array, scounts, displs, individual_type, 0, MPI_COMM_WORLD);

    // Reduce all of the local sums into the global sum
    CountryStats globalStats[countriesCount];
    if (my_rank == 0) memset(globalStats, 0, sizeof(globalStats));
    MPI_Reduce(localStats, globalStats, countriesCount, country_stats_type, country_stats_op, 0, MPI_COMM_WORLD);

    if (my_rank == 0) {
      int totalSusceptible = getTotalSusceptible(globalStats, countriesCount);
      searchOnInfected = ((float)totalSusceptible / config.POPULATION_SIZE > config.DENSITY_THR) ? true : false;
    }

    MPI_Bcast(&searchOnInfected, 1, MPI_C_BOOL, 0, MPI_COMM_WORLD);
    //printf("(R: %d, t: %d) SEARCH STRATEGY: %d\n", my_rank, t, searchOnInfected);

    bool noInfectedLeft = false;
    if (my_rank == 0) {
      for (int i = 0; i < config.POPULATION_SIZE; i++) {
        individuals[i] = final_gather_array[i];
        // printf("(R: %d, t:%d) ", my_rank, t);
        // printIndividualData(individuals[i], grid[individuals[i].row][individuals[i].column].countryID);
      }

      if (t > 0 && t % config.DAY == 0) {
        for (int i = 0; i < countriesCount; i++) {
          printf("FINAL (R: %d, DAY: %d) COUNTRY STATS %d) infected: %d, immune: %d, susceptible: %d, strategy: %d\n", my_rank, t / config.DAY, i, globalStats[i].infected, globalStats[i].immune, globalStats[i].susceptible, searchOnInfected);
        }
        noInfectedLeft = !anyInfected(globalStats, countriesCount);
        if (noInfectedLeft) printf("(R: %d, DAY: %d) No infected left, sending termination signal \n", my_rank, t / config.DAY);
      }
    }

    if (t % config.DAY == 0) {
      bool exitSimulation;
      MPI_Allreduce(&noInfectedLeft, &exitSimulation, 1, MPI_C_BOOL, MPI_LOR, MPI_COMM_WORLD);
      if (exitSimulation) {
        printf("(R: %d, DAY: %d) EXITING SIMULATION: No infected left \n", my_rank, t / config.DAY);
        break;
      }
    }

    MPI_Barrier(MPI_COMM_WORLD);
  }

  //Completely free the memory

  if (my_rank == 0) free(final_gather_array);

  free(gather_array);
  clearGrid(config.GRID_HEIGHT, config.GRID_WIDTH, grid, config);
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
