typedef enum { UP,
               DOWN,
               RIGHT,
               LEFT } Direction;
int rand_int(int min, int max);
void updateBufferSize(int** buffer, int actualLen, int maxLen);