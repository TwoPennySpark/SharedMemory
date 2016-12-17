#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>

#define BUFFER_SIZE 128
#define SHR_SIZE    50

void dieWithError(char *msg)
{
	printf("[-]ERROR: %s\n");
	exit(0);
}

int main (void)
{
	key_t key;
	int fd, shmid, id, bufLen, buffer_written; 
	char *shm, buffer[BUFFER_SIZE], fifoBuffer[BUFFER_SIZE];
	time_t rawtime;
	struct tm* timeinfo;

	id = 1234;

	//create FIFO
	if ((mkfifo("sharFile" , 0744)) < 0)
		dieWithError("mkfifo() failed");
	
	if ((fd = open("sharFile", O_RDONLY)) < 0)
		dieWithError("open() failed");
	
	if ((key = ftok("sharFile", id)) == (key_t)-1)
		dieWithError("ftok() failed"); 

	//create the segment
 	if ((shmid = shmget(key, SHR_SIZE, IPC_CREAT | IPC_EXCL | 0666)) < 0)
		dieWithError("shmid() failed");

	/*attaches the shared memory segment identifier by shmid to the address 
	returns the address of the attached shared memory segment*/
	if ((shm = shmat(shmid, NULL, 0)) == (char*)-1)
		dieWithError("shmat() failed");
	
	printf("----------------------------------\n");
	printf("Reading from FIFO:\n");
	printf("----------------------------------\n");
	printf("PID\tKey\t\tTime\n");
	for (;;)
	{
		//copy the information into the shared memory
		bzero(&buffer, BUFFER_SIZE);
		time(&rawtime);
		timeinfo = localtime(&rawtime);
		sprintf(buffer, "[%x]\t%d\t%d:%d:%d\n", getpid(), key, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
		bufLen = strlen(buffer);
		memcpy(shm, buffer, bufLen);
 
		//read information from a FIFO
		bzero(&fifoBuffer, BUFFER_SIZE);
		if (read(fd, fifoBuffer, BUFFER_SIZE) < 0)
			dieWithError("read() failed");	
		printf("%s", fifoBuffer);

		sleep(1);
	}

	return 0;
}
