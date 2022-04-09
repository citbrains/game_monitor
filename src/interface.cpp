#include <iostream>
#include <cstring>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <string>
#include <cstring>
#include <ctime>
#include <QLCDNumber>
#include <QtGui>

#include "pos_types.h"
#include "interface.h"

static inline int distance(const int x1, const int y1, const int x2, const int y2)
{
	const int x = x1 - x2;
	const int y = y1 - y2;
	return std::sqrt(x * x + y * y);
}

Interface::Interface(): fLogging(true), fReverse(false), fViewGoalpost(false), fViewRobotInformation(true), fPauseLog(false), fRecording(false), fViewSelfPosConf(true), score_team1(0), score_team2(0), max_robot_num(6), log_speed(1), field_param(FieldParameter()), field_space(1040, 740)
{
	qRegisterMetaType<comm_info_T>("comm_info_T");
	setAcceptDrops(true);
	log_writer.setEnable();
	positions = std::vector<PositionMarker>(max_robot_num);

	statusBar = new QStatusBar;
	statusBar->showMessage(QString("GameMonitor: Ready"));
	setStatusBar(statusBar);

	settings = new QSettings("./config.ini", QSettings::IniFormat);
	initializeConfig();
	logo_pos_x = field_param.field_length / 2 + field_param.field_length / 4;
	logo_pos_y = field_param.border_strip_width / 2;

	// Run receive thread
	const int base_udp_port = settings->value("network/port").toInt();
	th.push_back(new UdpServer(base_udp_port));
//	for(int i = 0; i < max_robot_num; i++)
//		th.push_back(new UdpServer(base_udp_port + i));

	constexpr int gc_receive_port = 3838;
	gc_thread = new GCReceiver(gc_receive_port);

	createWindow();
	createMenus();
	connection();

	updateMapTimerId = startTimer(1000); // timer by 1000msec
	drawField();

	this->setWindowTitle("Humanoid League Game Monitor");
}

Interface::~Interface()
{
}

void Interface::createMenus(void)
{
	fileMenu = menuBar()->addMenu(tr("&File"));

	loadLogFileAction = new QAction(tr("&Load Log File"), 0);
	settingsAction = new QAction(tr("&Size setting"), 0);

	fileMenu->addAction(loadLogFileAction);
	fileMenu->addAction(settingsAction);

	connect(loadLogFileAction, SIGNAL(triggered()), this, SLOT(loadLogFile(void)));
	connect(settingsAction, SIGNAL(triggered()), this, SLOT(openSettingWindow(void)));

	viewMenu = menuBar()->addMenu(tr("&View"));

	viewGoalPostAction = new QAction(tr("&View Goal Posts"), 0);
	viewGoalPostAction->setCheckable(true);
	viewGoalPostAction->setChecked(fViewGoalpost);
	viewRobotInformationAction = new QAction(tr("&View Detailed robot information"), 0);
	viewRobotInformationAction->setCheckable(true);
	viewRobotInformationAction->setChecked(fViewRobotInformation);
	viewSelfPosConfAction = new QAction(tr("&View Self Position confidence of the Robot"), 0);
	viewSelfPosConfAction->setCheckable(true);
	viewSelfPosConfAction->setChecked(fViewSelfPosConf);

	viewMenu->addAction(viewGoalPostAction);
	viewMenu->addAction(viewRobotInformationAction);
	viewMenu->addAction(viewSelfPosConfAction);
	viewMenu->addSeparator();

	connect(viewGoalPostAction, SIGNAL(toggled(bool)), this, SLOT(viewGoalpost(bool)));
	connect(viewRobotInformationAction, SIGNAL(toggled(bool)), this, SLOT(viewRobotInformation(bool)));
	connect(viewSelfPosConfAction, SIGNAL(toggled(bool)), this, SLOT(viewSelfPosConf(bool)));
}

void Interface::initializeConfig(void)
{
	const int field_w = field_param.border_strip_width * 2 + field_param.field_length;
	const int field_h = field_param.border_strip_width * 2 + field_param.field_width;
	// field figure size (pixel size of drawing area)
	settings->setValue("field_image/width" , settings->value("field_image/width", field_w));
	settings->setValue("field_image/height", settings->value("field_image/height", field_h));
	// field size is 9000x6000 millimeters (See rule book of 2018)
	// In this program, field dimensions are defined in centimeters.
	settings->setValue("field_size/x", settings->value("field_size/x", field_param.field_length * 10));
	settings->setValue("field_size/y", settings->value("field_size/y", field_param.field_width * 10));
	settings->setValue("field_size/line_width", settings->value("field_size/line_width", 5));
	// marker configurations
	settings->setValue("marker/pen_size", settings->value("marker/pen_size", 6));
	settings->setValue("marker/robot_size", settings->value("marker/robot_size", 15));
	settings->setValue("marker/ball_size", settings->value("marker/ball_size", 12));
	settings->setValue("marker/goal_pole_size", settings->value("marker/goal_pole_size", 5));
	settings->setValue("marker/direction_marker_length", settings->value("marker/direction_marker_length", 20));
	settings->setValue("marker/font_size", settings->value("marker/font_size", 24));
	settings->setValue("marker/font_offset_x", settings->value("marker/font_offset_x", 8));
	settings->setValue("marker/font_offset_y", settings->value("marker/font_offset_y", 24));
	settings->setValue("marker/time_up_limit", settings->value("marker/time_up_limit", 5));
	// size setting
	settings->setValue("size/font_size", settings->value("size/font_size", 48));
	settings->setValue("size/display_minimum_height", settings->value("size/display_minimum_height", 50));
	// using UDP communication port offset
	settings->setValue("network/port", settings->value("network/port", 3737));
}

