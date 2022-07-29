#include "decoder.h"

in_node_ano_t nodebuf[NUMENTRIES];
uchar depth_b = 0;
uchar depth_t = 0;
uchar nb_ptr = 0;

ou_pair_t find_match();
uchar zdist1(uchar&, uchar&); // assume z1 < z2
uchar zdist2(uchar&, uchar&); // no assumption
uchar dist1(point_t&, point_t&); // assume p1.z < p2.z
uchar dist2(point_t&, point_t&); // no assumption
uint16_t calc_t(in_node_ano_t&);
bool nb_is_full();
bool push_nb(in_node_ano_t&);
bool pop_nb(uchar);

int decoder(AXI_STREAM_ANO_IN &in, AXI_STREAM_OU &ou) {
#pragma HLS dataflow

#pragma HLS INTERFACE axis port=in
#pragma HLS data_pack variable=in
#pragma HLS INTERFACE axis port=ou

// #pragma HLS INTERFACE s_axilite port=codesize bundle=BUS_AXI4LS
#pragma HLS INTERFACE s_axilite port=return bundle=BUS_AXI4LS

#pragma HLS array_partition variable=nodebuf dim=0
#pragma HLS reset variable=nodebuf
#pragma HLS reset variable=depth_b
#pragma HLS reset variable=depth_t
#pragma HLS reset variable=nb_ptr

	// data flow : input_stream -> temporal_memory -> node_buffer -> output_stream
	// variables:
	//  tmp_empty: 		True if temporal_memory is not used.
	//  nb_ptr: 		Number of nodes in node_buffer
	//  depth_t_local: 	The largest Z in the node_buffer = Z of the node at the top of buffer.
	//  depth_t: 		The largest Z that is allowed to match
	//  depth_b: 		The smallest Z in the node_buffer = Z of the node at the bottom of buffer.
	//  pop_ena:		True if node is NOT popped from the node_buffer.

	// In temporal_memory is not in use, decoder caches in_node to a temporal_memory (size=1).
	//  If temporal_memory is in use, tmp_empty = false. If not in use, tmp_empty = true.
	//
	// If node_buffer is not full, and temporal_memory is in use, fetch it and process the followings.
	//  If the node is not dummy (x!=255), push the node to node buffer. This increment nb_ptr, which keeps the number of nodes in node buffer.
	//  update depth_t_local, which is the largest Z-position in the node buffer.
	//  Set temporal_memory unused.
	//
	// If nothing matched in the last loop, and if depth_t_local is ahead of depth_t, increment depth_t.
	//
	// If depth_b >= depth_t, and if node_buffer is not emtpy, find match and pop them if found.
	//  If popped, set pop_ena = 0



	in_node_ano_t in_node; // temporal buffer for a new node
	bool pop_ena = 1; // flag for aquiring a new node from stream
	bool tmp_empty = 1;
	bool call_find_match = 0;

    // initialize depths processing now
    depth_b = 0;
    depth_t = 0;
    uchar depth_t_local = 0;

	for (int i=0; i<MAXCYCLE; i++) {
		if (tmp_empty) {
			in >> in_node; // pop new node from stream
#ifdef DEBUG_PRINT
			if(in_node.p.x!=255) printf("*decoder.cpp IN: (%d,%d,%d %d) anomaly(%d,%d,%d,%d)\n",in_node.p.x, in_node.p.y, in_node.p.z, in_node.d_b,in_node.q.x, in_node.q.y, in_node.q.z, in_node.d_a);
#endif
			tmp_empty = 0;
		}
		if (!nb_is_full() && !tmp_empty) {
			if (in_node.p.x != 255) push_nb(in_node); // x = 255 -> no active node
			depth_t_local = in_node.p.z;
			tmp_empty = 1;
		}

		/*
		call_find_match = (!pop_ena);
		if(pop_ena && zdist1(depth_t, depth_t_local) > 1) {
			depth_t++;
			call_find_match = 1;
		}
		pop_ena = 1;
		if (call_find_match && nb_ptr > 0) {
		*/
		if(pop_ena && zdist1(depth_t, depth_t_local) > 1) depth_t++;
		pop_ena = 1;
		if (zdist1(depth_b, depth_t) > 0 && nb_ptr > 0) {
			ou_pair_t mp;
#ifdef DEBUG_PRINT
            printf("\n*find_match @ (depth_b, depth_t, depth_t_local) = (%d,%d,%d)\n",depth_b,depth_t, depth_t_local);
            printf(" * buffer content\n");
            for(int i=0;i<nb_ptr;++i){
            	printf(" %d : (%d,%d,%d)\n",i,nodebuf[i].p.x, nodebuf[i].p.y, nodebuf[i].p.z);
            }
            printf("\n");
#endif
            mp = find_match();
            if (mp.p1.x != 255) {
#ifdef DEBUG_PRINT
                printf("*decoder.cpp OUT: (%d,%d,%d)-(%d,%d,%d) at depth_t=%d\n",mp.p1.x, mp.p1.y, mp.p1.z, mp.p2.x, mp.p2.y, mp.p2.z,depth_t);
#endif
                ou << mp; // push matching result to stream
                pop_ena = 0;
            }
        }
    }

    return 0;
}


