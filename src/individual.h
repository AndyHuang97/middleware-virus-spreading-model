#include <stdbool.h>

typedef struct {
  int ID;
  bool isInfected;
  bool isImmune;
  int infection_count;
  int susceptible_count;
  int row;
  int column;
} Individual;

void updatePosition(Individual *ind, int speed);
void printIndividualData(Individual ind);
