#include "mainwindow.h"

MainWindow::MainWindow(QWidget* parent) :
	QMainWindow(parent), ui(new Ui::MainWindow) {
	ui->setupUi(this);
	SerialThread=new SPSThread();

//----------------------setup title----------------------------
	{
		scene = new QGraphicsScene(this);
		QImage image;
		image.load(":images/title.png");
		QPixmap pixmap = QPixmap::fromImage(image);
		QGraphicsPixmapItem* item = new QGraphicsPixmapItem(pixmap);
		//item->setScale(0.1);
		//item->setFlags(
	//			QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable);
		item->setPos(scene->sceneRect().width() / 2.0,
				scene->sceneRect().height() / 2.0);
		scene->addItem(item);

		ui->title->setScene(scene);
		ui->title->setAlignment(Qt::AlignLeft | Qt::AlignTop);
		ui->title->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		ui->title->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	}
//--------------------------------------------

//----------------------setup overview----------------------------
	{
		sceneOverview = new QGraphicsScene(this);
		QImage image;
		image.load(":images/background.png");
		QPixmap pixmap = QPixmap::fromImage(image);
		QGraphicsPixmapItem* item = new QGraphicsPixmapItem(pixmap);
	//	item->setFlags(
	//			QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable);
		item->setPos(sceneOverview->sceneRect().width() / 2.0,
				sceneOverview->sceneRect().height() / 2.0);
		sceneOverview->addItem(item);

		ui->overview->setScene(sceneOverview);
		ui->overview->setAlignment(Qt::AlignLeft | Qt::AlignTop);
		ui->overview->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		ui->overview->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

		QGraphicsRectItem* item1 = new QGraphicsRectItem(267,11,40,50);
		item1->setBrush(QBrush(displayed_color ));
		sceneOverview->addItem(item1);
		rectangles["ACC2"]=item1;

		item1 = new QGraphicsRectItem(313,20,32,35);
		item1->setBrush(QBrush(displayed_color ));
		sceneOverview->addItem(item1);
		rectangles["BMP2"]=item1;

		item1 = new QGraphicsRectItem(268,513,41,51);
		item1->setBrush(QBrush(displayed_color ));
		sceneOverview->addItem(item1);
		rectangles["ACC1"]=item1;

		item1 = new QGraphicsRectItem(234,521,32,35);
		item1->setBrush(QBrush(displayed_color ));
		sceneOverview->addItem(item1);
		rectangles["BMP1"]=item1;

		item1 = new QGraphicsRectItem(209,43,8,18);
		item1->setBrush(QBrush(displayed_color ));
		sceneOverview->addItem(item1);
		rectangles["DUMP"]=item1;

		item1 = new QGraphicsRectItem(522,272,35,32);
		item1->setBrush(QBrush(displayed_color ));
		sceneOverview->addItem(item1);
		rectangles["BMP3"]=item1;

		item1 = new QGraphicsRectItem(474,314,56,46);
		item1->setBrush(QBrush(displayed_color ));
		sceneOverview->addItem(item1);
		rectangles["INJ"]=item1;

		item1 = new QGraphicsRectItem(447,280,48,35);
		item1->setBrush(QBrush(displayed_color ));
		sceneOverview->addItem(item1);
		rectangles["SCREW"]=item1;
	}
//--------------------------------------------


QFont f( "Arial", 16, QFont::Bold);
ui->injections_label->setFont( f);
ui->turns_label->setFont( f);
ui->time_label->setFont( f);

//-------------------font of labels----------------

///
QFont f_label( "Arial", 11, QFont::Bold);
//---------------------setup plotting-------------------
// ui->plotCurrent->addGraph(0); // blue line
// ui->plotCurrent->graph(0)->setPen(QPen(QColor(255, 0, 0)));
// QSharedPointer<QCPAxisTickerTime> timeTicker0(new QCPAxisTickerTime);
// timeTicker0->setTimeFormat("%h:%m:%s");
// ui->plotCurrent->xAxis->setTicker(timeTicker0);
// ui->plotCurrent->axisRect()->setupFullAxesBox();
// ui->plotCurrent->yAxis->setRange(-1.2, 1.2);
// connect(ui->plotCurrent->xAxis, SIGNAL(rangeChanged(QCPRange)), ui->plotCurrent->xAxis2, SLOT(setRange(QCPRange)));
// connect(ui->plotCurrent->yAxis, SIGNAL(rangeChanged(QCPRange)), ui->plotCurrent->yAxis2, SLOT(setRange(QCPRange)));
// ui->plotCurrent->yAxis->setLabel("Voltage [V]");
// ui->plotCurrent->yAxis->setLabelFont(f_label);

ui->plotSpeed->addGraph(); // red line
ui->plotSpeed->addGraph(); // red line
ui->plotSpeed->addGraph(); // red line
ui->plotSpeed->graph(0)->setPen(QPen(QColor(255, 0, 0)));
ui->plotSpeed->graph(1)->setPen(QPen(QColor(0, 255, 0)));
ui->plotSpeed->graph(2)->setPen(QPen(QColor(0, 0, 255)));	
QSharedPointer<QCPAxisTickerTime> timeTicker1(new QCPAxisTickerTime);
timeTicker1->setTimeFormat("%h:%m:%s");
ui->plotSpeed->xAxis->setTicker(timeTicker1);
ui->plotSpeed->axisRect()->setupFullAxesBox();
ui->plotSpeed->yAxis->setRange(0, 5);
connect(ui->plotSpeed->xAxis, SIGNAL(rangeChanged(QCPRange)), ui->plotSpeed->xAxis2, SLOT(setRange(QCPRange)));
connect(ui->plotSpeed->yAxis, SIGNAL(rangeChanged(QCPRange)), ui->plotSpeed->yAxis2, SLOT(setRange(QCPRange)));
ui->plotSpeed->yAxis->setLabel("Speed [m/s]");
ui->plotSpeed->yAxis->setLabelFont(f_label);

ui->plotRF->addGraph(); // red line
ui->plotRF->addGraph(); // red line
ui->plotRF->addGraph(); // red line
ui->plotRF->graph(0)->setPen(QPen(QColor(255, 0, 0)));
ui->plotRF->graph(1)->setPen(QPen(QColor(0, 255, 0)));
ui->plotRF->graph(2)->setPen(QPen(QColor(0, 0, 255)));	
QSharedPointer<QCPAxisTickerTime> timeTicker2(new QCPAxisTickerTime);
timeTicker2->setTimeFormat("%h:%m:%s");
ui->plotRF->xAxis->setTicker(timeTicker2);
ui->plotRF->axisRect()->setupFullAxesBox();
ui->plotRF->yAxis->setRange(-1.2, 1.2);
connect(ui->plotRF->xAxis, SIGNAL(rangeChanged(QCPRange)), ui->plotRF->xAxis2, SLOT(setRange(QCPRange)));
connect(ui->plotRF->yAxis, SIGNAL(rangeChanged(QCPRange)), ui->plotRF->yAxis2, SLOT(setRange(QCPRange)));
ui->plotRF->yAxis->setLabel("Revolution frequency [Hz]");
ui->plotRF->yAxis->setLabelFont(f_label);

losses = new QCPBars(ui->plotLosses->xAxis, ui->plotLosses->yAxis);
losses->setAntialiased(false);
losses->setPen(QPen(QColor(0, 51, 160)));
losses->setBrush(QColor(0, 51, 160));
QVector<double> ticks;
QVector<QString> labels;
ticks << 1 << 2 << 3 ;
labels << "Sector 0" << "Sector 1" << "Sector 2";
QSharedPointer<QCPAxisTickerText> textTicker(new QCPAxisTickerText);
textTicker->addTicks(ticks, labels);
ui->plotLosses->xAxis->setTicker(textTicker);
ui->plotLosses->xAxis->setTickLabelRotation(60);
// ui->plotLosses->xAxis->setSubTicks(false);
// ui->plotLosses->xAxis->setTickLength(0, 4);
 ui->plotLosses->xAxis->setRange(0, 4);
// ui->plotLosses->xAxis->setBasePen(QPen(Qt::white));
// ui->plotLosses->xAxis->setTickPen(QPen(Qt::white));
// ui->plotLosses->xAxis->grid()->setVisible(true);
// ui->plotLosses->xAxis->grid()->setPen(QPen(QColor(130, 130, 130), 0, Qt::DotLine));
// ui->plotLosses->xAxis->setTickLabelColor(Qt::white);
// ui->plotLosses->xAxis->setLabelColor(Qt::white);
 
ui->plotLosses->yAxis->setRange(-12.1, 12.1);
ui->plotLosses->yAxis->setPadding(5); // a bit more space to the left border
ui->plotLosses->yAxis->setLabel("Losses [J]");
// ui->plotLosses->yAxis->setBasePen(QPen(Qt::white));
// ui->plotLosses->yAxis->setTickPen(QPen(Qt::white));
// ui->plotLosses->yAxis->setSubTickPen(QPen(Qt::white));
// ui->plotLosses->yAxis->grid()->setSubGridVisible(true);
// ui->plotLosses->yAxis->setTickLabelColor(Qt::white);
// ui->plotLosses->yAxis->setLabelColor(Qt::white);
// ui->plotLosses->yAxis->grid()->setPen(QPen(QColor(130, 130, 130), 0, Qt::SolidLine));
// ui->plotLosses->yAxis->grid()->setSubGridPen(QPen(QColor(130, 130, 130), 0, Qt::DotLine));
ui->plotRF->yAxis->setLabelFont(f_label);
QVector<double> fossilData;
fossilData  << 0.86*10.5 << 0.83*5.5 << 0.84*5.5;
losses->setData(ticks, fossilData);
ui->plotLosses->yAxis->setLabelFont(f_label);

connect(SerialThread, SIGNAL(newSpeed1(float)), this, SLOT(newSpeed1(float)));
connect(SerialThread, SIGNAL(newSpeed2(float)), this, SLOT(newSpeed2(float)));
connect(SerialThread, SIGNAL(newSpeed3(float)), this, SLOT(newSpeed3(float)));

connect(SerialThread, SIGNAL(newRF1(float)), this, SLOT(newRF1(float)));
connect(SerialThread, SIGNAL(newRF2(float)), this, SLOT(newRF2(float)));
connect(SerialThread, SIGNAL(newRF3(float)), this, SLOT(newRF3(float)));

connect(SerialThread, SIGNAL(newTurns(int)), this, SLOT(newTurns(int)));
connect(SerialThread, SIGNAL(newInjections(int)), this, SLOT(newInjections(int)));

connect(SerialThread, SIGNAL(newAction(QString)), this, SLOT(newAction(QString)));
connect(SerialThread, SIGNAL(newMode(int)), this, SLOT(newMode(int)));
connect(SerialThread, SIGNAL(newBeamPresent(int)), this, SLOT(newBeamPresent(int)));

connect(SerialThread, SIGNAL(newSequence(QString)), this, SLOT(newSequence(QString)));
connect(SerialThread, SIGNAL(newSubSequence(QString)), this, SLOT(newSubSequence(QString)));

connect(SerialThread, SIGNAL(newEnergy(int)), this, SLOT(newEnergy(int)));
connect(SerialThread, SIGNAL(newHumanReadable(QString)), this, SLOT(newHumanReadable(QString)));
connect(SerialThread, SIGNAL(newReadyForInjection(int)), this, SLOT(newReadyForInjection(int)));
connect(SerialThread, SIGNAL(newRampDownIsRunning(int)), this, SLOT(newRampDownIsRunning(int)));



//-----------------setup serial thread---------------------
	connect(SerialThread, SIGNAL(newMessage(QString)), this, SLOT(newMessage(QString)));
	connect(SerialThread, SIGNAL(newReadableMessage(QString)), this, SLOT(newReadableMessage(QString)));
	SerialThread->start();
//------------------------------------------------------------

newHumanReadable("Hello CERN OD2019 visitors");



}


