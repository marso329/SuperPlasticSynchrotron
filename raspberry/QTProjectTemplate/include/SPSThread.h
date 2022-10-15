#ifndef INCLUDE_SPS_THREAD_H_
#define INCLUDE_SPS_THREAD_H_

#include <QThread>
#include <QObject>
#include <QTime>
#include <QtMath>
#include "serialController.h"
#include <sstream>
#include <string>
#include <regex>
#include <ctime>
#include <chrono>

enum MainState {START,SETUP,INJECTION,ACCELERATE,FLATTOP,EXTRACT};
enum SetupState {INJMOVEOUT,WAITINJMOVEOUT,EXTRMOVEOUT,WAITEXTRMOVEOUT,LOWERCAVITYVOLTAGE,WAITLOWERCAVITYVOLTAGE,SETUPFINISHED};
enum InjectionState {DROPREQ,WAITDROPREQ,WAITDROPSUCCESS,INJMOVEIN,WAITINJMOVEIN,INJECTIONFINISHED};
enum AccelerateState{INITACC,WAITACC,INCREASE,ACCELERATEFINISHED};
enum ExtractState{INITEXTRACT,WAITACC2,EXTRMOVEIN,WAITEXTRMOVEIN,SCREWROT1S,WAITSCREWROT1S,CHECKSCREWCOUNTER,EXTRACTIONFINISHED};
class SPSThread : public QThread{
Q_OBJECT
public:
	SPSThread();
	~SPSThread();
private:
	float convertFloat(std::string);
	int convertInt(std::string);
SerialController* controller;
	int automatic=-1;
	int beamPresent=-1;
	int injectorOut=-1;
	int extractorOut=-1;
	MainState mainstate=START;
	SetupState setupstate=INJMOVEOUT;
	InjectionState injectionstate=DROPREQ;
	AccelerateState acceleratestate=WAITACC;
	ExtractState extractstate=INITEXTRACT;
	int voltage=20;
	int readVoltage=20;
	std::chrono::system_clock::time_point acceleration_start; 
	//std::chrono::system_clock::time_point ticks_per_second; 
	int screwCounter=0;
protected:
	void run();
Q_SIGNALS:
void newMessage(QString);
void newReadableMessage(QString);
void newSpeed1(float);
void newSpeed2(float);
void newSpeed3(float);
void newRF1(float);
void newRF2(float);
void newRF3(float);
void newTurns(int);
void newInjections(int);
void newAction(QString);
void newMode(int);
void newBeamPresent(int);
void newSequence(QString);
void newSubSequence(QString);
void newEnergy(int);
void newHumanReadable(QString);
void newReadyForInjection(int);
void newRampDownIsRunning(int);

public Q_SLOTS:

};

#endif