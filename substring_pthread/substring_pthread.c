#include <assert.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

/* literal constants */
#define LINEMAX   2048
#define CHUNK     5000

/* function declarations */
void getSubStr(char* x, char* y, int m, int n, char** out);
uint32_t stoint(char* string);
void* findSubStrs(void* tid);

/* global variables */
uint32_t line_count;
uint32_t chunk_count;
char** lines;
char** sub_strs;
int numThreads;

/* Program entry point */
int main(int argc, char **argv)
{
  /* get command line arguments */
  FILE* fp = fopen(argv[1], "r");
  line_count = stoint(argv[2]);
  
  /* local variables */
  int numNodes;               /* number of nodes */
  int numTasks;               /* number of tasks */
  double elapsedTime;         /* program run time */
  struct timeval t1, t2;      /* program start/end times */
  int i;                      /* loop counter */
  bool done;                  /* continue working */
  char prev_line[LINEMAX];    /* hold previous line */
  uint32_t line_idx;          /* current line idx */
  pthread_attr_t attr;        /* pthread attribute */
  void* status;

  /* get number of nodes and tasks per node */
  numNodes = stoint(getenv("SLURM_JOB_NUM_NODES"));
  numTasks = stoint(getenv("SLURM_CPUS_ON_NODE"));

  /* set number of threads */
  if(numNodes && numTasks)
  {
    numThreads = numNodes * numTasks;
  }
  else
  {
    numThreads = 4;
  }
  
  /* initialize pthread variables */
  pthread_t threadid[numThreads];
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

  /* start timer and print start message */
  gettimeofday(&t1, NULL);
  printf("DEBUG: starting job on %s with %s nodes and %s tasks per node\n",
         getenv("HOSTNAME"),
         getenv("SLURM_JOB_NUM_NODES"),
         getenv("SLURM_CPUS_ON_NODE"));

  /* allocate all lines */
  lines = (char**)malloc((CHUNK + 1) * sizeof(char*));
  for(i = 0; i < CHUNK + 1; i++)
  {
    lines[i] = (char*)malloc(LINEMAX * sizeof(char));
  }
  
  line_idx = 0;
  chunk_count = CHUNK;
  done = false;
  
  /* allocate substring storage */
  sub_strs = (char**)malloc(CHUNK * sizeof(char*));
  for(i = 0; i < CHUNK; i++)
  {
    sub_strs[i] = NULL;
  }
  
  fgets(prev_line, LINEMAX, fp);
  
  while(!done)
  {
    /* read lines */
    memcpy(lines[0], prev_line, LINEMAX);
    for(i = 0; i < CHUNK; i++)
    {
      if(fgets(lines[i+1], LINEMAX, fp) == NULL || line_idx + i >= line_count)
      {
        chunk_count = i;
        done = true;
        break;
      }
    }
    memcpy(prev_line, lines[CHUNK], LINEMAX);
    
    /* parallelized code to find substrings */
    for(i = 0; i < numThreads; i++)
    {
      pthread_create(&threadid[i], &attr, findSubStrs, (void *)i);
    }
    
    /* Wait on the other threads */
    for(i = 0; i < numThreads; i++)
    {
      pthread_join(threadid[i], &status);
    }
    
    /* print substrings */
    for(i = 0; i < chunk_count; i++)
    {
      printf("%u-%u: %s\n", line_idx+i+1, line_idx+i+2, sub_strs[i]);
    }
    
    line_idx += CHUNK;
    if(line_idx >= line_count)
    {
      done = true;
    }
  }
  
  pthread_attr_destroy(&attr);
  
  /* free substrings */
  for(i = 0; i < CHUNK; i++)
  {
    free(sub_strs[i]);
  }
  free(sub_strs);
  
  /* free lines */
  for(i = 0; i < CHUNK; i++)
  {
    free(lines[i]);
  }
  free(lines);
  
  
  /* close file */
  fclose(fp);
  
  /* stop timer and calculate run time */
  gettimeofday(&t2, NULL);
  elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0;
  elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0;
  
  /* print final run data */
  printf("DATA, 2, %s, %s, %d, %f\n", getenv("SLURM_JOB_NUM_NODES"),
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
void* findSubStrs(void* tid)
{
  uint32_t start_pos;
  uint32_t end_pos;
  uint32_t interval;
  uint32_t i;
  uint32_t id = (uint32_t)tid;
  
  interval = chunk_count / numThreads;
  if(chunk_count % numThreads)
  {
    interval++;
  }
  
  start_pos = id * interval;
  end_pos = (id + 1) * interval;
  
  if(end_pos > chunk_count)
  {
    end_pos = chunk_count;
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
  uint16_t max_len;
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
  max_len = len;
  
  /* traverse up diagonally from the (row, col) cell */
  while (com_suff[row][col] != 0)
  {
    resultStr[--len] = x[row - 1];

    /* move diagonally up to previous cell */
    row--;
    col--;
  }
  
  if(resultStr[max_len-1] == '\n')
  {
    resultStr[max_len-1] = '\0';
  }
  
  /* save longest common substring */
  *out = resultStr;
  
  for (i = 0; i < (m + 1); i++)
  {
    free(com_suff[i]);
  }
  free(com_suff);
  
}
