#ifndef __ACTION_HLS_NVME_MEMCOPY_HLS_H__
#define __ACTION_HLS_NVME_MEMCOPY_HLS_H__

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
#include <action_hls_nvme_memcopy.h> 

#define MAX_NB_OF_BYTES_READ (64 * 1024)     
#define BPERCL 128                           //128Bytes for one PSL Cacheline
//---------------------------------------------------------------------
typedef struct {
	CONTROL Control;	        /*  16 bytes */
	hls_nvme_job_t Data;	    /* 108 bytes */
	uint8_t padding[SNAP_HLS_JOBSIZE - sizeof(hls_nvme_job_t)];
} action_reg;

#define HOST2DDR 0
#define DDR2HOST 1
#define DDR2DDR 2
#define NVME_RD_DDR 3
#define NVME_WR_DDR 4


#endif  /* __ACTION_HLS_NVME_MEMCOPY_HLS_H__ */
