import matplotlib.pyplot as plt
import math
import numpy
from mpl_toolkits.mplot3d import Axes3D

def vert(x):
	return (0.000734*(x**2))-(0.1042253*x)+4.9

def horr(x):
	if x>60:
		return (0.00037*(x**2))-(0.04462*x)+3.438
	else:
		return (0.00069*(x**2))-(0.08333*x)+4.6
		
def radialDistortion(x,y):
	camYaw=0.0/180.0*math.pi
	camPitch=0.0/180.0*math.pi
	camRoll=0.0/180.0*math.pi
	camTrans=numpy.array([[0],[0],[0]])
	camScaling = 1
	camYM = numpy.matrix([[math.cos(camYaw),0,math.sin(camYaw)],[0,1,0],[-math.sin(camYaw),0,math.cos(camYaw)]])
	camPM = numpy.matrix([[1,0,0],[0,math.cos(camPitch),-math.sin(camPitch)],[0,math.sin(camPitch),math.cos(camPitch)]])
	camRM = numpy.matrix([[math.cos(camRoll),-math.sin(camRoll),0],[math.sin(camRoll),math.cos(camRoll),0],[0,0,1]])
	# undo the camera rotation
	# convert x,y into rotations
	x = (x- 0)/180*math.pi
	y = (y-70)/180*math.pi
	ommYM = numpy.matrix([[math.cos(x),0,math.sin(x)],[0,1,0],[-math.sin(x),0,math.cos(x)]])
	ommPM = numpy.matrix([[1,0,0],[0,math.cos(y),-math.sin(y)],[0,math.sin(y),math.cos(y)]])
	forwardVect = numpy.array([[0],[0],[1]])
	vect2 = ommYM*ommPM*forwardVect
	#return vect2
	vect2 = vect2 + camTrans
	vect2 = camYM*camPM*camRM*vect2
	if (vect2[2] > 0.05):
		vect2 = vect2*camScaling/vect2[2]
	else: 
		return numpy.array([[100000],[100000],[1]])
	# normalise
	# now translate x-y into pixels to account for distortion
	r_c = math.sqrt((vect2[0])**2+(vect2[1])**2)
	k_1 = -0.3112
	k_2 = 0.10422
	k_3 = 0
	vect2[0] = vect2[0]*(1+k_1*r_c**2+k_2*r_c**4+k_3*r_c**6)
	vect2[1] = vect2[1]*(1+k_1*r_c**2+k_2*r_c**4+k_3*r_c**6)
	#vect2[0] = (vect2[0]+1.0)*(576.0/2.0)
	#vect2[1] = (vect2[1]+1.0)*(480.0/2.0)
	# return 
	# camera matrix:
	f_x = 187.93014
	f_y = 170.79432
	s = 0
	c_x = 185.139
	c_y = 111.64043
	camMat = numpy.matrix([[f_x,s,c_x],[0,f_y,c_y],[0,0,1]])
	# apply
	vect2 = camMat*vect2
	#vect2[0] += c_x
	#vect2[1] += c_y
	return vect2
	
startX=60
startY=70

startPixX=30
startPixY=54

currX=startX
currY=startY

currPixX = startPixX
currPixY = startPixY

itr = 0

xPoints = []
yPoints = []
xPix = []
yPix = []

scale = 1.0
scaleV = 1.0

while currY<140:

	if (itr%2)==0:
		currX+=(0.5*horr(currY)*scale)

	while currX<140:
		xPoints.append(currX)
		yPoints.append(currY)
		xPix.append(currPixX)
		yPix.append(currPixY)
		currX+=horr(currY)*scale
		currPixX+=1
	currX=startX
	currPixX=startPixX

	if (itr%2)==0:
		currX+=(0.5*horr(currY)*scale)

	while currX>0:
		currX-=horr(currY)*scale
		currPixX-=1
		xPoints.append(currX)
		yPoints.append(currY)
		xPix.append(currPixX)
		yPix.append(currPixY)
	currX=startX
	currPixX=startPixX
	currY+=vert(currX)*scale*scaleV
	currPixY+=1
	itr+=1
currY = startY
currPixY=startPixY

itr = 0

while currY>0:

	if (itr%2)==0:
		currX+=(0.5*horr(currY)*scale)

	while currX<140:
		xPoints.append(currX)
		yPoints.append(currY)
		xPix.append(currPixX)
		yPix.append(currPixY)
		currX+=horr(currY)*scale
		currPixX+=1
	currX=startX
	currPixX=startPixX

	if (itr%2)==0:
		currX+=(0.5*horr(currY)*scale)

	while currX>0:
		currX-=horr(currY)*scale
		currPixX-=1
		xPoints.append(currX)
		yPoints.append(currY)
		xPix.append(currPixX)
		yPix.append(currPixY)
	currX=startX
	currPixX=startPixX
	currY-=vert(currX)*scale*scaleV
	currPixY-=1
	itr+=1

#plt.plot(xPoints,yPoints, 'bx')
#plt.show()
#plt.plot(xPix,yPix, 'bx')
#plt.show()

print min(xPix)
print min(yPix)
print max(xPix)
print max(yPix)

# 

f = open('gigerdata.h', 'w')

f.write("#ifndef GIGERDATA_H\n#define GIGERDATA_H\n\nfloat gdata[][4] = { \ \n")

orderedCoords = sorted(zip(xPoints,yPoints,xPix,yPix))
count = 0
mooX = []
mooY = []
mooZ= []
for elem in orderedCoords:
	if elem[1]>=0 and elem[1]<=140 and elem[0]>=0 and elem[0]<=140:
		if elem[2]>=0 and elem[2]<70 and elem[3]>=0 and elem[3]<100:
			f.write("{"+str(elem[0])+","+str(elem[1])+","+str(elem[2])+","+str(elem[3])+"}, \ \n")
			count += 1
#fig = plt.figure()
#ax = fig.add_subplot(111, projection='3d')
#ax.scatter(mooX,mooY,mooZ)# bx
#plt.plot(mooX,mooY,'bx')
#plt.axis([0,576,0,240])
#plt.show()
f.write("{0};\nint gdataLength = {1};\n\n#endif\n".format("}",count))
f.close();