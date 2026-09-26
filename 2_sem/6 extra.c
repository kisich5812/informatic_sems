/* Создать программу, запускаемую на N терминалах, для обмена сообщениями между терминалами через файл.
 Каждый экземпляр программы, запущенный на каждом терминале содержит два процесса:
 один записывает файл, второй читает. Так на двух терминалах работает четыре процесса,
 обеспечивая двусторонний обмен. 
 Идентификатор пользователя (терминала), можно задавать параметром при запуске приложения.
 Предусмотреть хранение информации для отсутствующего получателя.
 Начать разработку со схемы данных. Предусмотреть поля для идентификаторов, отправителя и получателя,
а так же данных.
Разработать протокол обмена и структуру файла.
Продумать протокол очистки файла.
Исследовать вопрос с уникальностью идентификаторов.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <sys/file.h>
#include <sys/wait.h>

#define FILENAME "chat.bin"
#define MSG_SIZE 256

/* Структура сообщения */
struct Message {
	unsigned int id;	/* Уникальный ID сообщения для отслеживания прочитанных */
	pid_t sender;		/* PID отправителя */
	pid_t receiver;		/* PID получателя */
	time_t timestamp;	/* Время отправки */
	char text[MSG_SIZE];	/* Текст сообщения */
};

/* --- РАБОТА С ФАЙЛОМ (Дочерний процесс) --- */

/* Запись сообщения в конец файла chat.bin */
static void append_message_to_file(const struct Message *msg) {
	int fd = open(FILENAME, O_WRONLY | O_CREAT | O_APPEND, 0666);
	if (fd < 0) return;

	/* Ждем освобождения файла, если он занят, и блокируем */
	flock(fd, LOCK_EX);

	write(fd, msg, sizeof(*msg));

	flock(fd, LOCK_UN);
	close(fd);
}

/* Вычитывание новых сообщений из файла по id */
static void read_new_messages_from_file(pid_t my_pid, int *last_read_id) {
	int fd = open(FILENAME, O_RDONLY);
	if (fd < 0) return;

	/* Блокировка на чтение */
	flock(fd, LOCK_SH);

	struct Message msg;
	while (read(fd, &msg, sizeof(msg)) > 0) {
		//printf("DEBUG: %u\n%d\n", msg.id, msg.receiver);
		//if (msg.receiver == my_pid) printf("message for me!\n");
		if (msg.id > *last_read_id) {
			*last_read_id = msg.id; /* Обновляем последний увиденный ID */

			/* Печатаем только если сообщение адресовано нам */
			if (msg.receiver == my_pid) {
				printf("\n[Сообщение от PID %d]: %s\n> ", msg.sender, msg.text);
				fflush(stdout);
			}
		}
	}

	flock(fd, LOCK_UN);
	close(fd);
}

/* Цикл дочернего процесса (Обработка файла и вывода) */
static void run_child(int read_pipe_fd, pid_t parent_pid) {
	unsigned int last_read_id = 0;

	/* Переводим pipe в неблокирующий режим */
	fcntl(read_pipe_fd, F_SETFL, O_NONBLOCK);

	while (1) {
		struct Message msg;

		/* 1. Запись данных от родителя в chat.bin */
		if (read(read_pipe_fd, &msg, sizeof(msg)) > 0) {
			append_message_to_file(&msg);
		}

		/* 2. Чтение новых сообщений из chat.bin */
		read_new_messages_from_file(parent_pid, &last_read_id);

		usleep(100000); /* Пауза 0.1 сек (чтобы не нагружать CPU) */
	}
}

/* --- ВВОД С КЛАВИАТУРЫ (Родительский процесс) --- */

/* Чтение одной строки с клавиатуры */
static int get_user_input(pid_t parent_pid, struct Message *msg, int *msg_counter) {
	pid_t target_pid;
	char text_buf[MSG_SIZE];

	printf("> ");
	fflush(stdout);

	if (scanf("%d", &target_pid) != 1) {
		return 0;
	}

	fgets(text_buf, sizeof(text_buf), stdin);
	text_buf[strcspn(text_buf, "\n")] = '\0'; /* Удаляем символ перевода строки */

	/* Пропускаем пробелы перед текстом */
	char *clean_text = text_buf;
	while (*clean_text == ' ') clean_text++;

	/* Заполняем структуру */
	msg->id = (int)time(NULL) * 1000 + (*msg_counter)++;
	msg->sender = parent_pid;
	msg->receiver = target_pid;
	msg->timestamp = time(NULL);
	strncpy(msg->text, clean_text, MSG_SIZE - 1);
	msg->text[MSG_SIZE - 1] = '\0';

	return 1;
}

/* Цикл родительского процесса */
static void run_parent(int write_pipe_fd, pid_t parent_pid) {
	int msg_counter = 1;
	struct Message msg;

	printf("=== Чат запущен (Ваш PID: %d) ===\n", parent_pid);
	printf("Формат ввода: <PID_получателя> <текст_сообщения>\n");

	while (get_user_input(parent_pid, &msg, &msg_counter)) {
		write(write_pipe_fd, &msg, sizeof(msg));
	}
}

/* --- MAIN --- */

int main(void) {
	int pipefd[2];

	if (pipe(pipefd) < 0) {
		perror("Ошибка pipe");
		return 1;
	}

	pid_t parent_pid = getpid();
	pid_t child_pid = fork();

	if (child_pid < 0) {
		perror("Ошибка fork");
		return 1;
	}

	if (child_pid == 0) {
		/* Дочерний процесс: Работа с файлом */
		close(pipefd[1]); /* Закрываем неиспользуемый конец pipe */
		run_child(pipefd[0], parent_pid);
		close(pipefd[0]);
		_exit(0);
	} else {
		/* Родительский процесс: Чтение с клавиатуры */
		close(pipefd[0]); /* Закрываем неиспользуемый конец pipe */
		run_parent(pipefd[1], parent_pid);
		close(pipefd[1]);
		wait(NULL);
	}

	return 0;
}
