#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <stdio.h>
#include <stdlib.h>

// Написать комментарии, отладить работу
int main(int argc, char *argv[], char *envp[])
{
  int   semid;
  char pathname[]="1_sem.c";
  key_t key;
  struct sembuf mybuf;
  
  key = ftok(pathname, 0); // создаём ключ семафора, исходя из имени файла
  
  if((semid = semget(key, 1, 0666 | IPC_CREAT)) < 0)	// создаём семафор
  {
    printf("Can\'t create semaphore set\n");
    exit(-1);
  }
  
  mybuf.sem_num = 0;	// номер семафора, в нашем случае семафор единственный, поэтому 0
  mybuf.sem_op  = -1;	// прибавление 1 к значению семафора
  mybuf.sem_flg = 0;	// 0 - стандартный режим работы
  
  if(semop(semid, &mybuf, 1) < 0)	// делаем операцию с семафором, с данными объявленными выше
  {
    printf("Can\'t wait for condition\n");
    exit(-1);
  }  
    
  semctl(semid, 0, IPC_RMID);	// удаляем семафор  
  printf("The condition is present\n");
  return 0;
}