MainWindow::~MainWindow() {

}

void MainWindow::recalculateLosses(){
	float mass=0.001436755f;
	float sector0=mass*pow(speeds[1]-speeds[0],2.0);
	float sector1=mass*pow(speeds[2]-speeds[1],2.0);
	float sector2=mass*pow(speeds[0]-speeds[2],2.0);
	QVector<double> ticks;
	ticks << 1 << 2 << 3 ;
	QVector<double> fossilData;
	fossilData  << sector0 << sector1 << sector2 ;
	losses->setData(ticks, fossilData);
	ui->plotLosses->yAxis->rescale();
	ui->plotLosses->replot();

}

void MainWindow::newReadyForInjection(int data){
	if(data==0){
		ui->button_readyForinjection->setStyleSheet("border-style: solid;border-color: black;border-width: 2px;border-radius: 0px;background-color: red;");
	}
	else if(data==1){
		ui->button_readyForinjection->setStyleSheet("border-style: solid;border-color: black;border-width: 2px;border-radius: 0px;background-color: green;");
		newReadableMessage("Ready to inject");
	}
	else{
		newReadableMessage("MODE undefined in newReadyForInjection(int)");
	}

}

void MainWindow::newRampDownIsRunning(int data){
	if(data==0){
		ui->button_rampDown->setStyleSheet("border-style: solid;border-color: black;border-width: 2px;border-radius: 0px;background-color: red;");
	}
	else if(data==1){
		ui->button_rampDown->setStyleSheet("border-style: solid;border-color: black;border-width: 2px;border-radius: 0px;background-color: green;");
		newReadableMessage("Rampdown is running");
	}
	else{
		newReadableMessage("MODE undefined in newReadyForInjection(int)");
	}
}

