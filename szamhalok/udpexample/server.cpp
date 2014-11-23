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
//ssize_t assertRead(const char * msg, int fd, const void * buffer, size_t count);

int main(void){

	int server = assertSuccess("server socket creation", socket(AF_INET, SOCK_DGRAM, 0));
    struct sockaddr_in server_name;
	server_name.sin_family = AF_INET;
	server_name.sin_addr.s_addr= htonl(INADDR_LOOPBACK); //inet_addr("127.0.0.1"); //INADDR_LOOPBACK;
	//INADDR_LOOPBACK is equalent to  inet_addr(127.0.0.1), so why bother with conversion
	//INADDR_ANY could be used to listen to all interface (usually marked with 0.0.0.0 ip address)
	server_name.sin_port = htons(10000);

	const int one = 1;
	assertSuccess("setsockopts", setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(int)));
	assertSuccess("bind", bind(server, (const struct sockaddr *) &server_name, sizeof(server_name)));
	struct sockaddr_in clientaddr;
	int32_t message;
	socklen_t addrlength;
	while(1){
		addrlength = sizeof(clientaddr);
		recvfrom(server, &message, sizeof(message), 0, (struct sockaddr *) &clientaddr, &addrlength );
		++message;
		sendto(server, &message, sizeof(message), 0, (struct sockaddr *) &clientaddr, sizeof(clientaddr) );
	}

}


int assertSuccess(const char * msg, int rv){
	if(rv < 0){
		std::cerr<<"Error! "<< msg << " errno: " << strerror(errno)<<std::endl;
		std::exit(2);
	}
	return(rv);
}

