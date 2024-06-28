`timescale 1 ns/ 10 ps 	

module SPI_Slave_tb();

localparam PERIOD = 4;
reg [7:0] In_Data;
reg reset;
reg slaveCPOL,slaveCPHA;
reg slaveSCLK;
reg slaveCS_;
reg slave_start;
reg slaveMOSI;
wire slaveMISO;
wire [7:0] read_data;
reg [7:0] Master_Memory; //write memory
integer i;
reg [7:0] MOSI_Arr;

SPI_Slave Slave
(
 .reset(reset),
 .slaveSCLK(slaveSCLK),
 .slaveCPOL(slaveCPOL),
 .slaveCPHA(slaveCPHA),
 .slaveMISO(slaveMISO),
 .slaveMOSI(slaveMOSI),
 .slaveCS_(slaveCS_),
 .In_Data(In_Data),
 .read_data(read_data),
 .slave_start(slave_start)
);

always 
   #(PERIOD/2) slaveSCLK = ~slaveSCLK;


initial
begin
$monitor ("%g\t %b\t %b\t %b\t %b\t %b\t %b\t          %b\t    %b\t    %b\t  %b\t",
           $time, reset,slaveCPOL,slaveCPHA,slaveSCLK,slaveCS_,slave_start,read_data,slaveMOSI,slaveMISO,Master_Memory);


//mode 0
{slaveCPOL,slaveCPHA}=0; slaveSCLK = slaveCPOL;  In_Data = 8'b00001111;  reset = 1'b1; MOSI_Arr = 8'b10101010; slaveCS_ = 1'b1;
$display("#################################################_______MODE_0___####################################################################################################");
$display ("time\t reset\t slaveCPOL\t slaveCPHA\t slaveSCLK\t slaveCS_\tSlave_Start\t   read_data\t slaveMOSI\t slaveMISO\t Write_Memory");


#(PERIOD)  slaveCS_ = 1'b0;  reset = 1'b0;
for(i=0;i<8;i=i+1)
begin
  #(PERIOD/2) // +ve sclk, reading from slaveMOSI and writing the first bit on slaveMISO
  slaveMOSI = MOSI_Arr[i]; slave_start = 1;
  #(PERIOD/2) // -ve sclk , writing bits starting from 2nd bit on slaveMISO
  Master_Memory[i]=slaveMISO;
end

#(PERIOD/2)

  if(Master_Memory==In_Data && read_data==MOSI_Arr)
    $display("Data was transmitted successfully");
    else
begin
     for(i=0;i<8;i=i+1)
     begin
     if(Master_Memory[i]!=In_Data[i])
     $display("Error in write memory in index %d, expected %b, found %b",i,In_Data[i],Master_Memory[i]);
     end
     for(i=0;i<8;i=i+1)
     begin
     if(read_data[i]!=MOSI_Arr[i])
     $display("Error in slave memory in index %d, expected %b, found %b",i,In_Data[i],Master_Memory[i]);
     end
end


  //Mode 1
{slaveCPOL,slaveCPHA} = 1; slaveSCLK = slaveCPOL;  In_Data = 8'b11011000;  reset = 1'b1;
 MOSI_Arr = 8'b01110110; slaveCS_ = 1'b1; Master_Memory = 8'bx; slave_start=0;

$display("#################################################_______MODE_1______###########################################################################");
$display ("time\t reset\t slaveCPOL\t slaveCPHA\t slaveSCLK\t slaveCS_\tSlave_Start\t   read_data\t slaveMOSI\t slaveMISO\t Write_Memory");

for(i=0;i<8;i=i+1)
begin
  #(PERIOD/2)
slave_start = 1;
  slaveCS_ = 1'b0;  reset = 1'b0; 
 //+ve sclk
  #(PERIOD/2) // -ve sclk

  slaveMOSI = MOSI_Arr[i];
   Master_Memory[i] = slaveMISO;
 
 end
#(PERIOD)
  if(Master_Memory==In_Data && read_data==MOSI_Arr)
    $display("Data was transmitted successfully");
    else
      begin
     for(i=0;i<8;i=i+1)
     begin
     if(Master_Memory[i]!=In_Data[i])
     $display("Error in write memory in index %d, expected %b, found %b",i,In_Data[i],Master_Memory[i]);
     end
     for(i=0;i<8;i=i+1)
     begin
     if(read_data[i]!=MOSI_Arr[i])
     $display("Error in slave memory in index %d, expected %b, found %b",i,In_Data[i],Master_Memory[i]);
     end
end


//Mode2
{slaveCPOL,slaveCPHA} = 2; slaveSCLK = slaveCPOL;  In_Data = 8'b01011001;  reset = 1'b1; 
 MOSI_Arr = 8'b00000000; slaveCS_ = 1'b1; Master_Memory = 8'bx; slave_start=0;
$display("#################################################_______MODE_2______############################################################################");
$display ("time\t reset\t slaveCPOL\t slaveCPHA\t slaveSCLK\t slaveCS_\tSlave_Start\t   read_data\t slaveMOSI\t slaveMISO\t Write_Memory");


for(i=0;i<8;i=i+1)
begin
  #(PERIOD/2)
  slaveCS_ = 1'b0; reset = 1'b0;
   slaveMOSI = MOSI_Arr[i]; slave_start = 1;
  #(PERIOD/2)
  Master_Memory[i] = slaveMISO;
end
#(PERIOD)

  if(Master_Memory==In_Data && read_data==MOSI_Arr)
    $display("Data was transmitted successfully");
    else
     begin
     for(i=0;i<8;i=i+1)
     begin
     if(Master_Memory[i]!=In_Data[i])
     $display("Error in write memory in index %d, expected %b, found %b",i,In_Data[i],Master_Memory[i]);
     end
     for(i=0;i<8;i=i+1)
     begin
     if(read_data[i]!=MOSI_Arr[i])
     $display("Error in slave memory in index %d, expected %b, found %b",i,In_Data[i],Master_Memory[i]);
     end
     end

//Mode 3
{slaveCPOL,slaveCPHA} = 3; slaveSCLK = slaveCPOL;  In_Data = 8'b00011011;  reset = 1'b1;
 MOSI_Arr = 8'b11111111; slaveCS_ = 1'b1; Master_Memory = 8'bx; slave_start=0;
$display("#################################################_______MODE_3_______############################################################################");
$display ("time\t reset\t slaveCPOL\t slaveCPHA\t slaveSCLK\t slaveCS_\tSlave_Start\t   read_data\t slaveMOSI\t slaveMISO\t Write_Memory");


for(i=0;i<8;i=i+1)
begin
  #(PERIOD/2)
  slaveCS_ = 1'b0; reset = 1'b0; 
  slave_start = 1;
  #(PERIOD/2)
  slaveMOSI = MOSI_Arr[i];
  Master_Memory[i] = slaveMISO;
end
  #(PERIOD)
  if(Master_Memory==In_Data && read_data==MOSI_Arr)
    $display("Data was transmitted successfully");
    else
     begin
     for(i=0;i<8;i=i+1)
     begin
     if(Master_Memory[i]!=In_Data[i])
     $display("Error in write memory in index %d, expected %b, found %b",i,In_Data[i],Master_Memory[i]);
     end
     for(i=0;i<8;i=i+1)
     begin
     if(read_data[i]!=MOSI_Arr[i])
     $display("Error in slave memory in index %d, expected %b, found %b",i,In_Data[i],Master_Memory[i]);
     end
end



$finish;
end

initial
	begin
		$dumpfile(`DUMP_FILE_NAME);
        //$dumpfile("SPI_Master.vcd");
		$dumpvars();		//writing the vcd file
	end

endmodule

