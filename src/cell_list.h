#include "parameters.h"

struct CellList_t {
  int id;
  struct CellList_t* next;
};
typedef struct CellList_t CellList;

//Represent a cell of the grid
//It contains the pointer to the head of the associated list
typedef struct
{
  CellList* head;
} ListPointer;

void push(CellList** head, int val);
int* getIDList(CellList *head_ref, int* length, int ignoreID);
void printList(CellList* head, int row, int col);
void clearGrid(ListPointer grid[][MAX_WIDTH]);