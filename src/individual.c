#include "individual.h"

#include <math.h>
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
