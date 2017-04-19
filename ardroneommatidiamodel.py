import matplotlib.pyplot as plt
import math
import numpy

v_shift = 40

# side patches
num_omm_horiz = 24.0
num_omm_vert = 32.0
start_h_pix=100.0
end_h_pix=230.0
start_v_pix=70.0+40.0
end_v_pix=210.0+40.0
centre_v = 150+40.0
adj_rhs = 0

horiz = []
# create horiz
for x in xrange(0,int(num_omm_horiz)):
	h_r = end_h_pix-start_h_pix
	h = start_h_pix+math.sin((x/num_omm_horiz)*math.pi/2.0)*h_r
	horiz.append(round(h))
print horiz

vert = []
# create vert
for x in xrange(0,int(num_omm_vert)):
	v_r = end_v_pix-start_v_pix
	v = start_v_pix+x*v_r/num_omm_vert
	vert.append(round(v))
print vert

xSrc = []
ySrc = []
xDst = []
yDst = []

# create lookups
#left
d_i = 0
d_j = 0
for h in horiz:
	for v in vert:
		v_fin = ((v>centre_v)*0.5+0.8)*(v-centre_v)*(math.fabs(math.log(h)-math.log(100))/math.log(2))+centre_v+(v>centre_v)*80-0.1*h
		if math.sqrt((v_fin-(centre_v+0))**2 + 4.0*(h-250)**2)> 70:
			xSrc.append(h)
			ySrc.append(v_fin)
			yDst.append(d_j)
			xDst.append(d_i)
		d_j += 1
	d_i += 1
	d_j = 0

# middle
d_i = len(horiz)
d_j = 0
for h in xrange(280,370,16):
	for v in xrange(0,350,11):
		xSrc.append(h)
		ySrc.append(v)
		yDst.append(d_j)
		xDst.append(d_i)
		d_j += 1
	d_i += 1
	d_j = 0

#centre_v = centre_v - 10
#start_v_pix = start_v_pix -10
#end_v_pix = end_v_pix -10

# right
d_i = len(horiz)*2+5
d_j = 0
for h in horiz:
	for v in vert:
		v_fin = ((v>centre_v)*0.5+0.8)*(v-centre_v)*(math.fabs(math.log(h)-math.log(100))/math.log(2))+centre_v+(v>centre_v)*80-0.1*h
		if math.sqrt((v_fin-(centre_v+0))**2 + 4.0*(h-250)**2)> 70:
			xSrc.append(640-adj_rhs-h)
			ySrc.append(v_fin)
			yDst.append(d_j)
			xDst.append(d_i)
		d_j += 1
	d_i -= 1
	d_j = 0
	

plt.plot(xSrc,ySrc, 'bx')
plt.show()

#plt.plot(xDst,yDst, 'rx')
#plt.show()


f = open('ardroneommdata.h', 'w')

f.write("#ifndef ARDRONEOMMDATA_H\n#define ARDRONEOMMDATA_H\n\nfloat ommdata[][4] = {")

orderedCoords = zip(xSrc,ySrc,xDst,yDst)
count = 0
for elem in orderedCoords:
	f.write("{"+str(elem[0])+","+str(elem[1])+","+str(elem[2])+","+str(elem[3])+"},")
	count += 1
f.write("{0};\r\nint ommdataLength = {1};\r\n\r\n#endif\n".format("}",count))
f.close();