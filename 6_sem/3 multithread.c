#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <math.h>

/* Переделать программу для доказательстава: sin*sin + cos*cos ==1
 * Квадрат синуса считать в одном потоке, косинуса во втором,
 * а результат суммировать в главной программе.*/

double sin_arr[91];
double cos_arr[91];

void * thread_sin (void * arg)
{
	for (int i = 0; i <= 90; i++) {
		double rad = i * (M_PI / 180);
		double s = sin(rad);
		sin_arr[i] = s*s;
	}
	return NULL;
}

void * thread_cos (void * arg)
{
	for (int i = 0; i <= 90; i++) {
		double rad = i * (M_PI / 180);
		double s = cos(rad);
		cos_arr[i] = s*s;
	}
	return NULL;
}

int main (void)
{
	pthread_t thread1, thread2;

	if (pthread_create (&thread1, NULL,
			                &thread_sin, NULL) != 0)
	{
		fprintf (stderr, "Error (thread1)\n");
		return 1;
	}

	if (pthread_create (&thread2, NULL,
			&thread_cos, NULL) != 0) 
	{
		fprintf (stderr, "Error (thread2)\n");
		return 1;
	}

	pthread_join(thread1, NULL);
	pthread_join(thread2, NULL);

	printf("Считаем сумму квадратов синусов и косинусов:\n");
	for (int i = 0; i <= 90; i++) {
		double total = sin_arr[i] + cos_arr[i];
		printf("Угол %2d°: %.6f + %.6f = %.6f\n", i, sin_arr[i], cos_arr[i], total);
	}
	return 0;
}
