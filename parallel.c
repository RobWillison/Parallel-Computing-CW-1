#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <sys/time.h>

//The size of the grid
int size = 3;
int numberOfThreads = 1;
//A boolean 1/0 to tell the threads to continue or not
int cont = 0;
//The precision to work to
double precision = 0.01;

int iterations = 1;

pthread_barrier_t barrier;
//A matrix to write to and read from
double **readMatrix;
double **writeMatrix;
/**
* getMatrix
* width : the number of cells accross the matrix needs to be
* height : the number of cells down the matrix needs to be
*
* returns a double ** contatining a zeroed matrix width x height
**/
double **getMatrix(int width, int height)
{
  double** matrix;

  matrix = malloc(width * sizeof(double*));

  int i;
  for (i = 0; i < width; i++) {
    matrix[i] = calloc(height, sizeof(double));
  }

  return matrix;
}
/**
* printArray
* matrix : the matrix to print
*
* returns void
**/
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
/**
* relaxRow
* row : the intager relating to the index of the row in the matrix
*       to perform relaxation on
*
* sets cont to 1 or 0 depending on whether all cells met the precision
**/
void relaxRow(int row)
{
  //a double to keep track of the largest diffrence
  double maxDiffrence = 0.0;
  int x;

  //Loop through all cells in the row
  for (x = 1; x < size - 1; x++)
  {
      //calculate the average of all 4 neighbours
      double temp = readMatrix[row][x + 1];
      temp = temp + readMatrix[row][x - 1];
      temp = temp + readMatrix[row + 1][x];
      temp = temp + readMatrix[row - 1][x];
      temp = temp / 4.0f;
      //write the answer into the writeMatrix
      writeMatrix[row][x] = temp;
      //quick check if cont allready set then
      //don't bother checking precision for this cell
      if (cont) continue;
      //Work out the diffrence betoween new and old values
      double diffrence = readMatrix[row][x] - temp;
      //Check if the diffrence is bigger than the current max
      if (diffrence < 0)
      {
        diffrence = diffrence * -1.0f;
      }

      //If the precision is smaller than the diffrence set cont to true
      if (precision < diffrence) cont = 1;
  }
}
/**
* doSerialWork
*
* does the work required to be done in serial when using multiple threads
*
**/
void doSerialWork()
{
  //swap the read and write matrixs over
  double **temp = readMatrix;
  readMatrix = writeMatrix;
  writeMatrix = temp;
  //set continue back to 0
  cont = 0;
  //printf("NEXT ITERATION\n");
  //printArray(readMatrix);
}
/**
* relax
* rowNumber : the row to start applying relaxation too
**/
void *relax(int *rowNumber)
{
  int currentRow = *rowNumber;
  while(1)
  {
    //Check if the currentRow is a valid row
    if (currentRow > size - 2)
    {
      //If not reset the current row to the starting row
      currentRow = *rowNumber;
      pthread_barrier_wait(&barrier);
      //If cont = 0 all threads return
      if (!cont) {
        return NULL;
      };
      iterations++;
      //wait for threads the release one is designated the serial thread
      int returnValue = pthread_barrier_wait(&barrier);
      //One thread does the doSerialWork()
      if (returnValue == PTHREAD_BARRIER_SERIAL_THREAD)
      {
        doSerialWork();
      }
      //wait for all to colplete the release for another interation
      pthread_barrier_wait(&barrier);
    }
    //relax the currentRow
    relaxRow(currentRow);
    //Get next row to do
    currentRow = currentRow + numberOfThreads;
  }
  //shouldn't get here
  return NULL;
}
/**
* setupMatrix
* sets the read and write global matrixs with the boundary values
**/
void setupMatrix()
{
  readMatrix = getMatrix(size, size);
  writeMatrix = getMatrix(size, size);
  double boundingValues = 1;
  int x;
  //fill sides
  for (x = 0; x < size; x++)
  {
    readMatrix[x][0] = boundingValues;
    readMatrix[x][size - 1] = boundingValues;
    writeMatrix[x][0] = boundingValues;
    writeMatrix[x][size - 1] = boundingValues;
  }
  //fill top and bottom
  for (x = 0; x < size; x++)
  {
    readMatrix[0][x] = boundingValues;
    readMatrix[size - 1][x] = boundingValues;
    writeMatrix[0][x] = boundingValues;
    writeMatrix[size - 1][x] = boundingValues;
  }
}

int main(int argc, char **argv)
{
  if(argc <= 2) {
      printf("No Arguments");
      exit(1);
  }
  size = atoi(argv[1]);
  numberOfThreads = atoi(argv[2]);

  if (size - 2 < numberOfThreads) numberOfThreads = size - 2;

  //Setup timing stuff
  struct timeval startTime, endTime;
  gettimeofday(&startTime, NULL);

  setupMatrix();

  pthread_barrier_init(&barrier, NULL, numberOfThreads);
  //allocate array to store threads in
  pthread_t *threads = malloc(sizeof(pthread_t) * numberOfThreads);
  //create the threads
  int thread;
  for (thread = 1; thread < numberOfThreads + 1; thread++)
  {
    pthread_t newThread;
    int *threadNumber = (int*)malloc(sizeof(int));
    //give each thread there index, also the row to start on
    *threadNumber = thread;
    if(pthread_create(&newThread, NULL, (void *(*) (void*))relax, (void*)threadNumber)) return 1;
    threads[thread] = newThread;
  }
  //wait for all the threads to return
  for (thread = 1; thread < numberOfThreads + 1; thread++)
  {
    pthread_join(threads[thread], NULL);
  }
  //print matrix, debug only
  printf("FINAL THREADS %d\n", numberOfThreads);
  //printArray(readMatrix);
  //work out the time taken and print
  gettimeofday(&endTime, NULL);
  double cpuTimeUsed = (endTime.tv_sec - startTime.tv_sec) + ((endTime.tv_usec - startTime.tv_usec) / 1000000.0);
  printf("TIME USED %f\n", cpuTimeUsed);
  return 0;
}
