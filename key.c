/* 
 * comm.c
 * Authors: zorro, modify 
 *   Rickard E. (Rik) Faith <faith@redhat.com> 
 *  handle the key signal and uart cmd
 */ 
  
#include <stdio.h> 
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <linux/input.h>


#include "key.h"

static struct input_event event;


/*===============key_thread================
*Function: read the key button,and set the enable record flag
*Author: zorro
*Date :6/1/2015
*======================================*/
int
key_thread(void *arg)
{
	int  rc;
	int fd = 0;
	int enable_record=0;

	 /*open the /dev/input/event of key*/  
	 if ((fd = open(DEV_PATH, O_RDONLY, 0)) < 0) {
			   printf("\nzorro	%s: open failed, fd = %d\n", DEV_PATH, fd);
		 return 0;
	 }

         /*read the key event*/  
        while ((rc = read(fd, &event, sizeof(event))) > 0) {
	              printf("key enent :  %-24.24s.%06lu type 0x%04x; code 0x%04x;"
			" value 0x%08x ;\n", 
                      	ctime(&event.time.tv_sec),
                       	event.time.tv_usec,
                       	event.type, event.code, event.value);
                if (event.type == EV_KEY) { 
                    if (event.code > BTN_MISC) {
                        printf("Button %d %s", 
                               event.code & 0xff,
                               event.value ? "press" : "release");
                    } else {
                              printf("Key %d (0x%x) %s",
                               event.code & 0xff,
                               event.code & 0xff,
                               event.value ? "press" : "release");

			if ((event.code & 0xff) == 0x0073)
		          enable_record += (int)event.value;	/*Even number represents Enable, odd number Disable*/
                    }
                }
            }
	printf("rc = %d, (%s)\n", rc, strerror(errno));
	close(fd);

	return 0;
}


#if _INDEPENDENT_
int main(int argc, char *argv[])
{
	key_thread(argv[0]);
}
#endif


/*-------end of file-------*/
