#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

int size = 10;
int *locks;

double **get_matrix(int width, int height)
{
  double** matrix;

  matrix = malloc(width * sizeof(double*));

  int i;
  for (i = 0; i < width; i++) {
    matrix[i] = calloc(height, sizeof(double));
  }

  return matrix;
}

void printArray(double **matrix)
{
  int x;
  int y;
  for (x = 0; x < size; x++) {
    for (y = 0; y < size; y++) {
      printf("%f ", matrix[x][y]);
    }
    printf("\n");
  }
}

double relax_row(double **read, double **write, int row)
{
  double maxDiffrence = 0.0;
  int x;
  for (x = 1; x < size - 1; x++)
  {
      double temp = read[x + 1][row];
      temp = temp + read[x - 1][row];
      temp = temp + read[x][row + 1];
      temp = temp + read[x][row - 1];
      temp = temp / 4;

      double diffrence = write[x][row] - temp;
      if (diffrence < 0)
      {
        diffrence = diffrence * -1.0;
      }
      if (maxDiffrence < diffrence) maxDiffrence = diffrence;

      write[x][row] = temp;
  }

  return maxDiffrence;
}

double relax(void *arguments)
{
  struct arg_struct *args = (struct arg_struct *)args;
  double **read = (double**)args->read;
  double **write = (double**)args->write;
  int x;
  double maxDiffrence = 0.0;
  for (x = 1; x < size - 1; x++)
  {
    //Check if row is locked
    if (locks[x - 1] || locks[x] || locks[x + 1]) continue;

    locks[x] = 1;
    locks[x - 1] = 1;
    locks[x + 1] = 1;
    double diffrence = relax_row(read, write, x);
    locks[x] = 0;
    locks[x - 1] = 0;
    locks[x + 1] = 0;
    if (diffrence > maxDiffrence) maxDiffrence = diffrence;
  }

  return maxDiffrence;
}

int main()
{
  double **readMatrix = get_matrix(size, size);
  double **writeMatrix = get_matrix(size, size);
  locks = calloc(size, sizeof(int));

  int x;
  //fill sides
  for (x = 0; x < size; x++)
  {
    readMatrix[x][0] = 1.0;
    readMatrix[x][size - 1] = 1.0;
    writeMatrix[x][0] = 1.0;
    writeMatrix[x][size - 1] = 1.0;
  }
  //fill top and bottom
  for (x = 0; x < size; x++)
  {
    readMatrix[0][x] = 1.0;
    readMatrix[size - 1][x] = 1.0;
    writeMatrix[0][x] = 1.0;
    writeMatrix[size - 1][x] = 1.0;
  }
  double diffrence = 1.0;

  while (diffrence > 0.01)
  {
    pthread_t thread1;
    pthread_t thread2;

    struct arguments args;
    args.read = readMatrix;
    args.write = writeMatrix;

    if(pthread_create(&thread1, NULL, relax, &readMatrix, (void *)&args)) return 1;
    pthread_join(thread1, NULL);
    diffrence = 0.0;
    //diffrence = relax(readMatrix, writeMatrix);
    double **temp = readMatrix;
    readMatrix = writeMatrix;
    writeMatrix = temp;
  }

  printArray(writeMatrix);

  return 0;
}
