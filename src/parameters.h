#ifndef CONFIG_H_
#define CONFIG_H_

// Struct to contain all the configuration parameters passed as input arguments
typedef struct {
  int POPULATION_SIZE;
  int INITITAL_INFECTED;
  int GRID_HEIGHT;
  int GRID_WIDTH;
  int TIME_STEP;
  int MAX_SPEED;
  int SPREAD_DISTANCE;
  int SUSCEPTIBILITY_THR;
  int INFECTION_THR;
  int IMMUNITY_THR;
  int COUNTRY_HEIGHT;
  int COUNTRY_WIDTH;
  int DAY;
  int END_TIME;
  double DENSITY_THR;
} Config;
#endif