void Interface::createWindow(void)
{
	window = new QWidget;
	reverse = new QCheckBox("Reverse field");
	image = new AspectRatioPixmapLabel;
	image->setAlignment(Qt::AlignTop | Qt::AlignRight);
	log_step = new QLabel;
	label_remaining_time = new QLabel("Time");
	label_secondary_time = new QLabel("Secondary time");
	label_game_state = new QLabel("Game state");
	label_score = new QLabel("Score (Blue - Red)");
	label_game_state_display = new QLabel("Initial");
	QFont font = label_game_state_display->font();
	const int font_size = settings->value("size/font_size").toInt();
	font.setPointSize(font_size);
	label_game_state_display->setFont(font);
	log_slider = new QSlider(Qt::Horizontal);
	log_slider->setRange(0, 0);
	time_display = new QLCDNumber();
	time_display->display(QString("10:00"));
	const int display_minimum_height = settings->value("size/display_minimum_height").toInt();
	time_display->setMinimumHeight(display_minimum_height);
	secondary_time_display = new QLCDNumber;
	secondary_time_display->display(QString(" 0:00"));
	secondary_time_display->setMinimumHeight(display_minimum_height);
	score_display = new QLCDNumber();
	score_display->display(QString("0 - 0"));
	score_display->setMinimumHeight(display_minimum_height);
	log1Button = new QPushButton("x1");
	log1Button->setEnabled(false);
	log2Button = new QPushButton("x2");
	log5Button = new QPushButton("x5");
	mainLayout = new QGridLayout;
	checkLayout = new QVBoxLayout;
	logLayout = new QHBoxLayout;
	logSpeedButtonLayout = new QHBoxLayout;

	checkLayout->addWidget(label_game_state);
	checkLayout->addWidget(label_game_state_display);
	checkLayout->addWidget(label_remaining_time);
	checkLayout->addWidget(time_display);
	checkLayout->addWidget(label_secondary_time);
	checkLayout->addWidget(secondary_time_display);
	checkLayout->addWidget(label_score);
	checkLayout->addWidget(score_display);
	checkLayout->addWidget(reverse);

	logLayout->addWidget(log_step);
	logLayout->addWidget(log_slider);

	logSpeedButtonLayout->addWidget(log1Button);
	logSpeedButtonLayout->addWidget(log2Button);
	logSpeedButtonLayout->addWidget(log5Button);

	pal_state_bgcolor.setColor(QPalette::Window, QColor("#D0D0D0"));
	pal_red.   setColor(QPalette::Window, QColor("#FF8E8E"));
	pal_green. setColor(QPalette::Window, QColor("#8EFF8E"));
	pal_blue.  setColor(QPalette::Window, QColor("#8E8EFF"));
	pal_black. setColor(QPalette::Window, QColor("#000000"));
	pal_orange.setColor(QPalette::Window, QColor("#FFA540"));

	mainLayout->addLayout(checkLayout, 1, 1);
	mainLayout->addWidget(image, 1, 2);
	mainLayout->addLayout(logSpeedButtonLayout, 2, 1);
	mainLayout->addLayout(logLayout, 2, 2);

	window->setLayout(mainLayout);
	setCentralWidget(window);
}

void Interface::drawField()
{
	const int field_w = settings->value("field_image/width").toInt();
	const int field_h = settings->value("field_image/height").toInt();
	origin_map = QPixmap(field_w, field_h);
	origin_map.fill(Qt::black);
	QPainter p;
	p.begin(&origin_map);
	const int line_width = settings->value("field_size/line_width").toInt();
	QPen pen(Qt::white, line_width);
	p.setPen(pen);
	// draw field lines, center circle and penalty marks
	const int field_left = field_param.border_strip_width;
	const int field_right = field_param.border_strip_width + field_param.field_length;
	const int field_top = field_param.border_strip_width;
	const int field_bottom = field_param.border_strip_width + field_param.field_width;
	p.drawLine(field_left, field_top, field_right, field_top);
	p.drawLine(field_left, field_top, field_left, field_bottom);
	p.drawLine(field_right, field_bottom, field_left, field_bottom);
	p.drawLine(field_right, field_top, field_right, field_bottom);
	const int center_line_pos = field_left + field_param.field_length / 2;
	p.drawLine(center_line_pos, field_top, center_line_pos, field_bottom);
	const int left_goal_x = field_left - field_param.goal_depth;
	const int right_goal_x = field_right + field_param.goal_depth;
	const int goal_top = field_param.field_width / 2 - field_param.goal_width / 2 + field_top;
	const int goal_bottom = goal_top + field_param.goal_width;
	p.drawLine(left_goal_x, goal_top, left_goal_x, goal_bottom);
	p.drawLine(left_goal_x, goal_top, field_left, goal_top);
	p.drawLine(left_goal_x, goal_bottom, field_left, goal_bottom);
	p.drawLine(right_goal_x, goal_top, right_goal_x, goal_bottom);
	p.drawLine(right_goal_x, goal_top, field_right, goal_top);
	p.drawLine(right_goal_x, goal_bottom, field_right, goal_bottom);
	const int left_goal_area_x = field_left + field_param.goal_area_length;
	const int right_goal_area_x = field_right - field_param.goal_area_length;
	const int goal_area_top = field_param.field_width / 2 - field_param.goal_area_width / 2 + field_top;
	const int goal_area_bottom = goal_area_top + field_param.goal_area_width;
	p.drawLine(left_goal_area_x, goal_area_top, left_goal_area_x, goal_area_bottom);
	p.drawLine(left_goal_area_x, goal_area_top, field_left, goal_area_top);
	p.drawLine(left_goal_area_x, goal_area_bottom, field_left, goal_area_bottom);
	p.drawLine(right_goal_area_x, goal_area_top, right_goal_area_x, goal_area_bottom);
	p.drawLine(right_goal_area_x, goal_area_top, field_right, goal_area_top);
	p.drawLine(right_goal_area_x, goal_area_bottom, field_right, goal_area_bottom);
	const int left_penalty_area_x = field_left + field_param.penalty_area_length;
	const int right_penalty_area_x = field_right - field_param.penalty_area_length;
	const int penalty_area_top = field_param.field_width / 2 - field_param.penalty_area_width / 2 + field_top;
	const int penalty_area_bottom = penalty_area_top + field_param.penalty_area_width;
	p.drawLine(left_penalty_area_x, penalty_area_top, left_penalty_area_x, penalty_area_bottom);
	p.drawLine(left_penalty_area_x, penalty_area_top, field_left, penalty_area_top);
	p.drawLine(left_penalty_area_x, penalty_area_bottom, field_left, penalty_area_bottom);
	p.drawLine(right_penalty_area_x, penalty_area_top, right_penalty_area_x, penalty_area_bottom);
	p.drawLine(right_penalty_area_x, penalty_area_top, field_right, penalty_area_top);
	p.drawLine(right_penalty_area_x, penalty_area_bottom, field_right, penalty_area_bottom);
	const int center_of_field_y = field_top + field_param.field_width / 2;
	const int &dia = field_param.center_circle_diameter;
	const int radius = dia / 2; // radius of center circle
	p.drawEllipse(center_line_pos - radius, center_of_field_y - radius, dia, dia);
	p.drawPoint(field_left + field_param.penalty_mark_distance, center_of_field_y);
	p.drawPoint(field_right - field_param.penalty_mark_distance, center_of_field_y);
	p.end();
	map = origin_map;
	image->setPixmap(map);
}

