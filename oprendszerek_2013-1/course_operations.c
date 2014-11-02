#include "bead.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

void course_manintance(void){
	printf("\t[1] Kurzus hozzadasa\n");
	printf("\t[0] Exit\n");

	short sel;
	scanf("%hd",&sel);
	switch(sel){
		case 0: break; //no action
		case 1: course_add(); break;
		default:
			printf("Hibas valasztas!\n");
		break;
	}
}

void course_add(void){
	
	struct course_t course;
	course.curr_people = 0;


	printf("Kurzus kod? ");
	scanf("%s", course.code);
	printf("Max letszam? ");
	scanf("%hd", &(course.max_people) );

	lseek(courses_fd, 0, SEEK_END);
	ssize_t rwed = write(courses_fd, &course, sizeof(course));
	
	if( rwed != sizeof(course) ){
		fprintf(stderr,"Write failed. DB may be left inconsistent state.\n rv=%i, errno=%i",rwed,errno);
		exit(1);
	}
}

void course_list(void){
	lseek(courses_fd, 0,SEEK_SET); //to teh staaart!
	struct course_t course;
	while( sizeof(course) == read(courses_fd, &course, sizeof(course)) ){
		printf("Kod: %s, Max_jel: %hd, jel: %hd\n", course.code, course.max_people, course.curr_people);
	}	
}


//requires that course_entry_t sructure's person_name exactly 30 char
void course_apply(void){
	struct course_t course;
	struct course_entry_t applicant;
	applicant.date = time(NULL);
	applicant.deleted = 0;

	printf("Kurzus kod? ");
	getc(stdin);
	dancsa_getstr(applicant.course_code, 11);

	printf("Nev? ");
	dancsa_getstr(applicant.person_name, 30);

	printf("Nyemptun kod? ");
	dancsa_getstr(applicant.person_id, 9);

	char end = 0;
	lseek(courses_fd, 0,SEEK_SET);
	while( !end  && (sizeof(course) == read(courses_fd, &course, sizeof(course))) ){ //depends on lazy AND
		if( strcmp(course.code, applicant.course_code) == 0 ){ //matching course code
			end = 1;
			lseek(courses_fd, -sizeof(course), SEEK_CUR); //rewind one record
			course.curr_people++;
			if( course.curr_people > course.max_people ){
				printf("Max jelentkezesi szam elerve... sajnalom...=(\n");
				return;
			}

			ssize_t rwed = write(courses_fd, &course, sizeof(course));
			if( rwed != sizeof(course) ){
				fprintf(stderr,"Write failed. DB may be left inconsistent state.\n rv=%i, errno=%i",rwed,errno);
				exit(1);
			}
			
			lseek(courses_apply_fd, 0, SEEK_END);
			rwed = write(courses_apply_fd, &applicant, sizeof(applicant));
			if( rwed != sizeof(applicant) ){
				fprintf(stderr,"Write failed. DB may be left inconsistent state.\n rv=%i, errno=%i",rwed,errno);
				exit(1);
			}
		}
	}
	if( !end ){ //need moar course, cuz we didn't find it
		fprintf(stderr, "Nincs ilyen kurzus a kurzus adatbazisban, elobb fel kene venni.\n");
	}else{ //in case of okay, we can haz a cheezeburger
		course_list_students();
	}
}

void course_list_students(void){
	struct course_entry_t applicant;
	lseek(courses_apply_fd, 0, SEEK_SET);
	while( sizeof(applicant) == read(courses_apply_fd, &applicant, sizeof(applicant)) ){
		if(!applicant.deleted){
			char date[20];
		
			strftime(date,20, "%F %R",localtime(&(applicant.date)));
			printf("Kurzus: %s, Nev %s, nyeptun(c)ode: %s, datum: %s\n", applicant.course_code, applicant.person_name, applicant.person_id, date);
		}
	}	
	
}

void course_delete_student(){
	struct course_t course;
	struct course_entry_t to_delete;
	struct course_entry_t rec;

	printf("Kurzus kod? ");
	getc(stdin);	
	dancsa_getstr(to_delete.course_code, 11);

	printf("Nyemptun(c) kod? ");
	dancsa_getstr(to_delete.person_id, 9);
	
	char end = 0;
	lseek(courses_fd, 0,SEEK_SET);
	lseek(courses_apply_fd, 0,SEEK_SET);


	while( !end  && (sizeof(rec) == read(courses_apply_fd, &rec, sizeof(rec))) ){ //depends on lazy AND
		if( strcmp(rec.course_code, to_delete.course_code) == 0 && strcmp(rec.person_id, to_delete.person_id) == 0 ){ //matching enrollment
			end = 1;
			lseek(courses_apply_fd, -sizeof(rec), SEEK_CUR); //rewind one record
			rec.deleted=1;
			write(courses_apply_fd, &rec, sizeof(rec));
		}
	}
	if( !end ){
		printf("Nincs ilyen jelentkezes!");
		return;
	}
	lseek(courses_fd, 0,SEEK_SET); //to teh staaart!
	while( sizeof(course) == read(courses_fd, &course, sizeof(course)) ){
		lseek(courses_fd, -sizeof(course), SEEK_CUR); //rewind one record
		course.curr_people--;
		write(courses_fd, &course, sizeof(course));
	}		
}




void dancsa_getstr(char * str, int n){
	char * sajt = NULL;
	size_t nsajt;
	memset(str, 0, n); //to make sure of the ending zero
	getline(&sajt, &nsajt, stdin);
	strncpy(str, sajt, n);
	str[n-1]='\0';

	int i;
	for(i = 0; str[i] != '\0'; i++){
		if( str[i] == '\n'){
			str[i] = '\0';
		}
	}
	free(sajt);
}
