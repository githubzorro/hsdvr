/*
 * Copyright 2004-2013 Freescale Semiconductor, Inc.
 *
 * Copyright (c) 2006, Chips & Media.  All rights reserved.
 */

/*
 * The code contained herein is licensed under the GNU General Public
 * License. You may obtain a copy of the GNU General Public License
 * Version 2 or later at the following locations:
 *
 * http://www.opensource.org/licenses/gpl-license.html
 * http://www.gnu.org/copyleft/gpl.html
 */

#include <sys/stat.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <getopt.h>
#include "vpu_test.h"

#include "main.h"
#include "dm.h" //for disk manager funtion
#include "uart_com.h" 
#include "key.h" //for key funtion

#define ONE_FRAME_INTERV 100000 // 100 ms

config_t g_config;


char *usage = "Usage: ./mxc_vpu_test.out -D \"<decode options>\" "\
	       "-E \"<encode options>\" "\
	       "-L \"<loopback options>\" -C <config file> "\
	       "-T \"<transcode options>\" "\
	       "-H display this help \n "
	       "\n"\
	       "decode options \n "\
	       "  -i <input file> Read input from file \n "\
	       "	If no input file is specified, default is network \n "\
	       "  -o <output file> Write output to file \n "\
	       "	If no output is specified, default is LCD \n "\
	       "  -x <output method> output mode V4l2(0) or IPU lib(1) \n "\
	       "        0 - V4L2 of FG device, 1 - IPU lib path \n "\
	       "        Other value means V4L2 with other video node\n "\
	       "        16 - /dev/video16, 17 - /dev/video17, and so on \n "\
	       "  -f <format> 0 - MPEG4, 1 - H.263, 2 - H.264, 3 - VC1, \n "\
	       "	4 - MPEG2, 5 - DIV3, 6 - RV, 7 - MJPG, \n "\
	       "        8 - AVS, 9 - VP8\n "\
	       "	If no format specified, default is 0 (MPEG4) \n "\
	       "  -l <mp4Class / h264 type> \n "\
	       "        When 'f' flag is 0 (MPEG4), it is mp4 class type. \n "\
	       "        0 - MPEG4, 1 - DIVX 5.0 or higher, 2 - XVID, 5 - DIVX4.0 \n "\
	       "        When 'f' flag is 2 (H.264), it is h264 type. \n "\
	       "        0 - normal H.264(AVC), 1 - MVC \n "\
	       "  -p <port number> UDP port number to bind \n "\
	       "	If no port number is secified, 5555 is used \n "\
	       "  -c <count> Number of frames to decode \n "\
	       "  -d <deblocking> Enable deblock - 1. enabled \n "\
	       "	default deblock is disabled (0). \n "\
	       "  -e <dering> Enable dering - 1. enabled \n "\
	       "	default dering is disabled (0). \n "\
	       "  -r <rotation angle> 0, 90, 180, 270 \n "\
	       "	default rotation is disabled (0) \n "\
	       "  -m <mirror direction> 0, 1, 2, 3 \n "\
	       "	default no mirroring (0) \n "\
	       "  -u <ipu rotation> Using IPU rotation for display - 1. IPU rotation \n "\
	       "        default is VPU rotation(0).\n "\
	       "        This flag is effective when 'r' flag is specified.\n "\
	       "  -v <vdi motion> set IPU VDI motion algorithm l, m, h.\n "\
	       "	default is m-medium. \n "\
	       "  -w <width> display picture width \n "\
	       "	default is source picture width. \n "\
	       "  -h <height> display picture height \n "\
	       "	default is source picture height \n "\
	       "  -j <left offset> display picture left offset \n "\
	       "	default is 0. \n "\
	       "  -k <top offset> display picture top offset \n "\
	       "	default is 0 \n "\
	       "  -a <frame rate> display framerate \n "\
	       "	default is 30 \n "\
	       "  -t <chromaInterleave> CbCr interleaved \n "\
	       "        default is interleave(1). \n "\
	       "  -s <prescan/bs_mode> Enable prescan in decoding on i.mx5x - 1. enabled \n "\
	       "        default is disabled. Bitstream mode in decoding on i.mx6  \n "\
	       "        0. Normal mode, 1. Rollback mode \n "\
	       "        default is enabled. \n "\
	       "  -y <maptype> Map type for GDI interface \n "\
	       "        0 - Linear frame map, 1 - frame MB map, 2 - field MB map \n "\
	       "        default is 0. \n "\
	       "\n"\
	       "encode options \n "\
	       "  -i <input file> Read input from file (yuv) \n "\
	       "	If no input file specified, default is camera \n "\
	       "  -x <input method> input mode V4L2 with video node \n "\
	       "        0 - /dev/video0, 1 - /dev/video1, and so on \n "\
	       "  -o <output file> Write output to file \n "\
	       "	This option will be ignored if 'n' is specified \n "\
	       "	If no output is specified, def files are created \n "\
	       "  -n <ip address> Send output to this IP address \n "\
	       "  -p <port number> UDP port number at server \n "\
	       "	If no port number is secified, 5555 is used \n "\
	       "  -f <format> 0 - MPEG4, 1 - H.263, 2 - H.264, 7 - MJPG \n "\
	       "	If no format specified, default is 0 (MPEG4) \n "\
	       "  -l <h264 type> 0 - normal H.264(AVC), 1 - MVC\n "\
	       "  -c <count> Number of frames to encode \n "\
	       "  -r <rotation angle> 0, 90, 180, 270 \n "\
	       "	default rotation is disabled (0) \n "\
	       "  -m <mirror direction> 0, 1, 2, 3 \n "\
	       "	default no mirroring (0) \n "\
	       "  -w <width> capture image width \n "\
	       "	default is 176. \n "\
	       "  -h <height>capture image height \n "\
	       "	default is 144 \n "\
	       "  -b <bitrate in kbps> \n "\
	       "	default is auto (0) \n "\
	       "  -g <gop size> \n "\
	       "	default is 0 \n "\
	       "  -t <chromaInterleave> CbCr interleaved \n "\
	       "        default is interleave(1). \n "\
	       "  -q <quantization parameter> \n "\
	       "	default is 20 \n "\
	       "  -a <frame rate> capture/encode framerate \n "\
	       "	default is 30 \n "\
	       "  -L <left> output to display left \n "\
	       "	 default is 0. \n "\
	       "  -T <top> output to display top \n "\
	       "	  default is 0. \n "\
	       "  -W <width> output to display width \n "\
	       "	  default is 720. \n "\
	       "  -H <height> output to display height \n "\
	       "	  default is 480. \n "\
	       "\n"\
	       "loopback options \n "\
	       "  -x <input method> input mode V4L2 with video node \n "\
	       "        0 - /dev/video0, 1 - /dev/video1, and so on \n "\
	       "  -f <format> 0 - MPEG4, 1 - H.263, 2 - H.264, 7 - MJPG \n "\
	       "	If no format specified, default is 0 (MPEG4) \n "\
	       "  -w <width> capture image width \n "\
	       "	default is 176. \n "\
	       "  -h <height>capture image height \n "\
	       "	default is 144 \n "\
	       "  -t <chromaInterleave> CbCr interleaved \n "\
               "        default is interleave(1). \n "\
	       "  -a <frame rate> capture/encode/display framerate \n "\
	       "	default is 30 \n "\
	       "\n"\
	       "transcode options, encoder set to h264 720p now \n "\
	       "  -i <input file> Read input from file \n "\
	       "        If no input file is specified, default is network \n "\
	       "  -o <output file> Write output to file \n "\
	       "        If no output is specified, default is LCD \n "\
	       "  -x <output method> V4l2(0) or IPU lib(1) \n "\
	       "  -f <format> 0 - MPEG4, 1 - H.263, 2 - H.264, 3 - VC1, \n "\
	       "        4 - MPEG2, 5 - DIV3, 6 - RV, 7 - MJPG, \n "\
	       "        8 - AVS, 9 - VP8\n "\
	       "        If no format specified, default is 0 (MPEG4) \n "\
	       "  -l <mp4Class / h264 type> \n "\
	       "        When 'f' flag is 0 (MPEG4), it is mp4 class type. \n "\
	       "        0 - MPEG4, 1 - DIVX 5.0 or higher, 2 - XVID, 5 - DIVX4.0 \n "\
	       "        When 'f' flag is 2 (H.264), it is h264 type. \n "\
	       "        0 - normal H.264(AVC), 1 - MVC \n "\
	       "  -p <port number> UDP port number to bind \n "\
	       "        If no port number is secified, 5555 is used \n "\
	       "  -c <count> Number of frames to decode \n "\
	       "  -d <deblocking> Enable deblock - 1. enabled \n "\
	       "        default deblock is disabled (0). \n "\
	       "  -e <dering> Enable dering - 1. enabled \n "\
	       "        default dering is disabled (0). \n "\
	       "  -r <rotation angle> 0, 90, 180, 270 \n "\
	       "        default rotation is disabled (0) \n "\
	       "  -m <mirror direction> 0, 1, 2, 3 \n "\
	       "        default no mirroring (0) \n "\
	       "  -u <ipu rotation> Using IPU rotation for display - 1. IPU rotation \n "\
	       "        default is VPU rotation(0).\n "\
	       "        This flag is effective when 'r' flag is specified.\n "\
	       "  -v <vdi motion> set IPU VDI motion algorithm l, m, h.\n "\
	       "        default is m-medium. \n "\
	       "  -w <width> display picture width \n "\
	       "        default is source picture width. \n "\
	       "  -h <height> display picture height \n "\
	       "        default is source picture height \n "\
	       "  -j <left offset> display picture left offset \n "\
	       "        default is 0. \n "\
	       "  -k <top offset> display picture top offset \n "\
	       "        default is 0 \n "\
	       "  -a <frame rate> display framerate \n "\
	       "        default is 30 \n "\
	       "  -t <chromaInterleave> CbCr interleaved \n "\
	       "        default is interleave(1). \n "\
	       "  -s <prescan/bs_mode> Enable prescan in decoding on i.mx5x - 1. enabled \n "\
	       "        default is disabled. Bitstream mode in decoding on i.mx6  \n "\
	       "        0. Normal mode, 1. Rollback mode \n "\
	       "        default is enabled. \n "\
	       "  -y <maptype> Map type for GDI interface \n "\
	       "        0 - Linear frame map, 1 - frame MB map, 2 - field MB map \n "\
	       "  -q <quantization parameter> \n "\
	       "	default is 20 \n "\
	       "\n"\
	       "config file - Use config file for specifying options \n";


