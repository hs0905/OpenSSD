`ifndef __DEFINES_VH__
`define __DEFINES_VH__

`define CEIL(x,y) (((x) + (y) -1)/(y))
`define CEILDIV(x,y) (((x) + (y) -1)/(y))
`define MAX(x,y) ( ( (x) > (y) ) ? (x) : (y) )
`define MIN(x,y) ( ( (x) < (y) ) ? (x) : (y) )

`define DDR_ADDR_WIDTH		32
`define DDR_DATA_WIDTH		64
`define DDR_ID_WIDTH      4

`define GP_ADDR_WIDTH		32
`define GP_DATA_WIDTH		32

`define AXI_OUTPUT_IDLE       '{default:'0, arburst:2'b01, arsize : 3'b011, awburst : 2'b01, awsize : 3'b011, bready : 1'b1, rready : 1'b1, wstrb : '1}
`define AXI_READ_OUTPUT_IDLE  '{default:'0, arburst:2'b01, arsize : 3'b011, rready : 1'b1}
`define AXI_WRITE_OUTPUT_IDLE '{default:'0, awburst : 2'b01, awsize : 3'b011, bready : 1'b1, wstrb : '1}

`define CMD_SOFT_RSTN     8'h01
`define CMD_DATA 8'h10
 

`endif //__DEFINES_VH__