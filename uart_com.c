/************************************
  *file name: uart_com.c
  *description:Function of serial communication
  *author: zorro
  *created date: 06/17/2015
  ************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <termios.h>
#include <sys/types.h>   /*Provide the definition of type pid_t*/
#include <sys/stat.h>    /*I/O operations*/
#include <time.h>

#include <fcntl.h>
#include <unistd.h>

#include "uart_com.h"
#include "dm.h"


//memset(cmd_buf, 0, BUFFER_SIZE);


/*private definition-------------------------------------*/
#define MAX_COM_NUM  20
#define GNR_COM 0
#define USB_COM 1
#define COM_TYPE GNR_COM

#define SEL_FILE_NUM   	2
#define BUFFER_SIZE	30	
#define TARGET_COM_PORT	4	/*USB1 - 2*/
#define RECV_FILE_NAME          "/home/user/recv_file"
#define TIME_DELAY      100	/*select wait 3 seconds*/

/* The frame format is "start_byte,tx_id(tbd),cmd_len,cmd_id,data, check_sum"*/
#define FRAME_HEAD_SIZE 3       /*include start_byte,tx_id,cmd_len*/	
#define START_BYTE      0xAA

#define CTIME_SZIE	24



/*function declaration------------------------------------*/
static int msg_unpack(unsigned char buff[], int *rsp_msg_id, unsigned char *errvalue);
static int check_frame_head(unsigned char buff[], int *data_len);
static int check_frame_sum(unsigned char buff [],int data_len);
static int msg_pack(unsigned char buff[], int rsp_msg_id, int msg_counter, unsigned char errvalue, int *msg_len);



/*global variable----*/

bool dvr_com = true;
bool dvr_s = true;			/*DVR record*/
bool gsr_s = true;			/* G-sensor */
bool egy_s = true;			/*Emergency*/

struct play_video play_v;
struct reco_video reco_v = {1 ,FRONT_VIEW, 5, 0};

unsigned short int lock_num = 4;
unsigned short int total_num = 10;



unsigned char rt_video = 0;

/*private variable-----*/
int video_num = 0;
int rsp_cnt;

unsigned char hw_version = 0x00;
unsigned char sw_version = 0x00;





/*
struct termios
{
	unsigned short c_iflag;		/* input mode flag
	unsigned short c_oflag;		/* output mode flag
	unsigned short c_cflag;		/* control mode flag
	unsigned short c_lflag;		/* Local mode flag
	unsigned char c_line;		/* Line discipline
	unsigned char c_cc[NCC];		/* control feature
	speed_t c_ispeed;			/* input speed
	speed_t c_ospeed;			/* output speed
}; */


/*======================================
*Function: set_com_config
*description:Setting up the serial port parameters 
*Author: zorro
*Created Date :6/18/2015
*======================================*/
int
set_com_config(int fd, int baud_rate, int data_bits, char parity, int stop_bits)
{
	struct termios options;
	int speed;

	/*Save and test the existing serial interface parameter Settings, here if the serial number and other errors,
	   There will be a relevant error message*/
	if (tcgetattr(fd, &options) != 0){
		perror("tcgetattr");
		printf("\n\n\nzorro, tcgetattr err: %s\n", strerror(errno));
		return -1;
	}

	/*set the character size*/
	cfmakeraw(&options); /*configured to the original model*/
	options.c_cflag &= ~CSIZE;

	/*set the baud rate*/
	switch (baud_rate){
		case 2400:
		{
			speed = B2400;
		}
		break;
		case 4800:
		{
			speed = B4800;
		}
		break;
		case 9600:
		{
			speed = B9600;
		}
		break;
		case 19200:
		{
			speed = B19200;
		}
		break;
		case 38400:
		{
			speed = B38400;
		}
		break;
		default:
		case 115200:
		{
			speed = B115200;
		}
		break;
	}
	cfsetispeed(&options, speed);
	cfsetospeed(&options, speed);

	/* set the stop bit */
	switch (data_bits){
		case 7:
		{
			options.c_cflag |= CS7;
		}
		break;

		default:
		case 8:
		{
			options.c_cflag |= CS8;
		}
		break;
	}

	/* Set the parity bit */
	switch (parity){
		default:
		case  'n':
		case  'N':
		{
			options.c_cflag &= ~PARENB;
			options.c_iflag &= ~INPCK;
		}
		break;

		case  'o':
		case  'O':
		{
			options.c_cflag |= (PARODD | PARENB);
			options.c_iflag |= INPCK;
		}
		break;

		case 'e':
		case 'E':
		{
			options.c_cflag |= PARENB;
			options.c_cflag &= ~PARODD;
			options.c_iflag |= INPCK;
		}
		break;

		case 's': /*as no parity*/
		case 'S':
		{
			options.c_cflag &= ~PARENB;
			options.c_cflag &= ~CSTOPB;
		}
		break;			
	}

	switch (stop_bits){
		default:
		case  1:
		{
			options.c_cflag &= ~CSTOPB;
		}
		break;

		case 2:
		{
			options.c_cflag |= CSTOPB;
		}
		break;
	}

	/*Set the waiting time and minimum received characters*/
	options.c_cc[VTIME] = 0;
	options.c_cc[VMIN] = 1;

	/*Handle not receive characters, clean the receive or transmit buff*/
	tcflush(fd, TCIFLUSH);
	
	/*Activate the new configuration*/
	if ((tcsetattr(fd, TCSANOW, &options)) !=0){
		perror("tcsetattr");
		printf("\n\n\nzorro, tcsetattr err: %s\n", strerror(errno));
		return -1;
	}
	return 0;
}

