#include <iostream>
#include <cstdlib>
#include <stdint.h>

#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

using std::cout;
using std::cin;
using std::endl;

int assertSuccess(const char * msg, int rv);

int main(void){

	int server = assertSuccess("server socket creation", socket(AF_INET, SOCK_STREAM, 0));
    struct sockaddr_in server_name;
	server_name.sin_family = AF_INET;
	server_name.sin_addr.s_addr=inet_addr("127.0.0.1"); // INADDR_LOOPBACK;
	//INADDR_LOOPBACK is equalent to  inet_addr(127.0.0.1), so why bother with conversion
	server_name.sin_port = htons(10000);

	assertSuccess("connect", connect(server, (const struct sockaddr *) &server_name, sizeof(server_name)));
	//now we have connection with the server via `server` filedescriptor... we can talk to it

	int32_t message=0;
	do{
		cout<<"What number shall we send to the server? -1 to exit." <<endl;
		cin>>message;
		write(server, &message, sizeof(message));
		read(server, &message, sizeof(message));
		cout<<"server replied with " << message<<endl;
	}while( 0 != message);

	shutdown(server, SHUT_RDWR);		
	close(server);
}


int assertSuccess(const char * msg, int rv){
	if(rv < 0){
		std::cerr<<"Error! "<< msg << " errno: " << strerror(errno)<<std::endl;
		std::exit(2);
	}
	return(rv);
}

