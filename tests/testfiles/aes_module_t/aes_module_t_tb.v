module aes_module_t_tb;

// Inputs

    reg [7:0] in;
    reg [7:0] in2;

// Outputs

    wire [31:0] out;

    aes_module_t uut (
        .in(in),
        .out(out)
    );

    initial begin
        in = 0;
        #1
        in = 1;
        #1
        $finish;
    end


    initial begin
        	$dumpfile(`DUMP_FILE_NAME);
    		$dumpvars(0);
    	end
    endmodule