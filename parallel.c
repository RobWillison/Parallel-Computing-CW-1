#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <time.h>

//NOTES
//check if diffrence is smaller that percision in each thread then just return whether it was smaller
//to avoid having to find the largest double in a massive array

int size = 1000;
int number_of_threads = 40;
int *locks;
int *done;
double precision = 0.01;
pthread_t *threads;

double **readMatrix;
double **writeMatrix;

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
  double max_diffrence = 0.0;
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
      if (max_diffrence < diffrence) max_diffrence = diffrence;

      write[x][row] = temp;
  }

  return max_diffrence;
}

void *relax()
{

  int x;
  double max_diffrence = 0.0;
  int rowsleft = 1;
  while (rowsleft) {
    rowsleft = 0;
    for (x = 1; x < size - 1; x++)
    {
      //Check if row is locked
      if (locks[x - 1] || locks[x] || locks[x + 1] || done[x]) continue;
      rowsleft = 1;

      locks[x] = 1;
      locks[x - 1] = 1;
      locks[x + 1] = 1;

      double diffrence = relax_row(readMatrix, writeMatrix, x);

      done[x] = 1;
      locks[x] = 0;
      locks[x - 1] = 0;
      locks[x + 1] = 0;

      if (diffrence > max_diffrence) max_diffrence = diffrence;
    }
  }

  if (max_diffrence > precision)
  {
    return (void*)1;
  }

  return (void*)0;
}

int main()
{
  clock_t start = clock();
  readMatrix = get_matrix(size, size);
  writeMatrix = get_matrix(size, size);

  locks = calloc(size, sizeof(int));
  done = calloc(size, sizeof(int));
  threads = malloc(sizeof(pthread_t) * number_of_threads);
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
  int cont = 1;

  while (cont)
  {
    cont = 0;

    int thread;
    for (thread = 0; thread <= number_of_threads; thread++)
    {
      pthread_t newThread;
      if(pthread_create(&newThread, NULL, relax, NULL)) return 1;
      threads[thread] = newThread;
    }

    for (thread = 0; thread <= number_of_threads; thread++)
    {
      void *result;
      pthread_join(threads[thread], &result);
      if (result) cont = 1;
    }




    double **temp = readMatrix;
    readMatrix = writeMatrix;
    writeMatrix = temp;
    memset(done, 0, sizeof(int) * size);
  }

  //printArray(readMatrix);
  clock_t end = clock();
  double cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
  printf("TIME USED %f\n", cpu_time_used);
  return 0;
}
