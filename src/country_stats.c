#include "country_stats.h"

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

MPI_Datatype serializeCountryStatsStruct() {
  MPI_Datatype country_stats_type;
  int struct_length = 3;
  int lengths[3] = {1, 1, 1};

  MPI_Aint displacements[3];
  displacements[0] = offsetof(CountryStats, infected);
  displacements[1] = offsetof(CountryStats, immune);
  displacements[2] = offsetof(CountryStats, susceptible);

  MPI_Datatype types[3] = {MPI_INT, MPI_INT, MPI_INT};
  MPI_Type_create_struct(struct_length, lengths, displacements, types, &country_stats_type);
  MPI_Type_commit(&country_stats_type);

  return country_stats_type;
}

void country_stats_sum(void* inputBuffer, void* outputBuffer, int* len, MPI_Datatype* datatype) {
  CountryStats* input = (CountryStats*)inputBuffer;
  CountryStats* output = (CountryStats*)outputBuffer;

  for (int i = 0; i < *len; i++) {
    output[i].infected += input[i].infected;
    output[i].immune += input[i].immune;
    output[i].susceptible += input[i].susceptible;
  }
}

void updateCountryStats(Individual ind, int height, int width, Cell grid[height][width], CountryStats* localStats, int my_rank, int t) {
  int countryID = grid[ind.row][ind.column].countryID;

  if (ind.isInfected) {
    localStats[countryID].infected += 1;
  } else if (ind.isImmune) {
    localStats[countryID].immune += 1;
  } else {
    localStats[countryID].susceptible += 1;
  }
}

bool anyInfected(CountryStats globalStats[], int countryCount) {
  for (int i = 0; i < countryCount; i++) {
    if (globalStats[i].infected > 0) return true;
  }
  return false;
}

int getTotalSusceptible(CountryStats globalStats[], int countryCount) {
  int total = 0;
  for (int i = 0; i < countryCount; i++) {
    total += globalStats[i].susceptible;
  }

  return total;
}