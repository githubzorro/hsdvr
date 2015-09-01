#include <stdio.h>

#include <stdlib.h>
//#include <stddef.h>//sizeof
#include <string.h>

#include <dirent.h> 	      /*for opendir&readdir*/

#include <time.h>   /* for time funtion*/

#include "dm.h"

/*
file format : date(yyyymmdd)+num(6)+channel(vf/vb/vl/vr)
*/

extern char **environ;


#if 1
//Â†ÜÊéíÂ∫èÂª∫Á´ãÂ†Ü
//test fail only use in doubule 
void DMCreatHeap(vfile_t ** r,int i,int n) 
{  
	int j;  
	vfile_t * t;  
	j=2*i; 
	t=r[i];//shagnwei
	while(j<n)  
	{  
		if((j<n)&&( r[j]-> id < r[j+1] -> id))j++;  //shagnwei
		if(t-> id<r[j]-> id)  //shagnwei
		{  
			r[i]=r[j];  //shagnwei
			i=j;j=2*i;  
		}  
		else j=n;  
		r[i]=t;  //shagnwei
	} 
}  

//Â†ÜÊéíÂ∫è
void DMHeapSort(vfile_t ** vfilep , int n) 
{ 
	vfile_t * t;
	int i;

	for(i=n/2;i>=0;i--)  
	DMCreatHeap(vfilep,i,n);  
	for(i= n-1;i>=0;i--)  
	{ 
		 t=vfilep[0];  //shagnwei
		vfilep[0]=vfilep[i];  //shagnwei
		vfilep[i]=t;  //shagnwei
		DMCreatHeap(vfilep,0,i-1);  
	}  
	return;  
}  



#endif

//quicksort(r,0,N-1)
//øÏÀŸ≈≈–Ú
void DMQuickSort(vfile_t **  r,int left,int right)
{
   int i,j;
   vfile_t * swap;
   i=left;j=right;
   swap=r[left];
   while(i<j)
   {
	   while((i<j)&&(swap->id<r[j]->id))j--;
	   if(i<j)
	   {
		   printf("DMQ %d ",i);
		   r[i]=r[j];
		   i++;
	   }
	   while((i<j)&&(swap->id>r[i]->id))i++;
	   if(i<j)
	   {
		   r[j]=r[i];
		   j--;
	   }
   }
   r[i]=swap;
   if(i>left)
	   DMQuickSort(r,left,i-1);
   if(i<right)
	   DMQuickSort(r,i+1,right);
   return;
}



//quicksort(r,0,N-1)
//øÏÀŸ≈≈–Ú
void quicksort(int r[],int left,int right)
{
   int i,j;
   int swap;
   i=left;j=right;
   swap=r[left];
   while(i<j)
   {
	   while((i<j)&&(swap<r[j]))j--;
	   if(i<j)
	   {
		   r[i]=r[j];
		   i++;
	   }
	   while((i<j)&&(swap>r[i]))i++;
	   if(i<j)
	   {
		   r[j]=r[i];
		   j--;
	   }
   }
   r[i]=swap;
   if(i>left)
	   quicksort(r,left,i-1);
   if(i<right)
	   quicksort(r,i+1,right);
   return;
}



//∂—≈≈–Úœ»Ω®¡¢∂—
void creatheap(int r[],int i,int n)
{
	int j;
	int t;
	t=r[i];j=2*i; //shagnwei
	while(j<n)
	{
		printf("the r[%d] is %d \n",j+1,r[j+1]);
		if((j<n)&&(r[j]<r[j+1]))j++; //shagnwei
		if(t<r[j]) //shagnwei
		{
			r[i]=r[j]; //shagnwei
			i=j;j=2*i;
		}
		else j=n;
		r[i]=t; //shagnwei
	}
}
//∂—≈≈–Ú
void heapsort(int r[],int n)
{
	int t,i;
	for(i=n/2;i>=0;i--)
		creatheap(r,i,n);
	for(i= n-1;i>=0;i--)
	{
		t=r[0]; //shagnwei
		r[0]=r[i]; //shagnwei
		r[i]=t; //shagnwei
		creatheap(r,0,i-1);
	}
	return;
}

#define N 101
#define M 1

int testsort(void)
{
	int i,j,k;
	int r[N],a[N];
	for(i=0;i<N;i++)
	{
		a[i]=rand()%10000;
	}

for(i=0;i<M;i++)
{
	k=N-1;
	while(k+1)
	{
		r[k]=a[k];
		k--;
	}
//heapsort(r,N);//∂—≈≈–Ú∑®
quicksort(r,0,N-1);

}

for(i=0;i<N;i++)
{
	printf(" %d ",r[i]);
}

printf(" \n");


}

//create a int id by given buf
int DMatoi(const char * buf)
{
	return atoi(buf);
}
//int remove (const char * pathname)
//int rename (const char * oldpath , const char *newpath)
#if  1

