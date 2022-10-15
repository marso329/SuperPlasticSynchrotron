#include <SPI.h>
#include <Stepper.h>
#include <EEPROM.h>

// test change

enum Mode {OFF, manual, automatic};
Mode mode = OFF;
const bool debug = false; // debug mode, sends more stuff over serial

int PotentiometerPin = A7;

int KeyPin = A6;

int PotentiometerVal = 0; // current value of potentiometer
int PotentiometerValPrev = 0; // previous value of potentiometer

int LampButton1 = 12;
int LampButton2 = 11;
int LampButton3 = 10;
int LampButton4 = 9;
int LampButton5 = 8;
int LampButton6 = 7;
int LampRampDown = 6;
int LampReady = 48;
int LampBeamPresent = 49;
int DAC_CS = 53;

volatile int TurnsSinceInjection = 0;
int NumberOfInjections = 0;

unsigned long BPM1startTime = 0;
unsigned long BPM1endTime = 0;
unsigned long BPM1endTimePrev = 0;
unsigned long BPM2startTime = 0;
unsigned long BPM2endTime = 0;
unsigned long BPM2endTimePrev = 0;
unsigned long BPM3startTime = 0;
unsigned long BPM3endTime = 0;
unsigned long BPM3endTimePrev = 0;

unsigned long TimeNow = 0; // time stamps for actions in the main loop
unsigned long TimeLastEnergy = 0;
unsigned long TimeLastHostUpdate = 0;
unsigned long TimeLastButton = 0;
unsigned long TimeLastTemp = 0;

unsigned long NumberOfTurnsTotal = 0;

float BPM1speed = 0.0; // speed measured by BPM1
float BPM2speed = 0.0; // speed measured by BPM2
float BPM3speed = 0.0; // speed measured by BPM3
float BPM1frev = 0.0; // revolution frequency from BPM1
float BPM2frev = 0.0; // revolution frequency from BPM2
float BPM3frev = 0.0; // revolution frequency from BPM3

const float BPMlength = 0.038; // BPM length in meters (for speed calculation)

float AccVoltage = 0.0; // "Accelerating" voltage

const int CavityTime = 100; // us delay for the cavity loop
const int MaxPulseLength = 500; // maximum pulse length in units of cavity time (i.e. 50ms)

byte ReadPortE = 0;
byte PortKLast = 0; // stored last Port K value for edge detection
byte PortKActual = 0; // actual value of Port K
byte PortFLast = 0; // stored last Port F value for edge detection
byte PortFActual = 0; // actual value of Port F


// signal to the recipient the button was pressed. Cleared by the recipient.
bool Button1Pressed = false; // move injector
bool Button2Pressed = false; // request 1 ball
bool Button3Pressed = false; // move extractor
bool Button4Pressed = false; // rotate screw
bool Button5Pressed = false; // injection sequence request
bool Button6Pressed = false; // extraction sequence request
bool Button7Pressed = false;


bool Button1Released = false; // signal to the recipient the button was released. Cleared by the recipient.
bool Button2Released = false;
bool Button3Released = false;
bool Button4Released = false;
bool Button5Released = false;
bool Button6Released = false;
bool Button7Released = false;

volatile bool DumpPassage = false;
volatile bool DropperPassage = false;

bool LimitInjector = false;
bool LimitExtractor = false;

bool BeamPresence = false;
bool BeamPresencePrevious = false;

bool ReadyForInjection = false; // flag to indicate if the machine is ready for injection

volatile bool DumpHandshake = false; // flag used to detect the ball to be extracte passed the second BPM
volatile bool InjectionHandshake = false; // flag used to detect the injected ball passed the first BPM


const int stepsPerRevolution = 200;  //number of steps per revolution
Stepper MotorDropper(stepsPerRevolution, 34, 35); // initialize the stepper library on pins 34 and 35. Use 33 for Enable/Brake
const int DropperENAPin = 33;
Stepper MotorInjector(stepsPerRevolution, 30, 31); // initialize the stepper library on pins 30 and 31. Use 32 for Enable/Brake
const int InjectorENAPin = 32;
Stepper MotorExtractor(stepsPerRevolution, 46, 47); // initialize the stepper library on pins 46 and 47. Use 45 for Enable/Brake
const int ExtractorENAPin = 45;
Stepper MotorScrew(stepsPerRevolution, 42, 43); // initialize the stepper library on pins 42 and 43. Use 44 for Enable/Brake
const int ScrewENAPin = 44;

