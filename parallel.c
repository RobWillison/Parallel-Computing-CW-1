#include <stdio.h>
#include <stdlib.h>

int size = 400;

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

double relax(double **read, double **write)
{
  double maxDiffrence = 0.0;
  int x, y;
  for (x = 1; x < size - 1; x++)
  {
    for (y = 1; y < size - 1; y++)
    {

      double temp = read[x + 1][y];
      temp = temp + read[x - 1][y];
      temp = temp + read[x][y + 1];
      temp = temp + read[x][y - 1];
      temp = temp / 4;

      double diffrence = write[x][y] - temp;
      if (diffrence < 0)
      {
        diffrence = diffrence * -1.0;
      }
      if (maxDiffrence < diffrence) maxDiffrence = diffrence;

      write[x][y] = temp;
    }
  }

  return maxDiffrence;
}

int main()
{
  double **readMatrix = get_matrix(size, size);
  double **writeMatrix = get_matrix(size, size);
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
    diffrence = relax(readMatrix, writeMatrix);
    double **temp = readMatrix;
    readMatrix = writeMatrix;
    writeMatrix = temp;
  }

  printArray(writeMatrix);

  return 0;
}
