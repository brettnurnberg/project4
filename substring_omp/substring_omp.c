#include <assert.h>
#include <omp.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

/* literal constants */
#define LINEMAX 4096
#define CHUNK   20

/* function declarations */
void printLCSubStr(char* x, char* y, int m, int n, uint32_t idx);
uint32_t stoint(char* string);
int getNextLines(char** x, uint32_t* idx);
void findSubStrs(void);

/* global variables */
FILE *fp;
uint32_t curr_line_idx;
char prev_line[LINEMAX];
uint32_t line_count;

/* Program entry point */
int main(int argc, char **argv)
{
  /* get command line arguments */
  fp = fopen(argv[1], "r");
  line_count = stoint(argv[2]);
  
  /* local variables */
  int numThreads = 1;         /* number of threads */
  int numNodes;               /* number of nodes */
  int numTasks;               /* number of tasks */
  double elapsedTime;         /* program run time */
  struct timeval t1, t2;      /* program start/end times */
  int i;                      /* loop counter */
  
  /* get number of nodes and tasks per node */
  numNodes = stoint(getenv("SLURM_JOB_NUM_NODES"));
  numTasks = stoint(getenv("SLURM_CPUS_ON_NODE"));
  
  /* set number of threads */
  if(numNodes && numTasks)
  {
    numThreads = numNodes * numTasks;
  }
  
  omp_set_num_threads(numThreads);
  
  /* start timer and print start message */
  gettimeofday(&t1, NULL);
  printf("DEBUG: starting job on %s with %s nodes and %s tasks per node\n",
         getenv("HOSTNAME"),
         getenv("SLURM_JOB_NUM_NODES"),
         getenv("SLURM_CPUS_ON_NODE"));
  
  /* get first line of text */
  curr_line_idx = 1;
  fgets(prev_line, LINEMAX, fp);
  
  /* parallelized code to find substrings */
  #pragma omp parallel
  {
    findSubStrs();
  }
  
  /* close file */
  fclose(fp);
  
  /* stop timer and calculate run time */
  gettimeofday(&t2, NULL);
  elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0;
  elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0;
  
  /* print final run data */
  printf("DATA, %s, %s, %d, %f\n", getenv("SLURM_JOB_NUM_NODES"),
                                   getenv("SLURM_CPUS_ON_NODE"),
                                   line_count,
                                   elapsedTime);
  
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
void findSubStrs(void)
{
  char* x[CHUNK+1];
  int count;
  uint32_t line_idx;
  int i;
  
  #pragma omp private(x,count,line_idx,i)
  {
    for(i = 0; i < CHUNK+1; i++)
    {
      x[i] = (char*)malloc(LINEMAX);
    }
    
    #pragma omp critical
    {
      count = getNextLines(x, &line_idx);
    }
    
    while(count)
    {
      for(i = 0; i < count; i++)
      {
        printLCSubStr(x[i], x[i+1], strlen(x[i]), strlen(x[i+1]), line_idx+i);
      }
      
      #pragma omp critical
      {
        count = getNextLines(x, &line_idx);
      }
    }
    
    for(i = 0; i < CHUNK+1; i++)
    {
      free(x[i]);
    }
  }
}

/* gets next chunk of lines to evaluate */
int getNextLines(char** x, uint32_t* idx)
{
  int count = 0;
  int i;
  *idx = curr_line_idx;
  
  /* save the previous line for comparison */
  memcpy(x[0], prev_line, strlen(prev_line)+1);
  
  /* read the next chunk of lines, or until EOF */
  for(i = 0; i < CHUNK; i++)
  {
    if((fgets(x[i+1], LINEMAX, fp) == NULL)
    || (curr_line_idx > line_count))
    {
      break;
    }
    else
    {
      count++;
      curr_line_idx++;
    }
  }
  
  /* save the last line for future use */
  if(count)
  {
    memcpy(prev_line, x[count], strlen(x[count]) + 1);
  }
  
  /* return number of new lines read */
  return count;
}

/* finds and prints the longest common substring of x and y */
void printLCSubStr(char* x, char* y, int m, int n, uint32_t idx)
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
    printf("\n");
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

  /* print longest common substring */
  printf("%u-%u: %s\n", idx, idx+1, resultStr);
  free(resultStr);
  
  for (i = 0; i < (m + 1); i++)
  {
    free(com_suff[i]);
  }
  free(com_suff);
  
}