const int AddrTotalInjections = 0; // EEPROM address for total number of injections
const int AddrTotalTurns = 10; // EEPROM address for totasl number of turns

void setup() {
  // put your setup code here, to run once:


  // PortA assignment: 0 - Cavity #1 exit sensor, 1 - Cavity #2 exit sensor, 2 -, 3 -,
  // PortA assignment: 4 - , 5 - , 6 -, 7 -
  // PortA set all bits as inputs
  DDRA = DDRA & B00000000;


  // PortC assignemnt: 0 - Gate cavity #1, 1 - Gate cavity #2, 2 -, 3 -, 4 - , 5 - , 6 - , 7 -
  DDRC = DDRC | B11111111; // set all pins of PC as outputs
  PORTC = B00000000; // initialize port C. Gate driver is noninverting

  // PortE assignment: 0 - , 1 - , 2 -, 3 -,
  // PortE assignment: 4 - Cavity #1 entry sensor, 5 - Cavity #1 entry sensor, 6 - , 7 -
  DDRE = DDRE & B11001111; // set pins 4 and 5 of port E as inputs


  // PortF assignment: 0 - , 1 - , 2 -, 3 - button,
  // PortF assignment: 4 - button, 5 - button, 6 - switch, 7 - switch
  DDRF = DDRF & B11111111; // set pins 3 to 7 of port F as inputs


  // PortK set all bits as inputs for pin change interrupt (sensors attached)
  DDRK = DDRK & B00000000;

  pinMode(LampButton1, OUTPUT); digitalWrite(LampButton1, HIGH);
  pinMode(LampButton2, OUTPUT); digitalWrite(LampButton2, HIGH);
  pinMode(LampButton3, OUTPUT); digitalWrite(LampButton3, HIGH);
  pinMode(LampButton4, OUTPUT); digitalWrite(LampButton4, HIGH);
  pinMode(LampButton5, OUTPUT); digitalWrite(LampButton5, HIGH);
  pinMode(LampButton6, OUTPUT); digitalWrite(LampButton6, HIGH);
  pinMode(LampReady, OUTPUT);
  pinMode(LampRampDown, OUTPUT);
  pinMode(LampBeamPresent, OUTPUT);

  Serial.begin(57600); // communication with the programming computer, flashing, debugging
  Serial3.begin(9600); // communication to the Hameg power supply
  delay(100);

  // initialize interrupt, falling edge of INT4 and INT5
  EICRB = (EICRB | B00001010); // set the edge
  EICRB = (EICRB & B11111010); // set the edge
  EIMSK = (EIMSK | B00110000); // enable EXT4 and EXT5 interrupt for cavities

  // initialize interrupt, pin change PCINT23:16, input pins on port K
  PCICR  = B00000100;
  PCMSK2 = B11111111; //enable change interrupt for bit0 .. bit7

  sei(); //enable interrupts

  // initialize the SPI for DAC communication
  digitalWrite(DAC_CS,HIGH); 
  SPI.begin();
  SPI.setDataMode(SPI_MODE1);
  DAC_Initialize();  
  delay(10);

  //DAC_Send(970,0); // Send the velocity to the gauge


  digitalWrite(LampButton1, LOW); delay(100); digitalWrite(LampButton1, HIGH);
  digitalWrite(LampButton2, LOW); delay(100); digitalWrite(LampButton2, HIGH);
  digitalWrite(LampButton3, LOW); delay(100); digitalWrite(LampButton3, HIGH);
  digitalWrite(LampButton4, LOW); delay(100); digitalWrite(LampButton4, HIGH);
  digitalWrite(LampButton5, LOW); delay(100); digitalWrite(LampButton5, HIGH);
  digitalWrite(LampButton6, LOW); delay(100); digitalWrite(LampButton6, HIGH);
  digitalWrite(LampReady, HIGH); delay(100); digitalWrite(LampReady, LOW);
  digitalWrite(LampBeamPresent, HIGH); delay(100); digitalWrite(LampBeamPresent, LOW);
  digitalWrite(LampRampDown, HIGH); delay(100); digitalWrite(LampRampDown, LOW);

//  EEPROM.write(AddrTotalInjections,0);
//  EEPROM.write(AddrTotalTurns,0);

  NumberOfInjections = EEPROM.read(AddrTotalInjections);
  NumberOfTurnsTotal = EEPROM.read(AddrTotalTurns);

} // end setup



