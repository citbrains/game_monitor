#ifndef INTERFACE_H
#define INTERFACE_H

#include <vector>
#include <string>

#include <QtGui>
#include <QtCore>
#include <QApplication>
#include <QPushButton>
#include <QCheckBox>
#include <QLabel>
#include <QTextEdit>
#include <QLineEdit>
#include <QMainWindow>
#include <QDropEvent>
#include <QDrag>
#include <QUrl>
#include <QMimeData>
#include <QString>
#include <QSlider>
#include <QMetaType>
#include <QPainter>
#include <QSettings>
#include <QGridLayout>
#include <QProgressBar>
#include <QFileDialog>
#include <QPixmap>
#include <QLCDNumber>
#include <QStatusBar>
#include <QAction>
#include <QMenuBar>

#include "new_udp_thread.h"
#include "log_writer.h"
#include "pos_types.h"
#include "aspect_ratio_pixmap_label.h"
#include "gcreceiver.h"
#include "field_space_manager.h"
#include "setting_dialog.h"

static constexpr int STATE_IMPOSSIBLE = -1;
static constexpr int STATE_INITIAL = 0;
static constexpr int STATE_READY = 1;
static constexpr int STATE_SET = 2;
static constexpr int STATE_PLAYING = 3;
static constexpr int STATE_FINISHED = 4;

static const int LOG_TYPE_ROBOTINFO = 0;
static const int LOG_TYPE_SCORE1 = 1;
static const int LOG_TYPE_SCORE2 = 2;
static const int LOG_TYPE_REMAININGTIME = 3;
static const int LOG_TYPE_SECONDARYTIME = 4;
static const int LOG_TYPE_GAMESTATE = 5;

/*
 * Field parameters.
 * See Law 1 of rule book(2018) at http://www.robocuphumanoid.org/wp-content/uploads/RCHL-2018-Rules-Proposal_changesMarked_final.pdf
 */
class FieldParameter
{
	// unit: meter
public:
	FieldParameter() :
		field_length(9.0),
		field_width(6.0),
		goal_depth(0.6),
		goal_width(2.6),
		goal_height(1.8),
		goal_area_length(1.0),
		goal_area_width(3.0),
		penalty_mark_distance(1.5),
		center_circle_diameter(1.5),
		border_strip_width(1.0),
		penalty_area_length(2.0),
		penalty_area_width(5.0)
	{
	}
	~FieldParameter() {};
	const double field_length;
	const double field_width;
	const double goal_depth;
	const double goal_width;
	const double goal_height;
	const double goal_area_length;
	const double goal_area_width;
	const double penalty_mark_distance;
	const double center_circle_diameter;
	const double border_strip_width;
	const double penalty_area_length;
	const double penalty_area_width;
};

class FieldParameterInt
{
	// unit: centimeter
public:
	FieldParameterInt(FieldParameter param) :
		field_length(static_cast<int>(param.field_length * 100)),
		field_width(static_cast<int>(param.field_width * 100)),
		goal_depth(static_cast<int>(param.goal_depth * 100)),
		goal_width(static_cast<int>(param.goal_width * 100)),
		goal_height(static_cast<int>(param.goal_height * 100)),
		goal_area_length(static_cast<int>(param.goal_area_length * 100)),
		goal_area_width(static_cast<int>(param.goal_area_width * 100)),
		penalty_mark_distance(static_cast<int>(param.penalty_mark_distance * 100)),
		center_circle_diameter(static_cast<int>(param.center_circle_diameter * 100)),
		border_strip_width(static_cast<int>(param.border_strip_width * 100)),
		penalty_area_length(static_cast<int>(param.penalty_area_length * 100)),
		penalty_area_width(static_cast<int>(param.penalty_area_width * 100))
	{
	}
	~FieldParameterInt() {};
	const int field_length;
	const int field_width;
	const int goal_depth;
	const int goal_width;
	const int goal_height;
	const int goal_area_length;
	const int goal_area_width;
	const int penalty_mark_distance;
	const int center_circle_diameter;
	const int border_strip_width;
	const int penalty_area_length;
	const int penalty_area_width;
};

class PositionMarker {
public:
	PositionMarker() : self_conf(0.0), ball_conf(0.0), voltage(0.0), temperature(0.0), colornum(0), enable_pos(false), enable_ball(false), enable_goal_pole{false, false}, enable_target_pos(false) { color[0] = '\0'; }
	double self_conf;
	double ball_conf;
	double voltage;
	double temperature;
	int colornum;
	bool enable_pos;
	bool enable_ball;
	bool enable_goal_pole[2];
	bool enable_target_pos;
	struct tm lastReceiveTime;
	char color[20];
	Pos pos; /* self position */
	Pos ball; /* ball position */
	Pos goal_pole[2]; /* goal pole position */
	Pos target_pos;
	std::string message;
	std::string behavior_name;
};

class LogDataRobotComm {
public:
	LogDataRobotComm() : temperature(0.0) { }
	char time_str[100];
	int id;
	char color_str[100];
	int fps;
	double voltage;
	double temperature;
	int x;
	int y;
	double theta;
	int ball_x;
	int ball_y;
	int goal_pole_x1;
	int goal_pole_y1;
	int goal_pole_x2;
	int goal_pole_y2;
	int cf_own;
	int cf_ball;
	char msg[100];
};

