// Copyright 2018 ETH Zurich and University of Bologna.
// Copyright and related rights are licensed under the Solderpad Hardware
// License, Version 0.51 (the "License"); you may not use this file except in
// compliance with the License.  You may obtain a copy of the License at
// http://solderpad.org/licenses/SHL-0.51. Unless required by applicable law
// or agreed to in writing, software, hardware and materials distributed under
// this License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
// CONDITIONS OF ANY KIND, either express or implied. See the License for the
// specific language governing permissions and limitations under the License.
//
// Engineer:       Francesco Conti - f.conti@unibo.it
//
// Additional contributions by:
//                 Markus Wegmann - markus.wegmann@technokrat.ch
//
// Design Name:    RISC-V register file
// Project Name:   zero-riscy
// Language:       SystemVerilog
//
// Description:    Register file with 31 or 15x 32 bit wide registers.
//                 Register 0 is fixed to 0. This register file is based on
//                 flip flops.
//


module ariane_regfile #(
  parameter int unsigned DATA_WIDTH     = 32,
  parameter int unsigned NR_READ_PORTS  = 2,
  parameter int unsigned NR_WRITE_PORTS = 2,
  parameter bit          ZERO_REG_ZERO  = 0
)(
  // clock and reset
  input  logic                                      clk_i,
  input  logic                                      rst_ni,
  // disable clock gates for testing
  input  logic                                      test_en_i,
  // read port
  input  logic [NR_READ_PORTS-1:0][4:0]             raddr_i,
  output logic [NR_READ_PORTS-1:0][DATA_WIDTH-1:0]  rdata_o,
  // write port
  input  logic [NR_WRITE_PORTS-1:0][4:0]            waddr_i,
  input  logic [NR_WRITE_PORTS-1:0][DATA_WIDTH-1:0] wdata_i,
  input  logic [NR_WRITE_PORTS-1:0]                 we_i
);

  localparam    ADDR_WIDTH = 5;
  localparam    NUM_WORDS  = 2**ADDR_WIDTH;

  logic [NUM_WORDS-1:0][DATA_WIDTH-1:0]     mem;
  logic [NR_WRITE_PORTS-1:0][NUM_WORDS-1:0] we_dec;


    always_comb begin : we_decoder
        for (int unsigned j = 0; j < NR_WRITE_PORTS; j++) begin
            for (int unsigned i = 0; i < NUM_WORDS; i++) begin
                if (waddr_i[j] == i)
                    we_dec[j][i] = we_i[j];
                else
                    we_dec[j][i] = 1'b0;
            end
        end
    end

    // loop from 1 to NUM_WORDS-1 as R0 is nil
    always_ff @(posedge clk_i, negedge rst_ni) begin : register_write_behavioral
        if (~rst_ni) begin
            mem <= '{default: '0};
        end else begin
            for (int unsigned j = 0; j < NR_WRITE_PORTS; j++) begin
                for (int unsigned i = 0; i < NUM_WORDS; i++) begin
                    if (we_dec[j][i]) begin
                        mem[i] <= wdata_i[j];
                    end
                end
                if (ZERO_REG_ZERO) begin
                  mem[0] <= '0;
                end
            end
        end
    end

  for (genvar i = 0; i < NR_READ_PORTS; i++) begin
    assign rdata_o[i] = mem[raddr_i[i]];
  end

endmodule


