def mux2to1(VarInput, VarOutput, varWidth, f):
#VarInput = 6
#VarOutput = 6
#f = open(file, "a")
    f.write("module mux2to1(")
    strInput = "";
    BigVar = VarInput
    if(VarInput <= 2):
        BigVar = 3
    for x in range(BigVar):
        strInput += "in_" + str(x)
        if(x < BigVar - 1):
            strInput += ","
    f.write(strInput)
    f.write(",data_out);\n")
    strInput = ""
    for x in range(BigVar):
        strInput += "input [" + str(varWidth) + ":0] in_" + str(x)
        if(x < BigVar - 1):
            strInput += ";\n"
    f.write(strInput)
    f.write(";\noutput [" + str(varWidth) + ":0] data_out;\n")
    f.write("reg [" + str(varWidth) + ":0] data_out;\n")

    #     //Always block - the statements inside this block are executed when the given sensitivity list 
    #     //is satidfied. for example in this case the block is executed when any changes occur in the three signals 
    #     //named 'Data_in_0','Data_in_1' or 'sel'.
    f.write("always @(")
    for x in range (BigVar):
        f.write("in_" + str(x))
        if(x < BigVar - 1):
            f.write(",")
    f.write(")\n")

    f.write("\tbegin\n")
    f.write("\t\tif(")
    f.write("in_" + str(BigVar -1) + "== 0)\n")
    strWrite = "\t\t\tdata_out ="
    for x in range(BigVar ):
        strWrite += "in_" + str(x)
        if(x < BigVar -1):
            strWrite += "+"
    f.write(strWrite)
    f.write(";\n\t\telse\n")
    if(BigVar >= 2):
        f.write("\t\t\tdata_out = in_" + str(BigVar - 2) + " & in_" + str(BigVar - 1) + ";\n")
    else:
        f.write("\t\t\tdata_out = in_0" + " & in_" + str(BigVar) + ";\n")
    f.write("\n\tend");
    f.write("\nendmodule")