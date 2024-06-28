module mux_tb;
	
	reg a, b, s;
	wire out;

	reg [31:0] a_t;
	reg [31:0] b_t;
	reg [31:0] s_t;
	wire [31:0] out_t;	
	
	reg[95:0] read_data [0:7];
	integer i;
	// instance of the counter module
	m_0 a1 (.a(a),.a_t(a_t), .b(b), .b_t(b_t), .s(s), .s_t(s_t), .out(out), .out_t(out_t));


	initial begin
		$readmemb(`INPUT_FILE_NAME, read_data);
		for(i = 0; i<=7; i+=1) begin
		{ a_t,b_t,s_t} = read_data[i];
		a = 1'bx;
		b = 0;
		s = 0;
		#2
		a = 1'bx;
		b = 0;
		s = 1;
		#2
		a = 1'bx;
		b = 0;
		s = 0;
		#2
		a = 1'bx;
		b = 1;
		s = 0;
		#2
		a = 1'bx;
		b = 0;
		s = 0;
		end
	end
	
	initial
		begin
			$dumpfile(`DUMP_FILE_NAME);  
			$dumpvars(1);		//writing the vcd file
		end
	
	//initial
		//$monitor("At time %t, value = %h (%0d)", $time, out, out);
endmodule // test
