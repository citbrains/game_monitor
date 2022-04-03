#include <cmath>
#include <iostream>

#include "udp_thread.h"

inline static double radian(double degree)
{
	return degree * M_PI / 180.0;
}

UdpServer::UdpServer(int port_num)
{
	udpSocket = new QUdpSocket(this);
	udpSocket->bind(QHostAddress::Any, port_num);
	connect(udpSocket, SIGNAL(readyRead()), this, SLOT(readPendingDatagrams()));
}

void UdpServer::readPendingDatagrams(void)
{
	while(udpSocket->hasPendingDatagrams()) {
		QByteArray datagrams;
		datagrams.resize(udpSocket->pendingDatagramSize());
		QHostAddress sender;
		quint16 senderPort;
		udpSocket->readDatagram(datagrams.data(), datagrams.size(), &sender, &senderPort);
		char *buf = datagrams.data();
		char *p = (char *)&comm_info;
		for(size_t i = 0; (i < (unsigned)datagrams.size()) && (i < sizeof(struct comm_info_T)); i++) {
			*p++ = buf[i];
		}
		emit receiveData(comm_info);
	}
}

UdpServer::~UdpServer()
{
}

bool getCommInfoObject(unsigned char *data, Object *obj)
{
	bool data_exist = false;

	/* calculate position of object */
	if(data[0] & COMM_EXIST) {
		data_exist = true;
		int x = ((data[0] & 0x0f) << 6) + ((data[1] & 0xfc) >> 2);
		if((x & 0x0200) != 0) x = -(0x0400 - x);
		x *= 10;
		int y = ((data[1] & 0x03) << 8) + data[2];
		if((y & 0x0200) != 0) y = -(0x0400 - y);
		y *= 10;
		const int theta = (data[3] * 2) - 180;
		const float th = (float)radian(theta);
		bool is_our_side = ((data[0] & COMM_OUR_SIDE) != 0) ? true : false;
		bool is_opposite_side = ((data[0] & COMM_OPPOSITE_SIDE) != 0) ? true : false;
		if(is_our_side && is_opposite_side) obj->type = BALL;
		if(is_our_side && !is_opposite_side) obj->type = SELF_POS;
		if(!is_our_side && is_opposite_side) obj->type = ENEMY;
		if(!is_our_side && !is_opposite_side) obj->type = GOAL_POLE;
		obj->pos.x = x;
		obj->pos.y = y;
		obj->pos.th = th;
	}
	return data_exist;
}

