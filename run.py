import subprocess
import sys
import matplotlib.pyplot as plt

command = "./client " + sys.argv[2] + " &"

x = list()
y = list()
p = list()

for i in range(int(sys.argv[1])):    
    p1 = subprocess.run(command, shell=True)
    #p.append(p1)


output = p[0].stdout
seek = output.find("The average time",0,-1) + 42
while output[seek].isdecimal():
    time = time + output[seek]
    seek+=1
print(int(time))
y.append(int(time))
x.append(i+1)

plt.plot(x,y)
plt.show()
