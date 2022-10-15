#include <cstdlib>
#include <iostream>
#include <cstring>

#define STRINGLENGTH 64
#define REGISTERSLENGTH 128
#define MIN(a,b) (((a)<(b))?(a):(b))


class RegisterNode{
friend class SPSRegisterController;
public:
protected:
private:
char* data=0;
uint8_t size=0;
};

class SPSRegisterController{
public:
	SPSRegisterController(){
		for(int i =0;i<REGISTERSLENGTH;i++){
			registers[i]=0;
		}
		addRegister<bool>(0);
		setRegister<bool>(0,false);
			
	}
	~SPSRegisterController(){}
	template<typename T>
	void addRegister(uint16_t pos){
		RegisterNode* temp;
		if(pos>=REGISTERSLENGTH){
			return;
		}

		if(registers[pos]!=0){
			return;
		}
		temp=reinterpret_cast<RegisterNode*>(malloc(sizeof(RegisterNode)));
		registers[pos]=temp;
		temp->data=reinterpret_cast<char*>(malloc(sizeof(T)));
		temp->size=sizeof(T);
		}
	template<typename T>
	void setRegister(uint16_t pos,T data){
		if(pos>=REGISTERSLENGTH){
			return;
		}
		if(registers[pos]==0){
			return;
		}
		memcpy(registers[pos]->data,&data,MIN(registers[pos]->size,sizeof(T)));
		change=true;
		latestChange=pos;
	
	}
	template<typename T>
	T getRegister(uint16_t pos){
		T toReturn=T();
		if(pos>=REGISTERSLENGTH){
			return toReturn;
		}
		if(registers[pos]==0){
			return toReturn;
		}
		memcpy(&toReturn,registers[pos]->data,MIN(registers[pos]->size,sizeof(T)));
		return toReturn;
	
	}

	bool changed(){
		if(change){
			change=false;
			return true;
		}
		return false;
	}

	uint16_t latest(){
		return latestChange;
	}

protected:
private:
	bool change=false;
	uint16_t latestChange;
	RegisterNode* registers[REGISTERSLENGTH];



};

template<>
void SPSRegisterController::addRegister<const char*>(uint16_t pos)
{
		RegisterNode* temp;
		if(pos>=REGISTERSLENGTH){
			return;
		}

		if(registers[pos]!=0){
			return;
		}
		temp=reinterpret_cast<RegisterNode*>(malloc(sizeof(RegisterNode)));
		registers[pos]=temp;
		temp->data=reinterpret_cast<char*>(malloc(STRINGLENGTH));
		temp->size=STRINGLENGTH;
}

template<>
void SPSRegisterController::setRegister<const char*>(uint16_t pos,const char* data)
{
		if(pos>=REGISTERSLENGTH){
			return;
		}
		if(registers[pos]==0){
			return;
		}
		strcpy(registers[pos]->data,data);
		change=true;
		latestChange=pos;
    
}

	template<>
	const char* SPSRegisterController::getRegister<const char*>(uint16_t pos){

		const char* toReturn=0;
		if(pos>=REGISTERSLENGTH){
			return toReturn;
		}
		if(registers[pos]==0){
			return toReturn;
		}
		return reinterpret_cast<char*>(registers[pos]->data);
	
	}


