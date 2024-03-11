module path_selector(
  input   logic clk,
  input   logic rstn,
  input   logic start_signal,
  input   logic reset_signal,
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