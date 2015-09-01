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
	int *argtmp;
	argtmp=arg;
	//int e,d;

	 /*open the /dev/input/event of key*/  
	 if ((fd = open(DEV_PATH, O_RDONLY, 0)) < 0) {
			   printf("\nzorro	%s: open failed, fd = %d\n", DEV_PATH, fd);
		 return 0;
	 }

         /*read the key event*/  
        while ((rc = read(fd, &event, sizeof(event))) > 0) {
	              printf("key enent :  %-24.24s.%06lu type 0x%04x; code 0x%04x;"
			" value 0x%08x ;\n\n", 
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

/*
			  e=*argtmp/10;
			  d=*argtmp%10;
			if ((event.code & 0xff) == 0x0073)//115 key value(vol up)
			{
				if(event.value==0)//release
				{
					if (e)//10 bit
					{
						*argtmp=*argtmp-10;
					}
					else
					{
						*argtmp=*argtmp+10;
					}
					printf("enc mode change to %d\n",*argtmp);
				}
			}
			if ((event.code & 0xff) == 0x0072)//114 key value(vol dn)
			{
				if(event.value==0)//release
				{					
					if (d)// 1 bit
					{
						*argtmp=*argtmp-1;
					}
					else
					{
						*argtmp=*argtmp+1;
					}
					printf("dec mode change to %d\n",*argtmp);
				}
			}
			*/

			if ((event.code & 0xff) == 0x0072)//114 key value(vol dn)
			{
				if(event.value==0)//release
				{					
					if (*argtmp==1)// 1 bit
					{
						printf("dec mode change to enc:%d\n",*argtmp);
						*argtmp=10;
					}
					else
					{
						printf("enc mode change to dec :%d\n",*argtmp);
						*argtmp=1;
					}
				}
			}

			// enable_record += (int)event.value;	/*Even number represents Enable, odd number Disable*/
                    }
                }
            }
	printf("rc = %d, (%s)\n", rc, strerror(errno));
	close(fd);

	return 0;
}

//
#if _INDEPENDENT_
int main(int argc, char *argv[])
{
	key_thread(argv[1]);
}
#endif

