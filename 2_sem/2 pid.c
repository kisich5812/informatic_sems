/* Пример программы печатающей значения PPID и PID для текущего процесса */

#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
 
void main()
{
pid_t pid, ppid;
 
pid = getpid();
ppid = getppid();
 
printf("My pid = %d, my ppid = %d\n", pid, ppid);
}