/*======================================
*Function: open_port
*description:open a serial port
*Author: zorro
*Created Date :6/18/2015
*======================================*/
int open_port(int com_port)
{
	int fd;
#if (COM_TYPE == GNR_COM) /*use ordinary serial port*/
	char *dev[] = {"/dev/ttyS0", "/dev/ttyS1","/dev/ttyS2","/dev/ttymxc2"};
#else				/*use USB  serial port*/
	char *dev[] = {"/dev/ttyUSB0", "/dev/ttyUSB1", "/dev/ttyUSB0"};
#endif

	if ((com_port < 0) || (com_port > MAX_COM_NUM)){
		return -1;
	}

	/* open the serial port*/
	fd = open(dev[com_port -1], O_RDWR|O_NOCTTY|O_NDELAY);
	if (fd < 0){
		perror("open serial port");
		printf("\n\n\nzorro, can't open %s: %s\n", dev[com_port -1],strerror(errno));
		return -1;
	}

	/*recover to blocking mode */
	if (fcntl(fd, F_SETFL, 0) < 0){
		perror("fcntl F_SETFL\n");
		printf("\n\n\nzorro, fcntl F_SETFL: %s\n", strerror(errno));
	}

	/*  whether the terminal equipment*/
	if (isatty(STDIN_FILENO) == 0){
		perror("standard input is not a terminal device");
		printf("\n\nstandard input is not a terminal device: %s\n", strerror(errno));
	}
	return fd;
}

/*======================================
*Function: msg_unpack
*description:unpack the msg frame
*Author: zorro
*Created Date :6/18/2015
*======================================*/   
static int
msg_unpack(unsigned char buff[], int *rsp_cmd, unsigned char *errvalue)
{
	int msg_id;
	int ret = 0;
	msg_id = buff[3];
	*rsp_cmd = msg_id+1;
	switch (msg_id){
		case 0:
			printf("\nzorro, msg id  = 0, \n");
			break;
		case 1:
			printf("\nzorro, msg id  = 1, \n");
			break;
		case 2:
			printf("\nzorro, msg id  = 2, \n");
			break;
		/*nothing to do with below msg id, only reply */
		case 0x30:	
		case 0x32:
		case 0x3C:
		case 0x3E:
			*errvalue = 0;
			break;
		case 0x34:	/*DA_REQ_RECORD_SET */
			if (buff[4] = 1){
				reco_v.reco_flg = true;
				*errvalue = 0;
			}else{
				*errvalue = 1;
				reco_v.reco_flg = false;
			}
			break;
		case 0x36:
			rsp_cnt = video_num = DMOpenList();
			*errvalue = 0;
			break;
		case 0x38:
		{
			strncpy(play_v.play_nam, &buff[4], CTIME_SZIE);
			if (DMCheckByName(play_v.play_nam) < 0){
				*errvalue = -1;
			}
			play_v.play_flg = 1;

			//replay_video(video_id);
			*errvalue = 0;
			printf("\nzorro, msg req replay video, replay video name   = %s, \n", play_v.play_nam);
		}
			break;
		case 0x3A:
			//unsigned char control_type = buff[4];
			//replay_control(buff[4])
			*errvalue = 0;
			printf("\nzorro, msg req video control, control type = %d, \n",  buff[4]);
			break;
		case 0x40:
			*errvalue = 0;
			if (buff[4] & 0x80){
				reco_v.reco_prd=  (buff[4] & 0xEF);
				printf("\nzorro, msg req set parameter, dvr period = %d, \n",  reco_v.reco_prd);
			}
			if (buff[5] & 0x80){
				char mode = (buff[5] & 0xEF);
				if (mode <= 4)
					reco_v.reco_mod = mode;
				else
					*errvalue = 1;
				printf("\nzorro, msg req set parameter, dvr mode = %d, \n",  mode);
			}
			break;
		case 0x42:
			*errvalue = 0;
			reco_v.reco_egy= 1;
			printf("\nzorro, msg req emergency record\n");
			break;
		case 0x44:
			if (buff[4] <= 1){
				rt_video = buff[4];
				*errvalue = 0;
			}else{
				*errvalue = 1;
			}
			printf("\nzorro, msg req rt video, control type = %d, \n",  buff[4]);
			break;
		default:
			if ((msg_id < 0x30) ||(msg_id >0x44)||(msg_id%2)){
				ret = -1;
			}
			printf("\nzorro, msg id  = %d, \n", msg_id);
			break;
	}

	return ret;
}

