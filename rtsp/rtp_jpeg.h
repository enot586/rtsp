#pragma once
#include "stdafx.h"
#include "rtp.h"

#define RTP_PT_JPEG             26 

#define RTP_JPEG_RESTART           0x40 

struct jpeghdr {
	unsigned int tspec : 8;   /* type-specific field */
	unsigned int off : 24;    /* fragment byte offset */
	uint8_t type;            /* id of jpeg decoder params */
	uint8_t q;               /* quantization factor (or table id) */
	uint8_t width;           /* frame width in 8 pixel blocks */
	uint8_t height;          /* frame height in 8 pixel blocks */
};

struct jpeghdr_rst {
	uint16_t dri;
	unsigned int f : 1;
	unsigned int l : 1;
	unsigned int count : 14;
};

struct jpeghdr_qtable {
	uint8_t  mbz;
	uint8_t  precision;
	uint16_t length;
};

int MakeHeaders(uint8_t *p, int type, int w, int h, uint8_t *lqt,
				uint8_t *cqt, uint16_t dri);

static void MakeTables(int q, uint8_t *lqt, uint8_t *cqt);

static uint8_t* MakeDRIHeader(uint8_t *p, uint16_t dri);

static uint8_t* MakeHuffmanHeader(uint8_t *p, uint8_t *codelens, int ncodes,
				uint8_t *symbols, int nsymbols, int tableNo,
				int tableClass);

static uint8_t* MakeQuantHeader(uint8_t *p, uint8_t *qt, int tableNo);