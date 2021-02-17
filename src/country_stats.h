#include <mpi.h>

#include "individual.h"

typedef struct
{
  int infected;
  int immune;
  int susceptible;

} CountryStats;

MPI_Datatype serializeCountryStatsStruct();
void country_stats_sum(void* inputBuffer, void* outputBuffer, int* len, MPI_Datatype* datatype);
void updateCountryStats(Individual ind, Cell grid[GRID_HEIGHT][GRID_WIDTH], CountryStats* localStats, int my_rank, int t, bool verbose);