sigset_t sigset;
int quitflag;

struct input_argument input_arg[MAX_NUM_INSTANCE];
static int instance;
static int using_config_file;//if the param give is error it will be set 

int vpu_test_dbg_level;

int decode_test(void *arg);
int encode_test(void *arg);
int transcode_test(void *arg);

/* Encode or Decode or Loopback */
static char *mainopts = "HE:D:L:T:C:";

/* Options for encode and decode */
//static char *options = "i:o:x:n:p:r:f:c:w:h:g:b:d:e:m:u:t:s:l:j:k:a:v:y:q:";
static char *options = "i:o:x:n:p:r:f:c:w:h:g:b:d:e:m:u:t:s:l:j:k:a:v:y:q:L:T:W:H:";  //qiang_debug modified

int
parse_config_file(char *file_name)
{
	FILE *fp;
	char line[MAX_PATH];
	char *ptr;
	int end;

	fp = fopen(file_name, "r");
	if (fp == NULL) {
		err_msg("Failed to open config file\n");
		return -1;
	}

	while (fgets(line, MAX_PATH, fp) != NULL) {
		if (instance > MAX_NUM_INSTANCE) {
			err_msg("No more instances!!\n");
			break;
		}

		ptr = skip_unwanted(line);
		end = parse_options(ptr, &input_arg[instance].cmd,
					&input_arg[instance].mode);
		if (end == 100) {
			instance++;
		}
	}

	fclose(fp);
	return 0;
}