// interrupt service routine falling edge on external 4 (attached to Cavity #1 entry sensor)
ISR(INT4_vect)
{

  PORTC = PORTC & B11111110; // Gate OFF
  bool SensorEntry = false;
  bool SensorExit = false;
  int Timer;

  // wait 100us and check if the entry sensor is still active. Only then trigger the gate pulse
  delayMicroseconds(100);
  SensorEntry = (~PINE & 16); // Entry sensor for cavity 1 is connected to PortE 4. Negative pulse
  if (SensorEntry == true) {

    Timer = MaxPulseLength; // set timeout timer
    SensorExit = (~PINA & 1); // Exit sensor for cavity 1 is connected to PortA 0. Negative pulse

    while (SensorExit == false && Timer > 0) {
      PORTC = PORTC | B00000001; // Gate ON
      SensorExit = (~PINA & 1); // read new sensor status. Idle = false, ball passage = true
      delayMicroseconds(CavityTime);
      Timer--;
    }
    PORTC = PORTC & B11111110; // Gate OFF
  }

  PORTC = PORTC & B11111110; // Gate OFF

  Serial.println("ACC1");
  if (debug) {
    Serial.print(" ");
    Serial.print(SensorExit);
    Serial.print(" ");
    Serial.println(Timer);
  }

} // end interrupt service routine EXT4

// interrupt service routine falling edge on external 5 (attached to Cavity #2 entry sensor)
ISR(INT5_vect)
{
  bool SensorExit = false;
  bool SensorEntry = false;
  int Timer;

  // wait 100us and check if the entry sensor is still active. Only then trigger the gate pulse
  delayMicroseconds(100);
  SensorEntry = (~PINE & 32); // Entry sensor for cavity 2 is connected to PortE 5. Negative pulse
  if (SensorEntry == true) {

    Timer = MaxPulseLength; // set timeout timer
    SensorExit = (~PINA & 2); // Exit sensor for cavity 2 is connected to PortA 1. Negative pulse

    while (SensorExit == false && Timer > 0) {
      PORTC = PORTC | B00000010; // Gate ON
      SensorExit = (~PINA & 2); // read new sensor status. Idle = false, ball passage = true
      delayMicroseconds(CavityTime);
      Timer--;
    }
    PORTC = PORTC & B11111101; // Gate OFF
  }
  PORTC = PORTC & B11111101; // Gate OFF

  Serial.println("ACC2");
  if (debug) {
    Serial.print(" ");
    Serial.print(SensorExit);
    Serial.print(" ");
    Serial.println(Timer);
  }

} // end interrupt service routine EXT5