void MainWindow::newEnergy(int data){
	std::cout<<"new energy "<< data<<std::endl;
	ui->progressBar_energy->setValue(data);
}

void MainWindow::newSequence(QString data){
	ui->lineEdit_sequence->clear();
	ui->lineEdit_sequence->insert(data);
}
void MainWindow::newSubSequence(QString data){
	ui->lineEdit_subsequence->clear();
	ui->lineEdit_subsequence->insert(data);
}

void MainWindow::newHumanReadable(QString data){
	QFont f( "Arial", 35, QFont::Bold);
	ui->textEdit_humanreadable->clear();
	ui->textEdit_humanreadable->setCurrentFont(f);
	ui->textEdit_humanreadable->insertPlainText(data);
}

void MainWindow::newMode(int data){
	if(data==0){
		ui->button_auto->setStyleSheet("border-style: solid;border-color: black;border-width: 2px;border-radius: 0px;background-color: red;");
		ui->button_manual->setStyleSheet("border-style: solid;border-color: black;border-width: 2px;border-radius: 0px;background-color: green;");
		newReadableMessage("Switched mode to MANUAL");
	}
	else if(data==1){
		ui->button_auto->setStyleSheet("border-style: solid;border-color: black;border-width: 2px;border-radius: 0px;background-color: green;");
		ui->button_manual->setStyleSheet("border-style: solid;border-color: black;border-width: 2px;border-radius: 0px;background-color: red;");
		newReadableMessage("Switched mode to AUTO");
	}
	else{
		newReadableMessage("MODE undefined in newMode(int)");
	}

}

