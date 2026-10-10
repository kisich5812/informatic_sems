/* создать многопоточную программу для рассчёта синуса.
  * исследовать как количество потоков влияет на точность и скорость вычисления.
  * сравнить результаты со стандартной функцией.
*/
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#include <time.h>

long long factorial(int n) {
	long long f = 1;
	for (int i = 2; i <= n; i++) {
		f *= i;
	}
	return f;
}

/* Потоковая функция: считает конкретный член ряда Маклорена для всех углов от 0 до 90 */
void * calc_sine_term (void * arg)
{
	int k = *(int *)arg; // Номер члена ряда
	int power = 2 * k + 1; // Степень для синуса (1, 3, 5, 7...)
	double sign = (k % 2 == 0) ? 1.0 : -1.0; // Знакочередование (+ - + -)

	double *arr = malloc(91 * sizeof(double));
	if (!arr) return NULL;

	for (int deg = 0; deg <= 90; deg++) {
		double rad = deg * (M_PI / 180.0);
		
		double p = 1.0;
		for (int i = 0; i < power; i++) {
			p *= rad;
		}

		arr[deg] = sign * (p / factorial(power));
	}

	return (void *)arr; // Возвращаем указатель на массив
}

int main (int argc, char *argv[])
{
	if (argc < 2) return -fprintf(stderr, "Использование: %s <количество членов ряда>\n", argv[0]);

	int terms = atoi(argv[1]);
	if (terms <= 0) return -fprintf(stderr, "Количество членов должно быть больше 0\n");

	pthread_t *threads = malloc(terms * sizeof(pthread_t));
	int *idx = malloc(terms * sizeof(int));
	double *thread_results[terms];
	double my_sin[91] = {0};
	double std_sin[91];

	struct timespec start, end;

	// Свой метод
	clock_gettime(CLOCK_MONOTONIC, &start);
	for (int i = 0; i < terms; i++) {
		idx[i] = i;
		if (pthread_create(&threads[i], NULL, &calc_sine_term, &idx[i]) != 0) {
			return -fprintf(stderr, "Ошибка создания потока %d\n", i);
		}
	}

	// Собираем результаты работы потоков
	for (int i = 0; i < terms; i++) {
		void *res;
		pthread_join(threads[i], &res);
		thread_results[i] = (double *)res;
	}

	// Суммируем массивы от всех потоков в один итоговый
	for (int deg = 0; deg <= 90; deg++) {
		for (int i = 0; i < terms; i++) {
			my_sin[deg] += thread_results[i][deg];
		}
	}

	clock_gettime(CLOCK_MONOTONIC, &end);
	long custom_time = (end.tv_sec - start.tv_sec) * 1000000L + (end.tv_nsec - start.tv_nsec) / 1000L;

	// Очищаем временные массивы, возвращенные потоками
	for (int i = 0; i < terms; i++) {
		free(thread_results[i]);
	}
	free(threads);
	free(idx);

	// Стандартная функция
	clock_gettime(CLOCK_MONOTONIC, &start);

	for (int deg = 0; deg <= 90; deg++) {
		std_sin[deg] = sin(deg * (M_PI / 180.0));
	}

	clock_gettime(CLOCK_MONOTONIC, &end);
	long std_time = (end.tv_sec - start.tv_sec) * 1000000L + (end.tv_nsec - start.tv_nsec) / 1000L;


	// Вывод результатов
	printf("Количество членов ряда: %d\n", terms);
	printf("Время самописного многопоточного метода: %ld мкс\n", custom_time);
	printf("Время стандартной функции sin():	   %ld мкс\n\n", std_time);

	printf("Сравнение результатов (для каждого 10-го градуса):\n\n");
	printf("| Угол | Самописный | Стандартный | Разница | Погрешность, %% |\n");
	printf("| :---: | :---: | :---: | :---: | :---: |\n");

	for (int deg = 0; deg <= 90; deg += 10) {
		double diff = my_sin[deg] - std_sin[deg];
		if (diff < 0) diff = -diff; // Абсолютная разница
		
		double pct = (std_sin[deg] != 0.0) ? (diff / std_sin[deg]) * 100.0 : 0.0;
		if (pct < 0) pct = -pct;

		printf("| %2d° | %.6f | %.6f | %.6f | %.4f%% |\n", 
			   deg, my_sin[deg], std_sin[deg], diff, pct);
	}

	return 0;
}
