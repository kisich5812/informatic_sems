#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

/* Модифицировать программу, 
 * чтобы замерить среднее время завершения нити после сигнала на завершение.
 */
void * any_func (void * arg)
{
  while (1) 
  {
  	//fprintf (stderr, ".");
  	usleep (1000);
  }
  
  return NULL;
}

int main (void)
{
  int iterations = 50;
  long total_time_ns = 0;
  for (int i = 0; i < iterations; i++)
  {
    pthread_t thread;
    void * result;
    if (pthread_create (&thread, NULL, &any_func, NULL) != 0) 
    {
	fprintf (stderr, "Error\n");
	return 1;
    }
    usleep (10000);

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    pthread_cancel (thread);
    if (!pthread_equal (pthread_self (), thread))
    {
      pthread_join (thread, &result);
    }

    clock_gettime(CLOCK_MONOTONIC, &end);

    long seconds = end.tv_sec - start.tv_sec;
    long nanoseconds = end.tv_nsec - start.tv_nsec;
    long elapsed_ns = seconds * 1000000000L + nanoseconds;
    
    total_time_ns += elapsed_ns;
  }

  long avg_time_ns = total_time_ns / iterations;
  printf("Среднее время остановки: %ld микросекунд (%.3f мс)\n", 
         avg_time_ns / 1000, (float)avg_time_ns / 1000000.0f);  
  return 0;
}
