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

	int server = assertSuccess("server socket creation", socket(AF_INET, SOCK_STREAM, 0));
    struct sockaddr_in server_name;
	server_name.sin_family = AF_INET;
	server_name.sin_addr.s_addr= htonl(INADDR_LOOPBACK); //inet_addr("127.0.0.1"); //INADDR_LOOPBACK;
	//INADDR_LOOPBACK is equalent to  inet_addr(127.0.0.1), so why bother with conversion
	//INADDR_ANY could be used to listen to all interface (usually marked with 0.0.0.0 ip address)
	server_name.sin_port = htons(10000);

	//the following 2 line will configure the socket to  enable reusing it just after closing
	//the default is that the kernel reserve the port for a time even after quit.
	const int one = 1;
	assertSuccess("setsockopts", setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(int)));
	assertSuccess("bind", bind(server, (const struct sockaddr *) &server_name, sizeof(server_name)));

	//after binding, the kernel still doesn't accept connection to that port. We need to enable it, specifying how many connection can wait
	assertSuccess("listen", listen(server, 4)); //listen's 2nd argument is the blacklog size, currently decieded by http://xkcd.com/221
	//the blacklog size doesn't affect how many connection can be estabilished at a given time, rather how many connection can waiting
	//in the accept's queue.

	while(1){
		int client = accept(server, NULL, NULL); //If we don't care about the client's address, use NULL as 2nd, 3rd arguments
		if( -1 == client && errno == EINTR){
			continue; //EINTR is not error, simply a signal interrupted us
		}
		assertSuccess("accept", client);
		
		//now we have connection with the client via `client` filedescriptor... we can talk to it
		
		//in this case, the client will send us a nonnegative number, and we will reply with number+1, client can disconnect by sending us -1

		int32_t message=-1;
		while( 0 != message){
			read(client, &message, sizeof(message)); //this could be improoved with error handling, also note, that longer read, write can be interrupted which 
			//cause lesser bytes returned, or it hasn't read anything, return with -1 and EINTR errno
			++message;
			write(client, &message, sizeof(message));
		}
		shutdown(client, SHUT_RDWR);
		close(client);
	}

}


int assertSuccess(const char * msg, int rv){
	if(rv < 0){
		std::cerr<<"Error! "<< msg << " errno: " << strerror(errno)<<std::endl;
		std::exit(2);
	}
	return(rv);
}

