#ifndef __DM_H__
#define  __DM_H__

#define _DM_DEBUG_ 1
#define _INDEPENDENT_  0
#define _NFS_  1

#ifndef __TIME_T
#define __TIME_T     
typedef long     time_t;    /* time_t is long*/
#endif



#define CapBufSize 20

#define PATHSIZE 30//
#define NAMESIZE 25//


#define VideoPath "/DVR/OUTPUT"//相对于监控设备的根路径
#define VideoLockPath "lock"//相对与监控路径
#if  _INDEPENDENT_
#define VideoDevice "sda1"//
#define AllocateSize 110//unit KB

#else
#define VideoDevice "/$"//rootfs
//#define VideoDevice "root"//
//#define VideoDevice "mmcblk1p1"//
#define AllocateSize 1100000//unit KB

#endif

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
	int num;//vfile num
	int locknum;//lock vfile num
	int DiskAvailable;//disk available
	int DiskTotal;//disk total
	int lockflag;
	char path[PATHSIZE];//video save root path
	char lockpath[PATHSIZE];//video lock dir name,without path
	char CurID[NAMESIZE+PATHSIZE];
	struct vfile*vfileoldest;//oldest unlock file,if not exist it should point to vfileend
	struct vfile*vfilenewest;
	struct vfile*vfilehead;//point to g_vfile_head
	struct vfile*vfileend;//point to g_vfile_end	
	struct vfile*vfilepos;
	struct vfile** vfileArr;//use for 
	struct vfile*vfilereadpos;
	struct vfile** vfileLockArr;	
}vfilelist_t,*vfilelist_p;


vfilelist_t g_vfilelist;
vfile_t g_vfile_head;
vfile_t g_vfile_end;

int lockpathflag;// if the file is in the lock path,set the value 1


//void DMQuickSort(vfile_t **  r,int left,int right);
int DMInit(void);
int DMCreateID(int date , int channel , vfilelist_p vfilelistp);
int DMfclose(vfilelist_p vfilelistp , vfile_p vfilep);
int DMLockFile(const char * filename);
int DMunLockFile(const char * filename);
int DMDelFile(const char * filename);
int DMCheckByName(const char *name);
int DMOpenList(void);
int DMReadList(char* buf);
int DMGetAvailable(void);
int DMGetTotal(void);
int DMGetFullName(char *name);
int DMAddFile(const char *filename);
int runshellcmd(unsigned char * cmd);
#endif

/////////////////////
//if the disk have not enough disk manager should work normal
//but enc should not be start
