
module FSM_Controller_tb;

    // Testbench signals
    reg [7:0] wind_speed;
    reg [7:0] vibration_level;
    reg clk;
    reg reset;
    wire [7:0] action;

    // Instantiate the FSM_Controller
    FSM_Controller uut (
        .wind_speed(wind_speed),
        .vibration_level(vibration_level),
        .action(action),
        .clk(clk),
        .reset(reset)
    );

    // Clock generation
    initial begin
        clk = 0;
        forever #5 clk = ~clk; // 10 ns clock period
    end

    // Stimulus
    initial begin
        // Initialize inputs
        wind_speed = 8'd0;
        vibration_level = 8'd0;
        reset = 1;
        #10;

        // Release reset
        reset = 0;

        // Test Case 1: Normal operation
        wind_speed = 8'd100;
        vibration_level = 8'd50;
        #20;
        $display("TC1 -> State: NORMAL_OPERATION, Action: %b", action);

        // Test Case 2: Wind speed > 130, transition to REDUCE_SPEED
        wind_speed = 8'd135;
        #20;
        $display("TC2 -> State: REDUCE_SPEED, Action: %b", action);

        // Test Case 3: Wind speed > 150, transition to SHUT_DOWN
        wind_speed = 8'd155;
        vibration_level = 8'd110;
        #20;
        $display("TC3 -> State: SHUT_DOWN, Action: %b", action);

        // Test Case 4: Recover to NORMAL_OPERATION
        wind_speed = 8'd100;
        vibration_level = 8'd50;
        #20;
        $display("TC4 -> State: NORMAL_OPERATION, Action: %b", action);

        // Test Case 5: High vibration causes SHUT_DOWN
        wind_speed = 8'd120;
        vibration_level = 8'd150;
        #20;
        $display("TC5 -> State: SHUT_DOWN, Action: %b", action);

        // Test Case 6: Reset FSM
        reset = 1;
        #10;
        reset = 0;
        wind_speed = 8'd100;
        vibration_level = 8'd50;
        #20;
        $display("TC6 -> State: NORMAL_OPERATION after reset, Action: %b", action);

        // End simulation
        $finish;
    end
    initial
	begin
		$dumpfile(`DUMP_FILE_NAME);
		$dumpvars();		//writing the vcd file
	end

endmodule
