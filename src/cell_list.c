#include "cell_list.h"

#include <stdio.h>
#include <stdlib.h>

//Add item at the start of the list
void push(CellList **head, int val) {
  CellList *new_ind;
  new_ind = (CellList *)malloc(sizeof(CellList));

  new_ind->id = val;
  new_ind->next = *head;
  *head = new_ind;
}

//TODO: Add remove method

void printList(CellList *head) {
  CellList *curr = head;
  while (curr != NULL) {
    int id = curr->id;
    printf("%d ", id);
    curr = curr->next;
  }
  printf("\n");
}