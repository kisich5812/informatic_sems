/* Проверить совместную работу с 4.
   Написать комментарии, В ТОМ ЧИСЛЕ К ПАРАМЕТРАМ!*/
#include <stdio.h>
#include <string.h>
#include <sys/shm.h>

#define SHMEM_SIZE	4096
#define SH_MESSAGE	"Poglad Kota!\n"

int main (void)
{
  int shm_id;
  char * shm_buf;
  int shm_size;
  struct shmid_ds ds;
  
  shm_id = shmget (IPC_PRIVATE,		// запрос выделения shared mem, IPC_PRIVATE - ключ для доступа к файлу
                   SHMEM_SIZE,		// размер выделяемой памяти
  		           IPC_CREAT | IPC_EXCL | 0600);	// IPC_CREATE - создать shared mem с таким ключом,
								// IPC_EXCL - выдать ошибку, если память уже выделена,
								// 0600 - разрешить чтение и запись владельцу
  
  if (shm_id == -1) 
  {
    fprintf (stderr, "shmget() error\n");
    return 1;
  }
  
  shm_buf = (char *) shmat (shm_id,	// отображаем выделенный сегмент в адресное пространство процесса, shm_id - индентификатор, получен от shmget()
                            NULL,	// указатель на желаемый адрес, NULL - ОС сама выбирает подходящий
                            0);		// 0 - задаёт чтение и запись
  if (shm_buf == (char *)-1)
  {
    fprintf (stderr, "shmat() error\n");
  	return 1;
  }
  
  shmctl (shm_id,	// shm_id - индентификатор сегмента памяти
          IPC_STAT,	// IPC_STAT — команда для считывания служебных данных
          &ds);		// &ds - указатель на структуру для записи данных
  
  shm_size = ds.shm_segsz;	// узнаём размер данныхсегмента памяти
  if (shm_size < strlen (SH_MESSAGE)) 
  {
  	fprintf (stderr, "error: segsize=%d\n", shm_size);
  	return 1;
  }
  
  strcpy (shm_buf,	// копируем сообщение в разделяемую память
          SH_MESSAGE);	
  
  // печатаем id в консоль
  printf ("ID: %d\n", shm_id);
  printf ("Press <Enter> to exit...");	
  fgetc (stdin);
  
  shmdt (shm_buf);	// отделяет shared mem от адресного пространства процесса
  shmctl(shm_id,	// shm_id - индентификатор сегмента памяти
         IPC_RMID,	// IPC_RMID  - команда на удаление
         NULL);		// для удаления не нужна для записи служебных данных
  
  return 0;
}
