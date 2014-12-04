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
#include <time.h>

#include "common.h"

int main(void){

	srand(time(NULL));
	int server = assertSuccess("server socket creation", socket(AF_INET, SOCK_STREAM, 0));
    struct sockaddr_in server_name;
	server_name.sin_family = AF_INET;
	server_name.sin_addr.s_addr= htonl(INADDR_LOOPBACK); //inet_addr("127.0.0.1"); //INADDR_LOOPBACK;
	server_name.sin_port = htons(10000);

	const int one = 1;
	assertSuccess("setsockopts", setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(int)));
	assertSuccess("bind", bind(server, (const struct sockaddr *) &server_name, sizeof(server_name)));

	assertSuccess("listen", listen(server, 4)); //listen's 2nd argument is the blacklog size, currently decieded by http://xkcd.com/221

	struct timeval zerotime = {0,0};
	fd_set acceptset;
	fd_set readset;
	std::vector<client_t> clients;
	std::vector<client_t>::iterator cit,cit2;
	struct client_t client;
	int max;
	int choice;
	int sum;
	const char * nyert = "nyert";
	const char * vesztett = "vesztett";

	while(1){
		
		memset(&client, 0, sizeof(client));
		FD_SET(server, &acceptset);
		if( clients.empty() ){
			select(server+1, &acceptset, NULL, NULL, NULL); //If we don't have client, we'll wait till we have somebody
		}else{
			select(server+1, &acceptset, NULL, NULL, &zerotime); //else if nobody waits for connection,  return immediately
		}
		if( FD_ISSET(server, &acceptset) ){
			client.fd = accept(server, NULL, NULL);
			if ( client.fd < 0 ) continue; //error, try again.
			sendCard(client);
			sendCard(client);
			clients.push_back(client);
		}
		
		if ( clients.empty()) continue; //still possible to have no clients


		FD_ZERO(&readset);
		for( cit = clients.begin(), max=0; cit != clients.end(); cit++){
			if( max < cit->fd ) max = cit->fd;
			FD_SET(cit->fd, &readset);
		}

		select(max+1, &readset, NULL, NULL, NULL);
	
		for( cit = clients.begin(); cit != clients.end(); cit++){
			if ( !FD_ISSET( cit->fd, &readset) ) continue;
			read(cit->fd, &choice, sizeof(choice));
			if(0 == choice){ //kliens megall
				std::cout<< cit->fd <<" megall"<<std::endl;
				sum = sumCards(cit->cards);
				write(cit->fd, nyert, strlen(nyert));
				shutdown(cit->fd, SHUT_RDWR);
				close(cit->fd);
				clients.erase(cit);
				break; //cit will be invalid
			}else{ //lapot ker
				std::cout<< cit->fd <<" lapot ker"<<std::endl;
				sendCard(*cit);
				sum = sumCards(cit->cards);
				std::cout<< cit->fd << "sum: " << sum<< std::endl;
				if(sum > 21){
					write(cit->fd, vesztett, strlen(vesztett));
					shutdown(cit->fd, SHUT_RDWR);
					close(cit->fd);
					clients.erase(cit);
					break;
				}
			}
		}
	}

}