void MainWindow::newBeamPresent(int data){
		if(data==0){
		ui->button_beamPresent->setStyleSheet("border-style: solid;border-color: black;border-width: 2px;border-radius: 0px;background-color: red;");
	}
	else if(data==1){
		ui->button_beamPresent->setStyleSheet("border-style: solid;border-color: black;border-width: 2px;border-radius: 0px;background-color: green;");
	}
	else{
		newReadableMessage("MODE undefined in newBeamPresent(int)");
	}
}

void MainWindow::newAction(QString data){
	std::string data_std=data.toStdString();
	if(rectangles.find(data_std)==rectangles.end()){
		newReadableMessage("Action key not found in map");
		return;
	}
	for(auto it :rectangles){
		it.second->setBrush(QBrush(hidden_color ));
	}

	rectangles[data_std]->setBrush(QBrush(displayed_color ));
	sceneOverview->update();

}

void MainWindow::newSpeed1(float data){
static QTime time(QTime::currentTime());
double key = time.elapsed()/1000.0;
speeds[0]=data;
  //ui->plotSpeed->graph(0)->addData(key, data);
  speeds1.push_back(data);
  speeds1_key.push_back(key);
  if(speeds1.size()>MAX_SAMPLES){
  	speeds1.remove(0);
  	speeds1_key.remove(0);
  }
ui->plotSpeed->graph(0)->setData(speeds1_key, speeds1);

ui->plotSpeed->xAxis->rescale();
ui->plotSpeed->yAxis->rescale();
ui->plotSpeed->replot();
recalculateLosses();
}
void MainWindow::newSpeed2(float data){
static QTime time(QTime::currentTime());
double key = time.elapsed()/1000.0;
 speeds[1]=data;
  speeds2.push_back(data);
  speeds2_key.push_back(key);
  if(speeds2.size()>MAX_SAMPLES){
  	speeds2.remove(0);
  	speeds2_key.remove(0);
  }
ui->plotSpeed->graph(1)->setData(speeds2_key, speeds2);

ui->plotSpeed->xAxis->rescale();
ui->plotSpeed->yAxis->rescale();
ui->plotSpeed->replot();
recalculateLosses();
}

