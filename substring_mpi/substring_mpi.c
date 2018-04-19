#include <assert.h>
#include <mpi.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

/* literal constants */
#define LINEMAX   2048
#define STRMAX    512

/* function declarations */
void getSubStr(char* x, char* y, int m, int n, char** out);
uint32_t stoint(char* string);
void* findSubStrs(void* rank);

/* global variables */
uint32_t line_count;
char** lines;
char** sub_strs;
char** global_sub_strs;
int numThreads;

/* Program entry point */
int main(int argc, char **argv)
{
  /* local variables */
  double elapsedTime;         /* program run time */
  struct timeval t1, t2;      /* program start/end times */
  uint32_t i, j;              /* loop counters */
  pthread_attr_t attr;        /* pthread attribute */
  void* status;
  
  int rc;
  int rank;
	MPI_Status Status;

  /* initialize mpi */
	rc = MPI_Init(&argc,&argv);
	if (rc != MPI_SUCCESS)
  {
	  printf ("Error starting MPI program. Terminating.\n");
    MPI_Abort(MPI_COMM_WORLD, rc);
  }
  
  /* set number of threads */
  MPI_Comm_size(MPI_COMM_WORLD,&numThreads);
  MPI_Comm_rank(MPI_COMM_WORLD,&rank);

  /* allocate all lines */
  lines = (char**)malloc(line_count * sizeof(char*));
  for(i = 0; i < LINEMAX; i++)
  {
    lines[i] = (char*)malloc(LINEMAX * sizeof(char));
  }
  
  /* Initialization code to run once */
  if(rank == 0)
  {
    /* get command line arguments */
    FILE* fp = fopen(argv[1], "r");
    line_count = stoint(argv[2]);
    
    /* start timer and print start message */
    gettimeofday(&t1, NULL);
    printf("DEBUG: starting job on %s with %s nodes and %s tasks per node\n",
           getenv("HOSTNAME"),
           getenv("SLURM_JOB_NUM_NODES"),
           getenv("SLURM_CPUS_ON_NODE"));

    /* read in all lines */
    while(fgets(lines[i], LINEMAX, fp) != NULL && i < line_count)
    {
      i++;
    }

    /* close file */
    fclose(fp);

    if(i < line_count)
    {
      line_count = i;
    }
  }
  
  /* broadcast line data */
  MPI_Bcast(&line_count,                    1, MPI_UINT32_T, 0, MPI_COMM_WORLD);
  MPI_Bcast(      lines, line_count * LINEMAX,     MPI_CHAR, 0, MPI_COMM_WORLD);
  
  /* allocate global substring storage */
  global_sub_strs = (char**)malloc(line_count * sizeof(char*));
  for(i = 0; i < line_count; i++)
  {
    global_sub_strs[i] = (char*)malloc(sizeof(char) * STRMAX);
  }
  
  /* allocate and empty local substring storage */
  sub_strs = (char**)malloc(line_count * sizeof(char*));
  for(i = 0; i < line_count; i++)
  {
    sub_strs[i] = (char*)malloc(sizeof(char) * STRMAX);
    for(j = 0; j < STRMAX; j++)
    {
      sub_strs[i][j] = 0;
    }
  }
  
  /* parallelized code to find substrings */
  findSubStrs(&rank);
  
  /* bring data together */
  MPI_Reduce(sub_strs, global_sub_strs, line_count * STRMAX, MPI_CHAR, MPI_SUM, 0, MPI_COMM_WORLD);
  
  /* free lines */
  for(i = 0; i < line_count; i++)
  {
    free(lines[i]);
  }
  free(lines);
  
  /* free local substrings */
  for(i = 0; i < line_count-1; i++)
    {
      free(sub_strs[i]);
    }
    free(sub_strs);
  
  /* print results */
  if(rank == 0)
  {
    /* print and free substrings */
    for(i = 0; i < line_count-1; i++)
    {
      printf("%u-%u: %s\n", i+1, i+2, global_sub_strs[i]);
      free(global_sub_strs[i]);
    }
    free(global_sub_strs);
    
    /* stop timer and calculate run time */
    gettimeofday(&t2, NULL);
    elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0;
    elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0;
    
    /* print final run data */
    printf("DATA, %s, %s, %d, %f\n", getenv("SLURM_JOB_NUM_NODES"),
                                     getenv("SLURM_CPUS_ON_NODE"),
                                     line_count,
                                     elapsedTime);
  }
  
  MPI_Finalize();
  return 0;
}

/* converts a string to an integer */
uint32_t stoint(char* string)
{
  uint32_t val = 0;
  int i = 0;
  
  if(string == NULL)
  {
    return 0;
  }
  
  while (string[i] != '\0')
  {
    val = val*10 + string[i] - '0';
    i++;
  }
  
  return val;
}

/* finds all substrings in a file */
void* findSubStrs(void* rank)
{
  uint32_t start_pos;
  uint32_t end_pos;
  uint32_t interval;
  uint32_t i;
  uint32_t id = *((uint32_t*) rank);
  
  interval = line_count / numThreads;
  if(line_count % numThreads)
  {
    interval++;
  }
  
  start_pos = id * interval;
  end_pos = (id + 1) * interval;
  
  if(end_pos >= line_count)
  {
    end_pos = line_count;
  }
  
  for(i = start_pos; i < end_pos; i++)
  {
    getSubStr(lines[i], lines[i+1], strlen(lines[i]), strlen(lines[i+1]), &sub_strs[i]);
  }
  
}

/* finds and prints the longest common substring of x and y */
void getSubStr(char* x, char* y, int m, int n, char** out)
{
  uint16_t i, j;      /* loop counters */
  uint16_t len = 0;   /* length of the longest common substring */
  uint16_t row, col;  /* index of cell which contains the maximum value */
  
  /* lengths of longest common suffixes of substrings */
  uint16_t** com_suff = (uint16_t**)malloc((m + 1) * sizeof(uint16_t *));
  
  for (i = 0; i < (m + 1); i++)
  {
    com_suff[i] = (uint16_t*)malloc((n + 1) * sizeof(uint16_t));
  }

  /* Build com_suff[m+1][n+1] in bottom up fashion. */
  for (i = 0; i <= m; i++)
  {
    for (j = 0; j <= n; j++)
    {
      if (i == 0 || j == 0)
      {
        com_suff[i][j] = 0;
      }
      else if (x[i - 1] == y[j - 1])
      {
        com_suff[i][j] = com_suff[i - 1][j - 1] + 1;
        if (len < com_suff[i][j])
        {
          len = com_suff[i][j];
          row = i;
          col = j;
        }
      }
      else
      {
        com_suff[i][j] = 0;
      }
    }
  }
  
  /* no common substring exists */
  if (len == 0)
  {
    return;
  }

  /* allocate space for longest common substring */
  char* resultStr = (char*)malloc((len + 1) * sizeof(char));
  resultStr[len] = '\0';

  /* traverse up diagonally from the (row, col) cell */
  while (com_suff[row][col] != 0)
  {
    resultStr[--len] = x[row - 1];

    /* move diagonally up to previous cell */
    row--;
    col--;
  }

  /* save longest common substring */
  *out = resultStr;
  
  for (i = 0; i < (m + 1); i++)
  {
    free(com_suff[i]);
  }
  free(com_suff);
  
}
