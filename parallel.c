#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <time.h>

//NOTES
//check if diffrence is smaller that percision in each thread then just return whether it was smaller
//to avoid having to find the largest double in a massive array

int size = 100;
int number_of_threads = 8;
int currentRow = 1;
int cont = 1;
double precision = 0.01;

pthread_t *threads;
pthread_barrier_t barrier;
pthread_mutex_t rowLock;

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

int getNextRow()
{
  int next = -1;
  pthread_mutex_lock(&rowLock);

  if (currentRow <= size - 2)
  {
    next = currentRow++;
  }
  pthread_mutex_unlock(&rowLock);

  return next;
}

void doSerialWork()
{
  double **temp = readMatrix;
  readMatrix = writeMatrix;
  writeMatrix = temp;

  cont = 0;
  currentRow = 1;
}

void *relax()
{
  while(cont)
  {
    int row = getNextRow();
    if (row == -1)
    {
      int returnValue = pthread_barrier_wait(&barrier);
      if (returnValue == PTHREAD_BARRIER_SERIAL_THREAD)
      {
        doSerialWork();
      }
      pthread_barrier_wait(&barrier);
    }

    double diff = relax_row(readMatrix, writeMatrix, row);

    if (diff > precision) cont = 1;
  }
}

void setup_matrix()
{
  readMatrix = get_matrix(size, size);
  writeMatrix = get_matrix(size, size);

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
}

int main()
{
  clock_t start = clock();
  setup_matrix();
  pthread_barrier_init(&barrier, NULL, number_of_threads);
  pthread_mutex_init(&rowLock, NULL);

  threads = malloc(sizeof(pthread_t) * number_of_threads);
  int thread;
  for (thread = 0; thread <= number_of_threads; thread++)
  {
    pthread_t newThread;
    if(pthread_create(&newThread, NULL, relax, NULL)) return 1;
    threads[thread] = newThread;
  }

  for (thread = 0; thread <= number_of_threads; thread++)
  {
    pthread_join(threads[thread], NULL);
  }

  //printArray(readMatrix);
  clock_t end = clock();
  double cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
  printf("TIME USED %f\n", cpu_time_used);
  return 0;
}
