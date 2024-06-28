#!/usr/bin/python

import sys

print('Number of arguments:', len(sys.argv), 'arguments.')
print('Argument List:', str(sys.argv))

def usage():
  print("Purpose: Creating a scaled design.")
  print("Usage: " + sys.argv[0] + " <N> <M>")
  print("         N : number of module instantiations")
  print("         M : width of ports")

def RepresentsInt(s):
    try: 
        int(s)
        return True
    except ValueError:
        return False

if (len(sys.argv)<3 or sys.argv[0]=="-h" or not RepresentsInt(sys.argv[1]) or not RepresentsInt(sys.argv[2])):
  usage()
  exit(1)

numModules= int(sys.argv[1])
width= int(sys.argv[2])


def writeBasicModule(f,w):
  f.write("module Basic(a, c);\n")
  f.write(str("  input [")+str(w)+":0] a;\n")
  f.write(str("  output [")+str(w)+":0] c;\n")
  f.write("  assign c= ~a;\n")
  f.write("endmodule\n\n")

def writeTopModule(f,numModules, width):
  f.write("module tenNot(a,c);\n")
  f.write(str("  input [")+str(width)+":0] a;\n")
  f.write(str("  output [")+str(width)+":0] c;\n")
  for i in range(numModules+1):
    f.write(str("  wire [")+str(width)+":0] t"+str(i)+";\n")
  f.write("  assign t0= ;\n")
  for i in range(1,numModules+1):
    f.write(str("  Basic b")+str(i)+"(t"+ str(i) + ");\n")
  f.write("  assign c= t"+str(numModules)+";\n")
  f.write("endmodule\n")

def writeTestbench(f, width):
  f.write("module tenNot_tb;\n")
  f.write(str("  reg [")+str(width)+ ":0] a;\n")
  f.write("reg read_data[0:3];\n")
  f.write("integer i; \n")
  f.write(str("  output [")+str(width)+ ":0] c;\n")
  f.write("tenNot d1( a, c);")
  f.write("\n")
  f.write("  initial begin\n")
  f.write("    $readmemb(\"../../examples/tenGate.txt\", read_data); \n")
  f.write("       for(i = 0; i<=3; i = i + 1) begin \n")
  f.write("       {a} =  read_data[i];\n")
  f.write("       #2; \n")
  f.write("    end \n")
  f.write("  end \n")
  f.write("  initial \n")
  f.write("  begin \n")
  f.write("     $dumpfile(\"../../examples/tenNot.vcd \");\n")
  f.write("     $dumpvars(1,  a, c);	\n")
  f.write("  end \n")
  #f.write("  always@(a, c) \n")
  #f.write("	    $monitor(\"At time %d, value a = %b value c = %b \", $time, a, c);\n")
  f.write("	 endmodule \n")

f= open("tenNot.v", "w+")
writeBasicModule(f,width)
writeTopModule(f,numModules, width)

g= open("tenNot_tb.v", "w+")
writeTestbench(g,width)
