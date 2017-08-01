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

/*
 * HLS NVME Example
 * Adopt SNAP's framework for FPGA hardware action part.
 * Use Xilinx High Level Synthesis (HLS) tool to provide
 * example functions for data movements.
 * Using FGT card, need to set
 * export NVME_USED=TRUE
 * export SDRAM_USED=TRUE
 */


/* Version
 * 2017/7/8    0.1 Prepare Environment
 */

#include <string.h>
#include "ap_int.h"
#include <hls_stream.h>
#include "action_hls_nvme_memcopy.H"

#define HW_RELEASE_LEVEL       0x00000001


//--------------------------------------------------------------------------------------------
snapu32_t read_bulk ( snap_membus_t *src_mem,
        snapu64_t      byte_address,
        snapu32_t      byte_to_transfer,
        snap_membus_t *buffer)
{
    snapu32_t xfer_size;
    xfer_size = MIN(byte_to_transfer, (snapu32_t) MAX_NB_OF_BYTES_READ);
// Patch to Issue#320 - memcopy doesn't handle 4Kbytes xfer
    int xfer_size_in_words;
    if(xfer_size %BPERDW == 0)
        xfer_size_in_words = xfer_size/BPERDW;
    else
        xfer_size_in_words = (xfer_size/BPERDW) + 1;

    //memcpy(buffer,
    //       (snap_membus_t *) (src_mem + (byte_address >> ADDR_RIGHT_SHIFT)),
    //       xfer_size);
    rb_loop: for (int k=0; k< xfer_size_in_words; k++)
#pragma HLS PIPELINE
        buffer[k] = (src_mem + (byte_address >> ADDR_RIGHT_SHIFT))[k];
    
    return xfer_size;
}


snapu32_t write_bulk (snap_membus_t *tgt_mem,
        snapu64_t      byte_address,
        snapu32_t      byte_to_transfer,
        snap_membus_t *buffer)
{
    snapu32_t xfer_size;
    xfer_size = MIN(byte_to_transfer, (snapu32_t)  MAX_NB_OF_BYTES_READ);
    // Patch to Issue#320 - memcopy doesn't handle 4Kbytes xfer
    int xfer_size_in_words;
    if(xfer_size %BPERDW == 0)
        xfer_size_in_words = xfer_size/BPERDW;
    else
        xfer_size_in_words = (xfer_size/BPERDW) + 1;
    
    //memcpy((snap_membus_t *)(tgt_mem + (byte_address >> ADDR_RIGHT_SHIFT)), buffer, xfer_size);

    wb_loop: for (int k=0; k<xfer_size_in_words; k++)
#pragma HLS PIPELINE
          (tgt_mem + (byte_address >> ADDR_RIGHT_SHIFT))[k] = buffer[k];
    return xfer_size;
}

void memcopy_table(snap_membus_t  *din_gmem,
        snap_membus_t  *dout_gmem,
        snap_membus_t  *d_ddrmem,
        snapu64_t       source_address ,
        snapu64_t       target_address,
        snapu32_t       total_bytes_to_transfer,
        snapu16_t       direction)
{

    //source_address and target_address are byte addresses.
    snapu64_t address_xfer_offset = 0;
    snap_membus_t   buf_gmem[MAX_NB_OF_BYTES_READ/BPERDW];


    snapu32_t  left_bytes = total_bytes_to_transfer;
    snapu32_t  copy_bytes;


    // Be cautious with "unsigned", it always >=0
L_COPY: while (left_bytes > 0)
        {
            
            if(direction == HOST2DDR) {
                copy_bytes = read_bulk (din_gmem, source_address + address_xfer_offset,  left_bytes, buf_gmem);
                write_bulk (d_ddrmem, target_address + address_xfer_offset,  copy_bytes, buf_gmem);
            } else if (direction == DDR2HOST) {
                copy_bytes = read_bulk (d_ddrmem, source_address + address_xfer_offset,  left_bytes, buf_gmem);
                write_bulk (dout_gmem, target_address + address_xfer_offset,  copy_bytes, buf_gmem);
            } else if (direction == DDR2DDR) {
                copy_bytes = read_bulk (d_ddrmem, source_address + address_xfer_offset,  left_bytes, buf_gmem);
                write_bulk (d_ddrmem, target_address + address_xfer_offset,  copy_bytes, buf_gmem);
            }
            left_bytes -= copy_bytes;
            address_xfer_offset += MAX_NB_OF_BYTES_READ;
        } // end of L_COPY

}

//--------------------------------------------------------------------------------------------
//--- MAIN PROGRAM ---------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
void hls_action(snap_membus_t  *din_gmem, 
        snap_membus_t  *dout_gmem,
        snap_membus_t  *d_ddrmem,
        snap_membus_t  *d_nvme,
        action_reg *Action_Register, action_RO_config_reg *Action_Config)
{
    // Host Memory AXI Interface
#pragma HLS INTERFACE m_axi port=din_gmem bundle=host_mem offset=slave depth=512 \
    max_read_burst_length=64 max_write_burst_length=64
#pragma HLS INTERFACE m_axi port=dout_gmem bundle=host_mem offset=slave depth=512 \
    max_read_burst_length=64 max_write_burst_length=64
#pragma HLS INTERFACE s_axilite port=din_gmem bundle=ctrl_reg 		offset=0x030
#pragma HLS INTERFACE s_axilite port=dout_gmem bundle=ctrl_reg 		offset=0x040

    //DDR memory Interface
#pragma HLS INTERFACE m_axi port=d_ddrmem bundle=card_mem0 offset=slave depth=512 \
    max_read_burst_length=64 max_write_burst_length=64
#pragma HLS INTERFACE s_axilite port=d_ddrmem bundle=ctrl_reg 		offset=0x050
    
    //NVME Config Interface
#pragma HLS INTERFACE m_axi port=d_nvme bundle=nvme offset=slave 
#pragma HLS INTERFACE s_axilite port=d_nvme bundle=ctrl_reg 		offset=0x060


    // Host Memory AXI Lite Master Interface
#pragma HLS DATA_PACK variable=Action_Config
#pragma HLS INTERFACE s_axilite port=Action_Config bundle=ctrl_reg	offset=0x010
#pragma HLS DATA_PACK variable=Action_Register
#pragma HLS INTERFACE s_axilite port=Action_Register bundle=ctrl_reg	offset=0x100
#pragma HLS INTERFACE s_axilite port=return bundle=ctrl_reg

    // VARIABLES
    snapu32_t ReturnCode=SNAP_RETC_SUCCESS;

    //memcopy
    snapu64_t input_address;
    snapu64_t output_address;
    snapu32_t xfer_size;

    /* Required Action Type Detection */
    switch (Action_Register->Control.flags) {
        case 0:
            Action_Config->action_type = (snapu32_t)HLS_NVME_MEMCOPY_ACTION_TYPE;
            Action_Config->release_level = (snapu32_t)HW_RELEASE_LEVEL;
            Action_Register->Control.Retc = (snapu32_t)0xe00f;
            return;
        default:
            break;
    }

    input_address = Action_Register->Data.in.addr;
    output_address = Action_Register->Data.out.addr;
    xfer_size = MIN(Action_Register->Data.in.size, 
                    Action_Register->Data.out.size);

    memcopy_table(din_gmem, dout_gmem, d_ddrmem,  
                input_address, output_address, xfer_size, HOST2DDR);

    Action_Register->Control.Retc = (snapu32_t) ReturnCode;
    return;
}


