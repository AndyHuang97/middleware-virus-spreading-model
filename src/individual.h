typedef struct
{
  int ID;
  bool isInfected;
  bool isImmune;
  float speed;
  int positionX;
  int positionY;

} Individual;

Individual makeIndividual(int ID, bool infected, float speed);
void updatePosition(Individual *ind, int newPosX, int newPosY);
void printIndividualData(Individual ind);