void Interface::dragEnterEvent(QDragEnterEvent *e)
{
	if(e->mimeData()->hasFormat("text/uri-list")) {
		e->acceptProposedAction();
	}
}

void Interface::dropEvent(QDropEvent *e)
{
	filenameDrag = e->mimeData()->urls().first().toLocalFile();
}

void Interface::connection(void)
{
	connect(th[0], SIGNAL(receiveData(struct comm_info_T)), this, SLOT(decodeData1(struct comm_info_T)));
//	connect(th[1], SIGNAL(receiveData(struct comm_info_T)), this, SLOT(decodeData2(struct comm_info_T)));
//	connect(th[2], SIGNAL(receiveData(struct comm_info_T)), this, SLOT(decodeData3(struct comm_info_T)));
//	connect(th[3], SIGNAL(receiveData(struct comm_info_T)), this, SLOT(decodeData4(struct comm_info_T)));
//	connect(th[4], SIGNAL(receiveData(struct comm_info_T)), this, SLOT(decodeData5(struct comm_info_T)));
//	connect(th[5], SIGNAL(receiveData(struct comm_info_T)), this, SLOT(decodeData6(struct comm_info_T)));
	connect(reverse, SIGNAL(stateChanged(int)), this, SLOT(reverseField(int)));
	connect(log1Button, SIGNAL(clicked(void)), this, SLOT(logSpeed1(void)));
	connect(log2Button, SIGNAL(clicked(void)), this, SLOT(logSpeed2(void)));
	connect(log5Button, SIGNAL(clicked(void)), this, SLOT(logSpeed5(void)));
	connect(log_slider, SIGNAL(sliderPressed(void)), this, SLOT(pausePlayingLog(void)));
	connect(log_slider, SIGNAL(sliderReleased(void)), this, SLOT(changeLogPosition(void)));
	connect(gc_thread, SIGNAL(gameStateChanged(int)), this, SLOT(setGameState(int)));
	connect(gc_thread, SIGNAL(remainingTimeChanged(int)), this, SLOT(setRemainingTime(int)));
	connect(gc_thread, SIGNAL(secondaryTimeChanged(int)), this, SLOT(setSecondaryTime(int)));
	connect(gc_thread, SIGNAL(scoreChanged1(int)), this, SLOT(setScore1(int)));
	connect(gc_thread, SIGNAL(scoreChanged2(int)), this, SLOT(setScore2(int)));
}

void Interface::decodeData1(struct comm_info_T comm_info)
{
	int id = (int)(comm_info.id & 0x7F);
	decodeUdp(comm_info, id - 1);
	statusBar->showMessage(QString("Receive data from Robot"));
}

void Interface::decodeData2(struct comm_info_T comm_info)
{
	decodeUdp(comm_info, 1);
	statusBar->showMessage(QString("Receive data from Robot 2"));
}

void Interface::decodeData3(struct comm_info_T comm_info)
{
	decodeUdp(comm_info, 2);
	statusBar->showMessage(QString("Receive data from Robot 3"));
}

void Interface::decodeData4(struct comm_info_T comm_info)
{
	decodeUdp(comm_info, 3);
	statusBar->showMessage(QString("Receive data from Robot 4"));
}

void Interface::decodeData5(struct comm_info_T comm_info)
{
	decodeUdp(comm_info, 4);
	statusBar->showMessage(QString("Receive data from Robot 5"));
}

void Interface::decodeData6(struct comm_info_T comm_info)
{
	decodeUdp(comm_info, 5);
	statusBar->showMessage(QString("Receive data from Robot 6"));
}

void Interface::decodeUdp(struct comm_info_T comm_info, int num)
{
	int color, id;

	// MAGENTA, CYAN
	color = (int)(comm_info.id & 0x80) >> 7;
	id    = (int)(comm_info.id & 0x7F);
	positions[num].colornum = color;

	// record time of receive data
	time_t timer;
	struct tm *local_time;
	timer = time(NULL);
	local_time = localtime(&timer);
	positions[num].lastReceiveTime = *local_time;

	// ID and Color
	QString color_str;
	if(color == MAGENTA)
		color_str = QString("MAGENTA");
	else
		color_str = QString("CYAN");
	color_str = color_str + QString(" ") + QString::number(id);
	//robot_data->name->setText(color_str);
	// Self-position confidence
	positions[num].self_conf = comm_info.cf_own;
	// Ball position confidence
	positions[num].ball_conf = comm_info.cf_ball;
	// Role and message
	if(strstr((const char *)comm_info.command, "Attacker")) {
		// Red
		strcpy(positions[num].color, "red");
	} else if(strstr((const char *)comm_info.command, "Neutral")) {
		// Green
		strcpy(positions[num].color, "green");
	} else if(strstr((const char *)comm_info.command, "Defender")) {
		// Blue
		strcpy(positions[num].color, "blue");
	} else if(strstr((const char *)comm_info.command, "Keeper")) {
		// Orange
		strcpy(positions[num].color, "gray");
	} else {
		// Black
		strcpy(positions[num].color, "black");
	}
	positions[num].message = std::string((char *)comm_info.command);
	positions[num].behavior_name = std::string((char *)comm_info.behavior_name);

	positions[num].enable_pos = false;
	positions[num].enable_ball = false;
	positions[num].enable_goal_pole[0] = false;
	positions[num].enable_goal_pole[1] = false;
	positions[num].enable_target_pos = false;
	int goal_pole_index = 0;
	for(int i = 0; i < MAX_COMM_INFO_OBJ; i++) {
		Object obj;
		bool exist = getCommInfoObject(comm_info.object[i], &obj);
		if(!exist) continue;
		if(obj.type == NONE) continue;
		if(obj.type == SELF_POS) {
			positions[num].pos = globalPosToImagePos(obj.pos);
			positions[num].enable_pos  = true;
		}
		if(obj.type == BALL) {
			positions[num].ball = globalPosToImagePos(obj.pos);
			positions[num].enable_ball = true;
		}
		if(obj.type == GOAL_POLE) {
			if(goal_pole_index >= 2) continue;
			positions[num].goal_pole[goal_pole_index] = globalPosToImagePos(obj.pos);
			positions[num].enable_goal_pole[goal_pole_index] = true;
			goal_pole_index++;
		}
		if(obj.type == ENEMY) {
			positions[num].target_pos = globalPosToImagePos(obj.pos);
			positions[num].enable_target_pos = true;
		}
	}
	updateMap();
	// Voltage
	const double voltage = (comm_info.voltage << 3) / 100.0;
	positions[num].voltage = voltage;
	positions[num].temperature = comm_info.temperature;
	log_writer.write(num + 1, color_str.toStdString().c_str(), (int)comm_info.fps, (double)voltage,
		(int)positions[num].pos.x, (int)positions[num].pos.y, (float)positions[num].pos.th,
		(int)positions[num].ball.x, (int)positions[num].ball.y,
		(int)positions[num].goal_pole[0].x, (int)positions[num].goal_pole[0].y,
		(int)positions[num].goal_pole[1].x, (int)positions[num].goal_pole[1].y,
		(const char *)comm_info.command, (const char *)comm_info.behavior_name, (int)comm_info.cf_own, (int)comm_info.cf_ball);
}