int
parse_main_args(int argc, char *argv[])
{
	int status = 0, opt;

	do {
		opt = getopt(argc, argv, mainopts);
		switch (opt)
		{
		case 'D':
			input_arg[instance].mode = DECODE;
			strncpy(input_arg[instance].line, argv[0], 26);
			strncat(input_arg[instance].line, " ", 2);
			strncat(input_arg[instance].line, optarg, 200);
			instance++;
			break;
		case 'E':
			input_arg[instance].mode = ENCODE;
			strncpy(input_arg[instance].line, argv[0], 26);
			strncat(input_arg[instance].line, " ", 2);
			strncat(input_arg[instance].line, optarg, 200);
			instance++;
			break;
		case 'L':
			input_arg[instance].mode = LOOPBACK;
			strncpy(input_arg[instance].line, argv[0], 26);
			strncat(input_arg[instance].line, " ", 2);
			strncat(input_arg[instance].line, optarg, 200);
			instance++;
			break;
                case 'T':
                        input_arg[instance].mode = TRANSCODE;
                        strncpy(input_arg[instance].line, argv[0], 26);
                        strncat(input_arg[instance].line, " ", 2);
                        strncat(input_arg[instance].line, optarg, 200);
                        instance++;
                        break;
		case 'C':
			if (instance > 0) {
			 	warn_msg("-C option not selected because of"
							"other options\n");
				break;
			}

			if (parse_config_file(optarg) == 0) {
				using_config_file = 1;
			}

			break;
		case -1:
			break;
		case 'H':
		default:
			status = -1;
			break;
		}
	} while ((opt != -1) && (status == 0) && (instance < MAX_NUM_INSTANCE));

	optind = 1;
	return status;
}

