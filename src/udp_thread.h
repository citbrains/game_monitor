#ifndef UDP_THREAD_H
#define UDP_THREAD_H

#include <QtGui>
#include <QUdpSocket>
#include <QtCore>

#include "pos_types.h"

static const int NUM_PLAYERS = 6;
static const int COMM_INFO_PORT = 7110;
static const int MAX_COMM_INFO_OBJ = 7;
static const int MAX_STRING = 74-32;
static const int MAX_BEHAVIOR_STRING = 32;

static const unsigned char COMM_EXIST = 0x80;
static const unsigned char COMM_OUR_SIDE = 0x40;
static const unsigned char COMM_OPPOSITE_SIDE = 0x20;
static const unsigned char COMM_NOT_EXIST = 0x00;

static const int MAX_BLACK_POLES = 6;
static const int MAX_YELLOW_POLES = 1;
static const int MAX_BLUE_POLES = 1;
static const int MAX_MAGENTA_OBJECTS = 3;
static const int MAX_CYAN_OBJECTS = 3;

static const int MAGENTA = 0;
static const int CYAN    = 1;

struct comm_info_T {
	unsigned char id;
	unsigned char cf_own;
	unsigned char cf_ball;
	unsigned char object[MAX_COMM_INFO_OBJ][4];
	unsigned char status;
	unsigned char fps;
	unsigned char voltage;
	unsigned char temperature;
	unsigned char hishest_servo;
	unsigned char command[MAX_STRING];
	unsigned char behavior_name[MAX_BEHAVIOR_STRING];
};

Q_DECLARE_METATYPE(comm_info_T);

class UdpServer : public QObject
{
	Q_OBJECT
public:
	UdpServer(int);
	~UdpServer();
private:
	struct comm_info_T comm_info;
	QUdpSocket *udpSocket;
private slots:
	void readPendingDatagrams(void);
signals:
	void receiveData(struct comm_info_T);
};

bool getCommInfoObject(unsigned char *, Object *);

#endif // UDP_THREAD_H

