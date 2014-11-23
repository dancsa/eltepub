#include <iostream>
#include <cstdlib>
#include <stdint.h>

#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int assertSuccess(const char * msg, int rv);

int main(void){

	int client = assertSuccess("server socket creation", socket(AF_INET, SOCK_DGRAM, 0));

	//What interface and which port will be use:
	struct sockaddr_in client_name;
	client_name.sin_family = AF_INET;
	client_name.sin_addr.s_addr= htonl(INADDR_LOOPBACK); //this is the loopback interface, 127.0.0.1 
	client_name.sin_port = htons(0); //binding to port zero causes the kernel to choose one from the free ports for us

	assertSuccess("bind", bind(client, (const struct sockaddr *) &client_name, sizeof(client_name)));

	//The server's address which we want to talk to
	struct sockaddr_in server_name;
	server_name.sin_family = AF_INET;
	server_name.sin_addr.s_addr= htonl(INADDR_LOOPBACK); 
	server_name.sin_port = htons(10000); 

	int32_t message;
	while(1){
		std::cout<<"What number shall we send to the server?" <<std::endl;
		std::cin>>message;

		//Send the message to the server without connecting to it (because UDP is connection less)
		sendto(client, &message, sizeof(message), 0, (struct sockaddr *) &server_name, sizeof(server_name) );

		recvfrom(client, &message, sizeof(message), 0, NULL, NULL ); //We don't save the address from the recieved datagramm
		//'cause we assume that the server replied to us. An attacker also could spoof the addres in an udp datagramm
		std::cout<<"Server replied with "<< message<<std::endl;
	}

}


int assertSuccess(const char * msg, int rv){
	if(rv < 0){
		std::cerr<<"Error! "<< msg << " errno: " << strerror(errno)<<std::endl;
		std::exit(2);
	}
	return(rv);
}

