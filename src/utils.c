#include "utils.h"

#include <stdio.h>
#include <stdlib.h>

#include "parameters.h"

int rand_int(int min, int max) { return (rand() % (max - min + 1)) + min; }

void checkParameters(Config config) {
  if (config.COUNTRY_HEIGHT > config.GRID_HEIGHT || config.COUNTRY_WIDTH > config.GRID_WIDTH) {
    printf("Country size must be less then or equal to the grid size. \n");
    exit(1);
  }
  if (config.TIME_STEP >= config.END_TIME) {
    printf("Time step must be at least less then the end time. \n");
    exit(1);
  }
  if (config.INITITAL_INFECTED >= config.POPULATION_SIZE) {
    printf("The number of initial infected must be less then the population size. \n");
    exit(1);
  }
  if (config.DENSITY_THR <= 0 || config.DENSITY_THR > 1) {
    printf("The density threshold must be between 0 (excluded) and 1 \n");
    exit(1);
  }
}