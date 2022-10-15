import serial
import struct

typeIdentifier={bool:1,int:2,float:3}

registers={}

#	device, register number on device ,type, defaultValue							
registers["extractorReset"]=[0,0,bool,False]
registers["extractorPosition"]=[0,1,int,0]
class SPSRegisterControl:
	def __init__(self):
		#UART setup
  		self.ser = serial.Serial('/dev/ttyACM0',9600)

	def setRegister(self,register,value):

		if register not in registers:
			raise RuntimeError('register not defined in registers')
		
		if type(value)!=registers[register][2]:
			raise RuntimeError('type of value does not match the one specified in registers')

		if registers[register][1] >127 or registers[register][1]<0:
			raise RuntimeError('Register index is bigger than 127 or smaller than 0 which is not allowed')

		# the protocols sends the following:
		# byte0  [r/w,r6,r5,r4,r3,r2,r1,r0] where r=1=readRegister,r=0=setRegister and r6-0 is the register number on the device
		# byte1 [l7,l6,l5,l4,l3,l2,l1,l0] which is the length of the data in number of bytes
		# byte2 [t7,t6,t5,t4,t3,t2,t1,t0] which are identifiers for the different data types
		# 1= bool
		# 2= 32bit integer
		# byte3.... data
		data=[128|registers[register][1]]

		if registers[register][2]==bool:
			data.append(1)
			data.append(typeIdentifier[registers[register][2]])
			if value==True:
				data.append(255)
			else:
				data.append(0)
		elif registers[register][2]==int:
			temp=struct.pack("<i",value)
			data.append(4);
			data.append(typeIdentifier[registers[register][2]])
			for element in temp:
				data.append(element)
		else:
			return
		self.ser.write(data)
		#for element in data:
		#	self.ser.write([element])
		#	//sleep(0.001)				
		#print(self.ser)
		#print(data)
		returnData= self.ser.read(1)
		testResult = struct.unpack('>B', returnData)
		return 170==testResult[0]

	def readRegister(self,register):

		if register not in registers:
			raise RuntimeError('register not defined in registers')

		if registers[register][1] >127 or registers[register][1]<0:
			raise RuntimeError('Register index is bigger than 127 or smaller than 0 which is not allowed')

		# the protocols sends the following:
		# byte0  [r/w,r6,r5,r4,r3,r2,r1,r0] where r=1=readRegister,r=0=setRegister and r6-0 is the register number on the device
		# byte1 [l7,l6,l5,l4,l3,l2,l1,l0] which is the length of the data in number of bytes
		# byte2 [t7,t6,t5,t4,t3,t2,t1,t0] which are identifiers for the different data types
		# 1= bool
		# 2= 32bit integer
		# byte3.... data
		data=[registers[register][1]]
		returnData=[]
		if registers[register][2]==bool:
			data.append(1)
			data.append(typeIdentifier[registers[register][2]])
			#print(data)
			self.ser.write(data)
			returnData= self.ser.read(1)
			#print(returnData)
			return struct.unpack('>B', returnData)[0]
		elif registers[register][2]==int:
			data.append(4)
			data.append(typeIdentifier[registers[register][2]])
			#print(data)
			self.ser.write(data)
			returnData= self.ser.read(4)
			#print(returnData)
			return struct.unpack('<i', returnData)[0]			