int
parse_args(int argc, char *argv[], int i)
{
	int status = 0, opt, val;

	input_arg[i].cmd.chromaInterleave = 1;
	if (cpu_is_mx6x())
		input_arg[i].cmd.bs_mode = 1;

	do {
		opt = getopt(argc, argv, options);
		switch (opt)
		{
		case 'i':
			strncpy(input_arg[i].cmd.input, optarg, MAX_PATH);
			input_arg[i].cmd.src_scheme = PATH_FILE;
			break;
		case 'o':
			if (input_arg[i].cmd.dst_scheme == PATH_NET) {
				warn_msg("-o ignored because of -n\n");
				break;
			}
			strncpy(input_arg[i].cmd.output, optarg, MAX_PATH);
			input_arg[i].cmd.dst_scheme = PATH_FILE;
			break;
		case 'x':
			val = atoi(optarg);
			if ((input_arg[i].mode == ENCODE) || (input_arg[i].mode == LOOPBACK))
				input_arg[i].cmd.video_node_capture = val;
			else {
				if (val == 1) {
					input_arg[i].cmd.dst_scheme = PATH_IPU;
					info_msg("Display through IPU LIB\n");
				} else {
					input_arg[i].cmd.dst_scheme = PATH_V4L2;
					info_msg("Display through V4L2\n");
					input_arg[i].cmd.video_node = val;
				}
				if (cpu_is_mx27() &&
					(input_arg[i].cmd.dst_scheme == PATH_IPU)) {
					input_arg[i].cmd.dst_scheme = PATH_V4L2;
					warn_msg("ipu lib disp only support in ipuv3\n");
				}
			}
			break;
		case 'n':
			if (input_arg[i].mode == ENCODE) {
				/* contains the ip address */
				strncpy(input_arg[i].cmd.output, optarg, 64);
				input_arg[i].cmd.dst_scheme = PATH_NET;
			} else {
				warn_msg("-n option used only for encode\n");
			}
			break;
		case 'p':
			input_arg[i].cmd.port = atoi(optarg);
			break;
		case 'r':
			input_arg[i].cmd.rot_angle = atoi(optarg);
			if (input_arg[i].cmd.rot_angle)
				input_arg[i].cmd.rot_en = 1;
			break;
		case 'u':
			input_arg[i].cmd.ipu_rot_en = atoi(optarg);
			/* ipu rotation will override vpu rotation */
			if (input_arg[i].cmd.ipu_rot_en)
				input_arg[i].cmd.rot_en = 0;
			break;
		case 'f':
			input_arg[i].cmd.format = atoi(optarg);
			break;
		case 'c':
			input_arg[i].cmd.count = atoi(optarg);
			break;
		case 'v':
			input_arg[i].cmd.vdi_motion = optarg[0];
			break;
		case 'w':
			input_arg[i].cmd.width = atoi(optarg);
			break;
		case 'h':
			input_arg[i].cmd.height = atoi(optarg);
			break;
		case 'j':
			input_arg[i].cmd.loff = atoi(optarg);
			break;
		case 'k':
			input_arg[i].cmd.toff = atoi(optarg);
			break;
		case 'g':
			input_arg[i].cmd.gop = atoi(optarg);
			break;
		case 's':
			if (cpu_is_mx6x())
				input_arg[i].cmd.bs_mode = atoi(optarg);
			else
				input_arg[i].cmd.prescan = atoi(optarg);
			break;
		case 'b':
			input_arg[i].cmd.bitrate = atoi(optarg);
			break;
		case 'd':
			input_arg[i].cmd.deblock_en = atoi(optarg);
			break;
		case 'e':
			input_arg[i].cmd.dering_en = atoi(optarg);
			break;
		case 'm':
			input_arg[i].cmd.mirror = atoi(optarg);
			if (input_arg[i].cmd.mirror)
				input_arg[i].cmd.rot_en = 1;
			break;
		case 't':
			input_arg[i].cmd.chromaInterleave = atoi(optarg);
			break;
		case 'l':
			input_arg[i].cmd.mp4_h264Class = atoi(optarg);
			break;
		case 'a':
			input_arg[i].cmd.fps = atoi(optarg);
			break;
		case 'y':
			input_arg[i].cmd.mapType = atoi(optarg);
			break;
		case 'q':
			input_arg[i].cmd.quantParam = atoi(optarg);
			break;
		case 'L':
			input_arg[i].cmd.display_left = atoi(optarg);
			break;
		case 'T':
			input_arg[i].cmd.display_top = atoi(optarg);
			break;
		case 'W':
			input_arg[i].cmd.display_width = atoi(optarg);
			break;
		case 'H':
			input_arg[i].cmd.display_height = atoi(optarg);
			break;
		case -1:
			break;
		default:
			status = -1;
			break;
		}
	} while ((opt != -1) && (status == 0));

	optind = 1;
	return status;
}

static int
signal_thread(void *arg)
{
	int sig;

	pthread_sigmask(SIG_BLOCK, &sigset, NULL);

	while (1) {
		sigwait(&sigset, &sig);
		if (sig == SIGINT) {
			warn_msg("Ctrl-C received\n");
		} else {
			warn_msg("Unknown signal. Still exiting\n");
		}
		quitflag = 1;
		break;
	}

	return 0;
}

