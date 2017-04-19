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
	x = (x-75)/180*math.pi
	y = (y-70)/180*math.pi
	ommYM = numpy.matrix([[math.cos(x),0,math.sin(x)],[0,1,0],[-math.sin(x),0,math.cos(x)]])
	ommPM = numpy.matrix([[1,0,0],[0,math.cos(y),-math.sin(y)],[0,math.sin(y),math.cos(y)]])
	forwardVect = numpy.array([[0],[0],[1]])
	vect2 = ommYM*ommPM*forwardVect
	#return vect2
	vect2 = vect2 + camTrans
	vect2 = camYM*camPM*camRM*vect2
	if (vect2[2] > 0.01):
		vect2 = vect2*camScaling/vect2[2]
	else: 
		return numpy.array([[100000],[100000],[1]])
	# normalise
	# now translate x-y into pixels to account for distortion
	r_c = math.sqrt((vect2[0])**2+(vect2[1])**2)
	k_1 = -0.61233
	k_2 = 0.92386
	k_3 = 0
	vect2[0] = vect2[0]*(1+k_1*r_c**2+k_2*r_c**4+k_3*r_c**6)
	vect2[1] = vect2[1]*(1+k_1*r_c**2+k_2*r_c**4+k_3*r_c**6)
	#vect2[0] = (vect2[0]+1.0)*(576.0/2.0)
	#vect2[1] = (vect2[1]+1.0)*(480.0/2.0)
	# return 
	# camera matrix:
	f_x = 574.40666#*2.0
	f_y = 571.55377#*2.0
	s = 0
	c_x = 315.79322
	c_y = 193.62054#*2.0
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

	while currX>-20:
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

	while currX>-20:
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


#plt.plot(xPix,yPix, 'r.')
#plt.show()

print min(xPix)
print min(yPix)
print max(xPix)
print max(yPix)

# 

f = open('gigerdatacam_ardrone.h', 'w')

f.write("#ifndef GIGERDATA_H\n#define GIGERDATA_H\n\nfloat gdata[][4] = {")

orderedCoords = sorted(zip(xPoints,yPoints,xPix,yPix))
count = 0
mooX = []
mooX2 = []
mooY = []
mooY2 = []
mooXN2 = []
mooZ= []
mooelem2= []
mooelem3= []
for elem in orderedCoords:
	#if elem[1]>=0 and elem[1]<=140 and elem[0]>=0 and elem[0]<=140:
	# convert angles
	v = radialDistortion(elem[0],elem[1])
	#f.write("{"+str(v[0])+","+str(v[1])+","+str(elem[2])+","+str(elem[3])+"}, \ \n")
	if (v[0].min() > -0.0 and v[0].min() < 720.0 and v[1].min() > 0.0 and v[1].min() < 360.0):
		mooX.append(v[0].min())
		mooX2.append(elem[0])
		mooY.append(v[1].min())
		mooY2.append(elem[1])
		mooXN2.append(-elem[0])
		mooZ.append(v[2].min())
		mooelem2.append(elem[2])
		mooelem3.append(elem[3])
		f.write("{"+str(719-round(v[0].min()))+","+str(359-round(v[1].min()))+","+str(elem[2])+","+str(elem[3])+"},")
		count += 1
		
print "Yay:::"
print min(mooelem2)
print max(mooelem2)
print min(mooelem3)
print max(mooelem3)

#fig = plt.figure()
#ax = fig.add_subplot(111, projection='3d')
#ax.scatter(mooX,mooY,mooZ)# bx
fig1 = plt.figure(figsize=(8, 6))	
plt.plot(mooX,mooY,'r.')
plt.axis([0,720,0,360])
plt.xlabel("X pixel location",fontsize="20");
plt.ylabel("Y pixel location",fontsize="20");
plt.show()
fig1.savefig("ommmodelpixfig.pdf",format='pdf')

fig2 = plt.figure(figsize=(12, 6))	
plt.plot(xPoints,yPoints, 'g.')
xPoints2 = [ -x for x in xPoints]
plt.plot(xPoints2,yPoints, 'g.')
plt.hold
plt.plot(mooX2,mooY2, 'r.')
plt.plot(mooXN2,mooY2, 'b.')
plt.xlabel("Azimuth (degrees)",fontsize="20");
plt.ylabel("Elevation (degrees)",fontsize="20");
plt.show()
fig2.savefig("ommmodelfig.pdf",format='pdf')


f.write("{0};int gdataLength = {1};\n#endif\n".format("}",count))
f.close();