void Interface::setGameState(int game_state)
{
	QString state_str;
	if(game_state == STATE_INITIAL) {
		state_str = "Initial";
	} else if(game_state == STATE_READY) {
		state_str = "Ready";
	} else if(game_state == STATE_SET) {
		state_str = "Set";
	} else if(game_state == STATE_PLAYING) {
		state_str = "Playing";
	} else if(game_state == STATE_FINISHED) {
		state_str = "Finished";
	} else {
		state_str = "Impossible";
	}
	label_game_state_display->setText(state_str);
	log_writer.writeGameState(game_state);
}

void Interface::setRemainingTime(int remaining_time)
{
	bool f_negative_number = false;
	if(remaining_time < 0) {
		remaining_time = -remaining_time;
		f_negative_number = true;
	}
	const int remain_minutes = remaining_time / 60;
	const int remain_seconds = remaining_time % 60;
	QString remain_minutes_str, remain_seconds_str;
	remain_minutes_str.setNum(remain_minutes);
	remain_seconds_str.setNum(remain_seconds);
	QString time_str;
	if(f_negative_number)
		time_str = "-";
	if(remain_seconds < 10)
		time_str = time_str + remain_minutes_str + QString(":0") + remain_seconds_str;
	else
		time_str = time_str + remain_minutes_str + QString(":") + remain_seconds_str;
	time_display->display(time_str);
	log_writer.writeRemainingTime(remaining_time);
}

void Interface::setSecondaryTime(int secondary_time)
{
	bool f_negative_number = false;
	if(secondary_time < 0) {
		secondary_time = -secondary_time;
		f_negative_number = true;
	}
	const int secondary_minutes = secondary_time / 60;
	const int secondary_seconds = secondary_time % 60;
	QString secondary_minutes_str, secondary_seconds_str;
	secondary_minutes_str.setNum(secondary_minutes);
	secondary_seconds_str.setNum(secondary_seconds);
	QString time_str;
	if(f_negative_number)
		time_str = "-";
	if(secondary_seconds < 10)
		time_str = time_str + secondary_minutes_str + QString(":0") + secondary_seconds_str;
	else
		time_str = time_str + secondary_minutes_str + QString(":") + secondary_seconds_str;
	secondary_time_display->display(time_str);
	log_writer.writeSecondaryTime(secondary_time);
}

void Interface::setScore1(int score1)
{
	score_team1 = score1;
	QString score1_str, score2_str;
	score1_str.setNum(score_team1);
	score2_str.setNum(score_team2);
	QString score_str = score1_str + QString(" - ") + score2_str;
	score_display->display(score_str);
	constexpr int team_no = 0;
	log_writer.writeScore(team_no, score1);
}

void Interface::setScore2(int score2)
{
	score_team2 = score2;
	QString score1_str, score2_str;
	score1_str.setNum(score_team1);
	score2_str.setNum(score_team2);
	QString score_str = score1_str + QString(" - ") + score2_str;
	score_display->display(score_str);
	constexpr int team_no = 1;
	log_writer.writeScore(team_no, score2);
}

Pos Interface::globalPosToImagePos(Pos gpos)
{
	Pos ret_pos;
	const int field_image_width = settings->value("field_image/width").toInt();
	const int field_image_height = settings->value("field_image/height").toInt();
	const int field_size_x = settings->value("field_size/x").toInt();
	const int field_size_y = settings->value("field_size/y").toInt();
	ret_pos.x =
		field_image_width - (int)((double)gpos.x * ((double)field_param.field_length / (double)field_size_x) + ((double)field_param.field_length / 2) + field_param.border_strip_width);
	ret_pos.y =
		                    (int)((double)gpos.y * ((double)field_param.field_width / (double)field_size_y) + ((double)field_param.field_width / 2) + field_param.border_strip_width);
	ret_pos.th = -gpos.th + M_PI;
	return ret_pos;
}

void Interface::setParamFromFile(std::vector<std::string> lines)
{
	if(lines[0].find("Game Monitor") == std::string::npos) {
		// version: 1.0
		setParamFromFileV1(lines);
	} else {
		// version: 2.0
		lines.erase(lines.begin()); // erase first element, it's version signature
		setParamFromFileV2(lines);
	}
}

void Interface::setParamFromFileV1(std::vector<std::string> lines)
{
	for(auto line : lines) {
		LogDataRobotComm buf;
		QString qstr = QString(line.c_str());
		QStringList list = qstr.split(QChar(','));

		int size = list.size();
		if(size == 17) {
			strcpy(buf.time_str, list.at(0).toStdString().c_str());
			buf.id = list.at(1).toInt();
			strcpy(buf.color_str, list.at(2).toStdString().c_str());
			buf.fps = list.at(3).toInt();
			buf.voltage = list.at(4).toDouble();
			buf.x = list.at(5).toInt();
			buf.y = list.at(6).toInt();
			buf.theta = list.at(7).toDouble();
			buf.ball_x = list.at(8).toInt();
			buf.ball_y = list.at(9).toInt();
			buf.goal_pole_x1 = list.at(10).toInt();
			buf.goal_pole_y1 = list.at(11).toInt();
			buf.goal_pole_x2 = list.at(12).toInt();
			buf.goal_pole_y2 = list.at(13).toInt();
			buf.cf_own = list.at(14).toInt();
			buf.cf_ball = list.at(15).toInt();
			strcpy(buf.msg, list.at(16).toStdString().c_str());
			LogData ldata;
			ldata.type = LOG_TYPE_ROBOTINFO;
			ldata.robot_comm = buf;
			strcpy(ldata.time_str, list.at(0).toStdString().c_str());
			log_data.push_back(ldata);
		} else if(size == 2) {
			LogData ldata;
			ldata.type = LOG_TYPE_REMAININGTIME;
			strcpy(ldata.time_str, list.at(0).toStdString().c_str());
			ldata.remaining_time = list.at(1).toInt();
			log_data.push_back(ldata);
		} else if(size == 3) {
			LogData ldata;
			strcpy(ldata.time_str, list.at(0).toStdString().c_str());
			int team_no = list.at(1).toInt();
			if(team_no == 0) {
				ldata.type = LOG_TYPE_SCORE1;
				ldata.score1 = list.at(2).toInt();
			} else if(team_no == 1) {
				ldata.type = LOG_TYPE_SCORE2;
				ldata.score2 = list.at(2).toInt();
			} else {
				continue;
			}
			log_data.push_back(ldata);
		}
	}
	if(log_data.size() == 0) return;
	log_count = 0;
	setData(log_data[log_count]);
	QString before = QString(log_data[log_count++].time_str);
	QString after = QString(log_data[log_count].time_str);
	int interval = getInterval(before, after) / log_speed;
	if(interval < 0) interval = 0;
	QTimer::singleShot(interval, this, SLOT(updateLog()));
}

