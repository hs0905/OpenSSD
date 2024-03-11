module timer #( parameter DUE_CNT =100)
 (
  input   logic clk,
  input   logic rstn,
  input   logic cnt_start,
  output  logic timeout_wire
 );

logic [15:0]  count;
logic         timeout_reg;

always_ff@(posedge clk or negedge rstn) begin
  if(!rstn) begin
    count       <= 0;
    timeout_reg <= 0;
  end else begin
    if(cnt_start) begin
      if(count < DUE_CNT - 1) begin // count 가 DUE_CNT - 1 보다 작으면
        count <= count + 1;         // count 를 1 증가
      end else begin                // count 가 DUE_CNT - 1 보다 크거나 같으면
        count <= 0;                 // count 를 0 으로 초기화
        timeout_reg <= 1;           // timeout_reg 를 1 로 설정
      end 
    end else begin 
      count <= 0;
    end
  end
end

assign timeout_wire = timeout_reg;

endmodule