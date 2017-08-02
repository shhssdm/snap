#ifndef __ACTION_BFS_NVME_HLS_H__
#define __ACTION_BFS_NVME_HLS_H__

/*
 * Copyright 2017, International Business Machines
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "hls_snap.H"
#include <action_bfs_nvme.h> /*BFS job definition*/

#define NODE_BITWIDTH   16
#define MAX_NODE_NUM (uint64_t) (1<<NODE_BITWIDTH)

#define REL_BITWIDTH    32
#define MAX_REL_NUM (uint64_t) (1<<REL_BITWIDTH)
#define END_SIGN_HW (uint64_t) (0xFFFFFFFF)

#define PREFETCH_LINES 16

#define MAX_NB_OF_BYTES_READ (4 * 1024)      //4KBytes
#define BPERCL 128                           //128Bytes for one PSL Cacheline

//DDR Space Allocation
#define NodeRecord_ADDR_DDR (uint64_t) 0
#define RelRecord_ADDR_DDR  (uint64_t) MAX_NODE_NUM * sizeof(NodeRecordFmt_t)

typedef ap_uint<NODE_BITWIDTH> Q_t;


//---------------------------------------------------------------------
typedef struct {
	CONTROL Control;	/*  16 bytes */
	bfs_job_t Data;	/* 108 bytes */
	uint8_t padding[SNAP_HLS_JOBSIZE - sizeof(bfs_job_t)];
} action_reg;


#endif  /* __ACTION_BFS_NVME_HLS_H__ */