// Pin change interrupt PCI2 from Port K
ISR(PCINT2_vect) {
  PortKActual = PINK; // store Port K

  byte NEdge = (~PortKActual & PortKLast);
  //Serial.print("PCint "); Serial.print(PortKActual,BIN); Serial.print(" "); Serial.println(NEdge,BIN);

  // falling edge detected (ball passed)
  if (~NEdge) {
    if (NEdge & 1) { // bit 0 active
      BPM1startTime = micros();
      InjectionHandshake = true;
    }

    if (NEdge & 2) { // bit 1 active
      BPM1endTime = micros();
      BPM1speed = BPMlength / (BPM1endTime - BPM1startTime) * 1e6;
      Serial.print("BPM1:"); Serial.println(BPM1speed, 4); //Serial.println("m/s");
      BPM1frev = 1e6 / (BPM1endTime - BPM1endTimePrev);
      Serial.print("FREV1:"); Serial.println(BPM1frev, 4); //Serial.println("Hz");
      BPM1endTimePrev = BPM1endTime; // archive to calculate the frev

      TurnsSinceInjection = TurnsSinceInjection + 1; // increment number of turns since injection
      Serial.print("TURNS:"); Serial.println(TurnsSinceInjection);

      DAC_Send((TurnsSinceInjection << 2) & 0x3ff, 1); // Send the turns number to the gauge
      int velocity = static_cast<int>( 995.349 * BPM1frev);
      DAC_Send(velocity & 0x3ff ,0); // Send the velocity to the gauge
      // 3.0784 meters * frev * 970 counts /3.0 m/s fullk scale)
    }
    
    if (NEdge & 8) { // bit 3 active
      BPM2startTime = micros();
      DumpHandshake = true;
    }

    if (NEdge & 4) { // bit 2 active
      BPM2endTime = micros();
      BPM2speed = BPMlength / (BPM2endTime - BPM2startTime) * 1e6;
      Serial.print("BPM2:"); Serial.println(BPM2speed, 4); //Serial.println("m/s");
      BPM2frev = 1e6 / (BPM2endTime - BPM2endTimePrev);
      Serial.print("FREV2:"); Serial.println(BPM2frev, 4); //Serial.println("Hz");
      BPM2endTimePrev = BPM2endTime; // archive to calculate the frev
       int velocity = static_cast<int>( 995.349 * BPM2frev);
      DAC_Send(velocity & 0x3ff ,0); // Send the velocity to the gauge
     
    }

    if (NEdge & 16) { // bit 4 active
      BPM3startTime = micros();
    }

    if (NEdge & 32) { // bit 5 active
      BPM3endTime = micros();
      BPM3speed = BPMlength / (BPM3endTime - BPM3startTime) * 1e6;
      Serial.print("BPM3:"); Serial.println(BPM3speed, 4); //Serial.println("m/s");
      BPM3frev = 1e6 / (BPM3endTime - BPM3endTimePrev);
      Serial.print("FREV3:"); Serial.println(BPM3frev, 4); //Serial.println("Hz");
      BPM3endTimePrev = BPM3endTime; // archive to calculate the frev
      int velocity = static_cast<int>( 995.349 * BPM3frev);
      DAC_Send(velocity & 0x3ff ,0); // Send the velocity to the gauge
    }

    if (NEdge & 64) { // bit 6 active: Dump passage
      DumpPassage = true;
      EEPROM.update(AddrTotalTurns, static_cast<unsigned long>(NumberOfTurnsTotal + TurnsSinceInjection));
      Serial.print("TURNSTOTAL:"); Serial.println(NumberOfTurnsTotal + TurnsSinceInjection);
      TurnsSinceInjection = 0;
      BeamPresence = false;
      DAC_Send(0,0); // Send the velocity to the gauge

      if (debug) {
        Serial.println("DUMPSENS");
      }
    }

    if (NEdge & 128) { // bit 7 active: Dropper passage
      DropperPassage = true;
      NumberOfInjections = NumberOfInjections + 1;
      Serial.print("INJECTIONS:"); Serial.println(NumberOfInjections);
      if (debug) {
        Serial.println("DROPSENS");
      }
      EEPROM.update(AddrTotalInjections,NumberOfInjections);
    }

  }


  PortKLast = PortKActual; // store Port K

} // end interrupt service routine PCI2



