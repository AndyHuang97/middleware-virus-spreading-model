#include <mpi.h>

typedef struct
{
  int infected;
  int immune;
  int susceptible;

} CountryStats;

MPI_Datatype serializeCountryStatsStruct();
void country_stats_sum(void* inputBuffer, void* outputBuffer, int* len, MPI_Datatype* datatype);