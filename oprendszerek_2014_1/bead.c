#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <signal.h>

#include "bead.h"



void db_list();
void list();
void newentry();
int db_getmaxid();
void mod();
void search();
int does_it_match(const char * hay, const char * needle);

void dbchild();
void db_newentry();
void dbchild_handler(int);
void create_pipes();
void db_mod();
void slave_mod();
void copystring_if_not_empty(char *, const char*);
void slave_newentry();
void slavechild();
void db_delete();
void slave_delete();
void delete();

void cleanup();
void fail_with_error(const char * msg, int rv);
void slavechild();
void slave_search();
void db_search();
void logging(const char * );
void main_handler(int);

int fd = 0;
int db_to_parent = 0;
int parent_to_slave = 0;
int slave_to_db = 0;
pid_t dbchildpid;
pid_t slavechildpid;
FILE * logfile;


int main(void) {
	char ch;
	create_pipes();


	dbchildpid = fork();
	if( dbchildpid == 0  ){
		dbchild();
		exit(0);
	}else if( dbchildpid < 0 ){
		printf("Hiba tortent forkkor.%i", errno);
		exit(1);
	}

	slavechildpid = fork();
	if( slavechildpid == 0  ){
		slavechild();
		exit(0);
	}else if( slavechildpid < 0 ){
		printf("Hiba tortent forkkor.%i", errno);
		exit(1);
	}
	
	signal(SIGINT, main_handler);
	signal(SIGTERM,  main_handler);


	printf("slavechild: %i\n", slavechildpid);
	printf("dbchild: %i\n", dbchildpid);

	db_to_parent = open("/tmp/db-to-parent", O_RDONLY);
	parent_to_slave = open("/tmp/parent-to-slave", O_WRONLY);
	
	while(1){
		printf("Írja be a megfelelő karaktert: \n");	
		printf("L: listázás, U: új adat, M: módosítás, S: Keresés, D: torles,  K: kilépés \n");
		scanf("%c", &ch);
		getc(stdin);
		printf("valasztas: %c\n", ch);
		switch(ch){
			case 'l': case 'L': list(); break;
			case 'u': case 'U': newentry(); break;
			case 'm': case 'M': mod(); break;
			case 'k': case 'K': cleanup(); exit(0); break;
			case 's': case 'S': search(); break;
			case 'd': case 'D': delete(); break;
			default: printf("Hibás input! \n");
		}
	}

}

void delete(){
	printf("Melyik id-t? \n");
	int id;
	scanf("%i", &id);
	getc(stdin);
	
	const char ch='d';
	write(parent_to_slave, &ch, sizeof(ch));
	write(parent_to_slave, &id, sizeof(id));
	


}

void slave_delete(){
	int id;
	const char ch='d';
	read(parent_to_slave, &id, sizeof(id));
	write(slave_to_db, &ch, sizeof(ch));
	write(slave_to_db, &id, sizeof(id));
}

void slavechild(){

	logfile = fopen("slavelog.txt", "a");	
	parent_to_slave = open("/tmp/parent-to-slave", O_RDONLY);
	slave_to_db = open("/tmp/slave-to-db", O_WRONLY);

	char ch;
	
	while(1){
		ch = 0;
		fail_with_error("foooooo", read(parent_to_slave, &ch, sizeof(ch)));

		switch(ch){
			case 'u': case 'U': slave_newentry() ; break;
			case 'm': case 'M': slave_mod() ; break;
			case 's': case 'S': slave_search(); break;
			case 'd': case 'D': slave_delete(); break;
		}
	}



}


void slave_newentry(){
	struct stadion_t stadion;
	read(parent_to_slave, &stadion, sizeof(stadion));

	const char command = 'u';
	write(slave_to_db, &command, sizeof(command));
	write(slave_to_db, &stadion, sizeof(stadion));
}

void dbchild(){
	fd=open("stadionok.db", O_RDWR | O_CREAT, 0600);
	logfile = fopen("dblog.txt", "a");
	logging("Database child started");
	db_to_parent = open("/tmp/db-to-parent", O_WRONLY);
	slave_to_db = open("/tmp/slave-to-db", O_RDONLY);
	
	char ch;
	signal(SIGUSR1, dbchild_handler);
	while (1){
		ch = 0;
		if( -1 == read(slave_to_db, &ch, sizeof(ch)) ){
			if( errno == EINTR){
				continue;
			}else{
				fail_with_error("db child comamnd read failed", -1);
			}
		}

		switch(ch){
			case 'u': case 'U': db_newentry() ; break;
			case 'm': case 'M': db_mod(); break;
			case 's': case 'S': db_search(); break;
			case 'd': case 'D': db_delete(); break;
		}	
		
		sleep(1);
	}

}