void Interface::setParamFromFileV2(std::vector<std::string> lines)
{
	for(auto line : lines) {
		LogDataRobotComm buf;
		QString qstr = QString(line.c_str());
		QStringList list = qstr.split(QChar(','));

		int size = list.size();
		if(size == 1) continue;
		if(list[0] == "RobotInfo" && size == 18) {
			strcpy(buf.time_str, list.at(1).toStdString().c_str());
			buf.id = list.at(2).toInt();
			strcpy(buf.color_str, list.at(3).toStdString().c_str());
			buf.fps = list.at(4).toInt();
			buf.voltage = list.at(5).toDouble();
			buf.x = list.at(6).toInt();
			buf.y = list.at(7).toInt();
			buf.theta = list.at(8).toDouble();
			buf.ball_x = list.at(9).toInt();
			buf.ball_y = list.at(10).toInt();
			buf.goal_pole_x1 = list.at(11).toInt();
			buf.goal_pole_y1 = list.at(12).toInt();
			buf.goal_pole_x2 = list.at(13).toInt();
			buf.goal_pole_y2 = list.at(14).toInt();
			buf.cf_own = list.at(15).toInt();
			buf.cf_ball = list.at(16).toInt();
			strcpy(buf.msg, list.at(17).toStdString().c_str());
			LogData ldata;
			ldata.type = LOG_TYPE_ROBOTINFO;
			ldata.robot_comm = buf;
			strcpy(ldata.time_str, list.at(1).toStdString().c_str());
			log_data.push_back(ldata);
		} else if(list[0] == "Score" && size == 4) {
			LogData ldata;
			strcpy(ldata.time_str, list.at(1).toStdString().c_str());
			int team_no = list.at(2).toInt();
			if(team_no == 0) {
				ldata.type = LOG_TYPE_SCORE1;
				ldata.score1 = list.at(3).toInt();
			} else if(team_no == 1) {
				ldata.type = LOG_TYPE_SCORE2;
				ldata.score2 = list.at(3).toInt();
			} else {
				continue;
			}
			log_data.push_back(ldata);
		} else if(list[0] == "RemainingTime" && size == 3) {
			LogData ldata;
			ldata.type = LOG_TYPE_REMAININGTIME;
			strcpy(ldata.time_str, list.at(1).toStdString().c_str());
			ldata.remaining_time = list.at(2).toInt();
			log_data.push_back(ldata);
		} else if(list[0] == "SecondaryTime" && size == 3) {
			LogData ldata;
			ldata.type = LOG_TYPE_SECONDARYTIME;
			strcpy(ldata.time_str, list.at(1).toStdString().c_str());
			ldata.secondary_time = list.at(2).toInt();
			log_data.push_back(ldata);
		} else if(list[0] == "GameState" && size == 3) {
			LogData ldata;
			ldata.type = LOG_TYPE_GAMESTATE;
			strcpy(ldata.time_str, list.at(1).toStdString().c_str());
			ldata.game_state = list.at(2).toInt();
			log_data.push_back(ldata);
		} else {
			continue;
		}
	}
	if(log_data.size() == 0) return;
	log_count = 0;
	setData(log_data[log_count]);
	QString before = QString(log_data[log_count++].time_str);
	QString after = QString(log_data[log_count].time_str);
	int interval = getInterval(before, after) / log_speed;
	if(interval < 0) interval = 0;
	QTimer::singleShot(interval, this, SLOT(updateLog()));
}

int Interface::getInterval(QString before, QString after)
{
	QStringList list_before = before.split(QChar(':'));
	QStringList list_after = after.split(QChar(':'));
	if(list_before.size() != 3 || list_after.size() != 3) {
		std::cerr << "Found invalid log data (ignored)" << std::endl;
		std::cerr << before.toStdString() << ", " << after.toStdString() << std::endl;
		return 0;
	}
	int h = list_after.at(0).toInt() - list_before.at(0).toInt();
	int m = list_after.at(1).toInt() - list_before.at(1).toInt();
	int s = list_after.at(2).toInt() - list_before.at(2).toInt();
	return ((h * 60 + m) * 60 + s) * 1000;
}

void Interface::updateLog(void)
{
	if(fPauseLog)
		return;
	setData(log_data[log_count]);
	if(log_count + 1 >= log_data.size()) return;
	QString step, str_log_count, str_log_total;
	str_log_count.setNum(log_count+1);
	str_log_total.setNum(log_data.size());
	step += str_log_count + " / " + str_log_total;
	log_slider->setValue(log_count);
	log_step->setText(step);
	QString before = QString(log_data[log_count++].time_str);
	QString after = QString(log_data[log_count].time_str);
	int interval = getInterval(before, after) / log_speed;
	if(interval < 0) interval = 0;
	QTimer::singleShot(interval, this, SLOT(updateLog()));
}

void Interface::pausePlayingLog(void)
{
	fPauseLog = true;
}

void Interface::changeLogPosition(void)
{
	fPauseLog = false;
	log_count = log_slider->value();
	updateLog();
}

