#ifndef BEAD_H
#define BEAD_H

#include <time.h>

typedef int bool;
#define TRUE 1
#define FALSE 0
typedef char course_code_t[11];
typedef char person_code_t[9];

struct course_t{
	course_code_t code;
	short max_people;
	short curr_people;
};

struct course_entry_t{
	course_code_t course_code;
	person_code_t person_id;
	char person_name[30];
	time_t date;
	char deleted;
};


void course_manintance(void);
void course_list(void);
void course_apply(void);

void course_add(void);
bool course_is_ok_apply();
void course_list_students(void);
void course_delete_student(void);


void dancsa_getstr(char * str, int n);
extern int courses_fd;
extern int courses_apply_fd;





#endif
