/* 
 * uart_com.h
 * Authors: zorro
 *   Rickard E. (Rik) Faith <faith@redhat.com> 
 *  head file of uart_com.c, define some
 */  

#ifndef UART_COM_H
#define UART_COM_H

/*
typedef enum{
	false,
	true
}bool;
*/
enum reco_mode{
	FRONT_VIEW,
	REAR_VIEW,
	LEFT_VIEW,
	RIGHT_VIEW,
	ALL_VIEW
};


struct play_video{
	int play_one;					/*1: request to paly ful screen*/
	int play_flg;					/*1: request to paly divi screen*/
	char play_nam[24];
};

struct reco_video{
	unsigned char reco_flg;			/*enable or disable dvr record*/
	enum reco_mode reco_mod;		/*record mode */
	unsigned int reco_prd;			/*record period*/
	unsigned int reco_frm;			/*frame num in this period */
	unsigned char reco_egy;			/* record emergency*/
};


extern struct play_video play_v;		/*quest to play a video*/
extern struct reco_video reco_v;		/*record video parameters set */

extern unsigned char disp_mod;
extern unsigned char distortion;


extern unsigned char dvr_com;
extern unsigned char dvr_s;			/*DVR record*/
extern unsigned char gsr_s;			/* G-sensor */
extern unsigned char egy_s;			/*Emergency*/

extern unsigned short int lock_num;
extern unsigned short int total_num;


/*not!reco_egy :when egy_record Be equal to 1, you can start emergency record,
    and clear it to zero when complete*/


#endif