//
int m_enc_thread(void *arg)
{
	int num=1;
	int i;
	int ret_thr;
	vfile_p  vfileptmp;
	int err;
	int ret;
	config_p argtmp=(struct config *)arg;
	while(1)
	{
			if(argtmp!=NULL)
			argtmp->EncWatch=0;
			err = vpu_Init(NULL);
			if (err) {
				err_msg("VPU Init Failure.\n");
				return -1;
			}
		if(DMCreateID(0,0,&g_vfilelist))
			break;//can not allocate enough space
		if(quitflag)break;
		printf("test g_config.chs %d\n",g_config.chs);
		for(i=0;i<num;i++)
		{
			//shagnwei add
			printf("before file name is %s \n",input_arg[i].cmd.output);
			vfileptmp=(vfile_p)malloc(sizeof(vfile_t)*1);
			strcpy(vfileptmp->path,VideoPath);
			strcpy(vfileptmp->name,g_vfilelist.CurID);
			sprintf(vfileptmp->name,"%s%s%d%s",vfileptmp->name,"_D",i,".mp4");
			
			sprintf(input_arg[i].cmd.output,"%s/%s" , vfileptmp->path,vfileptmp->name);
			printf("after file name is %s \n",input_arg[i].cmd.output);
			if (check_params(&input_arg[i].cmd, input_arg[i].mode) == 0) {
				
				input_arg[i].cmd.instns= i;		//add by zorro
				if (open_files(&input_arg[i].cmd) == 0) {
					
					if (input_arg[i].mode == DECODE) {
						ret = decode_test(&input_arg[i].cmd);
						
						printf("decode_test thread %d.\n",i );
					}
					else if (input_arg[i].mode == ENCODE) {
		
						//shagnwei add
						printf("confirm file name is %s \n",input_arg[i].cmd.output);
						
						//ret = encode_test(&input_arg[0].cmd);
						//ret=m_enc_thread(ppargv,4)
						
						err=pthread_create(&input_arg[i].tid,
						  NULL,
						  (void *)&encode_test,
						  (void *)&input_arg[i].cmd);
						

						
						printf("encode_test %d thread return .\n",i, err);
						//DMAddVfile(&g_vfilelist, vfileptmp);
						if(g_config.lockvfileflag)
						{
							printf("emergent lock\n");
							DMLockFile(vfileptmp,g_vfilelist.lockpath);
						}
						if(ret) 
						{
							printf("encode_test err\n");
							break;
						}
						//quitflag=0;
						
					}
					else if (input_arg[i].mode == TRANSCODE) {
											ret = transcode_test(&input_arg[0].cmd);
					}
		
					//close_files(&input_arg[0].cmd);
				} 
				else {
					ret = -1;
				}
			} else {
				ret = -1;
			}
		}
		//if (input_arg[i].mode == LOOPBACK) {
		//	encdec_test(&input_arg[0].cmd);
		//}
		
		for (i = 0; i < num; i++) {
			if (input_arg[i].tid != 0) {
				pthread_join(input_arg[i].tid, (void *)&ret_thr);				
				printf("pthread_join %d return %d \n",i,ret_thr);
				if (ret_thr)
					ret = -1;
				close_files(&input_arg[i].cmd);
			}
		}
	
#ifdef COMMON_INIT
			vpu_UnInit();
#endif
		}

}


//
int m_dec_thread(void *arg)
{


}

int m_uart_thread(void * arg)
{
	return uart_thread(arg);
}

int m_key_thread(void * arg)
{
	return key_thread(arg);
}

int m_watch_thread(void * arg)
{
	config_p argtmp;
	argtmp=(config_p)arg;
	if (argtmp==NULL)
	{
		printf("m_watch_thread start fail\n");
		return -1;
	}	
	printf("m_watch_thread startsuccess\n");
	while(1)
	{
		argtmp->EncRestart =argtmp->EncWatch;//if EncRestart =1 means the enc thread was dump
		if(argtmp->EncWatch==0) 
		{
			printf("m_watch_thread report normal\n");
			argtmp->EncWatch=1;
		}
		sleep(60000);
	}
}


void config_init(void)
{
	//g_config.args
	g_config.fps=30;
	g_config.encflag=1;
	g_config.decflag=1;
	g_config.pargv=input_arg;
	g_config.max_channels=4;
	g_config.chs=4;
	g_config.EncWatch=0;
	g_config.EncRestart=0;
	instance=0;
	
}

#define channel 4//no param
#define pargc     3//no param


