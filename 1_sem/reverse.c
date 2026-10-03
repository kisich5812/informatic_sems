#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct word {
	int word_num;
	int word_ptr;
} Word;

int main(int argc, char* argv[]) {
	if (argc < 2) return -fprintf(stderr, "Укажите номер слова!");
	int num_word = atoi(argv[1]);

	char buf[100] = {0};
	Word words[100];
	int read_idx = 0;
	int word_count = 0;
	int flag_escape = 1;
	int flag_intr = 0;
	while (1) {
		buf[read_idx] = getchar();
		if (buf[read_idx] != '!' && flag_intr) flag_intr = 0;
		if (isspace(buf[read_idx])) {
			flag_escape = 1;
		}
		if (isalpha(buf[read_idx]) && flag_escape) {
			flag_escape = 0;
			words[word_count].word_num = word_count + 1;
			words[word_count].word_ptr = read_idx;
			word_count++;
		}
		if (buf[read_idx] == '!') {
			if (flag_intr) {
				buf[read_idx - 1] = '\0';
				flag_intr = 0;
				break;
			}
			flag_intr = 1;
		}
		read_idx++;
	}

	if (num_word > word_count) {
		fprintf(stderr, "Недостаточно слов введено\n");
		return 0;
	}

	for (int write_idx = word_count - 1; write_idx >= 0; write_idx--) {
		if (words[write_idx].word_num == num_word) {
			int idx = 0;
			while (!isspace(buf[words[write_idx].word_ptr + idx]) &&
					buf[words[write_idx].word_ptr + idx] != '\0') idx++;
			idx--;
			for(; idx >= 0; idx--) {
				printf("%c", buf[words[write_idx].word_ptr + idx]);
			}
			printf("%c", ' ');
			continue;
		}
		for (int idx = 0; ; idx++) {
			if (isspace(buf[words[write_idx].word_ptr + idx]) ||
					(buf[words[write_idx].word_ptr + idx]) == '\0') {
				break;
			}
			printf("%c", buf[words[write_idx].word_ptr + idx]);
		}
		printf("%c", ' ');
	}

	return 0;
}
