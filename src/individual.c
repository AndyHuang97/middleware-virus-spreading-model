#include <stdbool.h>
#include <stdio.h>
#include "individual.h"

Individual makeIndividual(int ID, bool isInfected, float speed)
{
  Individual tmp;
  tmp.ID = ID;
  tmp.isInfected = isInfected;
  tmp.speed = speed;

  return tmp;
}

void updatePosition(Individual *ind, int newPosX, int newPosY)
{
  ind->positionX = newPosX;
  ind->positionY = newPosY;
}

void printIndividualData(Individual ind)
{
  printf("ID: %d, isInfected: %d, isImmune: %d, Speed: %f, posX: %d, posY: %d \n", ind.ID, ind.isInfected, ind.isImmune, ind.speed, ind.positionX, ind.positionY);
}