int
#ifdef _FSL_VTS_
vputest_mainbkp(int argc, char *argv[])
#else
main_bkp(int argc, char *argv[])
#endif
{
	int err, nargc, i, ret = 0,j;//j is no param
	char *pargv[32] = {0}, *dbg_env;
	pthread_t sigtid;
	pthread_t uarttid;
	pthread_t enctid;
	pthread_t dectid;
	pthread_t keytid;
	vpu_versioninfo ver;
	int ret_thr;

	vfile_p vfileptmp;

	//no param
	char ***ppargv;
	ppargv=(char ***)malloc(sizeof(char **)*channel);
	for(i=0;i<channel;i++)
	{
		ppargv[i]=(char **)malloc(sizeof(char *)*pargc);
		for(j=0;j<pargc;j++)
		{
			ppargv[i][j]=(char *)malloc(sizeof(char )*256);
		}
		strcpy(ppargv[i][0],*argv);
		strcpy(ppargv[i][1],"-E");
	}
	strcpy(ppargv[0][2],"-x 0 -o enc0.mp4 -w 720 -h 480 -L 0 -T 0 -W 512 -H 384	-f 2 -a 30 -c 500");
	strcpy(ppargv[1][2],"-x 1 -o enc1.mp4 -w 720 -h 480 -L 512 -T 0 -W 512 -H 384 -f 2 -a 30 -c 500 ");
	strcpy(ppargv[2][2],"-x 2 -o enc2.mp4 -w 720 -h 480 -L 0 -T 384 -W 512 -H 384 -f 2 -a 30 -c 500");
	strcpy(ppargv[3][2],"-x 3 -o enc3.mp4 -w 720 -h 480 -L 512 -T 384 -W 512 -H 384 -f	2 -a 30 -c 500");
	//no param



	dbg_env=getenv("VPU_TEST_DBG");
	if (dbg_env)
		vpu_test_dbg_level = atoi(dbg_env);
	else
		vpu_test_dbg_level = 0;
	if(argc<2)
	{
		argc=3;//no param
		err = parse_main_args(argc, ppargv[1]);//no param
	}
	else 
	{
		printf("use parm start \n");
		err = parse_main_args(argc, argv);
	}
	if (err) {
		goto usage;
	}

	if (!instance) {
		goto usage;
	}

	info_msg("VPU test program built on %s %s\n", __DATE__, __TIME__);
#ifndef _FSL_VTS_
	sigemptyset(&sigset);
	sigaddset(&sigset, SIGINT);
	pthread_sigmask(SIG_BLOCK, &sigset, NULL);
	pthread_create(&sigtid, NULL, (void *)&signal_thread, NULL);
#endif
	pthread_create(&uarttid, NULL, (void *)&m_uart_thread, NULL);	/*zorro 2015/07/06*/
	framebuf_init();

	// just to enable cpu_is_xx() to be used in command line parsing
	err = vpu_Init(NULL);
	if (err) {
		err_msg("VPU Init Failure.\n");
		return -1;
	}

	vpu_UnInit();


	DMInit();

	if (using_config_file == 0) {
		get_arg(input_arg[0].line, &nargc, pargv);
		err = parse_args(nargc, pargv, 0);
		if (err) {
			vpu_UnInit();
			goto usage;
		}
	}

	while(1)
	{
		err = vpu_Init(NULL);
		if (err) {
			err_msg("VPU Init Failure.\n");
			return -1;
		}
	if(DMCreateID(0,0,&g_vfilelist))
		break;//can not allocate enough space
	if(quitflag)break;
	//shagnwei add
	printf("before file name is %s \n",input_arg[0].cmd.output);
	vfileptmp=(vfile_p)malloc(sizeof(vfile_t)*1);
	strcpy(vfileptmp->path,VideoPath);
	strcpy(vfileptmp->name,g_vfilelist.CurID);
	strcat(vfileptmp->name,"_F.h264");
	
	sprintf(input_arg[0].cmd.output,"%s/%s" , vfileptmp->path,vfileptmp->name);
	printf("after file name is %s \n",input_arg[0].cmd.output);
	if (check_params(&input_arg[0].cmd, input_arg[0].mode) == 0) {
		if (open_files(&input_arg[0].cmd) == 0) {
			if (input_arg[0].mode == DECODE) {
				ret = decode_test(&input_arg[0].cmd);
			} else if (input_arg[0].mode == ENCODE) {

				//shagnwei add
				printf("confirm file name is %s \n",input_arg[0].cmd.output);
				
				ret = encode_test(&input_arg[0].cmd);
				//ret=m_enc_thread(ppargv,4);
				printf("encode_test return.\n");
				DMAddVfile(&g_vfilelist, vfileptmp);
				if(g_config.lockvfileflag)
				{
					DMLockFile(vfileptmp,g_vfilelist.lockpath);
				}
				if(ret) 
				{
					printf("encode_test err\n");
					break;
				}
				//quitflag=0;
				
                            } else if (input_arg[0].mode == TRANSCODE) {
                                    ret = transcode_test(&input_arg[0].cmd);
			}

			close_files(&input_arg[0].cmd);
		} else {
			ret = -1;
		}
	} else {
		ret = -1;
	}

	if (input_arg[0].mode == LOOPBACK) {
		encdec_test(&input_arg[0].cmd);
	}



#ifdef COMMON_INIT
		vpu_UnInit();
#endif

	}



	return ret;

usage:
	info_msg("\n%s", usage);
	return -1;
}




