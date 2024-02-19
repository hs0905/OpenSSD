module auto_reset_timer(
  input   clk,
  input   watch_dog_signal,
  input   watch_dog_counter_start_signal,
  output  reset_signal 
  );

  localparam CLOCK_FREQ    = 100_000_000;  // 100 MHz
  localparam TIMEOUT_SEC   = 5;           
  localparam MAX_COUNT     = CLOCK_FREQ * TIMEOUT_SEC; 
  localparam RESET_DURATION = 5; // 리셋 신호 지속 클록 사이클 수

  logic [31:0] count                 = 0;
  logic reset_signal_reg             = 0;
  logic [2:0]  reset_signal_duration = 0; // 리셋 신호 지속 시간 카운터

  always_ff @(posedge clk) begin
    // 리셋 신호 지속 시간 처리
    if (reset_signal_duration > 0) begin
      reset_signal_duration <= reset_signal_duration - 1;
      reset_signal_reg <= 1; // 지속 시간 동안 리셋 신호 유지
    end 
    else begin
      reset_signal_reg <= 0; // 지속 시간이 끝나면 리셋 신호 해제
    end

    // 와치독 카운터 및 리셋 신호 로직
    if(watch_dog_counter_start_signal) begin
      if(!watch_dog_signal) begin
        if(count < MAX_COUNT - 1) begin
          count <= count + 1;
        end
        else begin
          count <= 0;
          reset_signal_duration <= RESET_DURATION; // 리셋 신호 지속 시간 설정
        end
      end
      else begin
        count <= 0;
      end 
    end
    else begin
      count <= 0;
    end
  end

  assign reset_signal = reset_signal_reg;
endmodule