/*======================================
*Function: msg_pack
*description:pack response msg frame
*Author: zorro
*Created Date :6/18/2015
*======================================*/   
static int msg_pack(unsigned char buff[], int rsp_cmd, int cmd_counter, unsigned char errvalue, int *msg_len)
{
        	int check_sum, data_len;
        	int i,j;
	int ret = 0;
	char video[24];
		
	buff[0] = 0xAA;  
	buff[1] = cmd_counter;
	buff[3] = rsp_cmd;
	 
	switch(rsp_cmd)
	{
		case 0x31:
			data_len = 0x03;
			if (dvr_com == true)
				buff[4] = 0x00;
			else
				buff[4] = 0x01;	
		break;
		case 0x33:
			data_len = 0x09;
			buff[4] = dvr_s;		
			buff[5] = gsr_s;		
			buff[6] = egy_s;		
			*(unsigned short int *)(&buff[7]) = lock_num;			
			*(unsigned short int *)(&buff[9]) = total_num;			
		break;
		case 0x35:
		case 0x39:
		case 0x3B:
		case 0x41:
		case 0x43:
		case 0x45:
			data_len = 0x03;
			buff[4] = errvalue;		
		break;
		case 0x37:
		/*response one video name */
		if (rsp_cnt > 0)
		{	
			if (DMReadList(video) < 0){
				rsp_cnt = 0;
				return -1;
			}
			rsp_cnt--;
			ret = 1;
			data_len = 0x1A;
			//time_t tv_sec = time(NULL);/* long */
			//strncpy(&buff[4], ctime(&tv_sec), CTIME_SZIE);
			strncpy(&buff[4], video, CTIME_SZIE); 
		}
		break;
		case 0x3D:
		{ 	
			data_len = 0x10;			
			buff[4] = hw_version;
			buff[5] = sw_version;
			int available_disk = 8120;
			int total_disk = 8120;
			*(unsigned int *)(&(buff[6])) = available_disk;
			*(unsigned int *)(&(buff[10])) = total_disk;
			*(unsigned int *)(&(buff[14])) =  time(NULL);
		}
		break;		
		case 0x3F:	/* DVR_RSP_PARAMETERS */
			data_len = 0x04;
			buff[4] = reco_v.reco_prd;
			buff[5] = reco_v.reco_mod;
		break;

		default:
			return -1;
		
		
	}

	buff[2]  = data_len;
	*msg_len = data_len+3;
	check_sum = 0xAA;
	for ( i=1; i < data_len+2; i++ ){
		check_sum^=buff[i];
	}

	buff[data_len+2] = check_sum;
	return ret;
}

/*======================================
*Function: check_frame_head
*description:check the frame head
*Author: zorro
*Created Date :6/18/2015
*======================================*/  
static int check_frame_head(unsigned char buff[], int *pdata_len)
{
	int i,temp_len,err;

	temp_len = 0;

	if (*pdata_len < 0){
		err = -1;
	}
	else{
		//search the frame head
		for (i = 3; i > 0; i--){
			if (START_BYTE == buff[i-1]){	
			    	temp_len = (*pdata_len - (i-1));	
				if (i != 1){
					/*translation the byte After the new frame head to front of the buff */
					int copy_pos = 0;
					
					for (; i <=  *pdata_len; i++){
						buff[copy_pos] = buff[i -1];
						copy_pos++;
					}
				}
				break;  //find the head then break this loop
			}
		}
	
		*pdata_len = temp_len;

		err = 0;
	}
	
	return err;
	
}

