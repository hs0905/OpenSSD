`timescale 1ps/1ps

module Watchdog_FSM(
  input   logic clk,
  input   logic rstn,
  input   logic I_HEARTBEAT_RESET,
  input   logic I_HEARTBEAT_START,
  output  logic O_SYSTEM_RESET
  );

// wating for Heart Heart attack
localparam CLOCK_FREQ     = 50_000_000;  // 50 MHz
localparam TIMEOUT_SEC    = 1;
localparam MAX_COUNT      = CLOCK_FREQ * TIMEOUT_SEC;

// Revive time
localparam RESET_DURATION = 10000;

// FSM state
localparam IDLE       = 2'b00;
localparam WAIT       = 2'b01;
localparam HEARTBEAT  = 2'b10;
localparam REVIVE     = 2'b11;

// FSM state register
logic [1:0] current_state, next_state;

logic system_reset_reg;

// logic for path_selector module instance
logic path_selector_on;
logic path_signal;
wire  wait_end_signal;

// logic for timer module instance
logic timer_on;
wire timeout_wire;



// instanciate path_selector module
  path_selector path_selector_inst
  (
    .clk              (clk),
    .rstn             (rstn),
    .start_signal     (path_selector_on),
    .reset_signal     (I_HEARTBEAT_RESET),
    .wait_end_signal  (wait_end_signal),
    .path_signal      (path_signal)
  );

// instanciate timer module for Revive duration
  timer Revive_duration
  (
    .clk          (clk),
    .rstn         (rstn),
    .timer_on     (timer_on),
    .timeout_wire (timeout_wire)
  );



// FSM default action
always_ff @(posedge clk or negedge rstn) begin
  if(!rstn) begin
    current_state <= IDLE;
  end else begin
    current_state <= next_state;
  end
end

// FSM transition logic
always_comb begin
  case(current_state)
    IDLE: begin
      if(I_HEARTBEAT_START) begin
        next_state = WAIT;
      end else begin
        next_state = IDLE;
      end
    end

    WAIT: begin
      if(!I_HEARTBEAT_START) begin
        next_state = IDLE;
      end else begin
        if(!wait_end_signal) begin
          next_state = WAIT;
        end else begin
          if(!path_signal) begin    // path_signal이 0이면
            next_state = HEARTBEAT;
          end else begin            // path_signal이 1이면
            next_state = REVIVE;
          end
        end
      end
    end

    HEARTBEAT: begin
      if(!I_HEARTBEAT_START) begin
        next_state = IDLE;
      end else begin
        next_state = WAIT;
      end
    end

    REVIVE: begin
      if(!I_HEARTBEAT_START) begin
        next_state = IDLE;
      end else begin
        if(!timeout_wire) begin   // Revive duration이 끝나지 않았으면
          next_state = REVIVE;    // Revive state 유지
        end else begin            // Revive duration이 끝났으면
          next_state = WAIT;      // Wait state로 이동
        end
      end
    end
  endcase
end


// each state action
always_ff @(posedge clk or negedge rstn) begin
  if(!rstn) begin
    system_reset_reg      <= 0; // 시스템 리셋 신호(ariane을 reset하는 신호)
    path_selector_on      <= 0; // path_selector를 켜는 신호
    timer_on              <= 0; // timer를 켜는 신호
  end else begin
    case(current_state)
      IDLE: begin
        system_reset_reg    <= 0;
        path_selector_on    <= 0; 
        timer_on            <= 0; 
      end

      WAIT: begin
        system_reset_reg    <= 0;
        path_selector_on    <= 1;
        timer_on            <= 0;
      end

      HEARTBEAT : begin
        system_reset_reg    <= 0;
        path_selector_on    <= 0; 
        timer_on            <= 0;
      end

      REVIVE: begin
        system_reset_reg    <= 1;
        path_selector_on    <= 0;
        timer_on            <= 1;
      end
    endcase
  end
end

assign O_SYSTEM_RESET = system_reset_reg;

endmodule


module timer (
  input   logic clk,
  input   logic rstn,
  input   logic timer_on,
  output  logic timeout_wire
 );

localparam DUE_CNT = 10000;

logic [15:0]  count;
logic         timeout_reg;

always_ff@(posedge clk or negedge rstn) begin
  if(!rstn) begin
    count       <= 0;
    timeout_reg <= 0;
  end else begin
    if(timer_on) begin
      if(count < DUE_CNT - 1) begin // count 가 DUE_CNT - 1 보다 작으면
        count       <= count + 1;   // count 를 1 증가
        timeout_reg <= 0;
      end else begin                // count 가 DUE_CNT - 1 보다 크거나 같으면
        count       <= 0;                 // count 를 0 으로 초기화
        timeout_reg <= 1;           // timeout_reg 를 1 로 설정
      end 
    end else begin 
      count <= 0;
    end
  end
end

assign timeout_wire = timeout_reg;

endmodule

module path_selector(
  input   logic clk,
  input   logic rstn,
  input   logic start_signal,     // come from WAIT state
  input   logic reset_signal,     // come from Top module
  output  logic wait_end_signal,  
  output  logic path_signal
);

localparam TIMEOUT_SEC = 1;
localparam CLOCK_FREQ = 50_000_000;
localparam MAX_COUNT = CLOCK_FREQ * TIMEOUT_SEC;

logic [31:0]  count;
logic         timeout_reg;
logic         wait_end_reg;
always_ff @(posedge clk or negedge rstn) begin
  if(!rstn) begin
    count           <= 0;
    timeout_reg     <= 0;
    wait_end_reg    <= 0;
  end else begin
    if(start_signal && !reset_signal) begin
      if(count < MAX_COUNT - 1)begin    // wait_state를 유지할 조건
        count         <= count + 1;
        timeout_reg   <= 0;
        wait_end_reg  <= 0;
      end else begin                    // revive state로 이동할 조건
        count         <= 0;
        timeout_reg   <= 1;
        wait_end_reg  <= 1;
      end
    end else if(reset_signal) begin     // heartbeat state로 이동할 조건
      count         <= 0;
      timeout_reg   <= 0;
      wait_end_reg  <= 1;
    end else if(!start_signal) begin    // idle state로 이동할 조건
      count         <= 0;
      timeout_reg   <= 0;
      wait_end_reg  <= 1;
  end
end
end

assign path_signal      = timeout_reg;
assign wait_end_signal  = wait_end_reg;
endmodule