void MainWindow::newSpeed3(float data){
static QTime time(QTime::currentTime());
double key = time.elapsed()/1000.0;
  speeds[2]=data;
  speeds3.push_back(data);
  speeds3_key.push_back(key);
  if(speeds3.size()>MAX_SAMPLES){
  	speeds3.remove(0);
  	speeds3_key.remove(0);
  }
ui->plotSpeed->graph(2)->setData(speeds3_key, speeds3);

ui->plotSpeed->xAxis->rescale();
ui->plotSpeed->yAxis->rescale();
ui->plotSpeed->replot();
recalculateLosses();
}


void MainWindow::newRF1(float data){
static QTime time(QTime::currentTime());
double key = time.elapsed()/1000.0;
  frev1.push_back(data);
  frev1_key.push_back(key);
  if(frev1.size()>MAX_SAMPLES){
  	frev1.remove(0);
  	frev1_key.remove(0);
  }
ui->plotRF->graph(0)->setData(frev1_key, frev1);

ui->plotRF->xAxis->rescale();
ui->plotRF->yAxis->rescale();
ui->plotRF->replot();
}

void MainWindow::newRF2(float data){
static QTime time(QTime::currentTime());
double key = time.elapsed()/1000.0;
  frev2.push_back(data);
  frev2_key.push_back(key);
  if(frev2.size()>MAX_SAMPLES){
  	frev2.remove(0);
  	frev2_key.remove(0);
  }
ui->plotRF->graph(1)->setData(frev2_key, frev2);

ui->plotRF->xAxis->rescale();
ui->plotRF->yAxis->rescale();
ui->plotRF->replot();
}

void MainWindow::newRF3(float data){
static QTime time(QTime::currentTime());
double key = time.elapsed()/1000.0;
  frev3.push_back(data);
  frev3_key.push_back(key);
  if(frev3.size()>MAX_SAMPLES){
  	frev3.remove(0);
  	frev3_key.remove(0);
  }
ui->plotRF->graph(2)->setData(frev3_key, frev3);

ui->plotRF->xAxis->rescale();
ui->plotRF->yAxis->rescale();
ui->plotRF->replot();
}

void MainWindow::newTurns(int data){
	if(data==1){
		ui->lcd_time->reset();
		speeds1.clear();
		speeds2.clear();
		speeds3.clear();
		speeds1_key.clear();
		speeds2_key.clear();
		speeds3_key.clear();
		frev1.clear();
		frev2.clear();
		frev3.clear();
		frev1_key.clear();
		frev2_key.clear();
		frev3_key.clear();

	}
	ui->lcd_turns->display(data);
}
void MainWindow::newInjections(int data){
	ui->lcd_injections->display(data);

}