void Interface::setData(LogData log_data)
{
	if(log_data.type == LOG_TYPE_REMAININGTIME) {
		// time
		setRemainingTime(log_data.remaining_time);
		return;
	} else if(log_data.type == LOG_TYPE_SCORE1) {
		// score
		setScore1(log_data.score1);
		return;
	} else if(log_data.type == LOG_TYPE_SCORE2) {
		setScore2(log_data.score2);
		return;
	} else if(log_data.type == LOG_TYPE_GAMESTATE) {
		setGameState(log_data.game_state);
	} else if(log_data.type == LOG_TYPE_SECONDARYTIME) {
		setSecondaryTime(log_data.secondary_time);
	} else if(log_data.type == LOG_TYPE_ROBOTINFO) {
		LogDataRobotComm data = log_data.robot_comm;

		int num = data.id - 1;
		// Role and message
		char *msg = data.msg;
		if(strstr((const char *)msg, "Attacker")) {
			// Red
			strcpy(positions[num].color, "red");
		} else if(strstr((const char *)msg, "Neutral")) {
			// Green
			strcpy(positions[num].color, "green");
		} else if(strstr((const char *)msg, "Defender")) {
			// Blue
			strcpy(positions[num].color, "blue");
		} else if(strstr((const char *)msg, "Keeper")) {
			// Orange
			strcpy(positions[num].color, "gray");
		} else {
			// Black
			strcpy(positions[num].color, "black");
		}
		positions[num].message = std::string(msg);
		positions[num].behavior_name = std::string(msg); // TODO

		time_t timer;
		timer = time(NULL);
		positions[num].lastReceiveTime = *localtime(&timer);
		positions[num].enable_pos  = true;
		positions[num].enable_ball = true;
		positions[num].enable_goal_pole[0] = true;
		positions[num].enable_goal_pole[1] = true;

		positions[num].pos.x = data.x;
		positions[num].pos.y = data.y;
		positions[num].pos.th = data.theta;
		positions[num].ball.x = data.ball_x;
		positions[num].ball.y = data.ball_y;
		positions[num].goal_pole[0].x = data.goal_pole_x1;
		positions[num].goal_pole[0].y = data.goal_pole_y1;
		positions[num].goal_pole[1].x = data.goal_pole_x2;
		positions[num].goal_pole[1].y = data.goal_pole_y2;
		positions[num].self_conf = data.cf_own;
		positions[num].ball_conf = data.cf_ball;
		positions[num].voltage = data.voltage;
		positions[num].temperature = data.temperature;

		updateMap();
	}
}

void Interface::drawTeamMarker(QPainter &painter, const int pos_x, const int pos_y)
{
	painter.setPen(QPen(Qt::white));
	QFont font = painter.font();
	constexpr int team_marker_font_size = 32;
	font.setPointSize(team_marker_font_size);
	painter.setFont(font);
	painter.drawText(pos_x, pos_y, QString("CIT Brains"));
}

void Interface::drawRobotMarker(QPainter &painter, const int self_x, const int self_y, const double theta, const int robot_id, const QColor marker_color, const double self_conf)
{
	// set marker color according to robot role
	const int robot_pen_size = settings->value("marker/pen_size").toInt();
	painter.setPen(QPen(marker_color, robot_pen_size));

	// draw robot marker
	painter.drawPoint(self_x, self_y);
	const int robot_marker_radius = settings->value("marker/robot_size").toInt();
	painter.drawEllipse(self_x - robot_marker_radius, self_y - robot_marker_radius, robot_marker_radius * 2, robot_marker_radius * 2);
	const int robot_marker_direction_length = settings->value("marker/direction_marker_length").toInt();
	const int direction_x = self_x + robot_marker_direction_length * std::cos(theta);
	const int direction_y = self_y + robot_marker_direction_length * std::sin(theta);
	painter.drawLine(self_x, self_y, direction_x, direction_y);

	// draw robot number
	QString id_str = QString::number(robot_id);
	const int font_offset_x = settings->value("marker/font_offset_x").toInt();
	const int font_offset_y = settings->value("marker/font_offset_y").toInt();
	painter.drawText(QPoint(self_x - font_offset_x, self_y - font_offset_y), id_str);

	// draw self position confidence
	if(fViewSelfPosConf) {
		constexpr int bar_width = 80;
		constexpr int bar_height = 12;
		const int bar_left = self_x - bar_width / 2;
		const int bar_top = self_y + 35;
		QPainterPath path_frame, path_conf;
		path_frame.addRect(bar_left - 2, bar_top - 2, bar_width + 4, bar_height + 4);
		painter.fillPath(path_frame, Qt::white);
		const auto conf = self_conf;
		const int conf_width = static_cast<int>(conf / 100.0 * bar_width);
		QPen pen = painter.pen();
		constexpr int pen_size = 1;
		painter.setPen(QPen(QColor(0, 0, 0), pen_size));
		painter.setRenderHint(QPainter::NonCosmeticDefaultPen);
		painter.drawRect(bar_left, bar_top, bar_width, bar_height);
		painter.setRenderHint(QPainter::Antialiasing);
		painter.setPen(pen);
		path_conf.addRect(bar_left, bar_top, conf_width, bar_height);
		QColor color;
		// change color by self-position confidence (red, orange or green)
		if(conf < 30) {
			color = Qt::red;
		} else if(conf < 70) {
			color = QColor(0xFF, 0xA5, 0x00); // orange
		} else {
			color = Qt::green;
		}
		painter.fillPath(path_conf, color);
	}
}

void Interface::drawRobotInformation(QPainter &painter, const int self_x, const int self_y, const double theta, const int robot_id, const QColor marker_color, const double self_conf, const double ball_conf, const std::string msg, const std::string behavior_name, const double voltage, const double temperature)
{
	constexpr int frame_width = 330;
	constexpr int frame_height = 120;
	int frame_x, frame_y;
	bool success = field_space.getEmptySpace(frame_x, frame_y, frame_width, frame_height, self_x, self_y);
	if(!success) {
		frame_x = self_x;
		frame_y = self_y + 120;
	}
	const int frame_left = frame_x - frame_width / 2;
	const int frame_top = frame_y - frame_height / 2;
	constexpr int pen_size = 3;
	painter.setPen(QPen(Qt::red, pen_size));
	painter.drawLine(frame_x, frame_y, self_x, self_y);
	QPainterPath path_frame;
	path_frame.addRect(frame_left, frame_top, frame_width, frame_height);
	const QColor frame_color(0xE6, 0xE6, 0xFA); // lavender
	painter.fillPath(path_frame, frame_color);
	const QColor frame_border_color(0x80, 0x00, 0x80); // purple
	painter.setPen(QPen(frame_border_color, pen_size));
	painter.drawRect(frame_left, frame_top, frame_width, frame_height);

	painter.setPen(QPen(Qt::red));
	QFont font = painter.font();
	constexpr int font_size = 20;
	font.setPointSize(font_size);
	painter.setFont(font);
	constexpr int font_offset_x = 12;
	constexpr int font_offset_1y = 20 + font_size / 2 + (font_size + 15) * 0;
	std::string s(msg); // message without role name
	s.erase(s.begin(), s.begin() + s.find(" "));
	painter.drawText(frame_left + font_offset_x, frame_top + font_offset_1y, QString(s.c_str()));
	QString behavior_str(behavior_name.c_str());
	constexpr int font_offset_2y = 20 + font_size / 2 + (font_size + 15) * 1;
	painter.drawText(frame_left + font_offset_x, frame_top + font_offset_2y, behavior_str);
	QString voltage_str = QString::number(voltage) + "[V] / " + QString::number(temperature) + "[C]";
	constexpr int font_offset_3y = 20 + font_size / 2 + (font_size + 15) * 2;
	painter.drawText(frame_left + font_offset_x, frame_top + font_offset_3y, voltage_str);

	constexpr int bar_width = 8;
	constexpr int bar_height = frame_height - 4;
	QColor bar_color(0xFF, 0xA5, 0x00); // orange
	painter.setPen(QPen(bar_color, 2));
	painter.drawRect(frame_left + 2, frame_top + 2, bar_width, bar_height - 2);
	QPainterPath path_bar;
	const int bar_left = frame_left + 2;
	const int bar_fill_height = static_cast<int>(ball_conf / 100.0 * bar_height);
	const int bar_fill_top = frame_top + 2 + (bar_height - bar_fill_height);
	path_bar.addRect(bar_left, bar_fill_top, bar_width, bar_fill_height);
	painter.fillPath(path_bar, bar_color);
}

