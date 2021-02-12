#include "individual.h"

#include <math.h>
#include <mpi.h>
#include <stdbool.h>
#include <stdio.h>

#include "parameters.h"
#include "utils.h"

void printIndividualData(Individual ind) {
  printf("ID: %d, isInfected: %d, isImmune: %d, posX: %d, posY: %d \n",
         ind.ID, ind.isInfected, ind.isImmune, ind.row, ind.column);
}

// Updates the position of an individual by moving in a random direction
// If the individual reaches the boundaries the direction is inverted
void updatePosition(Individual *individual, int speed) {
  Direction dir = (Direction)rand_int(0, 3);

  for (int s = speed; s > 0; s--) {
    switch (dir) {
      case UP:
        if (individual->row == 0) {
          individual->row++;
          dir = DOWN;
        } else
          individual->row--;
        break;
      case DOWN:
        if (individual->row == (MAX_HEIGHT - 1)) {
          individual->row--;
          dir = UP;
        } else
          individual->row++;
        break;
      case LEFT:
        if (individual->column == 0) {
          individual->column++;
          dir = RIGHT;
        } else
          individual->column--;
        break;
      case RIGHT:
        if (individual->column == (MAX_WIDTH - 1)) {
          individual->column--;
          dir = LEFT;
        } else
          individual->column++;
        break;
      default:
        break;
    }
  }
}

MPI_Datatype serializeStruct() {
  MPI_Datatype individual_type;
  int struct_length = 7;
  int lengths[7] = {1, 1, 1, 1, 1, 1, 1};
  // const MPI_Aint displacements[7] = {0,
  //                                    sizeof(int),
  //                                    sizeof(int) + sizeof(bool),
  //                                    sizeof(int) + (2 * sizeof(bool)),
  //                                    (2 * sizeof(int)) + (2 * sizeof(bool)),
  //                                    (3 * sizeof(int)) + (2 * sizeof(bool)),
  //                                    (4 * sizeof(int)) + (2 * sizeof(bool))};

  MPI_Aint displacements[7];
  displacements[0] = offsetof(Individual, ID);
  displacements[1] = offsetof(Individual, isInfected);
  displacements[2] = offsetof(Individual, isImmune);
  displacements[3] = offsetof(Individual, infection_count);
  displacements[4] = offsetof(Individual, susceptible_count);
  displacements[5] = offsetof(Individual, row);
  displacements[6] = offsetof(Individual, column);

  MPI_Datatype types[7] = {MPI_INT, MPI_C_BOOL, MPI_C_BOOL, MPI_INT, MPI_INT, MPI_INT, MPI_INT};
  MPI_Type_create_struct(struct_length, lengths, displacements, types, &individual_type);
  MPI_Type_commit(&individual_type);

  return individual_type;
}
