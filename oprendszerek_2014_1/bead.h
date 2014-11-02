#ifndef bead__h
#define bead__h

#include <time.h>


struct stadion_t {
int id;
char city[31];
char team[31];
unsigned int maxvis;
long long cost;
time_t date;
};

struct stadion_mod_t {
	int id;
	char mod;
	char newval[31];
};

#endif
