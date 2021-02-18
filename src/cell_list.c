#include "cell_list.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "parameters.h"

//Add item at the start of the list
void push(CellList **head, int val) {
  CellList *new_ind;
  new_ind = (CellList *)malloc(sizeof(CellList));

  new_ind->id = val;
  new_ind->next = *head;
  *head = new_ind;
}

void deleteList(CellList **head_ref) {
  CellList *current = *head_ref;
  CellList *next;

  while (current != NULL) {
    next = current->next;
    free(current);
    current = next;
  }
  *head_ref = NULL;
}

void clearGrid(Cell grid[][GRID_WIDTH]) {
  for (int i = 0; i < GRID_HEIGHT; i++) {
    for (int j = 0; j < GRID_WIDTH; j++) {
      deleteList(&grid[i][j].head);
    }
  }
}

void printList(CellList *head, int row, int col) {
  printf("Content of list in position (%d, %d): ", row, col);
  CellList *curr = head;
  while (curr != NULL) {
    int id = curr->id;
    printf("%d ", id);
    curr = curr->next;
  }
  printf("\n");
}

int assignCountries(Cell grid[GRID_HEIGHT][GRID_WIDTH]) {
  int countriesByWidth = floor(GRID_WIDTH / COUNTRY_WIDTH);
  int countriesByHeight = floor(GRID_HEIGHT / COUNTRY_HEIGHT);

  int countriesCount = 0;
  int startRow = 0;
  int startCol = 0;

  while (countriesCount < countriesByHeight * countriesByWidth) {
    for (int i = startRow; i < startRow + COUNTRY_HEIGHT; i++) {
      for (int j = startCol; j < startCol + COUNTRY_WIDTH; j++) {
        grid[i][j].countryID = countriesCount;
      }
    }

    int assignedCountryWidth = (startCol + COUNTRY_WIDTH) / COUNTRY_WIDTH;
    if (assignedCountryWidth == countriesByWidth) {
      startCol = 0;
      startRow = startRow + COUNTRY_HEIGHT;
    } else
      startCol = startCol + COUNTRY_WIDTH;

    countriesCount++;
  }

  if ((GRID_WIDTH % COUNTRY_WIDTH) != 0 || (GRID_HEIGHT % COUNTRY_HEIGHT) != 0) {
    //Assign all the remaining horizontal cells to a new country
    for (int i = countriesByHeight * COUNTRY_HEIGHT; i < GRID_HEIGHT; i++) {
      for (int j = 0; j < GRID_WIDTH; j++) {
        grid[i][j].countryID = countriesCount;
      }
    }

    //Assign the remaining vertial cells to a new country
    for (int i = 0; i < countriesByHeight * COUNTRY_HEIGHT; i++) {
      for (int j = countriesByWidth * COUNTRY_WIDTH; j < GRID_WIDTH; j++) {
        grid[i][j].countryID = countriesCount;
      }
    }
    countriesCount++;
  }

  return countriesCount;
}