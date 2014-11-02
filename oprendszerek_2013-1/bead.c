#include <stdio.h>
#include "bead.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>

int courses_fd = -1;
int courses_apply_fd= -1;

int main(void){

	courses_fd = open("courses.db", O_CREAT|O_RDWR, 0700);
	courses_apply_fd = open("courses_apply.db", O_CREAT|O_RDWR, 0700);
	
	if( courses_fd == -1 || courses_apply_fd == -1 ){ //failed to open file
		fprintf(stderr, "Nem sikerult megnyitni az egyik fajl. Errno: %d\n", errno);
		exit(1);
	}

	short sel;
	bool is_exit = FALSE;
	while(!is_exit){
		printf("Valassz a menubol\n");
		printf("\t[1] Kurzusok karbantartasa\n");
		printf("\t[2] Kurzusok listazasa\n");
		printf("\t[3] Kurzus jelentkezes\n");
		printf("\t[4] Kurzus jelentkezes lista\n");
		printf("\t[5] Kurzus jelentkezes torles\n");
		printf("\t[0] Exit\n");
		
		scanf("%hd",&sel);
		switch(sel){
			case 0: is_exit = TRUE; break;
			case 1: course_manintance(); break;
			case 2: course_list(); break;
			case 3: course_apply(); break;
			case 4: course_list_students(); break;
			case 5: course_delete_student(); break;
			default:
				printf("Hibas valasztas!\n");
			break;
		}



	}

	close(courses_fd);
	close(courses_apply_fd);

	return 0;

}



