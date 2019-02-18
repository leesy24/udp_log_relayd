/*****************************************************************************
Copyright (c) 2007 SystemBase Co., LTD  All Rights Reserved.

UDP Server/Clinet		09, 31, 2007. yhlee
******************************************************************************/
#include <sys/statfs.h>

#include "include/sb_include.h"
#include "include/sb_define.h"
#include "include/sb_shared.h"
#include "include/sb_config.h"
#include "include/sb_extern.h"
#include "include/sb_ioctl.h"

//---
#include "include/sb_misc.h"  ////-DRF-  //====(eddy v2.5)==<

#define MAX_BUFFER_SIZE 		3000
#define MAX_SOCKET_READ_SIZE 	512
#define MAX_SERIAL_READ_SIZE 	512

struct sys_info  
{
	int				sfd;
	int				lfd;
	int				port_no;
	int				protocol;
	int				wait_msec;
	int				socket;
	char  			bps;
	char 			dps;
	char 			flow;
	char			interface;
	char 			server_ip [100];
	int				server_port;	
};	

struct sys_info  SYS;

int port_no = 0;
char relay_ip [100];
int relay_port = 0;

int portview;
int snmp;
struct SB_PORTVIEW_STRUCT *PSM;
struct SB_SNMP_STRUCT	  *SSM;
char WORK [MAX_BUFFER_SIZE];
int	SB_DEBUG	=	1;
//struct SB_CONFIG cfg;

int main (int argc, char *argv[]);
void get_snmp_memory ();
int PA_login();
void close_init ();
void get_portview_memory ();
void mainloop(int protocol);
int receive_from_lan(void);
int receive_from_port (int protocol);
void SSM_write ();
void PSM_write (char sw, char *data, int len);

#define WORD_CNT (17)
#define WORD_SIZE (15)
//#define WORD_SIZE (5)

char words[WORD_CNT][WORD_SIZE];
int string_parse(char *str);
int year, month, day;
int sfd = 0;
int limit;
char prefix[9+1+1];
//
//#define rBUF_MAX (10)
#define rBUF_SIZE (300) //BufRecordSize...GDC&PJK
char rBUF[50][rBUF_SIZE];
int  rBUFcnt=0;
int  rBUFmax=10;


// /flash/udp_log_relayd 6000 192.168.0.251 6000
// /flash/udp_log_relayd 6000 192.168.0.251 6000 20 
// /<----------------------------------------------for rBUFmax
// 							 "udp_log_relayd_v1.12_20190214a2.c"

