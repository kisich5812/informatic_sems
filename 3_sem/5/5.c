#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>

#define SHM_KEY_BASE 0xDEADBABE
#define MAX_MESSAGES 500
#define USERNAME_SIZE 32
#define MSG_SIZE 256

struct Message {
	unsigned int id;		// Номер строки в таблице
	char sender[USERNAME_SIZE];	// Имя отправителя
	char receiver[USERNAME_SIZE];	// Имя получателя ("*" или "all" — всем)
	time_t timestamp;		// Время отправки
	char text[MSG_SIZE];		// Текст сообщения
};

struct SharedChat {
	int msg_count;	// Текущее количество занятых строк
	struct Message messages[MAX_MESSAGES];	// Линейный массив строк */
};

static struct SharedChat* init_shared_memory(int room_number) {
	key_t key = SHM_KEY_BASE + room_number;
	
	int shmid = shmget(key, sizeof(struct SharedChat), IPC_CREAT | 0666);
	if (shmid < 0) {
		perror("Ошибка shmget");
		return NULL;
	}
	
	struct SharedChat *shm = (struct SharedChat *)shmat(shmid, NULL, 0);
	if (shm == (struct SharedChat *)-1) {
		perror("Ошибка shmat");
		return NULL;
	}
	return shm;
}

static void send_message(struct SharedChat *shm, const char *sender, const char *receiver, const char *text) {
	if (!shm) return;
	
	if (shm->msg_count >= MAX_MESSAGES) {
		printf("\n[Система]: Память чата заполнена! (Достигнут лимит %d сообщений)\n> ", MAX_MESSAGES);
		fflush(stdout);
		return;
	}
	
	int current_idx = shm->msg_count;
	struct Message *msg = &shm->messages[current_idx];
	msg->id = current_idx + 1;
	
	strncpy(msg->sender, sender, USERNAME_SIZE - 1);
	msg->sender[USERNAME_SIZE - 1] = '\0';
	strncpy(msg->receiver, receiver, USERNAME_SIZE - 1);
	msg->receiver[USERNAME_SIZE - 1] = '\0';
	strncpy(msg->text, text, MSG_SIZE - 1);
	msg->text[MSG_SIZE - 1] = '\0';
	msg->timestamp = time(NULL);
	shm->msg_count++;
}

static void read_new_messages(struct SharedChat *shm, const char *my_name, int *last_read_index) {
	if (!shm) return;
	while (*last_read_index < shm->msg_count) {
		struct Message *msg = &shm->messages[*last_read_index];
		
		if (strcmp(msg->receiver, my_name) == 0 || 
			strcmp(msg->receiver, "all") == 0 || 
			strcmp(msg->receiver, "*") == 0) 
		{
			if (strcmp(msg->sender, my_name) != 0) {
				struct tm *tm_info = localtime(&msg->timestamp);
				char time_str[10];
				strftime(time_str, sizeof(time_str), "%H:%M:%S", tm_info);
				
				printf("\n[%s] Сообщение от %s: %s\n> ", time_str, msg->sender, msg->text);
				fflush(stdout);
			}
		}
		(*last_read_index)++;
	}
}

/* --- ДОЧЕРНИЙ ПРОЦЕСС --- */
static void run_child(struct SharedChat *shm, const char *my_name) {
	int last_read_index = 0;
	while (1) {
		read_new_messages(shm, my_name, &last_read_index);
		usleep(100000);
	}
}

/* --- РОДИТЕЛЬСКИЙ ПРОЦЕСС --- */
static int get_user_input(char *receiver, char *text_buf, size_t text_size) {
	char line[USERNAME_SIZE + MSG_SIZE + 2];
	printf("> ");
	fflush(stdout);

	if (fgets(line, sizeof(line), stdin) == NULL) return 0;

	line[strcspn(line, "\n")] = '\0';
	if (strlen(line) == 0) return 1;

	char *space_ptr = strchr(line, ' ');
	if (space_ptr == NULL) {
		printf("Ошибка ввода! Формат: <Имя_получателя> <Текст_сообщения>\n");
		printf("Пример личного сообщения: Bob Привет!\n");
		printf("Пример сообщения всем:	* Всем привет!\n");
		return 1;
	}

	*space_ptr = '\0';
	strncpy(receiver, line, USERNAME_SIZE - 1);
	receiver[USERNAME_SIZE - 1] = '\0';

	char *clean_text = space_ptr + 1;
	while (*clean_text == ' ') clean_text++;

	strncpy(text_buf, clean_text, text_size - 1);
	text_buf[text_size - 1] = '\0';

	return 1;
}

static void run_parent(struct SharedChat *shm, const char *my_name) {
	char receiver[USERNAME_SIZE];
	char text_buf[MSG_SIZE];
	
	printf("=== SHM-Chat запущен (Ваш логин: %s) ===\n", my_name);
	printf("Формат отправки: <Имя_получателя> <Текст>\n");
	printf("Отправка всем:   * <Текст>\n----------------------------------------\n");
	
	while (get_user_input(receiver, text_buf, sizeof(text_buf))) {
		if (strlen(text_buf) > 0) {
			send_message(shm, my_name, receiver, text_buf);
		}
	}
}

int main(int argc, char **argv) {
	if (argc < 2) {
		fprintf(stderr, "Использование: %s <ваше_имя> [номер_комнаты]\n", argv[0]);
		fprintf(stderr, "Пример: %s Alice 1\n", argv[0]);
		return 1;
	}
	
	char *my_name = argv[1];
	int room_number = (argc >= 3) ? atoi(argv[2]) : 0;
	
	struct SharedChat *shm = init_shared_memory(room_number);
	if (!shm) return 1;
	
	pid_t child_pid = fork();
	if (child_pid < 0) {
		perror("Ошибка fork");
		return 1;
	}
	
	if (child_pid == 0) {
		run_child(shm, my_name);
		_exit(0);
	} else {
		run_parent(shm, my_name);
		kill(child_pid, SIGTERM);
		wait(NULL);
	}
	
	return 0;
}
