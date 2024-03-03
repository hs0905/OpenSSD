import RISA_PKG::*;

module AXI_reg_intf(
    input logic clk,
    input logic rstn,
    input logic[AXI_LITE_ARG_NUM-1:0][AXI_LITE_WORD_WIDTH-1:0] kernel_engine_status,
    input axi_lite_output AXI_LITE_output,          

    output axi_lite_input AXI_LITE_input,
    output logic [7:0] kernel_command,
    output logic kernel_command_new,
    output logic[AXI_LITE_ARG_NUM-1:0][AXI_LITE_WORD_WIDTH-1:0] kernel_engine_arg
);

typedef struct packed {
    logic arready;
    logic rvalid;
    logic awready;
    logic wready;
    logic waddr_received;
    logic wdata_received;
    logic bvalid;
    
    logic kernel_command_new;

    logic [$clog2(AXI_LITE_ARG_NUM)-1:0]  write_reg_idx;
    logic [AXI_LITE_WORD_WIDTH-1:0]       write_reg_data;
    logic [AXI_LITE_WORD_WIDTH-1:0]       read_reg_data;

    logic [AXI_LITE_WORD_WIDTH-1:0]       firmware_signal;

    logic [AXI_LITE_ARG_NUM-1:0][AXI_LITE_WORD_WIDTH-1:0] kregs;
} reg_control;

reg_control reg_ctrl, reg_ctrl_next;

localparam BASE_ADDR              = 32'h43C80000; // AXI 슬레이브 베이스 주소
localparam CMD_DONE_OFFSET        = 32'h0;
localparam CMD_OFFSET             = 32'h4;
localparam DATA_REGNUM_OFFSET     = 32'h8;
localparam DATA_BITNUM_OFFSET     = 32'hC;
localparam DATA_COUNT_OFFSET      = 32'h10;
localparam FIRMWARE_SIGNAL_OFFSET = 32'h14; // 새로운 레지스터의 오프셋

always_comb begin
    reg_ctrl_next = reg_ctrl;

    AXI_LITE_input.arready    = reg_ctrl.arready;
    AXI_LITE_input.awready    = reg_ctrl.awready;
    AXI_LITE_input.bvalid     = reg_ctrl.bvalid;
    AXI_LITE_input.bresp      = 2'b00;          
    AXI_LITE_input.rvalid     = reg_ctrl.rvalid;
    AXI_LITE_input.rresp      = 2'b00; 
    AXI_LITE_input.wready     = reg_ctrl.wready;

    // Address write (AW) channel processing
    if (AXI_LITE_output.awvalid && reg_ctrl.awready) begin
        reg_ctrl_next.awready = 0; // AW handshake complete
        reg_ctrl_next.waddr_received = 1;
        // Calculate offset from base address
        reg_ctrl_next.write_reg_idx = (AXI_LITE_output.awaddr - BASE_ADDR) >> 2; // Assuming 32-bit wide registers
    end

    // Write data (W) channel processing
    if (AXI_LITE_output.wvalid && reg_ctrl.wready && reg_ctrl.waddr_received) begin
        reg_ctrl_next.wdata_received = 1;
        reg_ctrl_next.wready = 0; // W handshake complete

        // Write data to the corresponding register based on the calculated index
        case (reg_ctrl.write_reg_idx)
            (CMD_DONE_OFFSET >> 2): reg_ctrl_next.kregs[0] = AXI_LITE_output.wdata;
            (CMD_OFFSET >> 2): reg_ctrl_next.kregs[1] = AXI_LITE_output.wdata;
            (DATA_REGNUM_OFFSET >> 2): reg_ctrl_next.kregs[2] = AXI_LITE_output.wdata;
            (DATA_BITNUM_OFFSET >> 2): reg_ctrl_next.kregs[3] = AXI_LITE_output.wdata;
            (DATA_COUNT_OFFSET >> 2): reg_ctrl_next.kregs[4] = AXI_LITE_output.wdata;
            (FIRMWARE_SIGNAL_OFFSET >> 2): reg_ctrl_next.firmware_signal = AXI_LITE_output.wdata; // Directly writing to firmware_signal
            default: ; // Handle invalid addresses or extend for more registers
        endcase
    end

    // Read address (AR) channel processing
    if (AXI_LITE_output.arvalid && reg_ctrl.arready) begin
        reg_ctrl_next.arready = 0; // AR handshake complete
        reg_ctrl_next.rvalid = 1; // Mark data as valid for read response
        // Calculate offset and prepare read data
        case ((AXI_LITE_output.araddr - BASE_ADDR) >> 2) // Assuming 32-bit wide registers
            (CMD_DONE_OFFSET >> 2): reg_ctrl_next.read_reg_data = reg_ctrl.kregs[0];
            (CMD_OFFSET >> 2): reg_ctrl_next.read_reg_data = reg_ctrl.kregs[1];
            (DATA_REGNUM_OFFSET >> 2): reg_ctrl_next.read_reg_data = reg_ctrl.kregs[2];
            (DATA_BITNUM_OFFSET >> 2): reg_ctrl_next.read_reg_data = reg_ctrl.kregs[3];
            (DATA_COUNT_OFFSET >> 2): reg_ctrl_next.read_reg_data = reg_ctrl.kregs[4];
            (FIRMWARE_SIGNAL_OFFSET >> 2): reg_ctrl_next.read_reg_data = reg_ctrl.firmware_signal; // Reading firmware_signal
            default: reg_ctrl_next.read_reg_data = 32'hDEAD_BEEF; // Default or error value
        endcase
    end

    // Reset or initialize
    if (!rstn) begin
        reg_ctrl_next = '0; // Reset all fields to 0 or appropriate reset values
        reg_ctrl_next.arready = 1;
        reg_ctrl_next.awready = 1;
        reg_ctrl_next.wready = 1;
    end
end

always_ff @(posedge clk) begin
    if (!rstn)
        reg_ctrl <= '0; // Reset state
    else
        reg_ctrl <= reg_ctrl_next; // Update state
end

endmodule
