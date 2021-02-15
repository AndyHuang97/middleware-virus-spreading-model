#include "utils.h"

#include <stdlib.h>

int rand_int(int min, int max) { return (rand() % (max - min + 1)) + min; }

void updateBufferSize(int** buffer, int actualLen, int maxLen) {
    if (maxLen == actualLen-1) {
      maxLen *= 2;
      buffer = realloc(*buffer, sizeof(int)*maxLen);
    }
}