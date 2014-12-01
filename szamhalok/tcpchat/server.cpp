#include <iostream>
#include <cstdlib>
#include <stdint.h>
#include <vector>

#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "common.h"

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

	struct timeval zerotime = {0,0};
	fd_set acceptset;
	fd_set readset;
	std::vector<int> clients;
	std::vector<int>::const_iterator cit;
	std::vector<int>::const_iterator cit2;
	int client;
	int max;
	char *msg;

	while(1){

		FD_SET(server, &acceptset);
		if( clients.empty() ){
			select(server+1, &acceptset, NULL, NULL, NULL); //If we don't have client, we'll wait till we have somebody
		}else{
			select(server+1, &acceptset, NULL, NULL, &zerotime); //else if nobody waits for connection,  return immediately
		}
		if( FD_ISSET(server, &acceptset) ){
			client = accept(server, NULL, NULL);
			//TODO: error handling
			if ( client > 0 ) clients.push_back(client);
		}
		
		if ( clients.empty()) continue; //still possible to have no clients

		FD_ZERO(&readset);
		for( cit = clients.begin(), max=0; cit != clients.end(); cit++){
			if( max < *cit ) max = *cit;
			FD_SET(*cit, &readset);
		}
		select(max+1, &readset, NULL, NULL, NULL);
	
		for( cit = clients.begin(); cit != clients.end(); cit++){
			if ( !FD_ISSET( *cit, &readset) ) continue;
			msg = doreceivemsg(*cit);
			for( cit2 = clients.begin(); cit2 != clients.end(); cit2++){
				dosendmsg(*cit2, msg);
			}
			delete[] msg;
			msg=NULL;
		}
	}

}

int assertSuccess(const char * msg, int rv){
	if(rv < 0){
		std::cerr<<"Error! "<< msg << " errno: " << strerror(errno)<<std::endl;
		std::exit(2);
	}
	return(rv);
}

