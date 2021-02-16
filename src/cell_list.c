#include "cell_list.h"

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

void clearGrid(Cell grid[][MAX_WIDTH]) {
  for (int i = 0; i < MAX_HEIGHT; i++) {
    for (int j = 0; j < MAX_WIDTH; j++) {
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