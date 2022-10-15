from spsRegisterController import *
controller=SPSRegisterControl()
print(controller.setRegister("extractorReset",True))
while controller.readRegister("extractorReset"):
	pass
print("done")
print(controller.setRegister("extractorPosition",32))


#print(controller.readRegister("injectorPosition"))

#print(controller.setRegister("injectorPosition0",-1231))

#print(controller.readRegister("injectorPosition0"))