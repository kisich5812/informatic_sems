/* Программа, иллюстрирующая использование системных вызовов open(), read() и close() для чтения информации из файла */

#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
int fd;
ssize_t size;
char string[60];

/* Попытаемся открыть файл с именем в первом параметре выззова только
для операций чтения */

if (argc < 2) return -printf("Введите имя файла\n");

if((fd = open(argv[1], O_RDONLY)) < 0)
{

  /* Если файл открыть не удалось, печатаем об этом сообщение и прекращаем работу */
  printf("Can\'t open file\n");
  exit(-1);
}

int fd_out = open("copy.txt", O_WRONLY | O_CREAT | O_TRUNC, 0666);
if (fd_out < 0) {
    printf("Can\'t open output file\n");
    close(fd);
    exit(-1);
}

/* Читаем фаил пока не кончится и печатаем */
while ((size = read(fd, string, 59)) > 0) {
	string[size] = '\0';
	printf("%s", string); /* Печатаем прочитанное*/
	write(fd_out, string, size); /*  Записываем файл под новым именем */
}

/* Закрываем файл */
if(close(fd) < 0)
{
  printf("Can\'t close file\n");
}
/*  Открываем файл в редакторе */
system("vim copy.txt");

return 0;
} 