void Interface::drawTargetPosMarker(QPainter &painter, const int target_x, const int target_y, const int self_x, const int self_y)
{
	constexpr int marker_radius = 10;
	const QColor green(0x00, 0xFF, 0x00);
	constexpr int pen_size = 2;
	painter.setPen(QPen(green, pen_size));
	painter.drawEllipse(target_x - marker_radius, target_y - marker_radius, marker_radius * 2, marker_radius * 2);
	painter.drawLine(self_x, self_y, target_x, target_y);
}

void Interface::drawBallMarker(QPainter &painter, const int ball_x, const int ball_y, const int owner_id, const int distance_ball_and_robot, const int self_x, const int self_y)
{
	// draw ball position as orange
	const int ball_marker_size = settings->value("marker/ball_size").toInt();
	QColor orange(0xFF, 0xA5, 0x00);
	painter.setPen(QPen(orange, ball_marker_size));
	painter.drawPoint(ball_x, ball_y);
	constexpr int ball_near_threshold = 50; // Do not draw robot number if the ball is near the robot.
	if(distance_ball_and_robot > ball_near_threshold) {
		QString id_str = QString::number(owner_id);
		const int font_offset_x = settings->value("marker/font_offset_x").toInt();
		const int font_offset_y = settings->value("marker/font_offset_y").toInt();
		painter.drawText(QPoint(ball_x - font_offset_x, ball_y - font_offset_y), id_str);
	}
	painter.setPen(QPen(orange, 1));
	painter.drawLine(self_x, self_y, ball_x, ball_y);
}

void Interface::drawGoalPostMarker(QPainter &painter, const int goal_x, const int goal_y, const int self_x, const int self_y)
{
	QColor goal_post_color = Qt::red;
	const int goal_post_marker_size = settings->value("marker/goal_pole_size").toInt();
	painter.setPen(QPen(goal_post_color, goal_post_marker_size));
	painter.drawPoint(goal_x, goal_y);
	painter.setPen(QPen(goal_post_color, 1));
	painter.drawLine(self_x, self_y, goal_x, goal_y);
}

void Interface::drawHighlightCircle(QPainter &painter, const int center_x, const int center_y)
{
	QColor circle_color = Qt::red;
	QPen pen = painter.pen();
	const int pen_size = 2;
	int circle_size;
	painter.setPen(QPen(circle_color, pen_size));
	circle_size = 200;
	painter.drawEllipse(center_x - (circle_size / 2), center_y - (circle_size / 2), circle_size, circle_size);
	painter.setPen(QPen(circle_color, pen_size));
	circle_size = 100;
	painter.drawEllipse(center_x - (circle_size / 2), center_y - (circle_size / 2), circle_size, circle_size);
	painter.setPen(pen);
}

void Interface::updateMap(void)
{
	time_t timer;
	struct tm *local_time;
	timer = time(NULL);
	local_time = localtime(&timer);

	// Create new image for erase previous position marker
	field_space.clear();
	map = origin_map;
	QPainter paint(&map);
	paint.setRenderHint(QPainter::Antialiasing);
	drawTeamMarker(paint, logo_pos_x, logo_pos_y);

	QFont font = paint.font();
	const int font_size = settings->value("marker/font_size").toInt();
	font.setPointSize(font_size);
	paint.setFont(font);

	const int field_w = settings->value("field_image/width").toInt();
	const int field_h = settings->value("field_image/height").toInt();
	for(int i = 0; i < max_robot_num; i++) {
		if(positions[i].enable_pos) {
			bool flag_reverse = false;
			if((positions[i].colornum == 0 && fReverse) ||
					(positions[i].colornum == 1 && !fReverse)) {
				flag_reverse = true;
			}
			int self_x = positions[i].pos.x;
			int self_y = positions[i].pos.y;
			int ball_x = positions[i].ball.x;
			int ball_y = positions[i].ball.y;
			int target_x = positions[i].target_pos.x;
			int target_y = positions[i].target_pos.y;
			if(flag_reverse) {
				self_x = field_w - self_x;
				self_y = field_h - self_y;
				ball_x = field_w - ball_x;
				ball_y = field_h - ball_y;
				target_x = field_w - target_x;
				target_y = field_h - target_y;
			}
			field_space.setObjectPos(self_x, self_y, 200, 200);
			field_space.setObjectPos(ball_x, ball_y, 50, 50);
			field_space.setObjectPos(target_x, target_y, 50, 50);
			const int time_limit = settings->value("marker/time_up_limit").toInt();
			const int elapsed = (local_time->tm_min - positions[i].lastReceiveTime.tm_min) * 60 + (local_time->tm_sec - positions[i].lastReceiveTime.tm_sec);
			if(elapsed > time_limit) {
				positions[i].enable_pos = false;
				positions[i].enable_ball = false;
				continue;
			}
		}
	}
	for(int i = 0; i < max_robot_num; i++) {
		if(positions[i].enable_pos) {
			int self_x = positions[i].pos.x;
			int self_y = positions[i].pos.y;
			double theta = positions[i].pos.th;
			bool flag_reverse = false;
			if((positions[i].colornum == 0 && fReverse) ||
					(positions[i].colornum == 1 && !fReverse)) {
				flag_reverse = true;
			}
			if(flag_reverse) {
				self_x = field_w - self_x;
				self_y = field_h - self_y;
				theta = theta + M_PI;
			}
			const int robot_id = i + 1;
			const QColor color = getColor(positions[i].color);
			if(fViewRobotInformation)
				drawRobotInformation(paint, self_x, self_y, theta, robot_id, color, positions[i].self_conf, positions[i].ball_conf, positions[i].message, positions[i].behavior_name, positions[i].voltage, positions[i].temperature);
			if(positions[i].enable_ball && positions[i].ball_conf > 0) {
				int ball_x = positions[i].ball.x;
				int ball_y = positions[i].ball.y;
				if(flag_reverse) {
					ball_x = field_w - ball_x;
					ball_y = field_h - ball_y;
				}
				const int distance_ball_and_robot = distance(ball_x, ball_y, self_x, self_y);
				const int owner_id = i + 1;
				drawBallMarker(paint, ball_x, ball_y, owner_id, distance_ball_and_robot, self_x, self_y);
			}
			if(positions[i].enable_target_pos) {
				int target_x = positions[i].target_pos.x;
				int target_y = positions[i].target_pos.y;
				if(flag_reverse) {
					target_x = field_w - target_x;
					target_y = field_h - target_y;
				}
				drawTargetPosMarker(paint, target_x, target_y, self_x, self_y);
			}
			// draw goal posts
			if(fViewGoalpost) {
				for(int j = 0; j < 2; j++) {
					if(positions[i].enable_goal_pole[j]) {
						int goal_pole_x = positions[i].goal_pole[j].x;
						int goal_pole_y = positions[i].goal_pole[j].y;
						if(flag_reverse) {
							goal_pole_x = field_w - goal_pole_x;
							goal_pole_y = field_h - goal_pole_y;
						}
						drawGoalPostMarker(paint, goal_pole_x, goal_pole_y, self_x, self_y);
					}
				}
			}
			drawRobotMarker(paint, self_x, self_y, theta, robot_id, color, positions[i].self_conf);
		}
	}
	image->setPixmap(map);
}

