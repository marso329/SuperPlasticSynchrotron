#include "SPSRegisterController.h"

int main(){
	SPSRegisterController test= SPSRegisterController();
	test.addRegister<bool>(0);
	test.setRegister<bool>(0,true);
	std::cout<<test.getRegister<bool>(0)<<std::endl;

	test.addRegister<int>(1);
	test.setRegister<int>(1,32);
	std::cout<<test.getRegister<int>(1)<<std::endl;


	test.addRegister<const char*>(2);
	test.setRegister<const char*>(2,"data");
	std::cout<<test.getRegister<const char*>(2)<<std::endl;
}