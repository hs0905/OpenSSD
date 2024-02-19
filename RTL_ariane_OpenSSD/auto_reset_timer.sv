module auto_reset_timer(
  input   clk,
  input   watch_dog_signal,
  input   watch_dog_counter_start_signal,
  output  reset_signal 
  );

  localparam CLOCK_FREQ = 100_000_000;  // 100 MHz
  localparam TIMEOUT_SEC = 5;           // 5 seconds
  localparam MAX_COUNT = CLOCK_FREQ * TIMEOUT_SEC; // number of clock cycles for 5 seconds

  logic [31:0] count      = 0;
  logic reset_signal_reg  = 0;

  always_ff @(posedge clk) begin
    if(watch_dog_counter_start_signal) begin
      if(!watch_dog_signal) begin
        if(count < MAX_COUNT - 1) begin
          count             <= count + 1;
          reset_signal_reg  <= 0;
        end else begin
          count             <= 0;
          reset_signal_reg  <= 1;            // system reset signal
        end
      end else if(watch_dog_signal) begin
        count <= 0;
        reset_signal_reg <= 0;
      end 
    end else if(!watch_dog_counter_start_signal) begin
      count            <= 0;
      reset_signal_reg <= 0;
    end
  end

  assign reset_signal = reset_signal_reg;

endmodule