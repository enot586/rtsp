#pragma once
#include "stdafx.h"

/*
* Current protocol version.
*/
#define RTP_VERSION    2 
#define RTP_SEQ_MOD (1<<16) 
#define RTP_MAX_SDES 255      /* maximum text length for SDES */ 

/* The following definition is from RFC1890 */
#define RTP_HDR_SZ 12 

/*
* RTP data header
*/
struct rtp_hdr_t {
	unsigned int version : 2;   /* protocol version */
	unsigned int p : 1;         /* padding flag */
	unsigned int x : 1;         /* header extension flag */
	unsigned int cc : 4;        /* CSRC count */
	unsigned int m : 1;         /* marker bit */
	unsigned int pt : 7;        /* payload type */
	unsigned int seq : 16;      /* sequence number */
	uint32_t ts;               /* timestamp */
	uint32_t ssrc;             /* synchronization source */
	//uint32_t csrc[1];          /* optional CSRC list */
};