QColor Interface::getColor(const char *color_name)
{
	if(!strcmp(color_name, "red")) {
		return QColor(0xFF, 0x8E, 0x8E);
	} else if(!strcmp(color_name, "black")) {
		return QColor(0x00, 0x00, 0x00);
	} else if(!strcmp(color_name, "green")) {
		return QColor(0x8E, 0xFF, 0x8E);
	} else if(!strcmp(color_name, "blue")) {
		return QColor(0x8E, 0x8E, 0xFF);
	} else if(!strcmp(color_name, "gray")) {
		return QColor(0x9F, 0xA5, 0xA0);
	} else {
		return QColor(0x00, 0x00, 0x00);
	}
}

void Interface::timerEvent(QTimerEvent *e)
{
	if(e->timerId() == updateMapTimerId) {
		updateMap();
	}
}

void Interface::reverseField(int state)
{
	if(state == Qt::Checked) {
		fReverse = true;
		logo_pos_x = field_param.field_length / 2 - field_param.field_length / 4;
	} else {
		fReverse = false;
		logo_pos_x = field_param.field_length / 2 + field_param.field_length / 4;
	}
	updateMap();
}

void Interface::viewGoalpost(bool checked)
{
	if(checked) {
		fViewGoalpost = true;
	} else {
		fViewGoalpost = false;
	}
	updateMap();
}

void Interface::viewRobotInformation(bool checked)
{
	if(checked) {
		fViewRobotInformation = true;
	} else {
		fViewRobotInformation = false;
	}
	updateMap();
}

void Interface::viewSelfPosConf(bool checked)
{
	if(checked) {
		fViewSelfPosConf = true;
	} else {
		fViewSelfPosConf = false;
	}
	updateMap();
}

void Interface::loadLogFile(void)
{
	QString fileName = QFileDialog::getOpenFileName(this, "log file", "./log", "*.log");
	std::ifstream ifs(fileName.toStdString());
	if(ifs.fail()) {
		std::cerr << "file open error" << std::endl;
		return;
	}
	std::string line;
	std::vector<std::string> lines;
	while(getline(ifs, line)) {
		lines.push_back(line);
	}
	log_slider->setMaximum(lines.size()-1);
	log_writer.setEnable(false);
	statusBar->showMessage(QString("Playing game from log"));
	setParamFromFile(lines);
}

void Interface::logSpeed1(void)
{
	log_speed = 1;
	log1Button->setEnabled(false);
	log2Button->setEnabled(true);
	log5Button->setEnabled(true);
}

void Interface::logSpeed2(void)
{
	log_speed = 2;
	log1Button->setEnabled(true);
	log2Button->setEnabled(false);
	log5Button->setEnabled(true);
}

void Interface::logSpeed5(void)
{
	log_speed = 5;
	log1Button->setEnabled(true);
	log2Button->setEnabled(true);
	log5Button->setEnabled(false);
}

void Interface::gameStateFontSizeChanged(int value)
{
	QFont font = label_game_state_display->font();
	font.setPointSize(value);
	label_game_state_display->setFont(font);
	settings->setValue("size/font_size", value);
}

void Interface::displaySizeChanged(int value)
{
	time_display->setMinimumHeight(value);
	secondary_time_display->setMinimumHeight(value);
	score_display->setMinimumHeight(value);
	settings->setValue("size/display_minimum_height", value);
}

void Interface::robotMarkerSizeChanged(int value)
{
	settings->setValue("marker/robot_size", value);
	settings->setValue("marker/direction_marker_length", static_cast<int>(value * (4.0 / 3.0)));
}

void Interface::robotMarkerLineWidthChanged(int value)
{
	settings->setValue("marker/pen_size", value);
}

void Interface::openSettingWindow(void)
{
	statusBar->showMessage(QString("setting"));
	SettingDialog dialog(this);
	const int font_size = settings->value("size/font_size").toInt();
	const int display_minimum_height = settings->value("size/display_minimum_height").toInt();
	const int robot_marker_size = settings->value("marker/robot_size").toInt();
	const int robot_marker_line_width = settings->value("marker/pen_size").toInt();
	dialog.setDefaultParameters(font_size, display_minimum_height, robot_marker_size, robot_marker_line_width);
	connect(&dialog, SIGNAL(fontSizeChanged(int)), this, SLOT(gameStateFontSizeChanged(int)));
	connect(&dialog, SIGNAL(displaySizeChanged(int)), this, SLOT(displaySizeChanged(int)));
	connect(&dialog, SIGNAL(robotMarkerSizeChanged(int)), this, SLOT(robotMarkerSizeChanged(int)));
	connect(&dialog, SIGNAL(robotMarkerLineWidthChanged(int)), this, SLOT(robotMarkerLineWidthChanged(int)));
	dialog.show();
	dialog.exec();
}

