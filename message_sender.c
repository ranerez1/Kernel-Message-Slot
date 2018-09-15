// Add includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <errno.h>

#define MAJOR_NUM 244
#define MSG_SLOT_CHANNEL 1
#define BUF_LEN 128



int validateInput(int argc,char **argv, int validatedArgs[]){
	int fd;
	int channelID = -1;
	int len = -1;
	char* fp = NULL;
	fp = argv[1];
	if(argc!= 4){ 
		fprintf(stderr, "Error: Number of arguments is invalid.\n"); 
		return -1;
	}
	fd = open(fp,O_RDWR);
	if(fd == -1){
		//print to stderr 
		fprintf(stderr, "Error: Failed to open file. %s\n", strerror( errno )); 
		return -1;
	}
	else{
		validatedArgs[0] = fd;
	}
	channelID = atoi(argv[2]);
	if(channelID <= 0){
		fprintf(stderr, "Error: Failed to convert atoi to channel id. %s\n", strerror( errno )); 
		return -1;
	}
	else{
		validatedArgs[1] = channelID;
	}
	len = strlen(argv[3]);
	if(len < 0 || len > 128){
		fprintf(stderr, "Error: Length argument is invalid.\n"); 

		return -1;
	}
	validatedArgs[2] = len;

	return 0;


}


int myioctlset(int validatedArgs[], int command){
	int res = 0;
	res = ioctl(validatedArgs[0],command, validatedArgs[1]);
	if (res <0){
		fprintf(stderr, "Error: ioctl failed. %s\n", strerror( errno )); 
		return -1;
	}
	return 0;
}

int main(int argc, char **argv)
{
	int res = 0;
	char userMessage[BUF_LEN];
	int validatedArgs[3]={-1};
	res = validateInput(argc,argv,validatedArgs);
	if(res < 0){
		return -1;
	}
	strcpy(userMessage,argv[3]);

	res = myioctlset(validatedArgs,MSG_SLOT_CHANNEL);
	if(res < 0){
		return -1;
	}

	res = write(validatedArgs[0],userMessage,validatedArgs[2]);
	if(res < 0){
		fprintf(stderr, "Error: Failed to write messages. %s\n", strerror( errno )); 
		return -1;
	}

	res = close(validatedArgs[0]);
		if(res < 0){
		fprintf(stderr, "Error: Failed to close file. %s\n", strerror( errno )); 
		return -1;
	}
	printf("MESSAGE_SENDER Has completed Successfuly.\n");
	return 0;
}
