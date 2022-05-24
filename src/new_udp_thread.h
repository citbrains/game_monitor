#ifndef UDP_THREAD_H
#define UDP_THREAD_H

#include <QtGui>
#include <QUdpSocket>
#include <QtCore>

#include "hpl_types.h"
#include "infoshare.h"


Q_DECLARE_METATYPE(Citbrains::infosharemodule::OtherRobotInfomation);
//Q_DECLARE_METATYPE(OtherRobotInfomation);

class UdpServer : public QObject
{
	Q_OBJECT
public:
	UdpServer(int);
	~UdpServer();

	//infoshareFunc();
private:
	struct Citbrains::infosharemodule::OtherRobotInfomation comm_info;
	//struct comm_info_T comm_info;
	QUdpSocket *udpSocket;
    Citbrains::infosharemodule::InfoShare infoshare_;
private slots:
	void readPendingDatagrams(void);
signals:
	void receiveData(struct Citbrains::infosharemodule::OtherRobotInfomation/*struct comm_info_T*/);
};

//bool getCommInfoObject(unsigned char *, Object *);

#endif // UDP_THREAD_H

