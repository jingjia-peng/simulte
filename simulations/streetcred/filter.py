import sys

output_name = sys.argv[1] + ".log"

with open("simulation.log",'r') as input, open(output_name,'w') as output:
    for line in input:
        if line.startswith("[WARN]"):
            output.write(line)