void loop() {
  // put your main code here, to run repeatedly:

  TimeNow = millis();

  int keypinValue = analogRead(KeyPin);
  // Serial.println(keypinValue);
  //key is in auto mode
  if (keypinValue >= 800) {
    switch (mode) {
      case automatic:
        break;
      default:
        Serial.println("MODE:AUTO");
        mode = automatic;
        break;
    }

  }
  else if (200 < keypinValue && keypinValue < 800) {
    switch (mode) {
      case OFF:
        break;
      default:
        Serial.println("MODE:OFF");
        mode = OFF;
        break;
    }
  }
  //key is in off or manual
  else {
    switch (mode) {
      case manual:
        break;
      default:
        Serial.println("MODE:MANUAL");
        mode = manual;
        break;
    }
  }


  //update host once per second
  if (TimeNow - TimeLastHostUpdate >= 1000) {
    TimeLastHostUpdate = millis();
    if (BeamPresence) {
      Serial.println("BEAM:PRESENT");
    }
    else {
      Serial.println("BEAM:NOTPRESENT");
    }
    if (mode == automatic) {
      Serial.println("MODE:AUTO");
    }
    else if (mode == manual) {
      Serial.println("MODE:MANUAL");
    }
    Serial.print("VACC:"); Serial.println(AccVoltage, 2);
  }

  // read buttons every 50ms
  if (TimeNow - TimeLastButton >= 50) {
    ReadButtons();

    if ( (micros() - BPM1endTime) <= 3e6 ) {
      BeamPresence = true;
    }
    else
    {
      BeamPresence = false;
    }

    if (BeamPresence != BeamPresencePrevious) {
      BeamPresencePrevious = BeamPresence;
      if (BeamPresence) {
        Serial.println("BEAM:PRESENT");
      }
      else {
        Serial.println("BEAM:NOTPRESENT");
      }
    }
  }


  if (mode == manual) {
    // read energy every 300ms
    if (TimeNow - TimeLastEnergy >= 300) {
      TimeLastEnergy = TimeNow;
      EnergySetting();
    }

    //  if (TimeNow - TimeLastTemp >= 500) {
    //    TimeLastTemp = TimeNow;
    //    Serial.println(PINA,BIN);
    //  }

    // ------- button actions ----------------
    // button 1 - toggle position of the injector
    if (Button1Pressed) {
      Button1Pressed = false;
      if (LimitInjector == false) {
        // If false, the injector is in the IN position and move OUT
        MoveInjectorOut();
      }
      else
      {
        // If true, the injector is in the OUT position and should move IN
        MoveInjectorIn();
      }
    }
    if (Button1Released) {
      Button1Released = false;
    }

    if (Button2Pressed) { // button 2 drop one ball
      Button2Pressed = false;

      // block injection if not at low energy
      if (AccVoltage >= 21.0)
      {
        digitalWrite(LampRampDown, HIGH);
      }
      else
      {
        digitalWrite(LampRampDown, LOW);
        DropForInjection();
      }
    }
    if (Button2Released) {
      Button2Released = false;
    }

    if (Button3Pressed) { // button 3
      Button3Pressed = false;
      if (LimitExtractor == true)
      {
        MoveExtractorOut(); // If true, the extractor is the IN position and should move OUT
      }
      else
      {
        MoveExtractorIn(); // If false, the extractor is the OUT position and should move IN
      }
    }
    if (Button3Released) {
      Button3Released = false;
    }

    if (Button4Pressed) { // button 4 pressed manually - keep rotating the VLINAC screw until released
      //Button4Pressed = false;
      RotateScrew1s(); // move the extractor to position for extraction
    }
    if (Button4Released) { // button 4 released - stop rotating the screw
      Button4Pressed = false;
      Button4Released = false;
    }



    if (Button5Pressed) { // button
      Button5Pressed = false;
      InjectionSequence(); // semi-automatic injection sequence
    }
    if (Button5Released) {
      Button5Released = false;
    }

    if (Button6Pressed) { // button
      Button6Pressed = false;
      ExtractionSequence(); // semi-automatic extraction sequence
    }
    if (Button6Released) {
      Button6Released = false;
    }

    // do nothing for 1ms
    delay(1);
  }

  //automatic mode
  else
  {
    if (Serial.available()) {
      String command = Serial.readString();
      Serial.print("received:");
      Serial.println(command);
      if (command == "INJMOVEOUT") {
        MoveInjectorOut();
      }
      else if (command == "INJMOVEIN") {
        MoveInjectorIn();
      }
      else if (command == "EXTRMOVEOUT") {
        MoveExtractorOut();
      }
      else if (command == "EXTRMOVEIN") {
        MoveExtractorIn();
      }
      else if (command == "DROPREQ") {
        DropForInjection();
      }
      else if (command == "SCREWROT1S") {
        RotateScrew1s();
      }
      //VOLTAGE:**
      else if (command.length() > 7) {
        if (command.substring(0, 7) == "VOLTAGE") {
          int voltage = command.substring(8).toInt();
          float voltage_f = float(voltage);
          AccVoltage = voltage_f;
          Serial3.println("INST OUT2");
          Serial3.print("VOLTAGE:IMMEDIATE "); Serial3.println(voltage_f / 2, 2);
          Serial3.println("INST OUT3");
          Serial3.print("VOLTAGE:IMMEDIATE "); Serial3.println(voltage_f / 2, 2);
          //  Serial.println("set voltage :");
          // Serial.println(voltage);
        } else {
          // Serial.println("did not match VOLTAGE");
          //Serial.println(command);
        }

      }
      else {
        //   Serial.println("not match");
        //  Serial.println(command);
      }

    }
  } // end automatic mode

  if (BeamPresence == true)
  {
    digitalWrite(LampBeamPresent, HIGH);
  }
  else
  {
    digitalWrite(LampBeamPresent, LOW);
  }

  // Ready for injection flag. Conditions:
  // Beam presence no, Extraction element in closed orbit mode
  // Injection elemtent in injection position, Energy is low
  if ( (BeamPresence == false) && (LimitExtractor == false) && (LimitInjector == true) && (AccVoltage <= 21.0) )
  {
    ReadyForInjection = true;
  } else {
    ReadyForInjection = false;
  }

  // Ready for injection lamp
  if (ReadyForInjection == true) {
    digitalWrite(LampReady, HIGH);
  }
  else {
    digitalWrite(LampReady, LOW);
  }

  if (AccVoltage < 21.0)
  {
    digitalWrite(LampRampDown, LOW);
  }

} // end main loop

