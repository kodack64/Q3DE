#ifndef DECODER_H
#define DECODER_H

#include <ap_int.h>
#include <hls_stream.h>

#define DEBUG_PRINT

#define MAXCYCLE 1000
#define CODESIZE 7
// #define NUMENTRIES 16 // at least 16
// #define NUMENTRIES 64 // at least 16
#define NUMENTRIES 70 // the case for d=11
// #define NUMENTRIES 98 // the case for d=13
// #define NUMENTRIES 256 // at least 16

#define W_N 1
#define W_A 0

#define X_L 1
#define X_R 2
//#define X_L 255
//#define X_R 255

typedef unsigned char uchar;
typedef unsigned short uint16_t;

typedef struct {
	uchar x;
	uchar y;
	uchar z;
} point_t;

typedef struct {
	point_t p; // position of the node
	uchar d_b; // distance to boundary
	point_t q; // nearest anomarous node
	uchar d_a; // distance to anomaly
    // uchar x_L; // left edge of anomaly
    // uchar x_R; // right edge of anomaly
} in_node_ano_t;

typedef struct {
	point_t p1;
	point_t p2;
} ou_pair_t;

typedef hls::stream<in_node_ano_t> AXI_STREAM_ANO_IN;
typedef hls::stream<ou_pair_t> AXI_STREAM_OU;

int decoder(AXI_STREAM_ANO_IN &in, AXI_STREAM_OU &ou);

#endif
