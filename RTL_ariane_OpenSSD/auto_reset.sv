module auto_reset_timer(
  input   clk,
  input   rstn,
  input   Inner_counter_reset,
  input   Inner_counter_start,
  output  System_reset
  );

  localparam CLOCK_FREQ    = 100_000_000;  // 100 MHz
  localparam TIMEOUT_SEC   = 5;           
  localparam MAX_COUNT     = CLOCK_FREQ * TIMEOUT_SEC; 
  localparam RESET_DURATION = 5; // 리셋 신호 지속 클록 사이클 수

  logic [31:0]  count                 = 0;
  logic         system_reset_reg      = 0;
  logic [2:0]   reset_signal_duration = 0; // 리셋 신호 지속 시간 카운터

  always_ff @(posedge clk) begin
    if (reset_signal_duration > 0) begin
      reset_signal_duration <= reset_signal_duration - 1;
      system_reset_reg      <= 1; 
    end else begin
      system_reset_reg <= 0; // 지속 시간이 끝나면 리셋 신호 해제
    end
    if(Inner_counter_start) begin
      if(!Inner_counter_reset) begin
        if(count < MAX_COUNT - 1) begin
          count <= count + 1;
        end else begin
          count <= 0;
          reset_signal_duration <= RESET_DURATION; // 리셋 신호 지속 시간 설정
        end
      end else begin
        count <= 0;
      end 
    end
    else begin
      count <= 0;
    end
  end

  assign System_reset = system_reset_reg;
endmodule
