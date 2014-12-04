#include <iostream>
#include <cstdlib>
#include <stdint.h>

#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "common.h"


using std::cout;
using std::cin;
using std::endl;


void printCards(int *a);
int main(void){

	int server = assertSuccess("server socket creation", socket(AF_INET, SOCK_STREAM, 0));
    struct sockaddr_in server_name;
	server_name.sin_family = AF_INET;
	server_name.sin_addr.s_addr=inet_addr("127.0.0.1"); // INADDR_LOOPBACK;
	server_name.sin_port = htons(10000);

	assertSuccess("connect", connect(server, (const struct sockaddr *) &server_name, sizeof(server_name)));

	int cards[22];	
	
	read(server, &cards, sizeof(cards));
	printCards(cards);

	read(server, &cards, sizeof(cards));
	printCards(cards);
	char buff[255];
	ssize_t readbytes;

	int choice;
	while(1){
		cout<<"Megall (0) tovabbmegy (1) ?" <<endl;
		cin>>choice;
		write(server, &choice, sizeof(choice));
		
		if(0 == choice){
			readbytes = read(server, buff, sizeof(buff));
			buff[readbytes]='\0';
			cout<<"server response was: "<< buff<<endl;
			return 0;
		}else{
			read(server, &cards, sizeof(cards));
			printCards(cards);
			if( 21 < sumCards(cards) ){
				readbytes = read(server, buff, sizeof(buff));
				buff[readbytes]='\0';
				cout<<"server response was: "<< buff<<endl;
				return 0;
			}
		}
	}

	shutdown(server, SHUT_RDWR);		
	close(server);
}


void printCards(int *a){
	while (0 != *a)	cout << *(a++)<<" ";
	cout<<endl;
}
