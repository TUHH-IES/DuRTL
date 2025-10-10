import csv
import subprocess

# Input CSV filename
INPUT_FILE = "modules.csv"

# Read the CSV file and generate Verilog files
if (True):
  with open(INPUT_FILE, newline="") as csvfile:
    reader = csv.reader(csvfile, delimiter=';')
    for row in reader:
        if len(row) != 3:
            continue  # Skip malformed lines

        module_name, logic_expr = row[0].strip(), row[1].strip()
        
        verilog_code = f"""// Auto-generated Verilog file for module {module_name}
module top (
    input [2:0] a,
    input [2:0] b,
    output [2:0] out
);

    assign {logic_expr};

endmodule
"""
        vlFile= f"{module_name}.v"
        # Write to MODULENAME.v
        with open(vlFile, "w") as vfile:
            vfile.write(verilog_code)
        # Create JSON from yosys
        subprocess.run(["bash", "../../../../scripts/synthYosys.sh", vlFile], check=True)
        
        verilog_code = f"""// Auto-generated Verilog file for module {module_name}
module top (
    input `DEF_A_SIGNED [`DEF_A_HI:`DEF_A_LO] a,
    input `DEF_B_SIGNED [`DEF_B_HI:`DEF_B_LO] b,
    output `DEF_OUT_SIGNED [`DEF_OUT_HI:`DEF_OUT_LO] out
);

    assign {logic_expr};

endmodule
"""
        vlFile= f"{module_name}_params.v"
        # Write to MODULENAME.v
        with open(vlFile, "w") as vfile:
            vfile.write(verilog_code)


print("Verilog files generated successfully.")
print("The .._params.v module scan be parameterized by preprocessor definitions for width and signedness of wires. Here is an example\n\
iverilog -DDEF_A_SIGNED=\"\" -DDEF_A_HI=5 -DDEF_A_LO=3 \\ \n \
         -DDEF_B_SIGNED=\"signed\" -DDEF_B_HI=5 -DDEF_B_LO=3  \\ \n \
         -DDEF_OUT_SIGNED=\"\" -DDEF_OUT_HI=5 -DDEF_OUT_LO=3  \\ \n \
      mul_params.v")