// Copyright 2018 ETH Zurich and University of Bologna.
// Copyright and related rights are licensed under the Solderpad Hardware
// License, Version 0.51 (the "License"); you may not use this file except in
// compliance with the License.  You may obtain a copy of the License at
// http://solderpad.org/licenses/SHL-0.51. Unless required by applicable law
// or agreed to in writing, software, hardware and materials distributed under
// this License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
// CONDITIONS OF ANY KIND, either express or implied. See the License for the
// specific language governing permissions and limitations under the License.
//
// Engineer:       Francesco Conti - f.conti@unibo.it
//
// Additional contributions by:
//                 Markus Wegmann - markus.wegmann@technokrat.ch
//
// Design Name:    RISC-V register file
// Project Name:   zero-riscy
// Language:       SystemVerilog
//
// Description:    Register file with 31 or 15x 32 bit wide registers.
//                 Register 0 is fixed to 0. This register file is based on
//                 flip flops.
//
/*
import RISA_PKG::*;
module ariane_regfile_debug #(
  parameter int unsigned DATA_WIDTH     = 32,
  parameter int unsigned NR_READ_PORTS  = 2,
  parameter int unsigned NR_WRITE_PORTS = 2,
  parameter bit          ZERO_REG_ZERO  = 0
)(
  // clock and reset
  input  logic                                      clk_i,
  input  logic                                      rst_ni,
  // disable clock gates for testing
  input  logic                                      test_en_i,
  // read port
  input  logic [NR_READ_PORTS-1:0][4:0]             raddr_i,
  output logic [NR_READ_PORTS-1:0][DATA_WIDTH-1:0]  rdata_o,
  // write port
  input  logic [NR_WRITE_PORTS-1:0][4:0]            waddr_i,
  input  logic [NR_WRITE_PORTS-1:0][DATA_WIDTH-1:0] wdata_i,
  input  logic [NR_WRITE_PORTS-1:0]                 we_i,
  
  input     CommandDataPort            commanddataport,
  output    StatePort                  stateport
  
);

  typedef struct packed{
    logic [COMMAND_WIDTH-1:0] command;        
    logic [FSIZE-1:0]  command_data0;
    logic [FSIZE-1:0]  command_data1;

    logic [FSIZE-1:0] cmd_state;
//    logic [NR_READ_PORTS-1:0][4:0]  raddr_temp;
  } Registers;
  
  Registers reg_current,reg_next, reg_ff;
  
  localparam    ADDR_WIDTH = 5;
  localparam    NUM_WORDS  = 2**ADDR_WIDTH;

  logic [NUM_WORDS-1:0][DATA_WIDTH-1:0]     mem;
  logic [NR_WRITE_PORTS-1:0][NUM_WORDS-1:0] we_dec;

//  logic [NUM_WORDS-1:0][15:0]               counter;

    always_comb begin : we_decoder
        for (int unsigned j = 0; j < NR_WRITE_PORTS; j++) begin
            for (int unsigned i = 0; i < NUM_WORDS; i++) begin
                if (waddr_i[j] == i)
                    we_dec[j][i] = we_i[j];
                else
                    we_dec[j][i] = 1'b0;
            end
        end
    end

    // loop from 1 to NUM_WORDS-1 as R0 is nil
    always_ff @(posedge clk_i, negedge rst_ni) begin : register_write_behavioral
        if (~rst_ni) begin
            mem <= '{default: '0};
        end else begin
            // reg_next = reg_current;

//            for (int unsigned i = 0; i < NR_READ_PORTS; i++) begin
//              reg_next.raddr_temp[i] = raddr_i[i];
//              if (reg_current.raddr_temp[i] != raddr_i[i]) begin
//                counter[raddr_i[i]] <= counter[raddr_i[i]] + 1;
//              end
//            end
            for (int unsigned j = 0; j < NR_WRITE_PORTS; j++) begin
                for (int unsigned i = 0; i < NUM_WORDS; i++) begin
                    if (we_dec[j][i]) begin
                        mem[i] <= wdata_i[j];

                        if(reg_ff.command == 1) begin
                            mem[reg_ff.command_data0][reg_ff.command_data1] <= !mem[reg_ff.command_data0][reg_ff.command_data1];
                            reg_ff.command <= 0;
                        end
                        
                    end
                end
                if (ZERO_REG_ZERO) begin
                  mem[0] <= '0;
                end
            end

            
            if(commanddataport.valid) begin
              reg_ff.command <= commanddataport.command;       
              reg_ff.command_data0 <= commanddataport.data0;       
              reg_ff.command_data1 <= commanddataport.data1;   
            end
            
//            if(reg_current.command == 2) begin
//                for (int unsigned i = 0; i < NUM_WORDS; i++) begin
//                    counter[i] = 0;
//                end
//                reg_next.command = 0;
//            end
        end
        // reg_current <= reg_next;
    end

  for (genvar i = 0; i < NR_READ_PORTS; i++) begin
    assign rdata_o[i] = mem[raddr_i[i]];
  end
  
 ila_reg ila_reg(
 .clk(clk_i),
 .probe0(commanddataport.valid),
 .probe1(mem[11]),
 .probe2(mem[12]),
 .probe3(mem[13]),
 .probe4(mem[14])
 );

//    ila_0 ila_counter(
//    .clk(clk_i),
//    .probe0(counter[0]),
//    .probe1(counter[1]),
//    .probe2(counter[2]),
//    .probe3(counter[3]),
//    .probe4(counter[4]),
//    .probe5(counter[5]),
//    .probe6(counter[6]),
//    .probe7(counter[7]),
//    .probe8(counter[8]),
//    .probe9(counter[9]),
//    .probe10(counter[10]),
//    .probe11(counter[11]),
//    .probe12(counter[12]),
//    .probe13(counter[13]),
//    .probe14(counter[14]),
//    .probe15(counter[15]),
//    .probe16(counter[16]),
//    .probe17(counter[17]),
//    .probe18(counter[18]),
//    .probe19(counter[19]),
//    .probe20(counter[20]),
//    .probe21(counter[21]),
//    .probe22(counter[22]),
//    .probe23(counter[23]),
//    .probe24(counter[24]),
//    .probe25(counter[25]),
//    .probe26(counter[26]),
//    .probe27(counter[27]),
//    .probe28(counter[28]),
//    .probe29(counter[29]),
//    .probe30(counter[30]),
//    .probe31(counter[31])
//    );
endmodule
*/
