
#include "SPSThread.h"

SPSThread::SPSThread(){
	controller=new SerialController(this);
}

SPSThread::~SPSThread(){

}
float SPSThread::convertFloat(std::string data){
	float converted_f;
	try{
		converted_f=stof(data);
	}
	catch (const std::invalid_argument& e){
		Q_EMIT(newReadableMessage("float conversion failed"));
		return 0.0;
	}
	return converted_f;
}

int SPSThread::convertInt(std::string data){
	int converted_int;
	try{
		converted_int=stoi(data);
	}
	catch (const std::invalid_argument& e){
		Q_EMIT(newReadableMessage("int conversion failed"));
		return 0;
	}
	return converted_int;
}

void SPSThread::run(){
	while(true){
		//get one update from SPS (none blocking)
		std::string data=controller->readData();

		//all regular expression--------------------------------
		//match any BPM speed readout
		std::regex any_bpm_regex("BPM[0-9]+:[+-]?[0-9]*\\.?[0-9]+[\\S\\s]*");
		//It is required to have a point so it does not match the number of the BPM or cavity
		std::regex float_regex("[+-]?[0-9]*\\.[0-9]+");
		std::regex int_regex("[0-9]+");
		std::regex any_frev_regex("FREV[0-9]+:[+-]?[0-9]*\\.?[0-9]+[\\S\\s]*");
		std::regex turns_regex("TURNS:[0-9]+[\\S\\s]*");
		std::regex acc_regex("ACC[0-9]+[\\S\\s]*");
		std::regex voltage_regex("VACC:[+-]?[0-9]*\\.?[0-9]+[\\S\\s]*");
		std::regex button_regex("BUT[0-9]+(PRESS|RLS)[\\S\\s]*");
		std::regex extractor_regex("EXTRMOVE(IN|OUT)[\\S\\s]*");
		std::regex screw_regex("SCREWROT1S[\\S\\s]*");
		std::regex injector_regex("INJMOVE(IN|OUT)[\\S\\s]*");
		std::regex error_regex("ERR:(INJALREADYOUT|EXTRALREADYOUT)[\\S\\s]*");
		std::regex drop_regex("DROP(REQ|SUCCESS|FAIL)[\\S\\s]*");
		std::regex injections_regex("INJECTIONS:[0-9]+[\\S\\s]*");
		std::regex mode_regex("MODE:(MANUAL|AUTO)[\\S\\s]*");
		std::regex beam_regex("BEAM:(PRESENT|NOTPRESENT)[\\S\\s]*");
		std::regex empty_regex("[\\S\\s]*");
		//------------------------------------------------------
		std::smatch number_match;
		std::smatch index_match;
		int index=1;
		float value=0.0f;
		//check that there is data
		//data="BPM2:4.1037";
		if(data.size()>1){
			//Output message to general logger
			Q_EMIT(newMessage(QString::fromStdString(data.substr(0, data.size()-1))));
			//speed from BPM
			if(std::regex_match(data,any_bpm_regex)){
				std::regex_search(data, number_match, float_regex);
				std::regex_search(data, index_match, int_regex);
				if(!number_match.empty() && !index_match.empty()){
					index=convertInt(index_match[0]);
					value=convertFloat(number_match[0]);
					if (index==1){
						Q_EMIT(newSpeed1(value));
						Q_EMIT(newAction("BMP1"));
					}
					else if (index==2){
						Q_EMIT(newSpeed2(value));
						Q_EMIT(newAction("BMP2"));
					}
					else if (index==3){
						Q_EMIT(newSpeed3(value));
						Q_EMIT(newAction("BMP3"));
					}
					else{
						Q_EMIT(newReadableMessage("Did not recognise BPM index"));
						
					}
				}
				else{
					Q_EMIT(newReadableMessage("No index or value in BPM match"));
				}

			}
			//Turns
			else if(std::regex_match(data,turns_regex)){
				std::regex_search(data, index_match, int_regex);
				if( !index_match.empty()){
					index=convertInt(index_match[0]);
					Q_EMIT(newTurns(index));
				}
				else{
					Q_EMIT(newReadableMessage("No value in TURNS match"));
				}

			}
			//Injections
			else if(std::regex_match(data,injections_regex)){
				std::regex_search(data, index_match, int_regex);
				if( !index_match.empty()){
					index=convertInt(index_match[0]);
					Q_EMIT(newInjections(index));
				}
				else{
					Q_EMIT(newReadableMessage("No value in INJECTIONS match"));
				}

			}
			//frev
			else if(std::regex_match(data,any_frev_regex)){
				std::regex_search(data, number_match, float_regex);
				std::regex_search(data, index_match, int_regex);
				if(!number_match.empty() && !index_match.empty()){
					index=convertInt(index_match[0]);
					value=convertFloat(number_match[0]);
					if (index==1){
						Q_EMIT(newRF1(value));
					}
					else if (index==2){
						Q_EMIT(newRF2(value));
					}
					else if (index==3){
						Q_EMIT(newRF3(value));
					}
					else{
						Q_EMIT(newReadableMessage("Did not recognise FREV index"));
						
					}
				}
				else{
					Q_EMIT(newReadableMessage("No index or value in FREV match"));
				}
			}
			//cavity
			else if(std::regex_match(data,acc_regex)){
				std::regex_search(data, index_match, int_regex);
				if( !index_match.empty()){
					index=convertInt(index_match[0]);
					if (index==1){
						Q_EMIT(newAction("ACC1"));
					}
					else if (index==2){
						Q_EMIT(newAction("ACC2"));
						if(mainstate==EXTRACT && extractstate==WAITACC2){
							extractstate=EXTRMOVEIN;
						}
					}
					else{
						Q_EMIT(newReadableMessage("Did not recognise ACC index"));
						
					}
				}
				else{
					Q_EMIT(newReadableMessage("No index in ACC match"));
				}
			}
			else if(std::regex_match(data,mode_regex)){
				if (data.find("MANUAL")!=std::string::npos){
					if(automatic!=0){
						Q_EMIT(newMode(0));
						Q_EMIT(newSequence("MANUAL"));
						Q_EMIT(newSubSequence("MANUAL"));
						Q_EMIT(newHumanReadable("MANUAL MODE"));
					}
					automatic=0;
				}
				else if(data.find("AUTO")!=std::string::npos){
					if(automatic!=1){
						Q_EMIT(newMode(1));

					}
					automatic=1;
				}
				else{
					Q_EMIT(newReadableMessage("Could not match mode in MODE command"));
				}
			}

			else if(std::regex_match(data,beam_regex)){
				if (data.find(":PRESENT")!=std::string::npos){
					if(beamPresent!=1){
						Q_EMIT(newBeamPresent(1));
					}
					beamPresent=1;
				}
				else if(data.find(":NOTPRESENT")!=std::string::npos){
					if(beamPresent!=0){
						Q_EMIT(newBeamPresent(0));
					}
					beamPresent=0;
				}
				else{
					Q_EMIT(newReadableMessage("Could not match mode in BEAM command"));
				}
			}

			else if(std::regex_match(data,injector_regex)){
				if (data.find("MOVEIN")!=std::string::npos){
					injectorOut=0;
					if(mainstate==INJECTION && injectionstate==WAITINJMOVEIN){
						injectionstate=INJECTIONFINISHED;
					}
					if(automatic==0){
						Q_EMIT(newHumanReadable("MANUAL MODE\nInjector in closed orbit position"));
					}
				}
				else if(data.find("MOVEOUT")!=std::string::npos){
					injectorOut=1;
					if(mainstate==SETUP && setupstate==WAITINJMOVEOUT){
						setupstate=EXTRMOVEOUT;
					}
					if(automatic==0){
						Q_EMIT(newHumanReadable("MANUAL MODE\nInjector in injection position"));
					}

				}
				else{
					Q_EMIT(newReadableMessage("Could not match position in INJMOVE command"));
				}
				if(extractorOut==1 &&injectorOut==1){
					Q_EMIT(newReadyForInjection(1));
				}
				else{
					Q_EMIT(newReadyForInjection(0));
				}
			}
			else if(std::regex_match(data,extractor_regex)){
				if (data.find("MOVEIN")!=std::string::npos){
					extractorOut=0;
					if(mainstate==EXTRACT && extractstate==WAITEXTRMOVEIN){
						extractstate=SCREWROT1S;
					}
					if(automatic==0){
						Q_EMIT(newHumanReadable("MANUAL MODE\nExtractor in extraction position"));
					}
				}
				else if(data.find("MOVEOUT")!=std::string::npos){
					extractorOut=1;
					if(mainstate==SETUP && setupstate==WAITEXTRMOVEOUT){
						setupstate=LOWERCAVITYVOLTAGE;
					}
					if(automatic==0){
						Q_EMIT(newHumanReadable("MANUAL MODE\nExtractor in closed orbit position"));
					}

				}
				else{
					Q_EMIT(newReadableMessage("Could not match position in INJMOVE command"));
				}
				if(extractorOut==1 &&injectorOut==1){
					Q_EMIT(newReadyForInjection(1));
				}
				else{
					Q_EMIT(newReadyForInjection(0));
				}
			}

			else if(std::regex_match(data,error_regex)){
				if(data.find("INJALREADYOUT")!=std::string::npos){
					if(mainstate==SETUP && setupstate==WAITINJMOVEOUT){
						setupstate=EXTRMOVEOUT;
					}

				}
				else if(data.find("EXTRALREADYOUT")!=std::string::npos){
					if(mainstate==SETUP && setupstate==WAITEXTRMOVEOUT){
						setupstate=LOWERCAVITYVOLTAGE;
					}

				}
				else if(data.find("EXTRALREADYIN")!=std::string::npos){
					if(mainstate==EXTRACT && extractstate==WAITEXTRMOVEIN){
						extractstate=SCREWROT1S;
					}

				}
				else if(data.find("INJALREADYIN")!=std::string::npos){
					if(mainstate==SETUP && injectionstate==WAITINJMOVEIN){
						injectionstate=INJECTIONFINISHED;
					}

				}

				else{
					Q_EMIT(newReadableMessage("Could not match error message in in ERR command"));
				}
			}
			else if(std::regex_match(data,drop_regex)){
				if(data.find("REQ")!=std::string::npos){
					if(mainstate==INJECTION && injectionstate==WAITDROPREQ){
						injectionstate=WAITDROPSUCCESS;
					}

				}
				else if(data.find("SUCCESS")!=std::string::npos){
					if(mainstate==INJECTION && injectionstate==WAITDROPSUCCESS){
						injectionstate=INJMOVEIN;
					}

				}
				else if(data.find("FAIL")!=std::string::npos){
					if(mainstate==INJECTION && injectionstate==WAITDROPSUCCESS){
						injectionstate=DROPREQ;
					}

				}
				else{
					Q_EMIT(newReadableMessage("Could not match error message in in ERR command"));
				}
			}
			else if(std::regex_match(data,screw_regex)){
				if(mainstate==EXTRACT && extractstate==WAITSCREWROT1S){
					extractstate=CHECKSCREWCOUNTER;
				}
			}
			else if(std::regex_match(data,button_regex)){
				if(automatic==1){
					Q_EMIT(newReadableMessage("Buttons are ignored when in automatic mode"));
				}
			}
			else if(std::regex_match(data,voltage_regex)){
				std::regex_search(data, number_match, float_regex);
				if(!number_match.empty() ){
					value=convertFloat(number_match[0]);
					int readVoltage=int(value);
					Q_EMIT(newEnergy((readVoltage-20)*5 ));
				}
				else{
					Q_EMIT(newReadableMessage("No voltage found in VACC command"));
				}

			}

			else{
				std::ostringstream temp;
				temp<<"could not parse command: "<<data;
				//std::cout<<std::hex<<temp.str()<<std::endl;
				Q_EMIT(newReadableMessage(QString::fromStdString(temp.str())));
			}
		}
		//the sequencer
		if(automatic==1){
			//long unsigned elapsed_millisecs = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - ticks_per_second).count();
			//ticks_per_second= std::chrono::system_clock::now();
		//	if (elapsed_millisecs!=0){
		//		std::cout<<elapsed_millisecs<<"ticks per second"<<std::endl;
		//	}
			switch(mainstate){
				case START:{
					setupstate=INJMOVEOUT;
					injectionstate=DROPREQ;
					acceleratestate=INITACC;
					extractstate=INITEXTRACT;
					Q_EMIT(newSequence("START"));
					Q_EMIT(newSubSequence(""));
					if (beamPresent==1){
						mainstate=EXTRACT;
					}
					else if(beamPresent==0){
						mainstate=SETUP;
					}
					else{
						Q_EMIT(newReadableMessage("AUTO mode but beam present is not known"));
					}
					break;
					}
				case SETUP:{
					Q_EMIT(newSequence("SETUP"));
					switch(setupstate){
						case INJMOVEOUT:{
							Q_EMIT(newHumanReadable("MOVING INJECTOR IN"));
							Q_EMIT(newSubSequence("INJMOVEOUT"));
							controller->writeData("INJMOVEOUT");
							setupstate=WAITINJMOVEOUT;
							break;
						}
						case WAITINJMOVEOUT:{
							Q_EMIT(newSubSequence("WAITINJMOVEOUT"));
							//wait for INJMOVEOUT or INJALREADYOUT command
							break;
						}
						case EXTRMOVEOUT:{
							Q_EMIT(newHumanReadable("MOVING EXTRACTOR OUT"));
							Q_EMIT(newSubSequence("EXTRMOVEOUT"));
							controller->writeData("EXTRMOVEOUT");
							setupstate=WAITEXTRMOVEOUT;
							break;
						}
						case WAITEXTRMOVEOUT:{
							Q_EMIT(newSubSequence("WAITEXTRMOVEOUT"));
							//wait for EXTRMOVEOUT command

							break;
						}
						case (LOWERCAVITYVOLTAGE):{
							Q_EMIT(newHumanReadable("LOWERING CAVITY VOLTAGE"));
							Q_EMIT(newSubSequence("LOWERCAVITYVOLTAGE"));
							Q_EMIT(newRampDownIsRunning(1));
							acceleration_start= std::chrono::system_clock::now();
							setupstate=WAITLOWERCAVITYVOLTAGE;
							std::ostringstream temp;
							temp<<"VOLTAGE:"<<20;
							controller->writeData(temp.str());
							break;
						}
						case (WAITLOWERCAVITYVOLTAGE):{
							Q_EMIT(newSubSequence("WAITLOWERCAVITYVOLTAGE"));
							long unsigned elapsed_secs = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - acceleration_start).count();
							//int percent=100-int(float(elapsed_secs)/150.0);
						//	Q_EMIT(newEnergy(percent));
							if(elapsed_secs>16000){
								setupstate=SETUPFINISHED;
							}
							/**if(elapsed_secs>16000){
								setupstate=SETUPFINISHED;
								Q_EMIT(newHumanReadable("RAMP DOWN TIMEOUT"));
								Q_EMIT(newReadableMessage("Ramp down took more than 17 seconds"));
								mainstate=START;
							}
							**/
							break;
						}
						case SETUPFINISHED:{
							Q_EMIT(newSubSequence("SETUPFINISHED"));
							Q_EMIT(newRampDownIsRunning(0));
							mainstate=INJECTION;
							break;
						}
					}
					break;
				}
				case (INJECTION):{
					Q_EMIT(newSequence("INJECTION"));
					switch(injectionstate){
						case DROPREQ:{
							Q_EMIT(newHumanReadable("TRYING TO INJECT"));
							Q_EMIT(newSubSequence("DROPREQ"));
							controller->writeData("DROPREQ");
							injectionstate=WAITDROPREQ;
						//	std::cout<<"sending DROPREQ"<<std::endl;
							break;
						}
						case WAITDROPREQ:{
							Q_EMIT(newSubSequence("WAITDROPREQ"));
						//	std::cout<<"waiting for DROPREQ"<<std::endl;
							break;
						}
						case WAITDROPSUCCESS:{
						//	std::cout<<"WAITDROPSUCCESS"<<std::endl;
							Q_EMIT(newSubSequence("WAITDROPSUCCESS"));
							break;
						}
						case INJMOVEIN:{
							Q_EMIT(newHumanReadable("MOVING INJECTOR OUT"));
							Q_EMIT(newSubSequence("INJMOVEIN"));
							controller->writeData("INJMOVEIN");
							injectionstate=WAITINJMOVEIN;
							break;
						}
						case WAITINJMOVEIN:{
							Q_EMIT(newSubSequence("WAITINJMOVEIN"));
							break;
						}
						case INJECTIONFINISHED:{
							Q_EMIT(newSubSequence("INJECTIONFINISHED"));
							mainstate=ACCELERATE;
							break;
						}
					}
					break;
				}
				case (ACCELERATE):{
					Q_EMIT(newSequence("ACCELERATE"));
					switch(acceleratestate){
						case(INITACC):{
							Q_EMIT(newHumanReadable("ACCELERATING BY INCREASING VOLTAGE IN CAVITY"));
							Q_EMIT(newSubSequence("INITACC"));
							voltage=20;
							acceleration_start= std::chrono::system_clock::now();
							acceleratestate=WAITACC;
							break;
						}
						case(WAITACC):{
							Q_EMIT(newSubSequence("WAITACC"));
							long unsigned elapsed_secs = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - acceleration_start).count();
							if(elapsed_secs>2000){
								acceleratestate=INCREASE;
							}
							break;
						}
						case(INCREASE):{
							Q_EMIT(newSubSequence("INCREASE"));
							voltage++;
							std::ostringstream temp;
							temp<<"VOLTAGE:"<<voltage;
							controller->writeData(temp.str());
							//Q_EMIT(newEnergy((voltage-20)*5 ));
							if(voltage>=40){
								acceleratestate=ACCELERATEFINISHED;
							}
							else{
								acceleratestate=WAITACC;
								acceleration_start= std::chrono::system_clock::now();
							}
							break;
						}
						case(ACCELERATEFINISHED):{
							Q_EMIT(newSubSequence("ACCELERATEFINISHED"));
							mainstate=FLATTOP;
							acceleration_start= std::chrono::system_clock::now();
							break;
						}


					} 


					break;
				}
				case(FLATTOP):{
					Q_EMIT(newHumanReadable("REACHED MAXIMUM ENERGY"));
					Q_EMIT(newSubSequence("FLATTOP"));
					Q_EMIT(newSequence("FLATTOP"));
					long unsigned elapsed_secs = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - acceleration_start).count();
					if(elapsed_secs>5000){
						mainstate=EXTRACT;
					}

					break;
				}
				case(EXTRACT):{
					Q_EMIT(newSequence("EXTRACT"));
					switch(extractstate){
						case(INITEXTRACT):{
							Q_EMIT(newSubSequence("INITEXTRACT"));
							extractstate=WAITACC2;
							break;
						}
						case(WAITACC2):{
							Q_EMIT(newSubSequence("WAITACC2"));
							acceleration_start= std::chrono::system_clock::now();
							//wait for ACC2 to trigger
							break;
						}
						case(EXTRMOVEIN):{
							long unsigned elapsed_secs = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - acceleration_start).count();
							if(elapsed_secs>200){
								Q_EMIT(newHumanReadable("MOVING EXTRACTOR IN"));
								Q_EMIT(newSubSequence("EXTRMOVEIN"));
								controller->writeData("EXTRMOVEIN");
								extractstate=WAITEXTRMOVEIN;
							}
							break;
						}
						case(WAITEXTRMOVEIN):{
							Q_EMIT(newSubSequence("WAITEXTRMOVEIN"));
							screwCounter=0;
							break;
						}
						case(SCREWROT1S):{
							Q_EMIT(newHumanReadable("TRANSFERRING BALLS TO INJECTOR"));
							Q_EMIT(newSubSequence("SCREWROT1S"));
							controller->writeData("SCREWROT1S");
							extractstate=WAITSCREWROT1S;
							screwCounter++;
							break;
						}
						case(WAITSCREWROT1S):{
							Q_EMIT(newSubSequence("WAITSCREWROT1S"));
							break;
						}
						case(CHECKSCREWCOUNTER):{
							Q_EMIT(newSubSequence("CHECKSCREWCOUNTER"));
							if(screwCounter<=30){
								extractstate=SCREWROT1S;
							}
							else{
								extractstate=EXTRACTIONFINISHED;
							}
							break;
						}
						case(EXTRACTIONFINISHED):{
							Q_EMIT(newSubSequence("EXTRACTIONFINISHED"));
							mainstate=START;
							break;
						}


					}

					break;
				}
				default:
					Q_EMIT(newReadableMessage("State not defined for MainState"));
					break;

			}

		}
		else{
			//Q_EMIT(newSequence("MANUAL"));
			//Q_EMIT(newSubSequence("MANUAL"));
			//Q_EMIT(newHumanReadable("MANUAL MODE"));
			Q_EMIT(newRampDownIsRunning(0));
			mainstate=START;
			setupstate=INJMOVEOUT;
			injectionstate=DROPREQ;
			acceleratestate=INITACC;
			extractstate=INITEXTRACT;

		}

	}
}