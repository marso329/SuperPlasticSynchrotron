#include <SPSRegisterController.h>
#include <Stepper.h>
SPSRegisterController* controller;

//-----------------stepper pins------------------
// Define number of steps per revolution:
const int stepsPerRevolution = 200; 
// Give the motor control pins names:
const int pwmA = 3;
const int pwmB = 11;
const int brakeA = 9;
const int brakeB = 8;
const int dirA = 12;
const int dirB = 13;
// Initialize the stepper library on the motor shield:
Stepper myStepper = Stepper(stepsPerRevolution,dirA,dirB);   
long currentPos=0;
//------------------------------------------------

void setup()
{
//-------------SPS register controller------------------
controller=new SPSRegisterController();
//run reset sequence
controller->addRegister<bool>(0);
controller->setRegister<bool>(0,false);
controller->addRegister<long>(1);
controller->setRegister<long>(1,0);
//------------------------------------------------------

//--------------------stepper pins------------------------
  pinMode(pwmA, OUTPUT);
  pinMode(pwmB, OUTPUT);
  pinMode(brakeA, OUTPUT);
  pinMode(brakeB, OUTPUT);
  
  digitalWrite(pwmA, HIGH);
  digitalWrite(pwmB, HIGH);
  digitalWrite(brakeA, LOW);
  digitalWrite(brakeB, LOW);
  
  // Set the motor speed (RPMs):
  myStepper.setSpeed(100);
  //stepper zero switch
  pinMode(53, INPUT); // set pin to input
  digitalWrite(53, HIGH); // turn on pullup resistors
//-------------------------------------------------------
}

void loop()
{
  if (controller->getRegister<bool>(0)){
    myStepper.setSpeed(10);
    myStepper.step(-20);

    while(digitalRead(53)==HIGH){
      myStepper.step(5);  
  }
   
  controller->setRegister<bool>(0,false);
  currentPos=0;
  myStepper.setSpeed(60);  
  
}

if(controller->getRegister<long>(1)!=currentPos){
  myStepper.step(currentPos-controller->getRegister<long>(1));
  currentPos=controller->getRegister<long>(1);
}
}

ISR(UART0_RECEIVE_INTERRUPT)
{
 // unsigned char usr=  UART0_STATUS;
    controller->RXInterrupt(UART0_DATA);
}


ISR(UART0_TRANSMIT_INTERRUPT)
{
  controller->TXInterrupt();
}
