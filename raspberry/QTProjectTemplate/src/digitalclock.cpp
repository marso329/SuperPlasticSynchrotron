#include "digitalclock.h"

DigitalClock::DigitalClock(QWidget *parent)
    : QLCDNumber(parent)
{
    setSegmentStyle(Filled);

    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &DigitalClock::showTime);
    timer->start(1000);

    showTime();

    setWindowTitle(tr("Digital Clock"));
    resize(150, 60);
}

void DigitalClock::showTime()
{
	seconds++;
    QTime time = QTime(0,0,0);
    time=time.addSecs(seconds);
    QString text = time.toString("mm:ss");
    if ((time.second() % 2) == 0)
        text[2] = ' ';
    display(text);
}