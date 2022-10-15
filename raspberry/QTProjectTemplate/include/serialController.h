/*
 * temperature_controller.h
 *
 *  Created on: May 21, 2018
 *      Author: martin
 */

#ifndef INCLUDE_SERIAL_CONTROLLER_H_
#define INCLUDE_SERIAL_CONTROLLER_H_

#include <QObject>
#include <QTimer>

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <stdio.h>

#include <atomic>
#include <iostream>



class SerialController : public QObject  {
Q_OBJECT
public:
	SerialController(QObject* _parent = NULL);
	~SerialController();
	std::string readData();
	bool writeData(std::string);
private:
	int set_interface_attribs(int fd, int speed, int parity);
	void set_blocking(int fd, int should_block);
	bool enabled=true;
	int fd;
	public Q_SLOTS:
	//void update();

	Q_SIGNALS:
	//void updated();



};

#endif /* INCLUDE_SERIAL_CONTROLLER_H_ */
