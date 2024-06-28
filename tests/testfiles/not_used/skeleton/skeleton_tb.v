module skeleton_tb;
    reg clk = 0;
    reg a,b,c;
    wire d,e,f;
    
    skeleton dut(.clk(clk), .a(a), .b(b), .c(c), .d(d), .e(e),.f(f));

    always #1 clk = !clk;
    
    // initial begin
    //     #0
    //     a = 0;
    //     b = 0;
    //     c = 0;  
    //     #1
    //     a = 1;
    //     b = 0;
    //     c = 0;
    //     #2
    //     a = 1;
    //     b = 1;
    //     c = 0;
    //     #3
    //     a = 0;
    //     b = 1;
    //     c = 0;
    //     #4
    //     a = 0;
    //     b = 0;
    //     c = 0;    
    //     $finish;
    // end

      
    initial begin
        #0
        a = 1;
        b = 0;
        c = 0;  
        #2
        a = 0;
        b = 0;
        c = 1;
        #1
        a = 0;
        b = 1;
        c = 1;
        #2
        a = 0;
        b = 0;
        c = 1;
        #20
        $finish;
    end


    initial
        begin
            $dumpfile("../../examples/skeleton/skeleton.vcd");
            $dumpvars();		//writing the vcd file
        end
        
    //always@(a, b, c)
            //$monitor("At time %t, a = %b b = %b, ", $time, a, b);
endmodule
