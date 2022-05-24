#include <cmath>
#include <iostream>

#include "new_udp_thread.h"

inline static double radian(double degree)
{
	return degree * M_PI / 180.0;
}

UdpServer::UdpServer(int port_num)
{
	std::cout << "UdpServer : " << __FUNCTION__ << std::endl;
	udpSocket = new QUdpSocket(this);
	udpSocket->bind(QHostAddress::Any, port_num);
	connect(udpSocket, SIGNAL(readyRead()), this, SLOT(readPendingDatagrams()));
}

void UdpServer::readPendingDatagrams(void)
{
	std::cout << "UdpServer : " << __FUNCTION__ << std::endl;
	while(udpSocket->hasPendingDatagrams()) {
		std::cout << "UdpServer : " << __FUNCTION__ << " while in" << std::endl;
		QByteArray datagrams;
		datagrams.resize(udpSocket->pendingDatagramSize());
		QHostAddress sender;
		quint16 senderPort;
		udpSocket->readDatagram(datagrams.data(), datagrams.size(), &sender, &senderPort);
		//char *buf = datagrams.data();
		//char *p = (char *)&comm_info;
		//infoshare_.parseOtherRobotInfomationFromString(std::string(buf));//この関数を使えばおそらく受け取ったデータをパースまでしてくれる
		/*
		for(size_t i = 0; (i < (unsigned)datagrams.size()) && (i < sizeof(struct comm_info_T)); i++) {
			*p++ = buf[i];
		}
		*/
		comm_info = infoshare_.parseOtherRobotInfomationFromString(std::string(datagrams.data(), datagrams.size()));
		emit receiveData(comm_info);
	}
}

UdpServer::~UdpServer()
{
}


