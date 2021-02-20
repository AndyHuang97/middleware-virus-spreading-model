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
void updateCountryStats(Individual ind, int height, int width, Cell grid[height][width], CountryStats* localStats, int my_rank, int t);
bool anyInfected(CountryStats globalStats[], int countryCount);
int getTotalSusceptible(CountryStats globalStats[], int countryCount);