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
  int countryID;
  CellList* head;
} Cell;

void push(CellList** head, int val);
void printList(CellList* head, int row, int col);
void clearGrid(Cell grid[][GRID_WIDTH]);
void assignCountries(Cell grid[GRID_HEIGHT][GRID_WIDTH]);