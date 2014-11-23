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
    struct sockaddr_in client_name;
	client_name.sin_family = AF_INET;
	client_name.sin_addr.s_addr= htonl(INADDR_LOOPBACK); 
	client_name.sin_port = htons(0); //binding to port zero causes the kernel to choose one from the free ports for us

	assertSuccess("bind", bind(client, (const struct sockaddr *) &client_name, sizeof(client_name)));

	struct sockaddr_in server_name;
	server_name.sin_family = AF_INET;
	server_name.sin_addr.s_addr= htonl(INADDR_LOOPBACK); 
	server_name.sin_port = htons(10000); 

	int32_t message;
	while(1){
		std::cout<<"What number shall we send to the server?" <<std::endl;
		std::cin>>message;
		sendto(client, &message, sizeof(message), 0, (struct sockaddr *) &server_name, sizeof(server_name) );
		recvfrom(client, &message, sizeof(message), 0, NULL, NULL );
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

