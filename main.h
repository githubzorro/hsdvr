/* 
 * main.h
 * Authors: shangwei
 *    
 *  head file of main.c, define some
 */  

#ifndef _MAIN_H_
#define _MAIN_H_

#define ARGSSIZE 20//

struct input_argument {
	int mode;
	pthread_t tid;
	char line[256];
	struct cmd_line cmd;
};

struct enc_arg
{
	int argc;
	struct input_argument *ppparg;
}enc_arg_t,*enc_arg_p;

typedef struct config
{
	int encflag;
	int decflag;
	int fps;
	void * pargv;
	int max_channels;
	int chs;//start channels
	int lockvfileflag;
	int UartID;
	int EnctID;
	int DecID;
	int KeyID;
	int EncWatch;
	int EncRestart;


}config_t,*config_p;


extern config_t g_config;



#endif