uchar encode_idx(bool d[], bool f) {
// #pragma HLS inline
    for (uchar i=0; i<NUMENTRIES; i++) {
#pragma HLS unroll
        bool t = (f) ? d[NUMENTRIES-i-1] : d[i];
        uchar ret = (f) ? NUMENTRIES-i-1 : i;
        if (t) return ret;
    }
    return NUMENTRIES;
}

void fm_compare_distance(uint16_t in[], bool ou[]) {
#pragma HLS pipeline ii=2
    for (uchar i=0; i<NUMENTRIES-1; i++) {
#pragma HLS unroll
        ou[i] = in[i] != (1UL<<16) - 1 ? 1 : 0;
        for (uchar k=i+1; k<NUMENTRIES; k++) {
#pragma HLS unroll
            ou[i] &= (in[i] <= in[k]);
        }
    }
}

bool fm_proc_flag(bool in[]) {
#pragma HLS pipeline ii=2
        bool ret = 0;
        for (uchar i=0; i<NUMENTRIES-1; i++) {
#pragma HLS unroll
            ret |= in[i];
        }
        return ret;
}

bool check_match(bool in[]) {
    bool ret = 0;
    for (uchar j=0; j<NUMENTRIES; j++) {
#pragma HLS unroll
        ret |= in[j];
    }
    return ret;
}