//===============================================================================	
int main (int argc, char *argv[])
{
	SB_SetPriority (5);						// Min 1 ~ Max 99 

	if (argc < 2)
	{
		printf("need port number, and/or relay ip and port\n");
		exit(-1);
	}
	else if (argc == 3)
	{
		printf("need port number, relay ip and port\n");
		exit(-1);
	}

	if (atoi(argv[1]) <= 0)
	{
		printf("need port number on first parameter\n");
		exit(-1);
	}

	port_no = atoi(argv[1]);

	if (argc >= 3)
	{
		if (strlen(argv[2]) >= 100)
		{
			printf("need ip on second parameter\n");
			exit(-1);
		}
		
		if (atoi(argv[3]) <= 0)
		{
			printf("need port number on third parameter\n");
			exit(-1);
		}

		strcpy(relay_ip, argv[2]);
		relay_port = atoi(argv[3]);
		
		if (argc >= 4)
		{
			rBUFmax = atoi(argv[4]);
			if(rBUFmax>50)
			{
				printf("fixed..rBUFmax:20count\n");
				rBUFmax=20;
			}
		}
	}

	if (SB_DEBUG) 
		SB_LogMsgPrint ("UDP port=%d\n", port_no);
	signal(SIGPIPE, SIG_IGN);	
	//get_portview_memory ();
	//get_snmp_memory ();

	struct statfs fsb;

	if(statfs("/tmp/usb", &fsb) == 0) {
		printf("device capacity is %ld blocks, free %ld blocks, block size is %ld bytes.\n", fsb.f_blocks, fsb.f_bfree, fsb.f_bsize);
		limit = fsb.f_blocks / 100;
		//limit = fsb.f_blocks - 100;
	}

	SYS.lfd = 0;
	SYS.sfd = 0;

	while (1) 
		{
/*
		SB_ReadConfig (SB_CFG_FILE, &cfg);
		SYS.protocol	= cfg.sio[port_no].protocol;
		SYS.bps		 	= cfg.sio[port_no].speed;
		SYS.dps		 	= cfg.sio[port_no].dps;
		SYS.flow		= cfg.sio[port_no].flow;
		SYS.wait_msec	= cfg.sio[port_no].bypass;
		SYS.socket		= cfg.sio[port_no].socket_no;
		SYS.interface	= cfg.sio[port_no].interface;
		SYS.server_port	= cfg.sio[port_no].remote_socket_no;
		SB_Hex2Ip ((char *)cfg.sio[port_no].remote_ip, (char *)SYS.server_ip);
*/
		SYS.socket = port_no;
/*
		if (SYS.wait_msec == SB_ENABLE)
			SYS.wait_msec = 0;
		else	
			SYS.wait_msec = SB_GetDelaySerial (SYS.bps);
*/
/*
		if (portview) 
			{
			PSM_PORT.reset_request = SB_DISABLE;	// Reset init
			PSM_PORT.flag		   = SB_ENABLE;		// Scope init
			PSM_PORT.status 	   = SB_ENABLE;		// Wait Stat
			}
		if (snmp) 
			{
			SSM_CONNECT_STAT = SB_DISABLE;
			SSM_PORTRESET = SB_DISABLE;
			}
*/
/*
		if (SB_DEBUG) 
			SB_LogMsgPrint ("UDP port=%d, Speed = %d, dps = %d, flow = %d, MRU=%d\n",
			port_no+1, SYS.bps, SYS.dps, SYS.flow, SYS.wait_msec);
*/

		mainloop(SYS.protocol);
		}
}
//===============================================================================
/*
void get_snmp_memory ()
{
void *shared_memory = (void *)0;

	snmp = SB_DISABLE;
	shared_memory = SB_GetSharedMemory (SB_SNMP_KEY, sizeof (struct SB_SNMP_STRUCT));
	if (shared_memory == (void *)-1)	return;
	SSM = (struct SB_SNMP_STRUCT *)shared_memory;
	snmp = SB_ENABLE;
}
*/
//===============================================================================
/*
void get_portview_memory (void)
{
void *shared_memory = (void *)0;

	portview = SB_DISABLE;
	shared_memory = SB_GetSharedMemory (SB_PORTVIEW_KEY, sizeof (struct SB_PORTVIEW_STRUCT));
	if (shared_memory == (void *)-1)	return;
	PSM = (struct SB_PORTVIEW_STRUCT *)shared_memory;
	portview = SB_ENABLE;
}
*/
//===============================================================================
void mainloop(int protocol)
{
unsigned long CTimer=0, BTimer;
unsigned long DTimer = 0;
unsigned long STimer = 0;

	close_init ();
	SB_msleep (1000);
	SYS.lfd = SB_BindUdp (SYS.socket);
	printf("SB_BindUdp(%d):return = %d\n", SYS.socket, SYS.lfd);
	if (SYS.lfd <= 0)
	{
		return;
	}

/*
	if (SYS.sfd <= 0)
		{
		SYS.sfd = SB_OpenSerial (port_no);
		if (SYS.sfd <= 0) return;
		}
	SB_InitSerial (SYS.sfd, SYS.bps, SYS.dps, SYS.flow);
	SB_ReadSerial (SYS.sfd, WORK, MAX_BUFFER_SIZE, 0);
	if (SYS.interface == SB_RS232)
		{
		SB_SetDtr (SYS.sfd, SB_ENABLE);
		SB_SetRts (SYS.sfd, SB_ENABLE);
		}
*/
	SYS.sfd = open("/tmp/usb/000000.txt", O_WRONLY | O_CREAT | O_APPEND);
	printf("open(\"/tmp/usb/000000.txt\"):return = %d\n", SYS.sfd);
	if (SYS.sfd <= 0)
	{
		return;
	}
/*
	if (snmp)	  
		{
		SSM_CONNECT_STAT = SB_ENABLE;
		SSM_CONNECT_COUNT ++;
		}
*/
	while (1)
		{
		if (SYS.bps != 13) SB_msleep (1);
		BTimer = SB_GetTick ();
		if (BTimer < CTimer)						// Check over Tick Counter;  0xffffffff (4 bytes) 
			{
			DTimer = BTimer + 7000;
			STimer = BTimer + 1000;
			}	
		CTimer = BTimer;
		
		if (DTimer < CTimer) 		
			{
			DTimer = CTimer + 7000;		// 7 sec
			sprintf (WORK, "/var/run/debug-%d", port_no);
    		if (access(WORK, F_OK) == 0)  SB_DEBUG = 1; else SB_DEBUG = 0;
    		}	
    		
		if (receive_from_lan () < 0) return;
/*
		if (receive_from_port(protocol) < 0) return;
*/
/*
		if (portview)
			{
			if (PSM_PORT.reset_request == SB_ENABLE) return;		// Portview Reset request
			}

		if (snmp)
			{
			if (STimer > CTimer) continue;
			STimer = CTimer + 1000;
			SSM_write ();
			if (SSM_PORTRESET) return;			// SNMP set (reset)				
			}
*/	 
		}
}
//===============================================================================
void close_init ()
{
	if (SYS.lfd > 0)
		{
		shutdown(SYS.lfd, SHUT_RDWR);	
		close (SYS.lfd);
		SYS.lfd = 0;
		}

	if (SB_DEBUG) SB_LogMsgPrint ("Close Socket & Serial Handle\n");
}	
//===============================================================================
int receive_from_lan(void)
{
int len, sio_len; 

/*
	sio_len = ioctl (SYS.sfd, TIOTGTXCNT);
	if (sio_len < 32)	
	{
		printf("ioctl (SYS.sfd, TIOTGTXCNT):sio_len = %d\n", sio_len);
		return 0;
	}
	if (sio_len > MAX_SOCKET_READ_SIZE) sio_len = MAX_SOCKET_READ_SIZE;
*/
	sio_len = MAX_SOCKET_READ_SIZE;
	len = SB_ReadUdp (SYS.lfd, WORK, sio_len);
	if (len <= 0)
	{
		//printf("SB_ReadUdp(%d, WORK, %d):len = %d\n", SYS.lfd, sio_len, len);
		return len;
	}
	//printf("SB_ReadUdp(%d, WORK, %d):len = %d\n", SYS.lfd, sio_len, len);
/*
	if (portview) PSM_PORT.status = SB_ACTIVE;	
	SB_SendSerial (SYS.sfd, WORK, len);		
	if (SB_DEBUG) SB_LogDataPrint ("L->S", WORK, len); 
	if (portview)  PSM_write ('S', WORK, len);
*/
	if (relay_port != 0)
		SB_SendUdpClient (SYS.lfd, WORK, len, relay_ip, relay_port);

	int cnt;
	cnt = string_parse(WORK);

	//printf("cnt = %d\n", cnt);

	if (cnt == 0) return 0;

	struct statfs fsb;

	if (statfs("/tmp/usb", &fsb) == 0) {
		//printf("device free is %ld blocks. %d\n", fsb.f_bfree, limit);
		if (fsb.f_bfree < limit) {
			DIR *d;
			struct dirent *dir;
			int min = 999999;
			char target[256];
			d = opendir("/tmp/usb");
			if (d)
			{
				while ((dir = readdir(d)) != NULL)
				{
					char f_name[256];
					int ymd;
					strncpy(f_name, dir->d_name, 8);
					f_name[8] = '\0';
					ymd = atoi(f_name);
					//printf("file name is %s, %d\n", f_name, ymd);
					if (ymd != 0 && ymd < min) {
						min = ymd;
						strcpy(target, dir->d_name);
					}
				}
				closedir(d);
			}
			char target_full[256];
			sprintf(target_full, "/tmp/usb/%s", target);
			//printf("target full file is \"%s\".\n", target_full);
			if (min != 999999) {
				remove(target_full);
			}
		}
	}

	if (cnt == 16) {
		int y, m, d;
		y = atoi(words[13]);
		m = atoi(words[14]);
		d = m % 100;
		m = m / 100;

		int ok = 1;
		if (y > 99 || y < 0) ok = 0;
		if (m > 12 || m < 1) ok = 0;
		if (d < 1) ok = 0;
		if ((m == 1 || m == 3 || m == 5 || m == 7 ||
			m == 8 || m == 10 || m == 12)
			&& d > 31) ok = 0;
		if ((m == 4 || m == 6 || m == 9 || m == 11)
			&& d > 30) ok = 0;
		if ((m == 2)
			&& d > 29) ok = 0;

		if (ok == 1) {
			//printf("year = %d, month = %d, day = %d\n", y, m, d);
			if (sfd <= 0 || y != year || m != month || d != day) {
				if (sfd > 0) {
					fsync(sfd);
					close(sfd);
				}
				char filename[100];
				year = y;
				month = m;
				day = d;
				sprintf(filename, "/tmp/usb/%02d%02d%02d.txt", year, month, day);
				sfd = open(filename, O_WRONLY | O_CREAT | O_APPEND);
				//printf("open(\"%s\"):return = %d\n", filename, SYS.sfd);
			}
			if (sfd > 0)
			{
				if (strlen(words[2]) == 9) sprintf(prefix, "%s ", words[2]);
				/**
				write(sfd, prefix, strlen(prefix));
				write(sfd, WORK, len);
				fsync(sfd);
				**/
				
				strncpy( rBUF[rBUFcnt], prefix, strlen(prefix) );
				if (len >= (rBUF_SIZE-20)) 
				{ 	sprintf(&WORK[rBUF_SIZE-20], "\r\n");  len=strlen(WORK);  }
				strncpy( &rBUF[rBUFcnt][strlen(prefix)], WORK, len );  //Buf..LogRecords
				//
				if ( rBUFcnt >= (rBUFmax - 1) )  //rBUF
				{
					for ( rBUFcnt=0; rBUFcnt<rBUFmax ; rBUFcnt++ )
					{
						write(sfd, rBUF[rBUFcnt], strlen(rBUF[rBUFcnt]) );
					}
					fsync(sfd);
					rBUFcnt=0;
				}
				else rBUFcnt++;
			}
		}
		else {
			//printf("E:year = %d, month = %d, day = %d\n", y, m, d);
			if (sfd > 0) {
				fsync(sfd);
				close(sfd);
				sfd = 0;
			}
			write(SYS.sfd, WORK, len);
		}
	}
	else {
		if (sfd > 0)
		{
			/**
			write(sfd, prefix, strlen(prefix));
			write(sfd, WORK, len);
			fsync(sfd);
			**/
			
			strncpy( rBUF[rBUFcnt], prefix, strlen(prefix) );
			if (len >= (rBUF_SIZE-20)) 
			{ 	sprintf(&WORK[rBUF_SIZE-20], "\r\n");  len=strlen(WORK);  }
			strncpy( &rBUF[rBUFcnt][strlen(prefix)], WORK, len );  //Buf..LogRecords
			//
			if ( rBUFcnt >= (rBUFmax - 1) )
			{
				for ( rBUFcnt=0; rBUFcnt<rBUFmax ; rBUFcnt++ )
				{
					write(sfd, rBUF[rBUFcnt], strlen(rBUF[rBUFcnt]) );
				}
				fsync(sfd);
				rBUFcnt=0;
			}
			else rBUFcnt++;
		}
		else {
			write(SYS.sfd, WORK, len);
		}
	}

	return 0;
} 
//===============================================================================
/*
int receive_from_port (int protocol)
{
int len;

	len = SB_ReadSerial (SYS.sfd, WORK, MAX_SERIAL_READ_SIZE, SYS.wait_msec);
	if (len <= 0) return 0;
	
	if (protocol == SB_UDP_SERVER_MODE)	// server
		SB_SendUdpServer (SYS.lfd, WORK, len);
	else
		SB_SendUdpClient (SYS.lfd, WORK, len, SYS.server_ip, SYS.server_port);

	if (portview) PSM_PORT.status = SB_ACTIVE;		
	if (SB_DEBUG) SB_LogDataPrint ("S->L", WORK, len); 
	if (portview) PSM_write ('R', WORK, len);
	return 0;
}
*/
//===============================================================================
/*
void PSM_write (char sw, char *data, int len)
{
static int f_err=0, o_err=0, p_err=0;		//    get first error
struct serial_icounter_struct   err_cnt;
int cplen, f_len, l_len;

	if (sw == 'R')
		{
		switch ( PSM_PORT.flag) {
		case 0 : return;
		case 2 :
			if (RX_GET <= RX_PUT)
				cplen = SB_RING_BUFFER_SIZE - RX_PUT + RX_GET - 1;	
			else
				cplen = RX_GET - RX_PUT - 1;
			if (cplen == 0) return;
			if (cplen < len) len = cplen;
			
			if ((RX_PUT+len) < SB_RING_BUFFER_SIZE) 
				{ f_len = len; l_len = 0; }
			else
				{ f_len = SB_RING_BUFFER_SIZE-RX_PUT;  l_len = len - f_len; }

			if (f_len) memcpy (&PSM->rx_buff[RX_PUT], data, f_len);
			if (l_len) 
				{
				memcpy (&PSM->rx_buff[0], &data[f_len], l_len);	
				RX_PUT = l_len; 
				}
			else
				RX_PUT += f_len;	
			
			PSM->rx_lastputtime = SB_GetTick();
		case 1 :		// error count
			ioctl (SYS.sfd, TIOCGICOUNT, &err_cnt);
			PSM_PORT.rx_bytes += len;
			PSM_PORT.frame_err += (err_cnt.frame - f_err);
			f_err = err_cnt.frame;
			PSM_PORT.parity_err += (err_cnt.parity - p_err);
			p_err = err_cnt.parity;
			PSM_PORT.overrun_err += (err_cnt.overrun - o_err);
			o_err = err_cnt.overrun;
			break;
			}	
		}
	else
		{			
		switch ( PSM_PORT.flag) {
		case 0 : return;
		case 2 :
			if (TX_GET <= TX_PUT)
				cplen = SB_RING_BUFFER_SIZE - TX_PUT + TX_GET - 1;	
			else
				cplen = TX_GET - TX_PUT - 1;
			if (cplen == 0) return;
			if (cplen < len) len = cplen;
			
			if ((TX_PUT+len) < SB_RING_BUFFER_SIZE) 
				{ f_len = len; l_len = 0; }
			else
				{ f_len = SB_RING_BUFFER_SIZE-TX_PUT;  l_len = len - f_len; }

			if (f_len) memcpy (&PSM->tx_buff[TX_PUT], data, f_len);
			if (l_len) 
				{
				memcpy (&PSM->tx_buff[0], &data[f_len], l_len);	
				TX_PUT = l_len; 
				}
			else
				TX_PUT += f_len;	
			
			PSM->tx_lastputtime = SB_GetTick();
		case 1 :		// error count
			PSM_PORT.tx_bytes += len;
			break;			
			}	
		}
}
*/
//===============================================================================
/*
void SSM_write ()
{
unsigned char msr;
struct serial_icounter_struct   err_cnt;
static int f_err=0, o_err=0, p_err=0;		//    get first error
static unsigned char msr_shadow = 0;

	msr = ioctl(SYS.sfd, TIOTSMSR) & 0xf0;
	if (msr != msr_shadow) 
		{
		if ((msr&0x10) != (msr_shadow&0x10)) SSM_CTS_CHANGE ++;
		if ((msr&0x20) != (msr_shadow&0x20)) SSM_DSR_CHANGE ++;
		if ((msr&0x80) != (msr_shadow&0x80)) SSM_DCD_CHANGE ++;
		if (msr&0x10) SSM_CTS_STAT = 1; else SSM_CTS_STAT = 0;
		if (msr&0x20) SSM_DSR_STAT = 1; else SSM_DSR_STAT = 0;
		if (msr&0x80) SSM_DCD_STAT = 1; else SSM_DCD_STAT = 0;		
		msr_shadow = msr;
		}

	ioctl (SYS.sfd, TIOCGICOUNT, &err_cnt);
	SSM_FRAMING_ERRS += (err_cnt.frame   - f_err);  f_err = err_cnt.frame;
	SSM_PARITY_ERRS  += (err_cnt.parity  - p_err);  p_err = err_cnt.parity;
	SSM_OVERRUN_ERRS += (err_cnt.overrun - o_err);  o_err = err_cnt.overrun;
}		
*/

#define WORD_CNT (17)
#define WORD_SIZE (15)
//#define WORD_SIZE (5)

char words[WORD_CNT][WORD_SIZE];

int string_parse(char *str)
{
    if (strlen(str) == 0) return 0;

    char *tmp = (char *)str;
 	int cnt = 0;

    do {
        int l = strcspn(tmp, ",");
	    //printf("l = %d\n", l);
 	    if (l > (WORD_SIZE - 1)) {
            strncpy(words[cnt], tmp, (WORD_SIZE - 1));
            words[cnt][(WORD_SIZE - 1)] = '\0';
 	    } else {
            strncpy(words[cnt], tmp, l);
            words[cnt][l] = '\0';
 	    }
        //printf("\"%s\"\n", words[cnt]);
 	    cnt ++;
        tmp += l + 1;
    } while(tmp[-1]);

	//printf("cnt = %d\n", cnt);

	return (cnt);
}
