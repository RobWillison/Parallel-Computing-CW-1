#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <sys/time.h>


int size = 1000;
int number_of_threads = 1;

int cont = 1;
double precision = 0.001;

pthread_t *threads;
pthread_barrier_t barrier;

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

int relax_row(int row)
{
  double max_diffrence = 0.0;
  int x;
  int cont = 0;
  for (x = 1; x < size - 1; x++)
  {
      double temp = readMatrix[row][x + 1];
      temp = temp + readMatrix[row][x - 1];
      temp = temp + readMatrix[row + 1][x];
      temp = temp + readMatrix[row - 1][x];
      temp = temp / 4;

      double diffrence = writeMatrix[row][x] - temp;
      if (diffrence < 0)
      {
        diffrence = diffrence * -1.0;
      }
      if (precision < diffrence) cont = 1;

      writeMatrix[row][x] = temp;
  }

  return cont;
}

void doSerialWork()
{
  double **temp = readMatrix;
  readMatrix = writeMatrix;
  writeMatrix = temp;

  cont = 0;
}

void *relax(int *rowNumber)
{
  int currentRow = *rowNumber;
  while(1)
  {
    if (currentRow > size - 2)
    {
      currentRow = *rowNumber;
      int returnValue = pthread_barrier_wait(&barrier);

      if (!cont) {
        return NULL;
      };

      pthread_barrier_wait(&barrier);

      if (returnValue == PTHREAD_BARRIER_SERIAL_THREAD)
      {
        doSerialWork();
      }

      pthread_barrier_wait(&barrier);
    }

    cont = relax_row(currentRow);
    currentRow = currentRow + number_of_threads;
  }

  return NULL;
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
  struct timeval startTime, endTime;
  gettimeofday(&startTime, NULL);
  setup_matrix();
  pthread_barrier_init(&barrier, NULL, number_of_threads);

  threads = malloc(sizeof(pthread_t) * number_of_threads);
  int thread;
  for (thread = 1; thread < number_of_threads + 1; thread++)
  {
    pthread_t newThread;
    int *threadNumber = (int*)malloc(sizeof(int));
    *threadNumber = thread;
    if(pthread_create(&newThread, NULL, (void *(*) (void*))relax, (void*)threadNumber)) return 1;
    threads[thread] = newThread;
  }

  for (thread = 1; thread < number_of_threads + 1; thread++)
  {
    pthread_join(threads[thread], NULL);
  }

  //printArray(readMatrix);
  gettimeofday(&endTime, NULL);
  double cpu_time_used = (endTime.tv_sec - startTime.tv_sec) + ((endTime.tv_usec - startTime.tv_usec) / 1000000.0);
  printf("TIME USED %f\n", cpu_time_used);
  return 0;
}
