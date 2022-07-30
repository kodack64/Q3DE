#include "decoder.h"

in_node_t nodebuf[NUMENTRIES];
uchar depth_b = 0;
uchar depth_t = 0;
uchar nb_ptr = 0;

ou_pair_t find_match();
uchar zdist(uchar, uchar);
uchar dist(point_t&, point_t&);
bool nb_is_full();
bool push_nb(in_node_t&);
bool pop_nb(uchar);

int decoder(AXI_STREAM_IN &in, AXI_STREAM_OU &ou) {
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

	in_node_t in_node; // temporal buffer for a new node
	bool pop_ena = 1; // flag for aquiring a new node from stream
	bool tmp_empty = 1;

    // initialize depths processing now
    depth_b = 0;
    depth_t = 0;
    uchar depth_t_local = 0;

	for (int i=0; i<MAXCYCLE; i++) {
		if (tmp_empty) {
			in >> in_node; // pop new node from stream
			tmp_empty = 0;
		}
		if (!nb_is_full() && !tmp_empty) {
			if (in_node.p.x != 255) push_nb(in_node); // x = 255 -> no active node
			depth_t_local = in_node.p.z;
			tmp_empty = 1;
		}
		if(pop_ena && depth_t_local != depth_t) depth_t++;
		pop_ena = 1;
		if (zdist(depth_b, depth_t) > 0 && nb_ptr > 0) {
            ou_pair_t mp;
            mp = find_match();
            if (mp.p1.x != 255) {
                ou << mp; // push matching result to stream
                pop_ena = 0;
            }
		}
	}

	return 0;
}

uchar encode_idx(bool d[], bool f) {
    for (uchar i=0; i<NUMENTRIES; i++) {
#pragma HLS unroll
        bool t = (f) ? d[NUMENTRIES-i-1] : d[i];
        uchar ret = (f) ? NUMENTRIES-i-1 : i;
        if (t) {
        	return ret;
        }
    }
    return NUMENTRIES;
}

void fm_compare_distance(uchar in[], bool ou[]) {
#pragma HLS pipeline ii=2
    for (uchar i=0; i<NUMENTRIES-1; i++) {
#pragma HLS unroll
        ou[i] = in[i] != 255 ? 1 : 0;
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

        uchar d_0[NUMENTRIES];
#pragma HLS array_partition variable=d_0 dim=1
        uchar d_1[NUMENTRIES];
#pragma HLS array_partition variable=d_1 dim=1
        uchar d_s[NUMENTRIES];
#pragma HLS array_partition variable=d_s dim=1

        point_t &p1 = nodebuf[j].p;
        uchar &b1 = nodebuf[j].d_b;
        uchar thv = zdist(p1.z, depth_t);

        for (uchar i=0; i<NUMENTRIES; i++) {
#pragma HLS unroll
            if (j >= nb_ptr) {
                d_s[i] = 255;
            } else if (zdist(depth_t, p1.z) <= zdist(p1.z, depth_t)) {
            	d_s[i] = 255;
            } else if (i == j) {
                d_s[i] = 2 * b1;
                d_s[i] = d_s[i] <= thv ? d_s[i] : 255;
            } else if (i < nb_ptr) {
                point_t &p2 = nodebuf[i].p;
                uchar &b2 = nodebuf[i].d_b;

                if (zdist(depth_t, p2.z) <= zdist(p2.z, depth_t)) {
                	d_s[i] = 255;
                } else {
                	d_0[i] = j < i ? dist(p1, p2) : dist(p2, p1);
                	d_1[i] = b1 + b2;
                	d_s[i] = d_0[i] < d_1[i] ? d_0[i] : d_1[i];
                	d_s[i] = (d_s[i] <= thv && d_s[i] <= zdist(p2.z, depth_t)) ? d_s[i] : 255;
                }
            } else {
                d_s[i] = 255;
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
uchar zdist(uchar z1, uchar z2) {
	return z2 >= z1 ? z2 - z1 : 256 + z2 - z1;
}

// calculate distance between two 3-d positions
uchar dist(point_t &p1, point_t &p2) {
#pragma HLS inline
	uchar dx = p1.x > p2.x ? p1.x - p2.x : p2.x - p1.x;
	uchar dy = p1.y > p2.y ? p1.y - p2.y : p2.y - p1.y;
	uchar dz = zdist(p1.z, p2.z); // to consider overflow
	uint16_t d = (uint16_t)dx + (uint16_t)dy + (uint16_t)dz;
	uchar ret = d < 256 ? (uchar)d : 255;
	return ret;
}

// return whether nodebuf is full or not
bool nb_is_full() {
	return nb_ptr == NUMENTRIES-1 ? 1 : 0;
}

// append node to the tail
bool push_nb(in_node_t &node) {
	if (nb_ptr == NUMENTRIES) {
		return 1; // FULL
	} else {
		nodebuf[nb_ptr] = node;
		nb_ptr++;
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
