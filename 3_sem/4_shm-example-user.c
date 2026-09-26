/* разобраться как работает, написать комментарии,
   в том числе ко всем параметрам. */
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>

int main (int argc, char ** argv)
{
  int shm_id;
  char * shm_buf;

  // проверяем, что индентификатор памяти передан
  if (argc < 2) 
  {
  	fprintf (stderr, "Too few arguments\n");
  	return 1;
  }
  
  shm_id = atoi(argv[1]);	// преобразуем строку argv[1] в число. shm_id - указатель на сегмент памяти
  shm_buf = (char *) shmat (shm_id, 0, 0); // подключаем сегмент в адресное пространство процесса,
					   // 0 - выбор адреса осуществляет ОС,
					   // 0 - права доступа по умолчанию
  if (shm_buf == (char *) -1) 
  {
  	fprintf (stderr, "shmat() error\n");
  	return 1;
  }
  
  printf ("Message: %s\n", shm_buf);
  shmdt (shm_buf);	// отделить память от текущего процесса
  
  return 0;
}