class LogData
{
public:
	LogData() : type(0), score1(0), score2(0), remaining_time(0), secondary_time(0), game_state(0) { }
	~LogData() { }
	int type;
	LogDataRobotComm robot_comm;
	int score1;
	int score2;
	int remaining_time;
	int secondary_time;
	int game_state;
	char time_str[100];
};

class Interface : public QMainWindow
{
	Q_OBJECT

private:
	LogWriter log_writer;
	std::vector<UdpServer *> th;
	GCReceiver *gc_thread;
	QMenu *fileMenu;
	QMenu *viewMenu;
	QMenu *videoMenu;
	QAction *loadLogFileAction;
	QAction *settingsAction;
	QAction *viewGoalPostAction;
	QAction *viewRobotInformationAction;
	QAction *viewSelfPosConfAction;
	QStatusBar *statusBar;
	QCheckBox *reverse;
	QPushButton *log1Button, *log2Button, *log5Button;
	QSettings *settings;
	QString filenameDrag;
	QWidget *window;
	AspectRatioPixmapLabel *image;
	QLabel *log_step;
	QLabel *label_remaining_time;
	QLabel *label_secondary_time;
	QLabel *label_game_state;
	QLabel *label_game_state_display;
	QLabel *label_score;
	QLCDNumber *time_display;
	QLCDNumber *secondary_time_display;
	QLCDNumber *score_display;
	QPixmap map;
	QPixmap origin_map;
	QSlider *log_slider;
	QGridLayout *mainLayout;
	QVBoxLayout *checkLayout;
	QHBoxLayout *logLayout;
	QHBoxLayout *logSpeedButtonLayout;
	QPalette pal_state_bgcolor;
	QPalette pal_red;
	QPalette pal_green;
	QPalette pal_blue;
	QPalette pal_black;
	QPalette pal_orange;
	std::vector<PositionMarker> positions;
	std::vector<LogData> log_data;
	bool fLogging;
	bool fReverse;
	bool fViewGoalpost;
	bool fViewRobotInformation;
	bool fPauseLog;
	bool fRecording;
	bool fViewSelfPosConf;
	int updateMapTimerId;
	int score_team1;
	int score_team2;
	unsigned int log_count;
	const int max_robot_num;
	int logo_pos_x, logo_pos_y;
	int log_speed;
	FieldParameterInt field_param;
	FieldSpaceManager field_space;
	void initializeConfig(void);
	void createWindow(void);
	void connection(void);
	int getInterval(QString, QString);
	Pos globalPosToImagePos(Pos);
	void timerEvent(QTimerEvent *);
	void setParamFromFile(std::vector<std::string>);
	void setParamFromFileV1(std::vector<std::string>);
	void setParamFromFileV2(std::vector<std::string>);
	void setData(LogData);
	QColor getColor(const char *);
	void createMenus(void);
	void drawTeamMarker(QPainter &, const int, const int);
	void drawRobotMarker(QPainter &, const int, const int, const double, const int, const QColor, const double);
	void drawRobotInformation(QPainter &, const int, const int, const double, const int, const QColor, const double, const double, const std::string, const std::string, const double, const double);
	void drawBallMarker(QPainter &, const int, const int, const int, const int, const int, const int);
	void drawTargetPosMarker(QPainter &, const int, const int, const int, const int);
	void drawGoalPostMarker(QPainter &, const int, const int, const int, const int);
	void drawHighlightCircle(QPainter &, const int, const int);

public:
	Interface();
	~Interface();
	void drawField(void);
	void dragEnterEvent(QDragEnterEvent *);
	void dropEvent(QDropEvent *);
	void decodeUdp(struct Citbrains::infosharemodule::OtherRobotInfomation, int num);
	void updateMap(void);

private slots:
	void decodeData1(struct Citbrains::infosharemodule::OtherRobotInfomation);
	void decodeData2(struct Citbrains::infosharemodule::OtherRobotInfomation);
	void decodeData3(struct Citbrains::infosharemodule::OtherRobotInfomation);
	void decodeData4(struct Citbrains::infosharemodule::OtherRobotInfomation);
	void decodeData5(struct Citbrains::infosharemodule::OtherRobotInfomation);
	void decodeData6(struct Citbrains::infosharemodule::OtherRobotInfomation);
	void setGameState(int);
	void setRemainingTime(int);
	void setSecondaryTime(int);
	void setScore1(int);
	void setScore2(int);
	void reverseField(int);
	void viewGoalpost(bool);
	void viewRobotInformation(bool);
	void viewSelfPosConf(bool);
	void loadLogFile(void);
	void updateLog(void);
	void logSpeed1(void);
	void logSpeed2(void);
	void logSpeed5(void);
	void pausePlayingLog(void);
	void changeLogPosition(void);
	void openSettingWindow(void);
	void gameStateFontSizeChanged(int);
	void displaySizeChanged(int);
	void robotMarkerSizeChanged(int);
	void robotMarkerLineWidthChanged(int);
};

#endif // INTERFACE_H

