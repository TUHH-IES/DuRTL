`timescale 1ns / 1ps

module i2c_controller_tb;

	// Inputs
	reg clk;
	reg rst;
	reg [6:0] addr;
	reg [7:0] data_in;
	reg enable;
	reg rw;

	// Outputs
	wire [7:0] data_out;
	wire ready;

	// Bidirs
	wire i2c_sda;
	wire i2c_scl;

	// Instantiate the Unit Under Test (UUT)
	i2c_controller master (
		.clk(clk), 
		.rst(rst), 
		.addr(addr), 
		.data_in(data_in), 
		.enable(enable), 
		.rw(rw), 
		.data_out(data_out), 
		.ready(ready), 
		.i2c_sda(i2c_sda), 
		.i2c_scl(i2c_scl)
	);
	
		
	i2c_slave_controller slave (
    .sda(i2c_sda), 
    .scl(i2c_scl)
    );
	
	initial begin
		clk = 0;
		forever begin
			clk = #1 ~clk;
		end		
	end

	initial begin
		// Initialize Inputs
		clk = 0;
		rst = 1;

		// Wait 100 ns for global reset to finish
		#100;
        
		// Add stimulus here
		rst = 0;		
		addr = 7'b0101010;
		data_in = 8'b10101010;
		rw = 0;	
		enable = 1;
		#10;
		enable = 0;
				
		#500
		$finish;
		
	end      
endmodule

module i2c_slave_controller(
	inout sda,
	inout scl
	);

	localparam ADDRESS = 7'b0101010;

	localparam READ_ADDR = 0;
	localparam SEND_ACK = 1;
	localparam READ_DATA = 2;
	localparam WRITE_DATA = 3;
	localparam SEND_ACK2 = 4;

	reg [7:0] addr;
	reg [7:0] counter;
	reg [7:0] state = 0;
	reg [7:0] data_in = 0;
	reg [7:0] data_out = 8'b11001100;
	reg sda_out = 0;
	reg sda_in = 0;
	reg start = 0;
	reg write_enable = 0;

	assign sda = (write_enable == 1) ? sda_out : 'bz;

	always @(negedge sda) begin
		if ((start == 0) && (scl == 1)) begin
			start <= 1;
			counter <= 7;
		end
	end

	always @(posedge sda) begin
		if ((start == 1) && (scl == 1)) begin
			state <= READ_ADDR;
			start <= 0;
			write_enable <= 0;
		end
	end

	always @(posedge scl) begin
		if (start == 1) begin
			case(state)
				READ_ADDR: begin
					addr[counter] <= sda;
					if(counter == 0) state <= SEND_ACK;
					else counter <= counter - 1;
				end

				SEND_ACK: begin
					if(addr[7:1] == ADDRESS) begin
						counter <= 7;
						if(addr[0] == 0) begin
							state <= READ_DATA;
						end
						else state <= WRITE_DATA;
					end
				end

				READ_DATA: begin
					data_in[counter] <= sda;
					if(counter == 0) begin
						state <= SEND_ACK2;
					end else counter <= counter - 1;
				end

				SEND_ACK2: begin
					state <= READ_ADDR;
				end

				WRITE_DATA: begin
					if(counter == 0) state <= READ_ADDR;
					else counter <= counter - 1;
				end

			endcase
		end
	end

	always @(negedge scl) begin
		case(state)

			READ_ADDR: begin
				write_enable <= 0;
			end

			SEND_ACK: begin
				sda_out <= 0;
				write_enable <= 1;
			end

			READ_DATA: begin
				write_enable <= 0;
			end

			WRITE_DATA: begin
				sda_out <= data_out[counter];
				write_enable <= 1;
			end

			SEND_ACK2: begin
				sda_out <= 0;
				write_enable <= 1;
			end
		endcase
	end

	 initial
          begin
             $dumpfile(`DUMP_FILE_NAME);
            $dumpvars(1);		//writing the vcd file
          end

endmodule