//------shagnwei------//
int main(int argc,char *argv[])
	{
		int err, nargc, i, ret = 0,j;//j is no param
		char *pargv[32] = {0}, *dbg_env;
		pthread_t sigtid;
		pthread_t uarttid;
		pthread_t enctid;
		pthread_t dectid;
		pthread_t keytid;
		pthread_t watchtid;
		vpu_versioninfo ver;
		int xthreads;
		int ret_thr;
		vfile_p  vfileptmp;
		int chs;
		//no param
		char ***ppargv;
		ppargv=(char ***)malloc(sizeof(char **)*channel);
		chs=channel;
		printf("chs is	%d\n",chs);
		for(i=0;i<chs;i++)
		{
			ppargv[i]=(char **)malloc(sizeof(char *)*pargc);
			for(j=0;j<pargc;j++)
			{
				ppargv[i][j]=(char *)malloc(sizeof(char )*256);
			}
			strcpy(ppargv[i][0],*argv);
			strcpy(ppargv[i][1],"-E");
			printf("ppargv  %d\n",i);
		}
		if(argc==2)
			xthreads=atoi(*argv);
		if((xthreads<1) ||(xthreads>4))
			xthreads=4;
		printf("xthreads is %d.\n",xthreads);
		if(chs-- >0){		
		printf(" ppargv[0][2]\n");
		strcpy(ppargv[0][2],"-x 0 -o enc0.mp4 -w 720 -h 480 -L 0 -T 0 -W 512 -H 384 -f 2 -a 30 -c 500");
		}
		if(chs-->0){
			printf(" ppargv[1][2]\n");
		strcpy(ppargv[1][2],"-x 1 -o enc1.mp4 -w 720 -h 480 -L 512 -T 0 -W 512 -H 384 -f 2 -a 30 -c 500 ");
		}
		if(chs-->0){
			printf(" ppargv[2][2]\n");
		strcpy(ppargv[2][2],"-x 2 -o enc2.mp4 -w 720 -h 480 -L 0 -T 384 -W 512 -H 384 -f 2 -a 30 -c 500");
		}
		if(chs-->0){		
			printf(" ppargv[3][2]\n");
		strcpy(ppargv[3][2],"-x 3 -o enc3.mp4 -w 720 -h 480 -L 512 -T 384 -W 512 -H 384 -f 2 -a 30 -c 500");
		}
		//no param
		dbg_env=getenv("VPU_TEST_DBG");
		if (dbg_env)
			vpu_test_dbg_level = atoi(dbg_env);
		else
			vpu_test_dbg_level = 0;
	
		info_msg("VPU test program built on %s %s\n", __DATE__, __TIME__);
		framebuf_init();
	
		// just to enable cpu_is_xx() to be used in command line parsing
		err = vpu_Init(NULL);
		if (err) {
			err_msg("VPU Init Failure.\n");
			return -1;
		}
	
		vpu_UnInit();
	
	
		DMInit();
		chs=channel;
		argc=3;//no param
		for(i=0;i<chs;i++)
		{
			err = parse_main_args(argc, ppargv[i]);//no param
			printf(" parse_main_args %d\n",i);
			if (err) {
				goto usage;
			}

			if (!instance) {
				goto usage;
			}
		}
		for(i=0;i<chs;i++)
		{
			if (using_config_file == 0) {
				printf(" input_arg[%d]is  %s\n",i,input_arg[i].line);
				get_arg(input_arg[i].line, &nargc, pargv);
				err = parse_args(nargc, pargv, i);
				if (err) {
					vpu_UnInit();
					goto usage;
				}
			}
		}

		sigemptyset(&sigset);
		sigaddset(&sigset, SIGINT);
		pthread_sigmask(SIG_BLOCK, &sigset, NULL);
		pthread_create(&sigtid, NULL, (void *)&signal_thread, NULL);
		pthread_create(&uarttid, NULL, (void *)&m_uart_thread, NULL);	/*zorro 2015/07/06*/
		pthread_create(&keytid, NULL, (void *)&m_key_thread, NULL);	
		pthread_create(&watchtid, NULL, (void *)&m_watch_thread, &g_config);

		pthread_create(&dectid, NULL, (void *)&m_dec_thread, NULL);

		pthread_create(&enctid, NULL, (void *)&m_enc_thread, &g_config);
		
		while(1)
		{
			if(g_config.EncRestart){
				printf("enc thread dump! restarting... \n");
				pthread_kill(enctid,SIGKILL);
				g_config.EncRestart=0;
				g_config.EncWatch=0;
				pthread_create(&enctid, NULL, (void *)&m_enc_thread, &g_config);
			}
	
		}
	
	
	
		return ret;
		//release ppargv
	usage:
		info_msg("\n%s", usage);
		return -1;
}