// read Energy setting from potentiometer and send value to the power supply
void EnergySetting() {

  PotentiometerVal = analogRead(PotentiometerPin);
  
  // write new power supply voltage only if the potentiometer value had changed
  if ((PotentiometerVal >> 1) != (PotentiometerValPrev >> 1)) {
    if (PotentiometerVal <= 140) {
      PotentiometerVal = 140;
    }

    AccVoltage = map(PotentiometerVal, 140, 630, 200, 400) / 10.0; // map potentiometer to Acc Voltage 20V to 40V

    //Serial.print("potval "); Serial.println(PotentiometerVal);

    Serial3.println("INST OUT2");
    Serial3.print("VOLTAGE:IMMEDIATE "); Serial3.println(AccVoltage / 2.0, 2);
    Serial3.println("INST OUT3");
    Serial3.print("VOLTAGE:IMMEDIATE "); Serial3.println(AccVoltage / 2.0, 2);
  }
  PotentiometerValPrev = PotentiometerVal;

} // end Energy setting


// Read buttons and switches
void ReadButtons() {

  PortFActual = PINF & B0111111; // store Port K. Mask the ADC input

  byte PEdge = (PortFActual & ~PortFLast);
  if (PEdge != 0) {
    if (PEdge & 1) {
      Button1Pressed = true;
      Serial.println("BUT1PRESS");
    }
    if (PEdge & 2) {
      Button2Pressed = true;
      Serial.println("BUT2PRESS");
    }
    if (PEdge & 4) {
      Button3Pressed = true;
      Serial.println("BUT3PRESS");
    }

    if (PEdge & 8) {
      Button4Pressed = true;
      Serial.println("BUT4PRESS");
    }

    if (PEdge & 16) {
      Button5Pressed = true;
      Serial.println("BUT5PRESS");
    }

    if (PEdge & 32) {
      Button6Pressed = true;
      Serial.println("BUT6PRESS");
    }

    if (PEdge & 64) {
      Button7Pressed = true;
      Serial.println("BUT7PRESS");
    }
  }

  byte NEdge = (PortFLast & ~PortFActual);
  if (NEdge != 0) {
    if (NEdge & 1) {
      Button1Released = true;
      Serial.println("BUT1RLS");
    }

    if (NEdge & 2) {
      Button2Released = true;
      Serial.println("BUT2RLS");
    }

    if (NEdge & 4) {
      Button3Released = true;
      Serial.println("BUT3RLS");
    }

    if (NEdge & 8) {
      Button4Released = true;
      Serial.println("BUT4RLS");
    }

    if (NEdge & 16) {
      Button5Released = true;
      Serial.println("BUT5RLS");
    }

    if (NEdge & 32) {
      Button6Released = true;
      Serial.println("BUT6RLS");
    }
    if (NEdge & 64) {
      Button7Released = true;
      Serial.println("BUT7RLS");
    }
  }

  PortFLast = PortFActual;

} // end Read buttons

// function to do drop one ball
void DropForInjection() {
  digitalWrite(LampButton2, LOW);

  Serial.println("DROPREQ");
  MotorDropper.setSpeed(40); // set speed in rpm
  digitalWrite(DropperENAPin, HIGH);

  int iter = 50; // maximum number of iterations for loop timeout, bit more than 1 turn
  while (iter > 0 & DropperPassage == false) {
    MotorDropper.step(-10);
    iter = iter - 1;
  }

  if (DropperPassage == true) {
    DropperPassage = false;
    BeamPresence = true;
    Serial.println("DROPSUCCESS"); // drop succesfull
    if (debug) {
      Serial.println(iter);
    }

  }
  else
  {
    Serial.println("DROPFAIL"); // drop failed
    if (debug) {
      Serial.println(iter);
    }
  }

  digitalWrite(DropperENAPin, LOW);
  DropperPassage = false; // clear the ball passage flag

  digitalWrite(LampButton2, HIGH);

} // end drop ball