void db_newentry(){
	logging("new entry arrieved");
	struct stadion_t stadion;
	read(slave_to_db, &stadion, sizeof(stadion));

	stadion.id = db_getmaxid()+1;
	lseek(fd, 0, SEEK_END);
	write(fd, &stadion, sizeof(stadion));
	
	logging("new entry added");
}


void list(){
	kill(dbchildpid, SIGUSR1);
	struct stadion_t stadion;
	size_t size_read;	
	while(1){
		size_read=read(db_to_parent, &stadion,sizeof(stadion));
		if (size_read==-1) {
			printf("Hiba tortent az olvasas kozben.%s\n", strerror(errno));
			exit(1);
		}
		else if(size_read==0 || stadion.id == -1){
			break;
		}
		else {
			printf("id: %i, város: %s, csapat: %s, max nézőszám: %u, költség: %lli, átadási idő: %s\n", stadion.id, stadion.city, stadion.team, stadion.maxvis, stadion.cost, ctime(&(stadion.date))); 
	}
}




}

void db_list(){
	logging("Full listing required");
	lseek(fd,0,SEEK_SET);
	struct stadion_t stadion;
	ssize_t size_read;
	while(1){
		size_read=read(fd,&stadion,sizeof(stadion));
		if (size_read==-1) {
			printf("Hiba tortent az olvasas kozben.%i", errno);
			exit(1);
		}
		else if(size_read==0){
			break;
		}
		else {
			write(db_to_parent, &stadion, sizeof(stadion));
		}
	}
	stadion.id=-1;
	write(db_to_parent, &stadion, sizeof(stadion));
	logging("Listing completed");

}

int db_getmaxid(){
	int res=lseek(fd, -1* sizeof(struct stadion_t),SEEK_END);
	if(res ==-1 && errno == EINVAL){
		return 0;
	}
	struct stadion_t stadion;
	ssize_t size_read;
	size_read=read(fd,&stadion,sizeof(stadion));
	if (size_read==-1) {
		printf("Hiba tortent az olvasas kozben.%i", errno);
		exit(1);
	}
	return stadion.id;

}


void newentry() {
	struct stadion_t stadion;
	printf("Város? \n");
	gets(stadion.city);
	printf("Csapat? \n");
	gets(stadion.team);
	printf("Max néző? \n");
	scanf("%u", &stadion.maxvis);
	printf("Költség? \n");
	scanf("%lli", &stadion.cost);
	printf("Átadás dátuma (YYYY-MM-DD) \n");
	char date[20];
	scanf("%s", date);
	getc(stdin);
	struct tm date2;
	memset(&date2, 0, sizeof(date2));
	strptime(date, "%Y-%m-%d", &date2);
	stadion.date = mktime(&date2);
	
	char command = 'u';
	write(parent_to_slave, &command, sizeof(command));
	write(parent_to_slave, &stadion, sizeof(stadion));

	
}

void copystring_if_not_empty( char * dest, const char * from) {
	if(strlen(from) != 0){
		strcpy(dest, from);
	}
}


void db_mod(){

	

	logging("modification request arrieved");
	struct stadion_t stadion, tochange;
	read(slave_to_db, &tochange, sizeof(tochange));
	

	int found;	
	lseek(fd,0,SEEK_SET);
	ssize_t size_read;
	while(1){
		size_read=read(fd,&stadion,sizeof(stadion));
		if (size_read==-1) {
			printf("Hiba tortent az olvasas kozben.%i", errno);
			exit(1);
		}
		else if(size_read==0){
			break;
		}
		else {
			if ( stadion.id==tochange.id) {
				found=1;
				break;
			}
		}
	}
	if(!found){
		printf("Nincs ilyen id\n");
		return;
	}
	lseek(fd,-sizeof(stadion), SEEK_CUR);

	//this is magiiic
	copystring_if_not_empty(stadion.city, tochange.city);
	copystring_if_not_empty(stadion.team, tochange.team);

	if( tochange.maxvis != 0) stadion.maxvis = tochange.maxvis;
	if( tochange.cost != 0) stadion.cost = tochange.cost;
	if( tochange.date != 0) stadion.date = tochange.date;


	write(fd, &stadion, sizeof(stadion));

	logging("modification done");

}