ou_pair_t find_match() {
    bool d_m_2[NUMENTRIES][NUMENTRIES];
#pragma HLS array_partition variable=d_m_2 dim=2
    bool flag_m[NUMENTRIES];
#pragma HLS array_partition variable=flag_m dim=1

    // calculate length of paths and choose shortest for each pair
    for (uchar j=0; j<NUMENTRIES; j++) {
#pragma HLS pipeline ii=2

        uint16_t d_0[NUMENTRIES];
#pragma HLS array_partition variable=d_0 dim=1
        uint16_t d_1[NUMENTRIES];
#pragma HLS array_partition variable=d_1 dim=1
        uint16_t d_5[NUMENTRIES];
#pragma HLS array_partition variable=d_5 dim=1
        uint16_t d_6[NUMENTRIES];
#pragma HLS array_partition variable=d_6 dim=1
        uint16_t d_s[NUMENTRIES];
#pragma HLS array_partition variable=d_s dim=1
        uint16_t d_s_1[NUMENTRIES];
#pragma HLS array_partition variable=d_s_1 dim=1
        uint16_t d_s_2[NUMENTRIES];
#pragma HLS array_partition variable=d_s_2 dim=1

        point_t &p1 = nodebuf[j].p;
        point_t &q1 = nodebuf[j].q;
        uchar &b1 = nodebuf[j].d_b;
        uchar &c1 = nodebuf[j].d_a;
        uchar thv1 = p1.z <= depth_t ? zdist1(p1.z, depth_t) * W_N : 0;

        uint16_t t_0 = calc_t(nodebuf[j]);

        for (uchar i=0; i<NUMENTRIES; i++) {
#pragma HLS unroll

        	// j out of range -> set INF
            if (j >= nb_ptr) {
                d_s[i] = (1UL<<16) - 1;
            }
            // j above depth_t -> set INF
            else if (zdist1(depth_t, p1.z) < zdist1(p1.z, depth_t)) {
#ifdef DEBUG_PRINT
                	//printf("*SKP* %d (%d,%d,%d) z=(%d,%d)\n", j, p1.x, p1.y, p1.z, thv1, depth_t);
#endif
                d_s[i] = (1UL<<16) - 1;
            }
            // j = i -> boundary path
            else if (i == j) {
                d_s[i] = 2 * t_0;
#ifdef DEBUG_PRINT
            	printf("%d-%d (%d,%d,%d)-(%d,%d,%d) : cost=%d thv=(%d,%d) result=%d\n", j, j, p1.x, p1.y, p1.z, p1.x, p1.y, p1.z, d_s[i], thv1, thv1, d_s[i] <= thv1 ? d_s[i] : (1UL<<16) - 1);
#endif
                d_s[i] = d_s[i] <= thv1 ? d_s[i] : (1UL<<16) - 1;
            }
            // 0 <= (i,j) < nb_ptr
            else if (i < nb_ptr) {
                point_t &p2 = nodebuf[i].p;
                point_t &q2 = nodebuf[i].q;
                uchar &b2 = nodebuf[i].d_b;
                uchar &c2 = nodebuf[i].d_a;
                uchar thv2 = p2.z <= depth_t ? (depth_t-p2.z)*W_N : 0;

                // i above depth_t -> set INF
                if (zdist1(depth_t, p2.z) < zdist1(p2.z, depth_t)) {
#ifdef DEBUG_PRINT
                	printf("*SKP* %d-%d (%d,%d,%d)-(%d,%d,%d) thv=(%d,%d) z=(%d,%d,%d)\n", j, i, p1.x, p1.y, p1.z, p2.x, p2.y, p2.z, thv1, thv2, p1.z, p2.z, depth_t);
#endif
                	d_s[i] = (1UL<<16) - 1;
                }
                // 0 <= (i,j) <= depth_t
                else {
                    d_0[i] = t_0 + calc_t(nodebuf[i]);
                    d_5[i] = j < i ? dist1(p1, p2) * W_N : dist1(p2, p1) * W_N;
                    if (q1.x == 255 or q2.x == 255) {
                        d_6[i] = (1UL<<16) - 1;
                    } else {
                        d_6[i] = (c1 + c2) * W_N + dist1(q1, q2) * W_A;
                    }
                	d_1[i] = (b1 + b2) * W_N;
                	d_s_1[i] = d_0[i] < d_1[i] ? d_0[i] : d_1[i];
                	d_s_2[i] = d_5[i] < d_6[i] ? d_5[i] : d_6[i];
                	d_s[i] = d_s_1[i] < d_s_2[i] ? d_s_1[i] : d_s_2[i];
#ifdef DEBUG_PRINT
                	printf("%d-%d (%d,%d,%d)-(%d,%d,%d) : cost=%d 0,1,5,6:(%d,%d,%d,%d) thv=(%d,%d) result=%d\n", j, i, p1.x, p1.y, p1.z, p2.x, p2.y, p2.z, d_s[i], d_0[i], d_1[i],d_5[i],d_6[i], thv1, thv2,(d_s[i] <= thv1 && d_s[i] <= thv2) ? d_s[i] : (1UL<<16) - 1);
#endif
                	d_s[i] = (d_s[i] <= thv1 && d_s[i] <= thv2) ? d_s[i] : (1UL<<16) - 1;
                }
            }
            // i out of range -> set INF
            else {
                d_s[i] = (1UL<<16) - 1;
            }
        }
        fm_compare_distance(d_s, d_m_2[j]);
        flag_m[j] = fm_proc_flag(d_m_2[j]);
    }

    // generate return value and remove matched node(s)
    ou_pair_t ret;
    if (check_match(flag_m)) {
        uchar idx_m_1 = encode_idx(flag_m, 1);
        uchar idx_m_2 = encode_idx(d_m_2[idx_m_1], 0);
        ret.p1 = nodebuf[idx_m_1].p;
        if (idx_m_2 != idx_m_1) {
            ret.p2 = nodebuf[idx_m_2].p;
            if (idx_m_2 > idx_m_1) {
                pop_nb(idx_m_2); pop_nb(idx_m_1);
            } else {
                pop_nb(idx_m_1); pop_nb(idx_m_2);
            }
        } else {
            ret.p2.x = 255; ret.p2.y = 255; ret.p2.z = 255;
            pop_nb(idx_m_1);
        }
    } else {
        ret.p1.x = 255; ret.p1.y = 255; ret.p1.z = 255;
        ret.p2.x = 255; ret.p2.y = 255; ret.p2.z = 255;
    }

    // update depth_b (global var.)
    depth_b = nodebuf[0].p.z;

    return ret;
}

