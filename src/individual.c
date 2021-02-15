#include "individual.h"

#include <math.h>
#include <mpi.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "utils.h"

void printIndividualData(Individual ind) {
  printf("ID: %d, (posX: %d, posY: %d), isInfected: %d, isImmune: %d, infection_count: %d, immunity_count: %d, susceptible_count: %d\n",
         ind.ID, ind.row, ind.column, ind.isInfected, ind.isImmune, ind.infection_count, ind.immunity_count, ind.susceptible_count);
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

void updateIndividualCounters(Individual *ind, ListPointer grid[MAX_HEIGHT][MAX_WIDTH], Individual individuals[], int spreadDistance, bool verbose) {
  // int bufferLen = 16;
  // int* neighbours = (int*) malloc(sizeof(int)*bufferLen);
  // *neighboursLen = 0;

  if (ind->isImmune) {
    ind->susceptible_count += TIME_STEP;
    if (ind->susceptible_count >= SUSCEPTIBILITY_THR) {
      ind->isImmune = false;
      ind->susceptible_count = 0;
    }
    return;
  } else if (ind->isInfected) {
    ind->immunity_count += TIME_STEP;
    if (ind->immunity_count >= IMMUNITY_THR) {
      ind->isImmune = true;
      ind->isInfected = false;
      ind->immunity_count = 0;
    }
    return;
  } else {
    for (int i = -spreadDistance; i <= spreadDistance; i++) {
      for (int j = -spreadDistance; j <= spreadDistance; j++) {
        // int cellLen = 0;
        //TODO need to check that ind.ID is not inserted in the list
        //TODO need to check borders of grid

        if ((ind->row + i >= 0 && ind->row + i < MAX_WIDTH) && (ind->column + j >= 0 && ind->column + j < MAX_HEIGHT)) {
          // printf("Individual ID %d) at cell (%d,%d) checking neighbouring cell (%d,%d)\n", ind->ID, ind->row, ind->column, ind->row+i, ind->column+j);
          bool infection = infectedInCell(grid[ind->row + i][ind->column + j].head, individuals);
          if (infection) {
            ind->infection_count += TIME_STEP;
            if (ind->infection_count >= INFECTION_THR) {
              ind->isInfected = true;
              ind->infection_count = 0;
            }
            return;
          }
        }
      }
    }
    // no infected in range
    ind->infection_count = 0;
    return;
  }
}

bool infectedInCell(CellList *head, Individual individuals[]) {
  CellList *curr = head;

  while (curr != NULL) {
    // printf("ID in list: %d, length: %d\n", curr->id, *length);
    if (individuals[curr->id].isInfected) return true;
    curr = curr->next;
  }
  return false;
}

void printNeighbours(int id, int *neighbours, int len) {
  printf("Individual ID %d) neighbours: [", id);
  for (int i = 0; i < len; i++) {
    printf("%d ", neighbours[i]);
  }
  printf("]\n");
}

MPI_Datatype serializeStruct() {
  MPI_Datatype individual_type;
  int struct_length = 8;
  int lengths[8] = {1, 1, 1, 1, 1, 1, 1, 1};
  // const MPI_Aint displacements[7] = {0,
  //                                    sizeof(int),
  //                                    sizeof(int) + sizeof(bool),
  //                                    sizeof(int) + (2 * sizeof(bool)),
  //                                    (2 * sizeof(int)) + (2 * sizeof(bool)),
  //                                    (3 * sizeof(int)) + (2 * sizeof(bool)),
  //                                    (4 * sizeof(int)) + (2 * sizeof(bool))};

  MPI_Aint displacements[8];
  displacements[0] = offsetof(Individual, ID);
  displacements[1] = offsetof(Individual, isInfected);
  displacements[2] = offsetof(Individual, isImmune);
  displacements[3] = offsetof(Individual, immunity_count);
  displacements[4] = offsetof(Individual, infection_count);
  displacements[5] = offsetof(Individual, susceptible_count);
  displacements[6] = offsetof(Individual, row);
  displacements[7] = offsetof(Individual, column);

  MPI_Datatype types[8] = {MPI_INT, MPI_C_BOOL, MPI_C_BOOL, MPI_INT, MPI_INT, MPI_INT, MPI_INT, MPI_INT};
  MPI_Type_create_struct(struct_length, lengths, displacements, types, &individual_type);
  MPI_Type_commit(&individual_type);

  return individual_type;
}
