/* 
 * main.h
 * Authors: shangwei
 *    
 *  head file of main.c, define some
 */  

#ifndef _MAIN_H_
#define _MAIN_H_

#define ARGSSIZE 20//

#define ArgvN 		9//no param
#define pargc     	3//no param

struct input_argument {
	int mode;
	pthread_t tid;
	char line[256];
	struct cmd_line cmd;
};

enum 
{
	DISP_OFF,
	DISP_NORMAL,
	DISP_FULL,
};

struct enc_arg
{
	int argc;
	struct input_argument *ppparg;
}enc_arg_t,*enc_arg_p;

typedef struct config
{
	void * pargv;
	int UartID;
	int EnctID;
	int DecID;
	int KeyID;
	int EncWatch;
	int EncRestart;
	char filename[50];
	char FileID[50];
	int lockvfileflag;
	void * DMGlobal;
	void * UartGlobal;
	int m_quitflag;//main quit flag
	int keyvalue;//key value 
	int UartSetUse;//some will use default if this value was 0
	int fps;
	int Frames;//frames per file
	int enc_chs;//enc channel status :1 means open, 0 means close
	int dec_chs;//dec channel status :1 means open, 0 means close
	int dec_one;//dec channel status :1 means open, 0 means close
	int enc_emergency;//emergency swith
	int enc_emergency_end;//emergency status
	int mode_flag;//enc mode or dec mode
	int enc_flag;//enc enable or disable
	int enc_flag_bkp;//enc enable or disable
	int dec_flag;//dec enable or disable
	int max_channels;
	int PlasySpeed;//reserve

	
	int disp_chs;//display channel :1 means open, 0 means close
	int display_width;
	int display_height;
	int display_top;
	int display_left;

	
	struct input_argument *input_arg;
}config_t,*config_p;

int m_set_enc_frames(int frames);
int m_set_enc_channel(int chs);
int m_set_enc_mode(int mode);
int m_set_dec_channel(int chs);
int m_start_dec(const char *filename,int ID,int chs);
int m_set_emergency_mode(int mode);
int m_get_enc_frames(void);
int m_get_enc_channel(void);
int m_get_enc_mode(void);
int m_get_dec_channel(void);
int m_get_dec_mode(void);
int m_get_emergency_mode(void);


extern config_t m_gconfig;



#endif

