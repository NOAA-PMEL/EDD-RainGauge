import csv
import math
import numpy as np
#from scipy.optimize import curve_fit

# Change filename to your CSV file
# CSV File should be in format:
# Volume, Counts

# Generates output file with format
# Volume, Counts, Midstep Calc, Converted

# Appends coefficients

filename = "test"
extension = ".csv"

reader = csv.reader(open(filename + extension,'rb'),delimiter=',')
w = open(filename + '_clean' + extension,'wb')
writer = csv.writer(w,delimiter=',')
volume = []
count = []

for row in reader:
    print(row)
    volume.append(float(row[0]))
    count.append(float(row[1]))

midstep = []
for i in range(0,len(volume)):
    temp = count[i]*60
    temp = math.pow(temp,2)
    temp = 1/temp
    midstep.append(temp)

y = np.array(volume)
x = np.array(midstep)
z = np.polyfit(x,y,1)



A0 = "{0:.5}".format(*z)
A1 = "{1:.5}".format(*z)
print(A0 + "x + " + A1)



converted = []
for i in range(0,len(volume)):
    dm = count[i] * 60
    dm = dm/1
    dm *= dm
    
    vol = float(A0)/(dm)
    vol += float(A1)
    converted.append(vol)

writer.writerow(['Volume','Counts','MidStep Calc','Converted'])

for i in range(0,len(volume)):
    writer.writerow([volume[i],count[i],"{0:.2f}".format(midstep[i]),"{0:.2f}".format(converted[i])])




writer.writerow("")
writer.writerow("")
writer.writerow(["A0",A0,"A1",A1])

w.close()