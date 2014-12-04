#ifndef COMMON_H_INCLUDED_
#define COMMON_H_INCLUDED_

struct client_t {
	int fd;
	int cards[22]; //fix array for the cards, last card is 0;
};


int assertSuccess(const char * msg, int rv);
int sumCards(int * a);
int getCardEndIdx(int * a);
void sendCard(struct client_t & client);

#endif //include guard
