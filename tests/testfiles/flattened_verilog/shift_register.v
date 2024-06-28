module shift_register_flattened(
  input clock,
  input shift_in,
  output shift_out
);

reg bit0;
reg bit1;
reg bit2;
reg bit3;

assign shift_out = bit3;

always @(posedge clock) begin
  bit3 <= bit2;
  bit2 <= bit1;
  bit1 <= bit0;
  bit0 <= shift_in;
end

endmodule