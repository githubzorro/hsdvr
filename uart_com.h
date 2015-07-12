/* 
 * uart_com.h
 * Authors: zorro
 *   Rickard E. (Rik) Faith <faith@redhat.com> 
 *  head file of uart_com.c, define some
 */  

#ifndef UART_COM_H
#define UART_COM_H


typedef enum{
	false,
	true
}bool;

enum reco_mode{
	FRONT_VIEW,
	LEFT_VIEW,
	RIGHT_VIEW,
	REAR_VIEW,
	ALL_VIEW
};

struct play_video{
	int play_flg;
	char play_nam[24];
};

struct reco_video{
	bool reco_flg;
	enum reco_mode reco_mod;		/*record mode */
	unsigned char reco_prd;		/*record period*/
	unsigned char reco_egy;		/* record emergency*/
};


extern struct play_video play_v;
extern struct reco_video reco_v;


extern bool dvr_s;			/*DVR record*/
extern  bool gsr_s;			/* G-sensor */
extern  bool egy_s;			/*Emergency*/

extern unsigned short int lock_num;
extern unsigned short int total_num;


/*when egy_record Be equal to 1, you can start emergency record,
    and clear it to zero when complete*/




#endif
