import subprocess
import sys

for i in range(int(sys.argv[1])):
    subprocess.run('./client &', shell=True)
