#include "individual.h"

#include <math.h>
#include <mpi.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

void printIndividualData(Individual ind, int countryID) {
  printf("ID: %d, (row: %d, col: %d) Country: %d \t isInfected: %d, isImmune: %d, infection_count: %d, immunity_count: %d, susceptible_count: %d, speed: %d, direction: %d\n",
         ind.ID, ind.row, ind.column, countryID, ind.isInfected, ind.isImmune, ind.infection_count, ind.immunity_count, ind.susceptible_count, ind.speed, ind.direction);
}

// Updates the position of an individual by moving in a random direction
// If the individual reaches the boundaries the direction is inverted
void updatePosition(Individual *individual, Config config) {
  for (int s = individual->speed; s > 0; s--) {
    switch (individual->direction) {
      case UP:
        if (individual->row == 0) {
          individual->row++;
          individual->direction = DOWN;
        } else
          individual->row--;
        break;
      case DOWN:
        if (individual->row == (config.GRID_HEIGHT - 1)) {
          individual->row--;
          individual->direction = UP;
        } else
          individual->row++;
        break;
      case LEFT:
        if (individual->column == 0) {
          individual->column++;
          individual->direction = RIGHT;
        } else
          individual->column--;
        break;
      case RIGHT:
        if (individual->column == (config.GRID_WIDTH - 1)) {
          individual->column--;
          individual->direction = LEFT;
        } else
          individual->column++;
        break;
      default:
        break;
    }
  }
}

// Search on susceptible individuals for infected in their neighborhood. If at least one infected is found update the counter
void searchAndUpdateOnSusceptibles(Individual *ind, int height, int width, Cell grid[height][width], Individual individuals[], int spreadDistance, int timeLeft, Config config) {
  if (ind->isImmune) {
    int newTimeLeft = timeLeft - (config.SUSCEPTIBILITY_THR - ind->susceptible_count);
    ind->susceptible_count += timeLeft;
    if (ind->susceptible_count >= config.SUSCEPTIBILITY_THR) {
      ind->isImmune = false;
      ind->susceptible_count = 0;
      searchAndUpdateOnSusceptibles(ind, height, width, grid, individuals, spreadDistance, newTimeLeft, config);
    }
    return;
  } else if (ind->isInfected) {
    int newTimeLeft = timeLeft - (config.IMMUNITY_THR - ind->immunity_count);
    ind->immunity_count += timeLeft;
    if (ind->immunity_count >= config.IMMUNITY_THR) {
      ind->isImmune = true;
      ind->isInfected = false;
      ind->immunity_count = 0;
      searchAndUpdateOnSusceptibles(ind, height, width, grid, individuals, spreadDistance, newTimeLeft, config);
    }
    return;
  } else {
    for (int i = -spreadDistance; i <= spreadDistance; i++) {
      for (int j = -spreadDistance; j <= spreadDistance; j++) {
        if ((ind->row + i >= 0 && ind->row + i < config.GRID_HEIGHT) && (ind->column + j >= 0 && ind->column + j < config.GRID_WIDTH)) {
          // printf("Individual ID %d) at cell (%d,%d) checking neighbouring cell (%d,%d)\n", ind->ID, ind->row, ind->column, ind->row+i, ind->column+j);
          bool infection = infectedInCell(grid[ind->row + i][ind->column + j].head, individuals);
          if (infection) {
            int newTimeLeft = timeLeft - (config.INFECTION_THR - ind->infection_count);
            ind->infection_count += timeLeft;
            if (ind->infection_count >= config.INFECTION_THR) {
              ind->isInfected = true;
              ind->infection_count = 0;
              searchAndUpdateOnSusceptibles(ind, height, width, grid, individuals, spreadDistance, newTimeLeft, config);
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

//Search for the susceptible individuals in the neighborhood of an infected individual
void searchSusceptibleOnInfected(Individual *ind, int height, int width, Cell grid[height][width], Individual individuals[], int spreadDistance, bool susceptibleFlags[], Config config) {
  if (!ind->isInfected)
    return;
  else {
    for (int i = -spreadDistance; i <= spreadDistance; i++) {
      for (int j = -spreadDistance; j <= spreadDistance; j++) {
        if ((ind->row + i >= 0 && ind->row + i < config.GRID_HEIGHT) && (ind->column + j >= 0 && ind->column + j < config.GRID_WIDTH)) {
          // printf("Individual ID %d) at cell (%d,%d) checking neighbouring cell (%d,%d)\n", ind->ID, ind->row, ind->column, ind->row+i, ind->column+j);
          updateSuscpetibleFlags(grid[ind->row + i][ind->column + j].head, individuals, susceptibleFlags);
        }
      }
    }
  }
}

void updateSuscpetibleFlags(CellList *head, Individual individuals[], bool susceptibleFlags[]) {
  CellList *curr = head;

  while (curr != NULL) {
    //printf("ID in list: %d\n", curr->id);
    susceptibleFlags[curr->id] = true;
    curr = curr->next;
  }
}

void updateIndividualCounters(Individual *ind, bool updateInfectionCounter, int timeLeft, Config config) {
  if (ind->isImmune) {
    int newTimeLeft = timeLeft - (config.SUSCEPTIBILITY_THR - ind->susceptible_count);
    ind->susceptible_count += timeLeft;
    if (ind->susceptible_count >= config.SUSCEPTIBILITY_THR) {
      ind->isImmune = false;
      ind->susceptible_count = 0;
      updateIndividualCounters(ind, updateInfectionCounter, newTimeLeft, config);
    }
    return;
  } else if (ind->isInfected) {
    int newTimeLeft = timeLeft - (config.IMMUNITY_THR - ind->immunity_count);
    ind->immunity_count += timeLeft;
    if (ind->immunity_count >= config.IMMUNITY_THR) {
      ind->isImmune = true;
      ind->isInfected = false;
      ind->immunity_count = 0;
      updateIndividualCounters(ind, updateInfectionCounter, newTimeLeft, config);
    }
    return;
  } else if (updateInfectionCounter) {
    int newTimeLeft = timeLeft - (config.INFECTION_THR - ind->infection_count);
    ind->infection_count += timeLeft;
    // printf("(%d) INF COUNT: %d\n", ind->ID, ind->infection_count);
    if (ind->infection_count >= config.INFECTION_THR) {
      ind->isInfected = true;
      ind->infection_count = 0;
      updateIndividualCounters(ind, updateInfectionCounter, newTimeLeft, config);
    }
    return;
  } else {
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

MPI_Datatype serializeIndividualStruct() {
  MPI_Datatype individual_type;
  int lengths[10] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1};

  MPI_Aint displacements[10];
  displacements[0] = offsetof(Individual, ID);
  displacements[1] = offsetof(Individual, isInfected);
  displacements[2] = offsetof(Individual, isImmune);
  displacements[3] = offsetof(Individual, immunity_count);
  displacements[4] = offsetof(Individual, infection_count);
  displacements[5] = offsetof(Individual, susceptible_count);
  displacements[6] = offsetof(Individual, row);
  displacements[7] = offsetof(Individual, column);
  displacements[8] = offsetof(Individual, speed);
  displacements[9] = offsetof(Individual, direction);

  MPI_Datatype types[10] = {MPI_INT, MPI_C_BOOL, MPI_C_BOOL, MPI_INT, MPI_INT, MPI_INT, MPI_INT, MPI_INT, MPI_INT, MPI_INT};
  MPI_Type_create_struct(10, lengths, displacements, types, &individual_type);
  MPI_Type_commit(&individual_type);

  return individual_type;
}