// function to move the injector to position for injection
void MoveInjectorIn() {

  LimitInjector = (PINA & B00000100) >> 2;
  if (debug) {
    Serial.print("Limit Inj "); Serial.println(LimitInjector);
  }

  // check the limit switch.
  if (LimitInjector == false)
  {
    // If false, the injector is already in the IN position and should not move IN anymore
    Serial.println("ERR:INJALREADYIN"); // just send error message and do nothing
  }
  else
  {
    // Not in limit position, do the movement
    digitalWrite(LampButton1, LOW);


    int steps = -150;
    digitalWrite(InjectorENAPin, HIGH);
    MotorInjector.setSpeed(45);
    //MotorInjector.step(-30);
    //MotorInjector.setSpeed(30);
    MotorInjector.step(steps);
    digitalWrite(InjectorENAPin, LOW);
    digitalWrite(LampButton1, HIGH);
    Serial.println("INJMOVEIN");
  }

  LimitInjector = (PINA & B00000100) >> 2;

} // end move in injector

// move the injector to closed orbit position
void MoveInjectorOut() {

  LimitInjector = (PINA & B00000100) >> 2;

  // check the limit switch.
  if (LimitInjector == true)
  {
    // If true, the injector is already in the OUT position and should not move OUT anymore
    Serial.println("ERR:INJALREADYOUT"); // just send error message and do nothing
  }
  else
  {
    // Not in limit position, do the movement
    digitalWrite(LampButton1, LOW);
    int steps = 150;
    digitalWrite(InjectorENAPin, HIGH);
    //MotorInjector.setSpeed(30);
    //MotorInjector.step(30);
    MotorInjector.setSpeed(30);
    MotorInjector.step(steps);
    //}
    digitalWrite(InjectorENAPin, LOW);
    digitalWrite(LampButton1, HIGH);
    Serial.println("INJMOVEOUT");
  }

  LimitInjector = (PINA & B00000100) >> 2;

} // end move out injector

// function to move the extractor to position for injection
void MoveExtractorIn() {

  LimitExtractor = (PINA & B00001000) >> 3;
  // check the limit switch.
  if (LimitExtractor == true)
  {
    // If true, the extractor is already in the IN position and should not move IN anymore
    Serial.println("ERR:EXTRALREADYIN"); // just send error message and do nothing
  }
  else
  {
    digitalWrite(LampButton3, LOW);
    int steps = 100;
    MotorExtractor.setSpeed(40); // set speed in rpm
    digitalWrite(ExtractorENAPin, HIGH);
    MotorExtractor.step(steps);
    digitalWrite(ExtractorENAPin, LOW);
    digitalWrite(LampButton3, HIGH);
    Serial.println("EXTRMOVEIN");
  }
  LimitExtractor = (PINA & B00001000) >> 3;

} // end move in extractor

// move the extractor to closed orbit position
// closed orbit position is with the switch closed, i.e. PortA bit 2 is zero
void MoveExtractorOut() {

  LimitExtractor = (PINA & B00001000) >> 3;
  // check the limit switch.
  if (LimitExtractor == false)
  {
    // If false, the extractor is already in the OUT position and should not move OUT anymore
    Serial.println("ERR:EXTRALREADYOUT"); // just send error message and do nothing
  }
  else
  {
    digitalWrite(LampButton3, LOW);
    int steps = -100;
    MotorExtractor.setSpeed(40); // set speed in rpm
    digitalWrite(ExtractorENAPin, HIGH);
    MotorExtractor.step(steps);
    digitalWrite(ExtractorENAPin, LOW);
    digitalWrite(LampButton3, HIGH);
    Serial.println("EXTRMOVEOUT");

  }
  LimitExtractor = (PINA & B00001000) >> 3;

} // end move out extractor

// function to move the VLINAC screw
void RotateScrew() {
  Serial.println("RotateScrew");
  digitalWrite(LampButton4, LOW);

  int steps = -20;
  MotorScrew.setSpeed(100); // set speed in rpm
  digitalWrite(ScrewENAPin, HIGH);
  MotorScrew.step(-200 * 27);

  digitalWrite(ScrewENAPin, LOW);
  digitalWrite(LampButton4, HIGH);

} // end move screw


