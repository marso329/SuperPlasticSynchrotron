
#ifndef MAINWINDOW_
#define MAINWINDOW_
#include "ui_mainwindow.h"
#include "serialController.h"
#include "SPSThread.h"
#include <QThread>
#include <QString>
#include <iostream>
#include <sstream>
#include <cmath>
#include <map>

#define MAX_SAMPLES 32

class MainWindow: public QMainWindow {
	Q_OBJECT
public:
	MainWindow(QWidget* parent = 0);
	~MainWindow();
private:
	Ui::MainWindow* ui;
		QGraphicsScene* scene;
		QGraphicsScene* sceneOverview;
		SPSThread* SerialThread=NULL;
		QCPBars *losses=NULL;
		float speeds[3]={0.0f,0.0f,0.0f};
		void recalculateLosses();
		QVector<double> speeds1;
		QVector<double> speeds1_key;
		QVector<double> speeds2;
		QVector<double> speeds2_key;
		QVector<double> speeds3;
		QVector<double> speeds3_key;
		QVector<double> frev1;
		QVector<double> frev1_key;
		QVector<double> frev2;
		QVector<double> frev2_key;
		QVector<double> frev3;
		QVector<double> frev3_key;

		QColor  hidden_color=QColor(255,0,0,0);
		QColor displayed_color=QColor(255,0,0,174);
		std::map<std::string,QGraphicsRectItem*> rectangles;

protected:
Q_SIGNALS:

public Q_SLOTS:
void newMessage(QString message){
	ui->logger->append(message);
	ui->logger->verticalScrollBar()->setValue(ui->logger->verticalScrollBar()->maximum());
}
void newReadableMessage(QString message){
	ui->logger_readable->append(message);
	ui->logger_readable->verticalScrollBar()->setValue(ui->logger_readable->verticalScrollBar()->maximum());
}
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

};
#endif