void slave_mod(){
	struct stadion_t stadion;
	struct stadion_mod_t tochange;
	bzero(&stadion, sizeof(stadion));

	read(parent_to_slave, &tochange, sizeof(tochange));
	stadion.id = tochange.id;

	struct tm date2;
	switch(tochange.mod){
		case 'v': case 'V': strcpy(stadion.city, tochange.newval); break;
		case 'c': case 'C':	strcpy(stadion.team, tochange.newval); break;
		case 'n': case 'N': stadion.maxvis = atoi(tochange.newval); break;
		case 'k': case 'K': stadion.cost = atoll(tochange.newval); break;
		case 'd': case 'D':
    	    memset(&date2, 0, sizeof(date2));
       	    strptime(tochange.newval, "%Y-%m-%d", &date2);
            stadion.date = mktime(&date2);
            break;	
	}
	
	const char ch = 'm';
	write(slave_to_db, &ch, sizeof(ch));
	write(slave_to_db, &stadion, sizeof(stadion));

}

void mod() {
	
	
	struct stadion_mod_t tochange;
	printf("Melyik id? \n");
	scanf("%i", &tochange.id);
	getc(stdin);
	
	printf("Mit szeretne módosítani? \n");
	printf("Írja be a megf. karaktert: \n");
	printf("V: város, C: csapat, N:nézőszám, K: költség, D: dátum  \n");
	char ch;
	scanf("%c", &ch);
	printf("Új adat? \n");
	getc(stdin);



	switch (ch)	{
		case 'v': case 'V':
		case 'c': case 'C': 
		case 'n': case 'N': 
		case 'k': case 'K':
		case 'd': case 'D':
			tochange.mod = ch;
			gets(tochange.newval);
			break;
		default: printf("Hibás input \n");
				exit(1);
	}

	const char ch2 = 'm';
	write(parent_to_slave, &ch2, sizeof(ch2));
	write(parent_to_slave, &tochange, sizeof(tochange));

}


void search(){


	printf("Mit szeretne keresni? \n");
	printf("Írja be a megf. karaktert: \n");
	printf("V: város, C: csapat, N:nézőszám, K: költség, D: dátum  \n");
	char ch;
	scanf("%c", &ch);
	printf("keresendo adat? \n");
	getc(stdin);
	char date[20];
	struct stadion_t minta;
	switch (ch)
	{
		case 'v': case 'V': gets(minta.city); break;
		case 'c': case 'C': gets(minta.team); break;
		case 'n': case 'N': scanf("%u", &minta.maxvis); getc(stdin); break;
		case 'k': case 'K': scanf("%lli", &minta.cost); getc(stdin); break;
		case 'd': case 'D':
			scanf("%s", date);
			getc(stdin);
			struct tm date2;
			memset(&date2, 0, sizeof(date2));
			strptime(date, "%Y-%m-%d", &date2);
			minta.date = mktime(&date2);
			break;
		default: printf("Hibás input \n");
				exit(1);
	}

	const char ch2 = 's';
	write(parent_to_slave, &ch2, sizeof(ch));
	write(parent_to_slave, &ch, sizeof(ch));
	write(parent_to_slave, &minta, sizeof(minta));

}


void slave_search(){
	char choice=0;
	const char ch = 's';
	struct stadion_t stadion;

	read(parent_to_slave, &choice, sizeof(choice));
	read(parent_to_slave, &stadion, sizeof(stadion));
	write(slave_to_db, &ch, sizeof(ch));  
	write(slave_to_db, &choice, sizeof(choice));  
	write(slave_to_db, &stadion, sizeof(stadion));  
}

void db_search(){
	
	logging("searching request arrieved");
	char choice=0;
	struct stadion_t minta;
	struct stadion_t stadion;
	read(slave_to_db, &choice, sizeof(choice));
	read(slave_to_db, &minta, sizeof(minta));

	int found=0, ok=0;
	lseek(fd,0,SEEK_SET);
	ssize_t size_read;
	while(1){
		size_read=read(fd,&stadion,sizeof(stadion));
		if (size_read==-1) {
			printf("Hiba tortent az olvasas kozben.%i", errno);
			exit(1);
		}
		else if(size_read==0){
			break;
		}
		else {  //feltelek ide
			switch(choice){
				case 'v': case 'V': if (does_it_match(stadion.city, minta.city) ) {ok=1;} break;
				case 'c': case 'C': if (does_it_match(stadion.team, minta.team) ) {ok=1;} break;
				case 'n': case 'N': if (minta.maxvis == stadion.maxvis) {ok=1;} break;
				case 'k': case 'K': if (minta.cost == stadion.cost) {ok=1;} break;
				case 'd': case 'D': if (minta.date == stadion.date) {ok=1;} break;
			}
		}

		if(ok == 1){
			printf("id: %i, város: %s, csapat: %s, max nézőszám: %u, költség: %lli, átadási idő: %s\n", stadion.id, stadion.city, stadion.team, stadion.maxvis, stadion.cost, ctime(&(stadion.date)));
			ok=0;
			found++;
		}
	}
	if(!found){
		logging("not found");
		printf("Nincs a feltetelnek megfelelo entry\n");
	}else{
		char msg[50];
		sprintf(msg, "%i matching found");
		logging(msg);
		printf("%i bejegyzest talaltam\n", found);
	}
}



