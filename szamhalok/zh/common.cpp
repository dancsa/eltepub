#include "common.h"
#include <iostream>
#include <unistd.h>
#include <cstdlib>
#include <string.h>
#include <errno.h>
int assertSuccess(const char * msg, int rv){
	if(rv < 0){
		std::cerr<<"Error! "<< msg << " errno: " << strerror(errno)<<std::endl;
		std::exit(2);
	}
	return(rv);
}

void sendCard(struct client_t & client){
	int card = (rand() % 10)+1;
	std::cout<<"Sending card "<<card<<std::endl;
	client.cards[getCardEndIdx(client.cards)] = card;
	write(client.fd, client.cards, sizeof(client.cards));
}

int sumCards(int * a){
	int sum=0;
	while( 0 != *a){
		sum+=*a;
		++a;
	}
	return sum;
}

int getCardEndIdx(int * a){
	int * b=a;
	while(*b != 0) b++;
	return b-a;
	
}