// function to move the VLINAC screw for 1 second
void RotateScrew1s() {
  if (debug) {
    Serial.println("rotate screw man");
  }

  digitalWrite(LampButton4, LOW);

  MotorScrew.setSpeed(120); // set speed in rpm
  digitalWrite(ScrewENAPin, HIGH);
  MotorScrew.step(-200);
  Serial.println("SCREWROT1S");

  digitalWrite(ScrewENAPin, LOW);
  digitalWrite(LampButton4, HIGH);

} // end move screw for 1sec


void InjectionSequence() {

  digitalWrite(LampButton5, LOW);

  delay(1000); // just for lamp to indicate the request was received

  // if beam is present do not inject
  if (BeamPresence == true)
  { // flash the beam presence lamp few times to warn the operator
    for (int i = 0; i <= 9; i++)
    { digitalWrite(LampBeamPresent, HIGH);
      delay(100);
      digitalWrite(LampBeamPresent, LOW);
      delay(100);
    }
  }
  else
  { // beam is not present, check the energy
    if (AccVoltage >= 21.0)
    { // flash the ramp down lamp few times to warn the operator
      for (int i = 0; i <= 9; i++)
      { digitalWrite(LampRampDown, LOW);
        delay(100);
        digitalWrite(LampRampDown, HIGH);
        delay(100);
      }
    }
    else
    { // beam not present and also low energy

      MoveInjectorOut(); // move injector to injection position
      delay(1000); // wait

      InjectionHandshake = false; // clear the handshake flag
      DropForInjection(); // try one injection
      if (BeamPresence == false) // check if injection was succesfull. If not repeat injection once
      {
        DropForInjection(); // try another injection
      }

      int timeout = 2000; // 2s timeout for the passage
      while ( (InjectionHandshake == false) && (timeout > 0))
      { // wait for the ball to pass, or timeout
        timeout = timeout - 1;
        delay(1);
      }

      MoveInjectorIn(); // move injector back to closed orbit position

    }

  } //end semi-automatic injection sequence


  digitalWrite(LampButton5, HIGH);

} // end semi-automatic injection sequence

void ExtractionSequence()
{
  digitalWrite(LampButton6, LOW);
  delay(1000);



  if (BeamPresence == true)
  { // beam is circulating, start the synchronization

    int timeout = 6000; // 6s timeout for passage synchronization (~4 turns)
    DumpHandshake = false;   
     
    while ( (DumpHandshake == false) && (timeout > 0))
    { // wait for the ball to pass, or timeout
      timeout = timeout - 1;
      if ((timeout >> 7) & 1) { // flast the lamp to indicate the synchronization is active
          digitalWrite(LampButton6, LOW);
      }
      else
      {
          digitalWrite(LampButton6, HIGH);
          }      
      delay(1);
    }
    
    //Serial.print(timeout); Serial.print(" "); Serial.println(DumpHandshake);

    
    if (DumpHandshake == true)
    { // synchronization did not time out, but detected the passage
      Serial.println("DUMPSYNCHRONIZED");
      MoveExtractorIn();
      delay(3000);
      MoveExtractorOut();
    }

  }

  digitalWrite(LampButton6, HIGH);

} // end semi-automatic extraction sequence



void DAC_Initialize(){
    // reset IC
    digitalWrite(DAC_CS,LOW); 
    SPI.transfer16(0b1111000000000000);
    digitalWrite(DAC_CS,HIGH); 
    
    // power-down IC
    digitalWrite(DAC_CS,LOW); 
    SPI.transfer16(0b1100000000000000);
    digitalWrite(DAC_CS,HIGH); 
    
    // setup LDAC
    digitalWrite(DAC_CS,LOW); 
    SPI.transfer16(0b1010000000000000);
    digitalWrite(DAC_CS,HIGH); 
    
    // reference selection
    digitalWrite(DAC_CS,LOW); 
    SPI.transfer16(0b1000000000000011);
    digitalWrite(DAC_CS,HIGH); 
} // end DAC initialization


void DAC_Send(int value, int channel){
    digitalWrite(DAC_CS,LOW); 
    SPI.transfer16(0x0000 | (channel << 12) | ( value << 2) );
    digitalWrite(DAC_CS,HIGH); 
}
