/* 
  Создать программу(программы) для последовательного вычетания 1 двумя процессами
  из некоторого числа, записанного в разделяемой памяти.
  Очерёдность работы процессов регулируем при помощи семафора.
  Ход выполнения следующий:
  Открываем два терминала и запускаем в них созданную(ые) программы
  В одном из них вводим некоторое число.
  После чего обе программы начинают выводить по очереди уменьшенное на 1,
  полученное значение.
  Предусмотреть задержки для наглядности.

*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

#define SHM_KEY 3000
#define SEM_KEY 3000

union semun {
	int val;
	struct semid_ds *buf;
	unsigned short *array;
};

// Функция выполнения операции над семафором
void sem_change(int sem_id, short sem_num, short op) {
	struct sembuf sb = { .sem_num = sem_num, .sem_op = op, .sem_flg = 0 };
	semop(sem_id, &sb, 1);
}

// Создание или подключение к разделяемой памяти и семафорам
int init_ipc(int *shm_id, int *sem_id, int **shm_ptr, int *is_master) {
	*shm_id = shmget(SHM_KEY, sizeof(int), 0666 | IPC_CREAT | IPC_EXCL);

	if (*shm_id >= 0) {
		*is_master = 1;
		*sem_id = semget(SEM_KEY, 2, 0666 | IPC_CREAT | IPC_EXCL);
	} else if (errno == EEXIST) {
		*is_master = 0;
		*shm_id = shmget(SHM_KEY, sizeof(int), 0666);
		*sem_id = semget(SEM_KEY, 2, 0666);
	} else {
		return -1;
	}

	if (*sem_id < 0) return -1;

	*shm_ptr = (int *)shmat(*shm_id, NULL, 0);
	return (*shm_ptr == (int *)-1) ? -1 : 0;
}

// Настройка начальных данных Главным процессом
void setup_master(int *shm_ptr, int sem_id, int argc, char *argv[]) {
	int start_val = 10; // Значение по умолчанию

	if (argc > 1) {
		start_val = atoi(argv[1]);
	} else {
		printf("[Главный] Число не передано как аргумент командной строки. Введите стартовое число: ");
		if (scanf("%d", &start_val) != 1) start_val = 10;
	}

	*shm_ptr = start_val;
	union semun arg;
	arg.val = 1; semctl(sem_id, 0, SETVAL, arg);
	arg.val = 0; semctl(sem_id, 1, SETVAL, arg);

	printf("[Главный] Стартовое число: %d. Ждем Ведомого...\n", start_val);
}

// Игровой цикл
void play_game(int sem_id, int *shm_ptr, int is_master) {
	int my_sem   = is_master ? 0 : 1;
	int next_sem = is_master ? 1 : 0;
	const char *role_name = is_master ? "Главный" : "Ведомый";

	while (1) {
		// Ждем своей очереди
		sem_change(sem_id, my_sem, -1);

		if (*shm_ptr <= 0) {
			sem_change(sem_id, next_sem, 1);
			break;
		}

		(*shm_ptr)--;
		printf(" -> [%s]: Счётчик = %d\n", role_name, *shm_ptr);
		usleep(250000);
		sem_change(sem_id, next_sem, 1);

		if (*shm_ptr <= 0) break;
	}
	printf("[%s] Завершил игру.\n", role_name);
}

// Очистка ресурсов
void cleanup_ipc(int shm_id, int sem_id, int *shm_ptr, int is_master) {
	shmdt(shm_ptr);
	if (is_master) {
		sleep(1); // Даем время ведомому отключиться
		shmctl(shm_id, IPC_RMID, NULL);
		semctl(sem_id, 0, IPC_RMID);
	}
}

int main(int argc, char *argv[]) {
	int shm_id, sem_id, is_master;
	int *shm_ptr;

	if (init_ipc(&shm_id, &sem_id, &shm_ptr, &is_master) < 0) {
		perror("Ошибка инициализации IPC");
		return 1;
	}

	if (is_master) {
		setup_master(shm_ptr, sem_id, argc, argv);
	} else {
		printf("[Ведомый] Подключился к игре!\n");
	}

	play_game(sem_id, shm_ptr, is_master);
	cleanup_ipc(shm_id, sem_id, shm_ptr, is_master);

	return 0;
}
