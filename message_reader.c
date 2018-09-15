

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
	int fd = -1;
	unsigned int channelID = -1;
	if(argc!= 3){
		//print to stderr
		fprintf(stderr, "Error: Number of args different than 3.\n"); 
		return -1;
	}
	fd = open(argv[1],O_RDONLY);
	if(fd == -1){
		fprintf(stderr, "Error: Failed to open file. %s\n",strerror(errno)); 
		return -1;
	}
	else{
		validatedArgs[0] = fd;
	}
	channelID = atoi(argv[2]);
	if(channelID <= 0){
		//print to stderr 
		fprintf(stderr, "Error: atoi can't convert argument to channel ID.\n"); 
		return -1;
	}
	else{
		validatedArgs[1] = channelID;
	}
	return 0;


}


int myioctlset(int validatedArgs[], int command){
	int res = 0;
	res = ioctl(validatedArgs[0],command, validatedArgs[1]);
	if (res <0){
		fprintf(stderr, "Error: in function ioctl. %s\n",strerror(errno)); 
		return -1;
	}
	return 0;
}

int main(int argc, char **argv)
{
	int res = 0;
	char userMessage[BUF_LEN];
	int validatedArgs[2]={-1};
	res = validateInput(argc,argv,validatedArgs);
	if(res < 0){
		return -1;
	}

	res = myioctlset(validatedArgs,MSG_SLOT_CHANNEL);
	if(res < 0){
		return -1;
	}

	res = read(validatedArgs[0],userMessage,BUF_LEN);
	if(res < 0){
		fprintf(stderr, "Error: Failed to read message. %s\n", strerror(errno));
		return -1;
	}



	res = close(validatedArgs[0]);
		if(res < 0){
		//print error to stderr
		fprintf(stderr, "Error: Failed to close file. %s\n", strerror(errno));
		return -1;
	}
	printf("MESSAGE_READER has completed successfuly. The message is: %s\n" ,userMessage);
	return 0;
}
