/*****************************************************************************
Copyright (c) 2007 SystemBase Co., LTD  All Rights Reserved.

UDP Server/Clinet		09, 31, 2007. yhlee
******************************************************************************/
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

int port_no;
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
//===============================================================================	
int main (int argc, char *argv[])
{
	SB_SetPriority (5);						// Min 1 ~ Max 99 

	if (argc < 2)
	{
		printf("need port number\n");
		exit(-1);
	}

	port_no = atoi (argv[1]);			// port no 1 ~ 16

	if (SB_DEBUG) 
		SB_LogMsgPrint ("UDP port=%d\n", port_no);
	signal(SIGPIPE, SIG_IGN);	
	//get_portview_memory ();
	//get_snmp_memory ();

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
	printf("SB_BindUdp(%d):retuen = %d\n", SYS.socket, SYS.lfd);
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
	SYS.sfd = open("/tmp/usb/log.txt", O_WRONLY | O_CREAT | O_APPEND);
	printf("open(\"/tmp/usb/log.txt\"):retuen = %d\n", SYS.sfd);
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
	write(SYS.sfd, WORK, len);

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

