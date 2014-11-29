#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include "common.h"

void dosendmsg(int fd, const char *msg){
	 //FIXME: if mesage is more than 16k this will overflow	
	int16_t msglen = strlen(msg); //note that the nullbyte will not be sent;
	size_t sent = 0;
	write(fd, &msglen, sizeof(msglen));
	while(sent < msglen){
		write(fd, msg+sent, msglen-sent);
	}

}

char * doreceivemsg(int fd){
	ssize_t recieved = 0;
	size_t to_recieve;
	int16_t msgsize;
	read(fd, &msgsize, sizeof(msgsize));	
	to_recieve = msgsize;
	ssize_t a;
	char * msg = new char[msgsize+1];
	while(to_recieve > 0){
		a = read(fd, msg+recieved, to_recieve);
		//TODO: error handling
		to_recieve -= a;
		recieved += a;
	}
	msg[msgsize] = '\0'; //To be sure we are correctly null terminated
	return msg;
}
