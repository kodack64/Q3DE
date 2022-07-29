#ifndef DECODER_H
#define DECODER_H

#include <ap_int.h>
#include <hls_stream.h>

#define MAXCYCLE 1000
// #define CODESIZE 9
// #define NUMENTRIES 64 // at least 16
#define NUMENTRIES 30 // the case for d=11
// #define NUMENTRIES 98 // the case for d=13
// #define NUMENTRIES 256 // at least 16

#define USE_RING
#define ENABLE_COPY_DM2

#ifdef USE_RING
#define RINGSIZE 2
#endif

typedef unsigned char uchar;
typedef unsigned short uint16_t;

typedef struct {
	uchar x;
	uchar y;
	uchar z;
} point_t;

typedef struct {
	point_t p;
	uchar d_b;
} in_node_t;

typedef struct {
	point_t p1;
	point_t p2;
} ou_pair_t;

typedef hls::stream<in_node_t> AXI_STREAM_IN;
typedef hls::stream<ou_pair_t> AXI_STREAM_OU;

int decoder(AXI_STREAM_IN &in, AXI_STREAM_OU &ou);

#endif
