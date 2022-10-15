#include <QLCDNumber>
#include <QObject>
#include <QTimer>
#include <QTime>


class DigitalClock : public QLCDNumber
{
    Q_OBJECT

public:
    DigitalClock(QWidget *parent = nullptr);
    void reset(){
    	seconds=0;
    }
protected:
	unsigned seconds=0;

private slots:
    void showTime();
};