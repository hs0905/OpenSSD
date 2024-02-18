import RISA_PKG::*;


module AXI_reg_intf( // AXI lite slave interface
    input logic clk,
    input logic rstn,

    input  axi_lite_output AXI_LITE_output,           //output/input name is seen from the master
    output axi_lite_input  AXI_LITE_input,

    output  logic [7:0]	kernel_command,
	  output  logic	      kernel_command_new,
    output  logic[AXI_LITE_ARG_NUM-1:0][AXI_LITE_WORD_WIDTH-1:0]	kernel_engine_arg,
	  input   logic[AXI_LITE_ARG_NUM-1:0][AXI_LITE_WORD_WIDTH-1:0]	kernel_engine_status
  );

  typedef struct {
    logic arready;
    logic rvalid;
    logic awready;
    logic wready;
    logic waddr_received;
    logic wdata_received;
    logic bvalid;
    
    logic kernel_command_new;

    logic [$clog2(AXI_LITE_ARG_NUM)-1:0] write_reg_idx;
    logic [AXI_LITE_WORD_WIDTH-1:0] write_reg_data;
    logic [AXI_LITE_WORD_WIDTH-1:0] read_reg_data;

    logic[AXI_LITE_ARG_NUM-1:0][AXI_LITE_WORD_WIDTH-1:0] kregs;
  } reg_control;

  reg_control reg_ctrl, reg_ctrl_next;
    
  localparam REG_ADDR_IDX_LOW = 2;// $clog2(AXI_LITE_WORD_WIDTH/8) ;//3
  localparam REG_ADDR_IDX_HI = 7;//REG_ADDR_IDX_LOW + $clog2(AXI_LITE_ARG_NUM); //3+5 = 8
    
	always_comb begin
    reg_ctrl_next = reg_ctrl;

    AXI_LITE_input.arready = 0;
    AXI_LITE_input.awready = 0;
    AXI_LITE_input.bresp = 0;
    AXI_LITE_input.bvalid = 0;
    AXI_LITE_input.rdata = 0;
    AXI_LITE_input.rresp = 0;
    AXI_LITE_input.rvalid = 0;
    AXI_LITE_input.wready = 0;

    kernel_engine_arg = reg_ctrl.kregs;
    kernel_command = reg_ctrl.kregs[0];
    kernel_command_new = reg_ctrl.kernel_command_new;

    if(reg_ctrl.arready) begin
      AXI_LITE_input.arready = 1;
      if(AXI_LITE_output.arvalid) begin
        reg_ctrl_next.arready = 0;
        reg_ctrl_next.rvalid = 1;        
        reg_ctrl_next.read_reg_data = kernel_engine_status[ AXI_LITE_output.araddr[REG_ADDR_IDX_HI:REG_ADDR_IDX_LOW] ];
      end
    end

    if(reg_ctrl.rvalid) begin
      AXI_LITE_input.rvalid = 1;
      AXI_LITE_input.rdata = reg_ctrl.read_reg_data;
      if(AXI_LITE_output.rready) begin
        reg_ctrl_next.rvalid = 0;
        reg_ctrl_next.arready = 1;        
      end
    end


    if(reg_ctrl.awready) begin
      AXI_LITE_input.awready = 1;
      if(AXI_LITE_output.awvalid) begin
        reg_ctrl_next.awready = 0;
        reg_ctrl_next.write_reg_idx = AXI_LITE_output.awaddr[REG_ADDR_IDX_HI:REG_ADDR_IDX_LOW];
        reg_ctrl_next.waddr_received = 1;        
      end
    end

    if(reg_ctrl.wready) begin
      AXI_LITE_input.wready = 1;
      if(AXI_LITE_output.wvalid) begin
        reg_ctrl_next.wready = 0;
        reg_ctrl_next.write_reg_data = AXI_LITE_output.wdata;
        reg_ctrl_next.wdata_received = 1;        
      end
    end


    if(reg_ctrl.waddr_received && reg_ctrl.wdata_received) begin
      reg_ctrl_next.kregs[reg_ctrl.write_reg_idx] = reg_ctrl.write_reg_data;
      reg_ctrl_next.bvalid = 1;    
      reg_ctrl_next.waddr_received = 0;        
      reg_ctrl_next.wdata_received = 0;   
      if(reg_ctrl.write_reg_idx == 0)
        reg_ctrl_next.kernel_command_new = 1;     
    end

    if(reg_ctrl.kernel_command_new) begin
      reg_ctrl_next.kernel_command_new = 0;
    end

    if(reg_ctrl.bvalid) begin
      AXI_LITE_input.bvalid = 1;
      if(AXI_LITE_output.bready) begin
        reg_ctrl_next.bvalid = 0;
        reg_ctrl_next.awready = 1;        
        reg_ctrl_next.wready = 1;
      end    
    end

    if(rstn==0) begin
      reg_ctrl_next.kregs[0] = 32'hDEADBEEF;
      reg_ctrl_next.arready = 1;    
      reg_ctrl_next.rvalid = 0;
      reg_ctrl_next.awready = 1;    
      reg_ctrl_next.wready = 1;    
      reg_ctrl_next.waddr_received = 0;    
      reg_ctrl_next.wdata_received = 0;    
      reg_ctrl_next.bvalid = 0;    
      reg_ctrl_next.kernel_command_new = 0;    
    end
  end
    
  always @( posedge clk ) begin
    reg_ctrl <= reg_ctrl_next;
  end
endmodule