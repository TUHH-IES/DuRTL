DESIGN="${1}"
echo "Design: $DESIGN"
JSON=`basename ${DESIGN} .v`.json
echo "Json:   $JSON"

#echo "
  #read_verilog -define DEF_A_SIGNED=\"\" -define DEF_A_HI=5 -define DEF_A_LO=3 -define DEF_B_SIGNED=\"signed\" -define DEF_B_HI=5 -define DEF_B_LO=3 -define DEF_OUT_SIGNED=\"\" -define DEF_OUT_HI=5 -define DEF_OUT_LO=3 -sv ${DESIGN} #read modules from Verilog file
  #hierarchy -auto-top #check, expand and clean up design hierarchy along with automatically determing the top module
  #proc #translate processes to netlists
  #memory_collect #translate memories to basic cells
  #opt #perform simple optimizations
  #write_json ${JSON} #write design to a JSON file" \
  #| yosys


#yosys -p "read \
    #-define DEF_A_SIGNED=\"\" -define DEF_A_HI=5 -define DEF_A_LO=3 \
    #-define DEF_B_SIGNED=\"signed\" -define DEF_B_HI=5 -define DEF_B_LO=3 \
    #-define DEF_OUT_SIGNED=\"\" -define DEF_OUT_HI=5 -define DEF_OUT_LO=3 \
    #-sv ${DESIGN}; hierarchy -auto-top; proc; memory_collect; write_json ${JSON}"

yosys -p "read \
    -sv ${DESIGN}; hierarchy -auto-top; proc; memory_collect; write_json ${JSON}"
