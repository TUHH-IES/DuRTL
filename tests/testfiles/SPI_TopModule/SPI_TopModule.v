module SPI_Slave
(
reset,
In_Data,
slaveCPOL, 
slaveCPHA,
slaveSCLK, 
slaveCS_, 
slaveMISO, 
slaveMOSI,
slave_start,
read_data
);                   //SCLK: SPI clock; //MOSI: Master out Slave in;  
                                //MISO: Master in Slave out; //ExData: External data to be communicated
input wire [7:0] In_Data;
input wire reset;
input wire slaveSCLK, slaveCPOL, slaveCPHA; 
input wire slaveCS_;
input wire slaveMOSI; 
input wire slave_start;

output reg slaveMISO;
output wire [7:0] read_data; // output_data, the data from Master

reg [7:0] memReg;//internal memory register


always@ (posedge reset)
begin
memReg = In_Data;
end

always@(negedge slaveCS_)
 slaveMISO = memReg[0]; //sending
    
always @ (negedge slaveSCLK) 
begin
if(slaveCS_==0)
begin
    if(slave_start==1)
    begin
     if ({slaveCPOL,slaveCPHA}==0 ||{slaveCPOL,slaveCPHA}==2 ) // send on falling 
     slaveMISO = memReg[0]; //sending
     
     if ({slaveCPOL,slaveCPHA}==1 ||{slaveCPOL,slaveCPHA}==3 ) // read on falling
     memReg ={slaveMOSI,memReg[7:1]};

    end
  
 end
end
always@(posedge slaveSCLK) // rising edge
begin
if(slaveCS_==0)
begin

 if(slave_start==1)
begin
    if ({slaveCPOL,slaveCPHA}==1 ||{slaveCPOL,slaveCPHA}==3 ) // send on rising
    slaveMISO = memReg[0];
    if ({slaveCPOL,slaveCPHA}==0 ||{slaveCPOL,slaveCPHA}==2 ) // read on rising
    memReg <={slaveMOSI,memReg[7:1]};
end
end
end
assign read_data = memReg;

endmodule 


module SPI_Master
                  (input clk ,input start,input load , input [7:0]data_to_write ,
                    input [1:0]slave_address , input cpol , input cpha , 
                    input MISO,

                    output reg SCLK , output reg CS1 , output reg CS2 , output reg CS3,
                    output reg MOSI , output reg [7:0] data_read ,output reg slave_start) ;


reg sampled = 1;
reg sclk_old;
reg communication_flag= 1;
reg CPOL ;
reg CPHA ;
reg [7:0]transmission_buffer ;

integer transmission_iterator =0 ;
integer count = 0;
integer countr = 0;

always @(posedge clk)
begin
if(load)
transmission_buffer = data_to_write;

if(count==17 && communication_flag==1)
begin
slave_start = 0;
count =0;
countr = 0;
end
if(transmission_iterator==8)
begin
sampled=0;
slave_start = 0;
end

if(start && communication_flag)
begin
communication_flag = 0;
countr = 0;
count = 0;
CPOL = cpol;
CPHA = cpha;
transmission_iterator = 0;
end
if(!communication_flag)
begin
    //1-} generating sclk from clk
    Generate_SCLK();

    read_write();
   
end
end
task Generate_SCLK();
begin
        if(count==0) // if it's the start of the communication
        begin
                SCLK=CPOL; // idel state
                count = count+1;
                sampled = 0;
{CS1,CS2,CS3} = 'b111 ;
    case(slave_address)
    0 : CS1 = 0 ;
    1 : CS2 = 0 ;
    2 : CS3 = 0 ;
    endcase
if(!CPHA)
begin
MOSI = transmission_buffer[0];
countr = countr+1;
end
        end
        else
        begin
                sclk_old=SCLK; 
                SCLK = ~SCLK;
                if(count==1)
                begin
                   if(CPOL==1)
                   sampled = 0;
                   else
                   begin
                   slave_start = 1;
                   sampled = 1;
                   end
                   count = count + 1;
                end
                else
                begin
                   if(sampled==0)
                   begin sampled = 1;
                   slave_start = 1;
                   end
                   else
                   count = count+1;
                end
        end
end
endtask

task read_write() ;
begin
	//CPHA == 1 , write on rising edge , read on falling 
if(sampled==1)
begin
	if(CPHA)
	begin

                if(countr ==2)
                begin
                transmission_iterator = transmission_iterator+1;
                countr = 0;
                end

                if(sclk_old == 0 && SCLK == 1 )
	        begin
                       MOSI = transmission_buffer[transmission_iterator] ;
                       countr = countr+1;
                end
	        if(sclk_old == 1 && SCLK == 0)
                begin
                       data_read[transmission_iterator] = MISO ;
                       transmission_buffer[transmission_iterator] = MISO;
                       countr = countr+1;
                end
        
	end

	//CPHA == 0 , read on rising , write on falling but the first bit will be read and sent at the start of the communication
	if(!CPHA)
	begin
                if(countr ==2)
                begin
                transmission_iterator = transmission_iterator+1;
                countr = 0;
                end

                if(sclk_old == 0 && SCLK == 1)
                begin
                     data_read[transmission_iterator] = MISO ;
                     transmission_buffer[transmission_iterator] = MISO;
                     countr = countr+1;
                end
                if (sclk_old == 1 && SCLK == 0 )
	        begin
                       MOSI = transmission_buffer[transmission_iterator] ;
                       countr = countr+1;
                end
               
	end

end
if(count==17)
begin
communication_flag<=1;
end
end
endtask 

endmodule 

module m41 ( input a, 
input b, 
input c,  
input s0, s1,
output out); 

 assign out = s1 ? (s0 ? 'bx : c) : (s0 ? b : a); 

endmodule

module m4x1 ( input[7:0] a, 
input [7:0] b, 
input [7:0] c,  
input  s0, s1,
output [7:0]out); 

 assign out = s1 ? (s0 ? 'bx : c) : (s0 ? b : a); 

endmodule

module SPI_Protocol(input start,input clk,input [7:0] data_in_master,input [7:0] data_in_slave,
output wire [7:0] data_out_master,input load,output wire [7:0] data_out_slave,input CPOL,input CPHA,
input [1:0] Address,input reset1,input reset2,input reset3);

wire SCLK;
wire MOSI;
wire MISO;
wire MISO1;
wire MISO2;
wire MISO3;
wire [7:0] data_out_slave1;
wire [7:0] data_out_slave2;
wire [7:0] data_out_slave3;
wire CS1,CS2,CS3;
wire slave_start;

m41 MUX (MISO1,MISO2,MISO3,Address[0],Address[1],MISO);
m4x1 MUX2(data_out_slave1,data_out_slave2,data_out_slave3,Address[0],Address[1],data_out_slave);

SPI_Master master (clk,start,load,data_in_master,Address,CPOL,CPHA,MISO,SCLK,CS1,CS2,CS3,MOSI,data_out_master,slave_start);

SPI_Slave slave_1 (reset1,data_in_slave,CPOL,CPHA,SCLK,CS1,MISO1,MOSI,slave_start,data_out_slave1);

SPI_Slave slave_2 (reset2,data_in_slave,CPOL,CPHA,SCLK,CS2,MISO2,MOSI,slave_start,data_out_slave2);

SPI_Slave slave_3 (reset3,data_in_slave,CPOL,CPHA,SCLK,CS3,MISO3,MOSI,slave_start,data_out_slave3);

endmodule


