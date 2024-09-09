import sys

output_name = sys.argv[1] + ".log"

with open("simulation.log",'r') as input, open(output_name,'w') as output:
    infos = ["[RSU]","[Vehicle 1027]"]
    details = ["Intersection.rsu[0]","Intersection.node[0]"]
    writedetails = False
    for line in input:
        if any(s in line for s in infos):
            output.write(line)
        elif line.startswith("** Event #") and any(s in line for s in details):
        	writedetails = True
        	output.write(line)
        elif line.startswith("[") and writedetails:
        	output.write(line)
        else:
        	writedetails = False
        	