// allow z pos overflows (assuming z1 < z2)
uchar zdist1(uchar &z1, uchar &z2) {
#pragma HLS inline
    return z2 >= z1 ? uchar(z2 - z1) : uchar(256 + z2 - z1);
}

uchar zdist2(uchar &z1, uchar &z2) {
#pragma HLS inline
    uchar ret;
    uchar d1;
    uchar d2;
    if (z1 == z2) ret = 0;
    else {
        if (z1 < z2) {
            d1 = 255 + z1 - z2;
            d2 = z2 - z1;
        } else {
            d1 = 255 + z2 - z1;
            d2 = z1 - z2;
        }
        ret = (d1 < d2) ? d1 : d2;
    }
    return ret;
}

uint16_t calc_t(in_node_ano_t& n) {
#pragma HLS inline
    uint16_t d_1 = n.d_b * W_N;
    uint16_t d_3 = (n.d_a + X_L) * W_N;
    if (X_L == 255) {
        d_3 = (1UL<<16) - 1;
    } else if (n.p.x < X_L) {
        d_3 += (X_L - n.p.x) * W_A;
    } else {
        d_3 += (n.p.x - X_L) * W_A;
    }
    uint16_t d_4 = (n.d_a + CODESIZE - X_R) * W_N;
    if (X_R == 255) {
        d_4 = (1UL<<16) - 1;
    } else if (n.p.x < X_R) {
        d_4 += (X_R - n.p.x) * W_A;
    } else {
        d_4 += (n.p.x - X_R) * W_A;
    }
    uint16_t ret;
    if (d_1 < d_3) {
        if (d_1 < d_4) {
            ret = d_1;
        } else {
            ret = d_4;
        }
    } else if (d_3 < d_4) {
        ret = d_3;
    } else {
        ret = d_4;
    }
    return ret;
}

// calculate distance between two 3-d positions
uchar dist1(point_t &p1, point_t &p2) {
#pragma HLS inline
    uchar dx = p1.x > p2.x ? p1.x - p2.x : p2.x - p1.x;
    uchar dy = p1.y > p2.y ? p1.y - p2.y : p2.y - p1.y;
    uchar dz = zdist1(p1.z, p2.z); // to consider overflow
    uint16_t d = uint16_t(dx) + uint16_t(dy) + uint16_t(dz);
    uchar ret = d < 256 ? uchar(d) : uchar(255);
    return ret;
}

uchar dist2(point_t &p1, point_t &p2) {
#pragma HLS inline
    uchar dx = p1.x > p2.x ? p1.x - p2.x : p2.x - p1.x;
    uchar dy = p1.y > p2.y ? p1.y - p2.y : p2.y - p1.y;
    uchar dz = zdist2(p1.z, p2.z); // to consider overflow
    uint16_t d = (uint16_t)dx + (uint16_t)dy + (uint16_t)dz;
    uchar ret = d < 256 ? (uchar)d : 255;
    return ret;
}

// return whether nodebuf is full or not
bool nb_is_full() {
    return nb_ptr == NUMENTRIES-1 ? 1 : 0;
}

// append node to the tail
bool push_nb(in_node_ano_t &node) {
    if (nb_ptr == NUMENTRIES - 1) {
        return 1; // FULL
    } else {
        nodebuf[nb_ptr++] = node;
    }
    return 0;
}

// remove node using idx
bool pop_nb(uchar idx) {
    if (nb_ptr == 0 || idx > nb_ptr) {
        return 1; // EMPTY OR INVALID IDX
    } else {
        nb_ptr--;
        for (uchar i=0; i<NUMENTRIES-1; i++) {
#pragma HLS unroll
            if (i >= idx) {
                nodebuf[i] = nodebuf[i+1];
            }
        }
    }
    return 0;
}
