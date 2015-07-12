#ifndef __DM_H__
#define  __DM_H__

#define _DM_DEBUG_ 1
#define _INDEPENDENT_   0



#define CapBufSize 20
#define AllocateSize 4100000//unit KB

#define PATHSIZE 30//
#define NAMESIZE 25//


#define VideoPath "/DVR/OUTPUT"//相对于监控设备的根路径
#define VideoLockPath "lock"//相对与监控路径
#define VideoDevice "mmcblk2p1"//


//vedio file struct
typedef struct vfile{
	int id;//vedio file id base on 
	char name[NAMESIZE];
	char path[PATHSIZE];
	int time;
	int lock;
	struct vfile *next_vfile;
	struct vfile *pre_vfile;

}vfile_t,*vfile_p;


//vedio file list struct
typedef struct vfilelist{
	int status;//bit 7:vfileArr sorting bit 6:vfilehead realloicate bit 5:
	int num;
	char path[PATHSIZE];//video save path
	char lockpath[PATHSIZE];//video lock dir name
	char CurID[NAMESIZE+PATHSIZE];
	struct vfile*vfileoldest;
	struct vfile*vfilenewest;
	struct vfile*vfilehead;
	struct vfile*vfilepos;
	struct vfile** vfileArr;
	struct vfile** vfileLockArr;	
	struct vfile*vfilereadpos;
}vfilelist_t,*vfilelist_p;


vfilelist_t g_vfilelist;
vfile_t g_vfile_head;

int lockpathflag;// if the file is in the lock path,set the value 1


void DMQuickSort(vfile_t **  r,int left,int right);
int DMInit(void);
int DMCreateID(int date , int channel , vfilelist_p vfilelistp);
int DMfclose(vfilelist_p vfilelistp , vfile_p vfilep);
int DMLockFile(vfile_p vfilep,char * vfilelockpath);
int DMCheckByName(char *name);
int DMOpenList(void);
int DMReadList(char* buf);


#endif