//------shagnwei------//


#ifdef _FSL_VTS_
#include "dut_api_vts.h"


#define MAX_CMD_LINE_LEN    1024

typedef struct _tagVideoDecoder
{
    unsigned char * strStream;          /* input video stream file */
    int             iPicWidth;          /* frame width */
    int             iPicHeight;         /* frame height */
    DUT_TYPE        eDutType;           /* DUT type */
    FuncProbeDut    pfnProbe;           /* VTS probe for DUT */
} VDECODER;


FuncProbeDut g_pfnVTSProbe = NULL;
unsigned char *g_strInStream = NULL;


DEC_RETURN_DUT VideoDecInit( void ** _ppDecObj, void *  _psInitContxt )
{
    DEC_RETURN_DUT eRetVal = E_DEC_INIT_OK_DUT;
    DUT_INIT_CONTXT_2_1 * psInitContxt = _psInitContxt;
    VDECODER * psDecObj = NULL;

    do
    {
        psDecObj = malloc( sizeof(VDECODER) );
        if ( NULL == psDecObj )
        {
            eRetVal = E_DEC_INIT_ERROR_DUT;
            break;
        }
        *_ppDecObj = psDecObj;
        psDecObj->pfnProbe   = psInitContxt->pfProbe;
        psDecObj->strStream  = psInitContxt->strInFile;
        psDecObj->iPicWidth  = (int)psInitContxt->uiWidth;
        psDecObj->iPicHeight = (int)psInitContxt->uiHeight;
        psDecObj->eDutType   = psInitContxt->eDutType;

        g_pfnVTSProbe = psInitContxt->pfProbe;
        g_strInStream = psInitContxt->strInFile;
    } while (0);

    return eRetVal;
}

DEC_RETURN_DUT VideoDecRun( void * _pDecObj, void * _pParam )
{
    VDECODER * psDecObj = (VDECODER *)_pDecObj;
    char * strCmdLine = NULL;
    char * argv[3];
    int argc = 3;
    int iDecID = 0;
    int iMPEG4Class = 0;
    DEC_RETURN_DUT eRetVal = E_DEC_ALLOUT_DUT;

    do
    {

        strCmdLine = malloc( MAX_CMD_LINE_LEN );
        if ( NULL == strCmdLine )
        {
            fprintf( stderr, "Failed to allocate memory for command line." );
            eRetVal = E_DEC_ERROR_DUT;
            break;
        }

        argv[0] = "./mxc_vpu_test.out";
        argv[1] = "-D";
        argv[2] = strCmdLine;

        /* get decoder file name */
        switch (psDecObj->eDutType)
        {
        case E_DUT_TYPE_H264:
            iDecID = 2;
            break;
        case E_DUT_TYPE_DIV3:
            iDecID = 5;
            break;
        case E_DUT_TYPE_MPG4:
            iMPEG4Class = 0;
            break;
        case E_DUT_TYPE_DIVX:
        case E_DUT_TYPE_DX50:
            iMPEG4Class = 1;
            break;
        case E_DUT_TYPE_XVID:
            iMPEG4Class = 2;
            break;
        case E_DUT_TYPE_DIV4:
            iMPEG4Class = 5;
            break;
        case E_DUT_TYPE_MPG2:
            iDecID = 4;
            break;
        case E_DUT_TYPE_RV20:
        case E_DUT_TYPE_RV30:
        case E_DUT_TYPE_RV40:
        case E_DUT_TYPE_RV89:
            iDecID = 6;
            break;
        case E_DUT_TYPE_WMV9:
        case E_DUT_TYPE_WMV3:
        case E_DUT_TYPE_WVC1:
            iDecID = 3;
            break;
        default:
            perror( "DUT type %d is not supported.\n" );
            iDecID = -1;
        }

        if ( -1 == iDecID )
        {
            eRetVal = E_DEC_ERROR_DUT;
            break;
        }

        /* run decoder */
        sprintf( strCmdLine, "-i %s -f %d -l %d -o vts",
                 psDecObj->strStream, iDecID, iMPEG4Class );

        vputest_main(argc, argv);
    } while (0);

    if ( strCmdLine )
    {
        free( strCmdLine );
    }

    return eRetVal;
}

DEC_RETURN_DUT VideoDecRelease( void * _pDecObj )
{
    DEC_RETURN_DUT eRetVal = E_DEC_REL_OK_DUT;

    if ( _pDecObj )
    {
        free( _pDecObj );
    }

    return eRetVal;
}

void QueryAPIVersion( long * _piAPIVersion )
{
    *_piAPIVersion = WRAPPER_API_VERSION;
}
#endif