time_t DMTimeStr2Sec(char * buf)
{
	struct tm tmDate;
	char tmpbuf[5]={0};
	if(NULL==buf)return -1;
	strncpy(tmpbuf,buf,4);//year
	tmDate.tm_year=DMatoi( tmpbuf)-1970;
	strncpy(tmpbuf,buf+4,2);//month
	tmDate.tm_mon=DMatoi( tmpbuf);
	strncpy(tmpbuf,buf+6,2);//day
	tmDate.tm_mday=DMatoi( tmpbuf);
	strncpy(tmpbuf,buf+8,2);//hour
	tmDate.tm_hour=DMatoi( tmpbuf);
	strncpy(tmpbuf,buf+10,2);//min
	tmDate.tm_min=DMatoi( tmpbuf);
	tmDate.tm_sec=0;
	return mktime(&tmDate);
}


#endif


//
int DMGetTime(vfilelist_p vfilelistp)
{
	char *wday[]={"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};/*÷∏’Î◊÷∑˚ ˝◊È*/
	time_t t;
	struct tm *p;
	
	t=time(NULL);/*ªÒ»°¥”1970ƒÍ1‘¬1»’¡„ ±µΩœ÷‘⁄µƒ√Î ˝£¨±£¥ÊµΩ±‰¡øt÷–*/
	p=gmtime(&t); /*±‰¡øtµƒ÷µ◊™ªªŒ™ µº »’∆⁄ ±º‰µƒ±Ì æ∏Ò Ω*/

	printf("Time info %ld \n",(long int)time);

	printf("%d year %02d month %02d day",(1900+p->tm_year), (1+p->tm_mon),p->tm_mday);
	printf(" %s ", wday[p->tm_wday]);
	printf("%02d:%02d:%02d\n", p->tm_hour, p->tm_min, p->tm_sec);
	sprintf(vfilelistp->CurID,"%d%02d%02d%02d%02d" ,(1900+p->tm_year), (1+p->tm_mon),p->tm_mday, p->tm_hour, p->tm_min);
	printf("create vfile name : %s\n" ,vfilelistp->CurID);
	return 0;
}

//test ok
int xremove(const char  * filename)
{
	printf("del %s  \n",filename);
	return remove(filename);
}

//test ok
int xrename(const char  * oldpath,const char * newpath)
{
	printf("move  %s into %s \n",oldpath,newpath);
	return  rename( oldpath, newpath);
}

//test ok
int runshellcmd(unsigned char * cmd)
{
	//int ret;
	//printf("runshellcmd : %s \n",cmd);
	return system(cmd);
}

//no ok
int DMCheckID(const int * ID)
{
	if((*ID>199000000000) &&(*ID<250000000000))
		return 0;
	return -1;
}

//search by name
vfile_p DMSearch(vfilelist_p vfilelistp , const char * vfilename)
{
	int i;
	vfile_p tmpvfilep;
	tmpvfilep=vfilelistp->vfilehead;
	for(i=0;i<vfilelistp->num-1;i++)//bug warning should not use num to control
	{
		tmpvfilep=tmpvfilep->next_vfile;
		if(NULL==tmpvfilep) return NULL;
		//printf("DMSearch filename is %s \n",tmpvfilep->name);
		if(!strcmp(tmpvfilep->name,vfilename))//if tmpvfilep is NULL,it will be fault
			return tmpvfilep;
	}
	return NULL;//file not exist

}

//test ok
//bug 1:if the vfilep is the endest file it will be segmentation fault
//bug 2:if the file not exist but the list is still in
//bug 3:if a same file in the dir , cover it?
//bug 4:if the vfile was the newest,should also update newestpos
int static DMDeletevFile(vfile_p vfilep)
{
	char fullname[PATHSIZE+NAMESIZE];
	
	if(vfilep->lock)
		return -1;//whether we should unlock the file before del lock file
	vfilep->pre_vfile->next_vfile=vfilep->next_vfile;	
	if (vfilep->next_vfile!=NULL)//if the vfile is the tail of the list
	vfilep->next_vfile->pre_vfile=vfilep->pre_vfile;
	else
		g_vfilelist.vfilenewest=vfilep->pre_vfile;//fix bug 4
	memset(fullname,'\0',sizeof(fullname));
	strcpy(fullname,vfilep->path);
	strcat(fullname,"/");
	strcat(fullname,vfilep->name);
	xremove(fullname);
	free(vfilep);
	return 0;
	//resorting file
}


//test ok extern
int static DMunLockvFile(vfile_p vfilep , char * vfilepath)
{
	int i;
	char vfileoldpath[PATHSIZE+NAMESIZE];
	char vfilenewpath[PATHSIZE+NAMESIZE];
	if(!vfilep->lock)//the vfile is already unlock
	{
		printf("DMLockFile the vfile is already unlock \n");
		return 0;
	}
	strcpy(vfileoldpath,vfilep->path);	
	strcpy(vfilenewpath,vfilepath);
	strcpy(vfilep->path, vfilenewpath );	
	
	strcat(vfileoldpath,"/");
	strcat(vfilenewpath,"/");
	
	strcat(vfileoldpath,vfilep->name);
	strcat(vfilenewpath,vfilep->name);
	xrename(vfileoldpath ,vfilenewpath);
	vfilep->lock=0;
	g_vfilelist.locknum--;
	printf("DMunLockFile %s  unlock success\n",vfilep->name);
	//update earliest vfile
}


//test ok extern
int  DMLockvFile(vfile_p vfilep,char * vfilelockpath)
{
	int ret;
	char vfileoldpath[PATHSIZE+NAMESIZE];
	char vfilenewpath[PATHSIZE+NAMESIZE];
	if(NULL==vfilep)return -1;
	if(vfilep->lock)//the vfile is already lock
	{
		printf("DMLockFile the vfile is already lock \n");
		return 0;
	}
	strcpy(vfileoldpath,vfilep->path);	
	strcat(vfileoldpath,"/");
	
	strcpy(vfilenewpath,vfileoldpath);
	strcat(vfilenewpath,vfilelockpath);
	strcpy(vfilep->path, vfilenewpath );	
	strcat(vfilenewpath,"/");
	
	strcat(vfileoldpath,vfilep->name);
	strcat(vfilenewpath,vfilep->name);
	
	ret=xrename(vfileoldpath ,vfilenewpath);
	//strcpy(vfilep->path, vfilelockpath );	
	vfilep->lock=1;
	g_vfilelist.locknum++;
	printf("DMLockFile %s  lock %d \n",vfilep->name,ret);
	//update earliest vfile
}


//
int static GetENV()
{

}

//
int static SetENV()
{

}



//test ok
int DMFreeVfileArr(vfilelist_p vfilelistp)
{
	 free(vfilelistp->vfileArr);
	 return 0;

}

//test ok
int DMFreeVfileList(vfilelist_p vfilelistp)
{
	int i,ret;
	vfile_p vfilecur,vfilenext;
	//if(vfilelistp->status);
	vfilecur=vfilelistp->vfilehead;
	vfilenext=vfilecur->next_vfile;
	for(i=0;i<vfilelistp->num-1;i++)
	{
		vfilecur=vfilenext;
		vfilenext=vfilenext->next_vfile;
		free(vfilecur);
	}
	vfilelistp->vfilepos=vfilelistp->vfilehead;
	vfilelistp->vfilenewest=vfilelistp->vfilehead;
	vfilelistp->vfileoldest=vfilelistp->vfilehead;
	return ret;
	
}

//test ok
//in : vfilelist 
//out : vfile struct 
int DMGetNewestFile(vfilelist_p vfilelistp)
{
	int i;
	//if(vfilelistp->status);
	//vfilelistp->vfilenewest=vfilelistp->vfileArr[vfilelistp->num-2];
	vfilelistp->vfilenewest=vfilelistp->vfilehead;
	for(i=0;i<vfilelistp->num-1;i++)
	{
		vfilelistp->vfilenewest=vfilelistp->vfilenewest->next_vfile;
		if (vfilelistp->vfilenewest==NULL)
		{
			vfilelistp->vfilenewest=vfilelistp->vfilehead;
			return  -1;//vfilelistp error;			
		}
	}
	return 0;
}

//test ok
//in : vfilelist 
//out : vfile struct 
//test more: if vfilelist is empty
int DMGetEariestFile(vfilelist_p vfilelistp)
{
	//if(vfilelistp->status);
	int i;
	vfilelistp->vfileoldest=NULL;
	vfilelistp->vfilepos=vfilelistp->vfilehead;
	if(vfilelistp->vfilepos->next_vfile==NULL) return -2;//filelist is empty
	for(i=0;i<vfilelistp->num-1;i++)
	{
		vfilelistp->vfilepos=vfilelistp->vfilepos->next_vfile;
		if (!vfilelistp->vfilepos->lock)
		{
			vfilelistp->vfileoldest=vfilelistp->vfilepos;
			return 0;//find the eariestfile	 		
		}
	}
	vfilelistp->vfileoldest=vfilelistp->vfilehead;
	return -1;//not found : all the file was lock  
}

//add vedio file to the list tail
//bug 1:if newest pos was null 
int DMAddvFile(vfilelist_p vfilelistp,vfile_p vfilep)
{

	printf("DMAddvFile\n");
	if (vfilep==NULL)return -1;
	if(NULL==vfilelistp->vfilenewest)return -1;//avoid newest pos should not be NULL(means the list have not init)
	vfilep->next_vfile=NULL;
	vfilep->pre_vfile=vfilelistp->vfilenewest;
	vfilelistp->vfilenewest->next_vfile=vfilep;
	vfilelistp->vfilenewest=vfilep;
	vfilelistp->num++;
	if(1==vfilep->lock)
	{
		printf("DMAddvFile locking file in a wrong way \n");
		vfilelistp->locknum++;
	}
	else if(vfilelistp->lockflag)
	{
		DMLockvFile(vfilep,VideoLockPath);
		if((vfilep->pre_vfile!=vfilelistp->vfilehead))
		DMLockvFile(vfilep->pre_vfile,VideoLockPath);
	}
}

//filename is full name with path
int DMAddFile(const char *filename)
{
	int i;
	int len;
	
	vfile_p vfileptmp;
	if(NULL==filename)return -1;//filename null 
	len=strlen(filename);
	if(!len)return -1;//filename is empty
	for(i=0;i<len;i++)
	{
		if('/'==filename[len-1-i])
			break;
	}
	printf("filename len is %d\n",i);
	if(0==i)return -1;//only a path name
	vfileptmp=(vfile_p)malloc(sizeof(vfile_t)*1);
	
	memset(vfileptmp->path,'\0',PATHSIZE);
	memset(vfileptmp->name,'\0',NAMESIZE);
	strncpy(vfileptmp->path,filename,len-i);
	strncpy(vfileptmp->name,&filename[len-i],i);
	vfileptmp->id=DMatoi(vfileptmp->name);
	printf("DMAddFile name is %s path is %s\n",vfileptmp->name,vfileptmp->path);
	DMAddvFile(&g_vfilelist,vfileptmp);
	//sync
	//runshellcmd("sync");
	//echo 3 > /proc/sys/vm/drop_caches
	//runshellcmd("echo 3 > /proc/sys/vm/drop_caches");
	return 0;
}



int DMGetLockVfile(vfilelist_p vfilelistp)
{



}


int DMReAllocateFileList(vfilelist_p vfilelistp)
{
	int i;
	vfilelistp->vfilepos=vfilelistp->vfilehead;
	for (i=0;i<vfilelistp->num-1;i++)
	{
		vfilelistp->vfileArr[i]->pre_vfile=vfilelistp->vfilepos;
		vfilelistp->vfilepos->next_vfile=vfilelistp->vfileArr[i];
		vfilelistp->vfilepos=vfilelistp->vfileArr[i];
		printf("DMReAllocateFileList  vfilepos %d is   %s  vfileArr[%d] is %s \n",i,vfilelistp->vfilepos->name,i,vfilelistp->vfileArr[i]->name);
	}
	vfilelistp->vfilepos->next_vfile=NULL;//bug,will return from end if without this step
}

//test ok
int DMGetFileList(char *path,char * lockpath ,vfilelist_p vfilelistp)
{
    DIR *dir;
    struct dirent *ptr;
    char base[1024];
    vfile_p vfileptmp ;//=vfilep;
    if ((dir=opendir(path)) == NULL)
    {
        perror("Open dir error...");
        return -1;//open dir fail
    }

    while ((ptr=readdir(dir)) != NULL)
    {
        if(strcmp(ptr->d_name,".")==0 || strcmp(ptr->d_name,"..")==0)    ///current dir OR parrent dir
            continue;
        else if(ptr->d_type == 8){      ///file

		
		vfileptmp = (vfile_p)malloc(sizeof(vfile_t)*1);
		memcpy(vfileptmp->path,path,PATHSIZE);
		memcpy(vfileptmp->name,ptr->d_name,NAMESIZE);
		vfileptmp->lock=lockpathflag;
		vfileptmp->id = DMatoi(&(vfileptmp->name[4]));//start from month
		
		DMAddvFile(vfilelistp,vfileptmp);
		printf("filename :%s/%s list num is  %d  file ID is %d\n",path , ptr->d_name , vfilelistp->num,vfileptmp->id);//create_list(list_head, ptr->d_name, 0, 0);

				
	  
        }
        else if(ptr->d_type == 10)    ///link file
            printf("link file name:%s/%s\n",path,ptr->d_name);
        else if(ptr->d_type == 4)      ///dir
        {
		printf("dir file name:%s/%s\n",path,ptr->d_name);
		memset(base,'\0',sizeof(base));
		strcpy(base,path);
		strcat(base,"/");
		strcat(base,ptr->d_name);
		if (!strcmp(ptr->d_name,lockpath))
		{
			lockpathflag=1;
		}


		printf("going into dir %s  num is %d vfilep addr is  %s  \n ",ptr->d_name, vfilelistp->num,vfilelistp->vfilepos->name);
		DMGetFileList(base, lockpath ,vfilelistp);
		printf("return from dir %s num is %d vfilep addr is  %s  \n",ptr->d_name, vfilelistp->num,vfilelistp->vfilepos->name);
        }
    }
	lockpathflag=0;
	closedir(dir);
    return 0;

}

//
int DMGetConfig()
{

}

//test ok
//bug : if there is not file the arr is empty
int DMCreatevFileArr(vfile_t ***  vfileArr,vfile_p vfileHead , int num)
{
	int i;
	//printf("befor creat %x \n",(int)*vfileArr);
	*vfileArr=(vfile_t **) malloc ( sizeof(vfile_t *) *num );
	//printf("after creat %x \n",(int)*vfileArr);
	for(i=0;i<num;i++)
	{
		if(vfileHead->next_vfile==NULL) return  -1;//vfilelist is broken
		vfileHead=vfileHead->next_vfile;
		(*vfileArr)[i]=vfileHead;
		//printf("%d filename is :%s\n",i,(*vfileArr)[i]-> name);
	}
	return 0;

}



/*
int GetConfig(void)
{
	int size,i,ret;

	FILE *fp=(FILE *)malloc(sizeof(FILE *));
	char buf[CapBufSize];
	memset(buf,0,sizeof(buf));
	memset(buf,0xff,sizeof(buf));
	memset(buf,0xff,sizeof(buf));

	fp=fopen("/home/dev/test","rb");
	if(fp==NULL)
	{
		printf("cant open the file");
		exit(0);
	}
	for(i=0;i<10;i++)
	{  
		ret=fread(buf,sizeof(char),sizeof(buf),fp);
		//if(ret!=1)
		//printf("file read error\n");
		size=987;
		size=atoi(buf);
		printf("the disk size is %dMB\n",size);
	}
 	fclose(fp);
}
*/


//test base on demo
int DMCheckDisk(void)
{
	int size,i,ret;
	char buf[CapBufSize];
	FILE *stream;
	char cmdBuf[128];

	//get total disk size
	memset(buf,'\0',sizeof(buf));	
	memset(cmdBuf,'\0',sizeof(buf));
#if _NFS_
	sprintf(cmdBuf,"df |grep %s |awk '{print $1}' ",VideoDevice);//for nfs
#else
	sprintf(cmdBuf,"df |grep %s |awk '{print $2}' ",VideoDevice);
#endif
	printf("DMCheckDisk cmdline is %s\n",cmdBuf);
	stream=popen(cmdBuf,"r");//
	if(NULL==stream)
	{
		printf("DMCheckDisk popen err\n");
		return -1;
	}
	ret=fread(buf,sizeof(char),sizeof(buf),stream);	
	size=DMatoi(buf);
	g_vfilelist.DiskTotal=size/1024;//translate to MB directly to avoid thread change
	printf("the disk size total is %d KB\n",size);
	fclose(stream);

	//get available disk size
	memset(buf,'\0',sizeof(buf));	
	memset(cmdBuf,'\0',sizeof(buf));
#if _NFS_
	sprintf(cmdBuf,"df |grep %s |awk '{print $3}' ",VideoDevice);//for nfs
#else
	sprintf(cmdBuf,"df |grep %s |awk '{print $4}' ",VideoDevice);
#endif
	printf("DMCheckDisk cmdline is %s\n",cmdBuf);
	stream=popen(cmdBuf,"r");
	
	if(NULL==stream)
	{
		printf("DMCheckDisk popen err\n");
		return -1;
	}
	ret=fread(buf,sizeof(char),sizeof(buf),stream);
	size=DMatoi(buf);
	g_vfilelist.DiskAvailable=size/1024;//translate to MB directly to avoid thread change
	printf("the disk Available size is %d KB\n",size);
	fclose(stream);
	if(size>AllocateSize)
	{
		printf("the capacity is enough!\n");
		ret = 0;
	}
	else
	{
		printf("allocating more space!\n");
		ret = -1;
	}	
	
	return ret;
}



int dmprintenv(void)
{
	//ÊâìÂç∞ÂÖ®ÈÉ®ÁéØÂ¢ÉÂèòÈáè
	printf("ÊâìÂç∞ÂÖ®ÈÉ®ÁéØÂ¢ÉÂèòÈáè\n");
	char **env = environ;
	while (*env)
	{
		printf("%s\n", *env);
		env++;
	}
}

int dmsetenv(void)
{
	int ret;
	char * envx;
	//envx=getenv("shagnwei");
	//ret=unsetenv("shagnwei");
	ret=setenv("shagnwei","test success",1);

}

//devÁõëÊéßÁöÑËÆæÂ§á,
//path ÁõëÊéßÁöÑË∑ØÂæÑ
//lockÈîÅÂÆöÁöÑË∑ØÂæÑ
int DMdiskwatch(char* dev,char * path,char * lock)
{


}


//create a file name base on date
//extern
int DMCreateID(int date , int channel , vfilelist_p vfilelistp)
{
	int tmpid,min_i;
	char min_s[20];
	printf("DMCreateID\n");
	DMGetTime(vfilelistp);
	/*
	if(NULL!=vfilelistp->vfilenewest)
	{
		tmpid=DMatoi(vfilelistp->CurID+4);
		if(tmpid>vfilelistp->vfilenewest->id)
		{
			printf("ID is correct\n");
		}
		else
		{
			tmpid=vfilelistp->vfilenewest->id+1;
			itoa(vfilelistp->vfilenewest->id,min_s,10);
			printf("itoa by newest->id is :%s",min_s);
			itoa(tmpid,min_s,10);
			printf("itoa by tmpid is :%s",min_s);
			strncpy(vfilelistp->CurID,vfilelistp->vfilenewest->name,10);
			min_i=DMatoi(vfilelistp->vfilenewest->name+10);
			min_i++;
			itoa(min_i,min_s);
			strcat(vfilelistp->CurID,min_s);
			printf("ID is error, set to %s \n",vfilelistp->CurID);
		}
	}*/
	return DMAllocateDisk();

}
char *chname[]={"_F","_R","_B","_L"};

//channel is :0 1 2 3
int DMCreateFile(char *buf,int channel)
{
	if(NULL==buf)return -1;
	if(DMCreateID(0,0,&g_vfilelist))return -1;
	sprintf(buf,"%s/%s%s.mp4",VideoPath,g_vfilelist.CurID,chname[channel]);
	return 0;
}

//record the video file in the vfile list
//extern
int DMfclose(vfilelist_p vfilelistp,vfile_p vfilep)
{
	DMAddvFile(vfilelistp,vfilep);
	DMAllocateDisk();
}


int DMDelEariestFiles(vfilelist_p vfilelistp)
{
	int tmpID;
	vfile_p tmpvfilep;

	if(DMGetEariestFile(vfilelistp))
		return -1;//all file was lock or the filelist is empty
	tmpID=vfilelistp->vfileoldest->id;
	do
	{
		//tmpvfilep=vfilelistp->vfileoldest->next_vfile;
		//if(vfilelistp->)
		
		printf("delete id is %d\n",vfilelistp->vfileoldest->id);
		DMDeletevFile(vfilelistp->vfileoldest);
		vfilelistp->num--;
		if(DMGetEariestFile(vfilelistp))
			return -1;//all file was lock after del the first file
	}while(vfilelistp->vfileoldest->id==tmpID);
	return 0;
}


//return must get enough memory or return disk not enough
int DMAllocateDisk(void)
{

	while (DMCheckDisk()) // -1 disk not enough
	{
		if(DMDelEariestFiles(&g_vfilelist))
		return  -1;//can not got enough space after del the eariest files 
	}
	return 0;//got enough disk space
	

}

//may be multi call
int DMLockFile(const char * filename)
{
	vfile_p tmpvfile;
	tmpvfile=DMSearch(&g_vfilelist,filename);
	if(tmpvfile==NULL)return -1;
	DMLockvFile(tmpvfile,VideoLockPath);
	return 0;
	
}
int DMunLockFile(const char * filename)
{
	vfile_p tmpvfile;
	tmpvfile=DMSearch(&g_vfilelist,filename);
	if(tmpvfile==NULL)return -1;
	DMunLockvFile(tmpvfile,VideoPath);
	return 0;

}
int DMDelFile(const char * filename)
{
	int ret;
	vfile_p tmpvfile;
	printf("DMDelFile strat\n");
	tmpvfile=DMSearch(&g_vfilelist,filename);
	if(tmpvfile==NULL)return -1;
	ret=DMDeletevFile(tmpvfile);
		printf("DMDeletevFile return is %d\n",ret);
	if(!ret)//del no err
	g_vfilelist.num--;//if file not exist delete it
	printf("DMDelFile done\n");
	return 0;
}
//given a file name without path
int DMGetFullName(char *name)
{
	vfile_p tmpvfile;
	tmpvfile=DMSearch(&g_vfilelist,name);
	if(tmpvfile==NULL)
		return -1;//file not exist
	sprintf(name,"%s/%s",tmpvfile->path,tmpvfile->name);//replace, need err check ?
	printf("DMGetFullName is %s\n",name);
	return 0;

}

int DMCheckByName(const char *name)
{
	if(DMSearch(&g_vfilelist,name)==NULL)
	return  -1;
	return 0;
}

int DMGetAvailable(void)
{
	return g_vfilelist.DiskAvailable;

}
int DMGetTotal(void)
{
	return g_vfilelist.DiskTotal;
}

//should not use for DMReadlist ending decide
int DMOpenList(void)
{
	g_vfilelist.vfilereadpos=g_vfilelist.vfilehead->next_vfile;
	printf("DMOpenList Done, g.num = %d \n",(g_vfilelist.num-1));
	return (g_vfilelist.num-1);
}

//return a filename
int DMReadList( char * buf)
{
	if(buf==NULL)
	{
		printf("DMReadList buf giver err\n");
		return -1;//buf should not be null
	}
	if(g_vfilelist.vfilereadpos==NULL)
	{
		printf("DMReadList vfilereadpos null\n");
		return -1;//list end
	}
	strncpy(buf,g_vfilelist.vfilereadpos->name, NAMESIZE);
	
	printf("DMReadList filename is :%s\n",buf);
	g_vfilelist.vfilereadpos=g_vfilelist.vfilereadpos->next_vfile;
	return 0;
}


int DMPrintList(vfilelist_p vfilelistp)
{	
	int i;
	vfile_p vfileptmp;
	vfileptmp=vfilelistp->vfilehead;
	printf("DMPrintList file num is %d \n",vfilelistp->num-1);
	for(i=0;i<vfilelistp->num-1;i++)
	{
		vfileptmp=vfileptmp->next_vfile;
		printf("DMPrintList %d file is:%s/%s\n",i,vfileptmp->path,vfileptmp->name);
	}
	printf("DMPrintList end address  is %d \n",(int)vfileptmp->next_vfile);
}

int DMDataInit(void)
{
	//vfilelist_t g_
	//vfile_t
	g_vfilelist.num=1;
	g_vfilelist.status=0;
	g_vfilelist.vfileArr=NULL;
	g_vfile_head.id=0;
	g_vfile_head.next_vfile=NULL;
	g_vfile_head.pre_vfile=NULL;
	//g_vfilelist.vfileArr=(vfile_p)malloc(sizeof(vfile_t)*g_vfilelist.num);
	g_vfilelist.vfilepos=&g_vfile_head;
	g_vfilelist.vfilehead=&g_vfile_head;
	g_vfilelist.vfilereadpos=&g_vfile_head;
	g_vfilelist.vfilenewest=&g_vfile_head;
	g_vfilelist.vfileoldest=&g_vfile_head;
	strcpy(g_vfilelist.lockpath,VideoLockPath);
	strcpy(g_vfilelist.path,VideoLockPath);
	lockpathflag=0;
}

//extern for disk manager start
//something err
int DMInit(void)
{
	int ret;
	char buf[NAMESIZE+PATHSIZE];
	
	printf("DMInit start\n");
	DMDataInit();
	
	ret =DMGetFileList(VideoPath , VideoLockPath ,  &g_vfilelist );
	printf("DMGetFileList return is %d\n",ret);
	if(ret)
	{
		printf("DMGetFileList dir  is not exit\n");
		return -1;//DMInit fail
	}
	if(g_vfilelist.num>1)
	{
		ret = DMCreatevFileArr(&g_vfilelist.vfileArr , &g_vfile_head, (g_vfilelist.num-1) );
		printf("DMCreatevFileArr return is %d\n",ret);
		DMQuickSort( g_vfilelist.vfileArr, 0 , g_vfilelist.num-2);
		printf("DMQuickSort success\n");
		DMReAllocateFileList(&g_vfilelist);
		printf("DMReAllocateFileList success\n");
		DMGetEariestFile(&g_vfilelist);
		printf("DMGetEariestFile success\n");
		printf("the oldest file id is :%d  \n",g_vfilelist.vfileoldest->id);
		DMGetNewestFile(&g_vfilelist);
		printf("the newest file id is :%d  \n",g_vfilelist.vfilenewest->id);
		
	}
	DMCheckDisk();
	//DMFreeVfileArr(&g_vfilelist);
	printf("DMReadlist ending test\n");
	DMOpenList();
	while(!DMReadList(buf))//bug,will return from end
	{
	}	
	printf("DMReadlist ending address is %d\n",(int)g_vfilelist.vfilereadpos);
	printf("DMInit success\n");
	
}

#if  _INDEPENDENT_
int main(int *argc,char ** argv)
#else 
int DMMain(int *argc,char ** argv)
#endif
{
	int ret,i;
	vfile_p tmpvfilep;
	char fullname[50];
	
	char buf[NAMESIZE+PATHSIZE];
	//testsort();
	strcpy(fullname,"197001010165_F.h264");
	DMInit();
	//DMCreateID(0,0,&g_vfilelist);
	DMPrintList(&g_vfilelist);
	DMGetFullName(fullname);
	printf("DMGetFullName return is:%s\n",fullname);

	DMCreateFile( buf,1);
	printf("after DMCreateFile buf name is :%s\n",buf);
	tmpvfilep = (vfile_p)malloc(sizeof(vfile_t)*1);
	memcpy(tmpvfilep->path,"/DVR/OUTPUT",PATHSIZE);
	memcpy(tmpvfilep->name,"197005060102_B.mp4",NAMESIZE);
	tmpvfilep->lock=1;
	tmpvfilep->id = DMatoi(&(tmpvfilep->name[4]));//start from month
	
	DMAddvFile(&g_vfilelist,tmpvfilep);


	printf("DMReadlist ending test\n");
	DMOpenList();
	while(!DMReadList(buf))//bug,will return from end
	{
	}
	
	printf("DMReadlist ending address is %d\n",(int)g_vfilelist.vfilereadpos);
	
	DMPrintList(&g_vfilelist);
	DMDelFile("197001010101_F.mp4");
	DMDelFile("197001010101_F.mp4");
#if  0
	dmsetenv();
	dmprintenv();
	runshellcmd();
	//GetConfig();

	DMCheckDisk();
	

	//DMdiskwatch("/home/dev","/home/dev/vfilepath","/home/dev/vfilepath/lock");
	
	ret =DMGetFileList("/DVR/OUTPUT" , "lock" ,  &g_vfilelist );
	printf("DMGetFileList return is %d\n" , ret);
	
	tmpvfilep=&g_vfile_head;
	if (g_vfile_head.next_vfile!=NULL )
		{
		/*
		printf("  after DMGetFileList \n");
		for(i=0;i<g_vfilelist.num-1;i++)
		{
			tmpvfilep=tmpvfilep->next_vfile;
			printf(" %d is %s  \n",i,tmpvfilep->name);
		}
		
		*/
		printf(" in %x \n",(int)g_vfilelist.vfileArr);
		ret = DMCreatevFileArr(&g_vfilelist.vfileArr , &g_vfile_head, (g_vfilelist.num-1) );
		printf("DMCreatevFileArr return is %d\n" , ret);
		printf("out %x \n",(int)g_vfilelist.vfileArr);

		
		printf("befor sorting \n");
		for(i=0;i<g_vfilelist.num-1;i++)
		{
			//tmpvfilep=tmpvfilep->next_vfile;
			//g_vfilelist.vfileArr[i]=tmpvfilep;
			printf("%d outprint filename is :%s  :  ",i,g_vfilelist.vfileArr[i]->name);
			printf("  %d  ",g_vfilelist.vfileArr[i]->id);
			printf("  %d  \n",g_vfilelist.vfileArr[i]->lock);
		}

		//DMHeapSort( g_vfilelist.vfileArr,g_vfilelist.num-1);
		DMQuickSort( g_vfilelist.vfileArr, 0 , g_vfilelist.num-2);

		printf("after sorting \n");
		for(i=0;i<g_vfilelist.num-1;i++)
		{
			//tmpvfilep=tmpvfilep->next_vfile;
			//g_vfilelist.vfileArr[i]=tmpvfilep;
			printf("%d outprint filename is :%s  :  ",i,g_vfilelist.vfileArr[i]->name);
			printf("  %d  ",g_vfilelist.vfileArr[i]->id);
			printf("  %d  \n",g_vfilelist.vfileArr[i]->lock);
		}

		
		DMReAllocateFileList(&g_vfilelist);
		DMGetEariestFile(&g_vfilelist);
		DMGetNewestFile(&g_vfilelist);
		printf("the oldest file id is :%d  \n",g_vfilelist.vfileoldest->id);
		printf("the newest file id is :%d  \n",g_vfilelist.vfilenewest->id);
		
		printf("after DMReAllocateFileList \n");

		
		tmpvfilep=&g_vfile_head;
		for(i=0;i<g_vfilelist.num-1;i++)
		{
			tmpvfilep=tmpvfilep->next_vfile;
			printf(" %d is %s  \n",i,tmpvfilep->name);
		}	
		//printf(" before DMDelEariestFiles  num is %d  \n",g_vfilelist.num);
		
		//printf(" before DMDelEariestFiles  oldest name  is %s  \n",g_vfilelist.vfileoldest->name);
		ret=0;
		//ret=DMDelEariestFiles(&g_vfilelist);
		//ret=DMDelEariestFiles(&g_vfilelist);
		//printf(" after DMDelEariestFiles  ret is %d  \n",ret);
		//printf(" after DMDelEariestFiles  num is %d  \n",g_vfilelist.num);
		//printf(" after DMDelEariestFiles  oldest name  is %s  \n",g_vfilelist.vfileoldest->name);
		printf(" DMLockFile  test locking all file \n");
		tmpvfilep=&g_vfile_head;

		for(i=0;i<g_vfilelist.num-1;i++)
		{
			tmpvfilep=tmpvfilep->next_vfile;
			DMLockFile(tmpvfilep,"lock");
		}

		printf(" DMunLockFile  test unlocking all file \n");
		tmpvfilep=&g_vfile_head;

		for(i=0;i<g_vfilelist.num-1;i++)
		{
			tmpvfilep=tmpvfilep->next_vfile;
			DMunLockFile(tmpvfilep,"/DVR/OUTPUT");
		}

		}
	else 
	{
		printf("the file list is empty\n");
	}

	DMGetTime(&g_vfilelist);
	xrename("/home/dev/rename","/home/dev/ltib/rename");


	xremove("/home/dev/test");
	DMFreeVfileArr(&g_vfilelist);
	DMFreeVfileList(&g_vfilelist);
	#endif
	//printf("the test  file name is :%s \n",g_vfilelist.vfilehead->next_vfile->name);
	return ret;

	
}