int does_it_match(const char * hay, const char * needle){

	int hay_l = strlen(hay);
	int needle_l = strlen(needle);
	char needlecp[256];

	if (needle_l == 0 ){
		return 1;
	}

	if(needle[0] == '^' || needle[needle_l-1] == '$' ){
		//blewwwww
		if(needle[0] == '^' && needle[needle_l-1] != '$'){
			return( strstr(hay, needle+1) == hay);
		}else if(needle[0] != '^' && needle[needle_l-1] == '$'){
			if(needle_l-1 > hay_l)  {return 0;}
			strcpy(needlecp, needle);
			needlecp[needle_l-1] = '\0';
			return ( strcmp(hay + (hay_l-needle_l+1), needlecp) == 0);
		}else{
			if(needle_l-2 > hay_l) { return 0;}
			strcpy(needlecp, needle);
			needlecp[needle_l-1] = '\0';
			return ( strcmp(hay, needlecp+1) == 0);
		}


	}else{
		return ( strstr(hay, needle) != NULL);
	}


}

void db_delete(){
	logging("delete request arrieved");
	struct stadion_t stadion, tochange;
	read(slave_to_db, &tochange, sizeof(tochange));

	int found;	
	lseek(fd,0,SEEK_SET);
	ssize_t size_read;
	while(1){
		size_read=read(fd,&stadion,sizeof(stadion));
		if (size_read==-1) {
			printf("Hiba tortent az olvasas kozben.%i", errno);
			exit(1);
		}
		else if(size_read==0){
			break;
		}
		else {
			if ( stadion.id==tochange.id) {
				found=1;
				break;
			}
		}
	}
	if(!found){
		printf("Nincs ilyen id\n");
		return;
	}

	off_t whereiam = lseek(fd, 0, SEEK_CUR);
	off_t fileend = lseek(fd, 0, SEEK_END);

	if( whereiam == fileend){
		ftruncate(fd, fileend-sizeof(stadion));
		logging("delete completed");
		return;
	}
	
	lseek(fd, whereiam, SEEK_SET);
	while( lseek(fd, 0, SEEK_CUR) != fileend){
		read(fd, &stadion, sizeof(stadion));
		lseek(fd, -2 * sizeof(stadion), SEEK_CUR);
		write(fd, &stadion, sizeof(stadion));
		lseek(fd, sizeof(stadion), SEEK_CUR);
	}
	lseek(fd, 0, SEEK_SET);
	ftruncate(fd, fileend-sizeof(stadion));
	logging("delete completed");



}

void dbchild_handler(int s){

	if( s == SIGUSR1 ){
		db_list();
	}

}

void main_handler(int signal){
	cleanup();
	exit(0);
}

void create_pipes(){
	fail_with_error("failed fifo creation" , mkfifo("/tmp/db-to-parent", 0600));
	fail_with_error("failed fifo creation" , mkfifo("/tmp/parent-to-slave", 0600));
	fail_with_error("failed fifo creation" , mkfifo("/tmp/slave-to-db", 0600));

}

void cleanup(){
	kill(SIGTERM, slavechildpid);
	kill(SIGTERM, dbchildpid);

	unlink("/tmp/db-to-parent");
	unlink("/tmp/parent-to-slave");
	unlink("/tmp/slave-to-db");
}

void logging(const char * msg){
	time_t now;
	time(&now);

	fprintf(logfile, "%s \t-- %s\n", ctime(&now), msg);	
	fflush(logfile);

}

void fail_with_error(const char * msg, int rv){
	if(rv == -1){
		printf("%s, %s\n", msg, strerror(errno));
		cleanup();
		exit(1);
	}
}
