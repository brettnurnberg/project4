#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#define LINEMAX 4096
#define CHUNK   20

void printLCSubStr(char* X, char* Y, int m, int n, uint32_t idx);
uint32_t stoint(char* string);
int getNextLines(char** x, uint32_t* idx);
void findSubStrs(void);

FILE *fp;
uint32_t curr_line_idx;
char prev_line[LINEMAX];
uint32_t line_count;

/* Program entry point */
int main(int argc, char **argv)
{
  fp = fopen(argv[1], "r");
  line_count = stoint(argv[2]);
  uint32_t numThreads = stoint(argv[3]);
  
  double elapsedTime;
  struct timeval t1, t2;
  int i;
  
  gettimeofday(&t1, NULL);
  
  assert(fp != NULL);
  assert(line_count > 1);
  
  printf("DEBUG: starting job on %s\n", getenv("HOSTNAME"));
  
  curr_line_idx = 1;
  fgets(prev_line, LINEMAX, fp);
  
  /* this section must be chunked */
  for(i = 0; i < numThreads; i++)
  {
    findSubStrs();
  }
  
  fclose(fp);
  
  gettimeofday(&t2, NULL);
  
  elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0;
  elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0;
  
  printf("DATA, %s, %s, %s, %d, %f\n", getenv("SLURM_NNODES"),
                                       getenv("SLURM_NPROCS"),
                                       getenv("SLURM_NTASKS"),
                                       line_count,
                                       elapsedTime);
  
  return 0;
}

uint32_t stoint(char* string)
{
  uint32_t val = 0;
  int i = 0;
  
  while (string[i] != '\0')
  {
    val = val*10 + string[i] - '0';
    i++;
  }
  
  return val;
}

void findSubStrs(void)
{
  char* x[CHUNK+1];
  int count;
  uint32_t line_idx;
  int i;
  
  for(i = 0; i < CHUNK+1; i++)
  {
    x[i] = (char*)malloc(LINEMAX);
  }
  
  /* Critical line */
  count = getNextLines(x, &line_idx);
  
  while(count)
  {
    for(i = 0; i < count; i++)
    {
      printLCSubStr(x[i], x[i+1], strlen(x[i]), strlen(x[i+1]), line_idx+i);
    }
    
    /* Critical line */
    count = getNextLines(x, &line_idx);
  }
  
  for(i = 0; i < CHUNK+1; i++)
  {
    free(x[i]);
  }
  
}

int getNextLines(char** x, uint32_t* idx)
{
  int count = 0;
  int i;
  *idx = curr_line_idx;
  
  memcpy(x[0], prev_line, strlen(prev_line)+1);
  
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
  
  if(count)
  {
    memcpy(prev_line, x[count], strlen(x[count]) + 1);
  }
  
  return count;
}

/* function to find and print the longest common 
   substring of X[0..m-1] and Y[0..n-1] */
void printLCSubStr(char* X, char* Y, int m, int n, uint32_t idx)
{
  /* lengths of longest common suffixes of substrings */
  uint16_t LCSuff[m + 1][n + 1];

  /* length of the longest common substring */
  uint16_t len = 0;

  /* index of cell which contains the maximum value */
  uint16_t row, col;
  uint16_t i, j;

  /* Build LCSuff[m+1][n+1] in bottom up fashion. */
  for (i = 0; i <= m; i++)
  {
    for (j = 0; j <= n; j++)
    {
      if (i == 0 || j == 0)
      {
        LCSuff[i][j] = 0;
      }
      else if (X[i - 1] == Y[j - 1])
      {
        LCSuff[i][j] = LCSuff[i - 1][j - 1] + 1;
        if (len < LCSuff[i][j])
        {
          len = LCSuff[i][j];
          row = i;
          col = j;
        }
      }
      else
      {
        LCSuff[i][j] = 0;
      }
    }
  }
  
  /* no common substring exists */
  if (len == 0)
  {
    printf("STRING,\n");
    return;
  }

  /* allocate space for longest common substring */
  char* resultStr = (char*)malloc((len + 1) * sizeof(char));
  resultStr[len] = '\0';

  /* traverse up diagonally form the (row, col) cell
  until LCSuff[row][col] != 0 */
  while (LCSuff[row][col] != 0)
  {
    resultStr[--len] = X[row - 1]; // or Y[col-1]

    /* move diagonally up to previous cell */
    row--;
    col--;
  }

  /* print longest common substring */
  printf("STRING %u-%u: %s\n", idx, idx+1, resultStr);
  free(resultStr);
}
