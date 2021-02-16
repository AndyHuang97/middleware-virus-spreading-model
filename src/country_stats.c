#include "country_stats.h"

#include <mpi.h>
#include <stdlib.h>

MPI_Datatype serializeCountryStatsStruct() {
  MPI_Datatype country_stats_type;
  int struct_length = 3;
  int lengths[3] = {1, 1, 1};
  // const MPI_Aint displacements[7] = {0,
  //                                    sizeof(int),
  //                                    sizeof(int) + sizeof(bool),
  //                                    sizeof(int) + (2 * sizeof(bool)),
  //                                    (2 * sizeof(int)) + (2 * sizeof(bool)),
  //                                    (3 * sizeof(int)) + (2 * sizeof(bool)),
  //                                    (4 * sizeof(int)) + (2 * sizeof(bool))};

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