/*======================================
*Function: check_frame_sum
*description: check the frame crc
*Author: zorro
*Created Date :6/18/2015
*======================================*/ 
static int check_frame_sum(unsigned char buff [ ],int data_len)
{
	int err, i, xor_sum = 0;
	if (data_len < 0){
		err = -1;
	}
	else{
		for (i =0; i < (data_len - 1); i++){
			xor_sum ^= buff[i];
		}
		
		if (xor_sum == buff[data_len - 1]){
			err = 0;
		}
		else{
			err = -1;
		}
	}

	return err;
}


/*======================================
*Function: uart_com_thread
*description: this is communication thread with MP5
*Author: zorro
*Created Date :6/18/2015
*======================================*/
//int main(void) zorro
int
uart_thread(void *arg)
{

	int serial_fd, recv_fd, maxfd;	

	
	fd_set inset, tmp_inset;
	struct timeval tv;
	unsigned loop = 1;
	int  real_read, i, real_write, data_len;
	int rsp_msg, rsp_counter, write_len = 0;
	static int step = 0;
	unsigned char rcv_buf[BUFFER_SIZE]; 
	unsigned char snd_buf[BUFFER_SIZE];
	unsigned char data_t;
	unsigned char errvalue;
	int rcv_trans = 0;
	

	/*The serial port data written to this file*/
	if ((recv_fd = open(RECV_FILE_NAME, O_CREAT|O_WRONLY, 0644))  < 0){
		perror("open");
		printf("\n\n\nzorro, can't open recv_file: %s\n", strerror(errno));
		return 1;
	}
	
	printf("the recv_file fd = %d,\n", recv_fd);

	//fds[0] = STDIN_FILENO; /*The standard input*/

	if ((serial_fd = open_port(TARGET_COM_PORT)) < 0){
		perror("open_port");
		printf("\n\n\nzorro, can't open target com port: %s\n", strerror(errno));
		return 1;
	}

	printf("the serial_file fd = %d,\n", serial_fd);
	/*config com*/
	if (set_com_config(serial_fd, 115200, 8, 'N', 1) < 0){
		perror("set_com_config");
		printf("\n\n\nzorro, can't set com fonfig: %s\n", strerror(errno));
		return 1;
	}

	printf("Input some words(enter 'quit' to exit):\n");

	while (1)
	{

		if (!rcv_trans){  /*rcv uart data */
			/*read frame head from serial port*/
			real_read = read(serial_fd, &data_t, 1);

			if ((real_read <= 0) && (errno != EAGAIN)){
				  printf("\nzorro, ERROR:READ FRAME HEAD error1 real_read = %d, \n", real_read);
			 }

			 switch (step){
				 case 0x00:  //DA_DECODE_SYN_HEAD
					 if (data_t == 0xAA){
						 rcv_buf[0] = data_t;
						 step=1;
					 }
				 break;
				 case 0x01: //DA_DECODE_GET_SYN_COUNTER
					   if (data_t == 0xAA)
					   	step = 0;
					   else{
							   rcv_buf[1] = data_t;
							   step = 2;
						 }		  
					 break;
				 case 0x02: //DA_DECODE_GET_DATA_LENGTH				 
					 if ((data_t < 2)  ||(data_t > 17))
					           step = 0;
					else{
						data_len = rcv_buf[2] = data_t;
						 step  = 3;
						 i = 0;
					 }
					 break;
				 case 0x03: //DA_DECODE_GET_DATA and DA_DECODE_CHECK	
					rcv_buf[3+i++] = data_t;
					if (i == data_len){
						step = 0;	          //reset the unpack step
						int check;
						check = check_frame_sum(rcv_buf, (data_len+3));

						if (!check){
							if (msg_unpack(rcv_buf,&rsp_msg, &errvalue) < 0){
								printf("zorro, ERROR:cmd unpack err\n");
							}
							else{
								rcv_trans = 1;
							}
				
							rcv_buf[data_len+3] = '\0';
							/*write the data to commen file*/
							write(recv_fd, rcv_buf, data_len+3);
						}
						else if (check < 0){
							printf("\nzorro, ERROR:check sum error \n");
						}
						
					}
					break;	 
					 
				 }
		}
		else{	/*transmit uart data */
			
			 if (msg_pack(snd_buf,rsp_msg, rsp_counter, errvalue, &write_len) <= 0){
			 	rcv_trans = 0;
			 }
			 /*read frame head from serial port*/
			real_write = write(serial_fd, &snd_buf, write_len);
			if ((real_write <= 0) && (errno != EAGAIN)){
				  printf("\nzorro, ERROR:write cmd error1 real_write = %d, \n", real_write);
			}

			if ((write_len - real_write) != 0)
				 printf("error!\n");
			else
				rsp_counter ++;


		}
	}

}

/*------------end of file------------*/
