#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>

#define BUFFER_SIZE 128
#define SHR_SIZE    50

void dieWithError(char *msg)
{
	printf("[-]ERROR: %s\n");
	exit(0);
}

int main(void)
{
	key_t key;
	int fd, shmid, bufLen, id = 1234;
	char *shm, buffer[BUFFER_SIZE], fifoBuffer[BUFFER_SIZE];
	time_t rawtime;
	struct tm* timeinfo;

	if ((fd = open("sharFile", O_WRONLY)) < 0)
		dieWithError("open() failed");
	
	//getting the key from a FIFO file that was created by server
	if ((key = ftok("sharFile", id)) == (key_t) -1)
		dieWithError("ftok() failed");

	//returns the identifier of the shared memory segment
	if ((shmid = shmget(key, SHR_SIZE, 0)) < 0)
		dieWithError("shmget() failed");

	if ((shm = shmat(shmid, NULL, 0)) == (char*)-1)
		dieWithError("shmat() failed");
	printf("----------------------------------\n");
	printf("Reading from shared memory:\n");
	printf("----------------------------------\n");
	printf("PID\tKey\t\tTime\n");
	for(;;)
	{
		//copy the information from the shared memory into a buffer than print the buffer
		bzero(&buffer, BUFFER_SIZE);
		memcpy(buffer, shm, strlen(shm));
		printf("%s", buffer);

		//writing information into a FIFO file
		bzero(&fifoBuffer, BUFFER_SIZE);
		time(&rawtime);
		timeinfo = localtime(&rawtime);
		sprintf(fifoBuffer, "[%x]\t%d\t%d:%d:%d\n", getpid(), key, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
		bufLen = strlen(fifoBuffer);
		if (write(fd, fifoBuffer, bufLen) < 0)
			dieWithError("write() failed");	

		sleep(1);
	}
	return 0;
}
