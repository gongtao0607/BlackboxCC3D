#!/usr/bin/python3
import sys
import os
import struct
import math

class Quaternion:
	def __init__(self,w,x,y,z):
		self.w=w
		self.x=x
		self.y=y
		self.z=z
	def getMagnitude(self):
		return math.sqrt(self.w*self.w + self.x*self.x + self.y*self.y + self.z*self.z)
	def getConjugate(self):
		return Quaternion(self.w, -self.x, -self.y, -self.z)
	def normalize(self):
		m=self.getMagnitude()
		self.w/=m
		self.x/=m
		self.y/=m
		self.z/=m
	def getProduct(self,q):
		return Quaternion(
			self.w*q.w - self.x*q.x - self.y*q.y - self.z*q.z,
			self.w*q.x + self.x*q.w + self.y*q.z - self.z*q.y,
			self.w*q.y - self.x*q.z + self.y*q.w + self.z*q.x,
			self.w*q.z + self.x*q.y - self.y*q.x + self.z*q.w
		)
	def getEular(self):
		return [
			math.atan2(2*self.w*self.z + 2*self.x*self.y, 1-2*self.y*self.y-2*self.z*self.z)*180/math.pi,
			math.asin(2*self.w*self.y - 2*self.z*self.x)*180/math.pi,
			math.atan2(2*self.w*self.x + 2*self.y*self.z, 1-2*self.x*self.x-2*self.y*self.y)*180/math.pi
		]
	

def pwm_trim(ch):
	if ch<500:
		return 500
	if ch>2500:
		return 2500
	return ch
q0=Quaternion(1,0,0,0)
def angle_integral(ypr,q):
	global q0
	q.normalize()
	qd=q0.getConjugate().getProduct(q)
	yprd=qd.getEular()
	q0=q
	ypr[0]+=yprd[0]
	ypr[1]+=yprd[1]
	ypr[2]+=yprd[2]
	for i in range(0,3):
		while ypr[i]>90:
			ypr[i]-=90
		while ypr[i]<0:
			ypr[i]+=90
	return ypr	
def convert(filename):
	csv=""
	f = open(filename, "rb")
	last_pn=0
	byte = 1
	csv+="T,CH1,CH2,CH3,CH4,CH5,RPM,X,Y,Z,TX1,TX2,TX3,TX4,TX5,TX6,TX7\n"
	csv+="65536,2000,2000,2000,2000,2000,0,90,90,90,1023,1023,1023,1023,1023,1023,1023\n"
	csv+="0,1000,1000,1000,1000,1000,0,0,0,0,0,0,0,0,0,0,0\n"
	ypr=[0,0,0]
	byte = [1]
	while True:
		byte = [byte[len(byte)-1]]
		while byte[0] != 0x7f:
			byte = f.read(1)
			if len(byte)<1:
				break
		byte = f.read(1)
		if len(byte)<1:
			break
		if byte[0] != 0x7f:
			continue
		#sync with 0x7f7f

		byte = f.read(47)
		if len(byte)<47:
			break
		if byte[46] != 0x7f:
			continue
		
		arr = struct.unpack("<HH6H4f7H",byte[0:46])
		pn=arr[0]
		t=arr[1]
		ch=list(arr[2:7])
		if arr[7]!=0:
			rpm=1/arr[7]
		else:
			rpm=0
		q=arr[8:12]
		ypr=angle_integral(ypr,Quaternion(q[0],q[1],q[2],q[3]))
		for i in range(0,5):
			ch[i]=pwm_trim(ch[i])
		tx=arr[12:19]
		
		if pn == (last_pn+1)&0xffff:
			csv+=str(t)
			for i in range(0,5):
				csv+=","+str(ch[i])
			csv+=","+str(rpm)
			for i in range(0,3):
				csv+=","+str(ypr[i])
			for i in range(0,7):
				csv+=","+str(tx[i])
			csv+="\n"
		else:
			print("Nonconsecutive packets", last_pn, "->", pn, "SD card too slow")
		last_pn = pn
	f.close()
	return csv

if len(sys.argv)<2:
	filenumber=0
	for filename in os.listdir():
		if filename.endswith(".TXT") and filename.startswith("LOG"):
			n=int(filename[3:8])
			if n>filenumber:
				filenumber=n
	filebasename="LOG"+str(filenumber).zfill(5)
	filename=filebasename+".TXT"
	if os.path.isfile(filename):
		import plot
		from io import StringIO
		csvstring = convert(filename)
#		print(csvstring)
		fout = open(filebasename+".csv","w")
		fout.write(csvstring)
		fout.close()
		f = StringIO(csvstring)
		plot.plot(f)
	else:
		print("no file found")

else:
	print(convert(sys.argv[1]))
