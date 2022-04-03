#include <iostream>
#include <cstdio>
#include <ctime>

#include "log_writer.h"

LogWriter::LogWriter() : opened(false), enable(false)
{
}

LogWriter::~LogWriter()
{
	closeFile();
}

void LogWriter::openFileCurrentTime(void)
{
	time_t timer;
	struct tm *local_time;
	char filename[1024];
	char logfile_path[] = "log/";

	timer = time(NULL);
	local_time = localtime(&timer);
	sprintf(filename, "%s%d-%d-%d-%d-%d.log", logfile_path, local_time->tm_year+1900, local_time->tm_mon+1, local_time->tm_mday, local_time->tm_hour, local_time->tm_min);
	openFile(filename);
	printVersionInfo();
}

int LogWriter::startRecord(const char *filename)
{
	time_t timer;
	struct tm *local_time;

	timer = time(NULL);
	local_time = localtime(&timer);
	if(enable && !opened)
		openFileCurrentTime();
	if(opened && enable) {
		fprintf(fp, "RecordStart,%d:%d:%d,", local_time->tm_hour, local_time->tm_min, local_time->tm_sec);
		fprintf(fp, "%s", "record,");
		fprintf(fp, "%s", filename);
		fprintf(fp, "\n");
	}
	return 0;
}

int LogWriter::stopRecord(void)
{
	time_t timer;
	struct tm *local_time;

	timer = time(NULL);
	local_time = localtime(&timer);
	if(opened && enable) {
		fprintf(fp, "RecordStop,%d:%d:%d,", local_time->tm_hour, local_time->tm_min, local_time->tm_sec);
		fprintf(fp, "%s", "stop");
		fprintf(fp, "\n");
	}
	return 0;
}

int LogWriter::write(int id, const char *color, int fps, double voltage,
	int posx, int posy, float posth, int ballx, int bally,
	int goal_pole_x1, int goal_pole_y1, int goal_pole_x2, int goal_pole_y2,
	const char *str, const char *behavior_name, int cf_own, int cf_ball)
{
	time_t timer;
	struct tm *local_time;

	timer = time(NULL);
	local_time = localtime(&timer);
	if(enable && !opened)
		openFileCurrentTime();
	if(opened && enable) {
		fprintf(fp, "RobotInfo,%d:%d:%d,", local_time->tm_hour, local_time->tm_min, local_time->tm_sec);
		fprintf(fp, "%d,", id);
		fprintf(fp, "%s,", color);
		fprintf(fp, "%d,", fps);
		fprintf(fp, "%.2lf,", voltage);
		fprintf(fp, "%d,%d,%f,", posx, posy, posth);
		fprintf(fp, "%d,%d,", ballx, bally);
		fprintf(fp, "%d,%d,", goal_pole_x1, goal_pole_y1);
		fprintf(fp, "%d,%d,", goal_pole_x2, goal_pole_y2);
		fprintf(fp, "%d,%d,", cf_own, cf_ball);
		fprintf(fp, "%s,", str);
		fprintf(fp, "%s", behavior_name);
		fprintf(fp, "\n");
	}
	return 0;
}

void LogWriter::writeScore(const int team_no, const int score)
{
	time_t timer;
	struct tm *local_time;

	timer = time(NULL);
	local_time = localtime(&timer);
	if(enable && !opened)
		openFileCurrentTime();
	if(opened && enable) {
		fprintf(fp, "Score,%d:%d:%d,", local_time->tm_hour, local_time->tm_min, local_time->tm_sec);
		fprintf(fp, "%d,%d", team_no, score);
		fprintf(fp, "\n");
	}
}

void LogWriter::writeRemainingTime(const int remaining_time)
{
	time_t timer;
	struct tm *local_time;

	timer = time(NULL);
	local_time = localtime(&timer);
	if(enable && !opened)
		openFileCurrentTime();
	if(opened && enable) {
		fprintf(fp, "RemainingTime,%d:%d:%d,", local_time->tm_hour, local_time->tm_min, local_time->tm_sec);
		fprintf(fp, "%d", remaining_time);
		fprintf(fp, "\n");
	}
}

void LogWriter::writeSecondaryTime(const int secondary_time)
{
	time_t timer;
	struct tm *local_time;

	timer = time(NULL);
	local_time = localtime(&timer);
	if(enable && !opened)
		openFileCurrentTime();
	if(opened && enable) {
		fprintf(fp, "SecondaryTime,%d:%d:%d,", local_time->tm_hour, local_time->tm_min, local_time->tm_sec);
		fprintf(fp, "%d", secondary_time);
		fprintf(fp, "\n");
	}
}

void LogWriter::writeGameState(const int game_state)
{
	time_t timer;
	struct tm *local_time;

	timer = time(NULL);
	local_time = localtime(&timer);
	if(enable && !opened)
		openFileCurrentTime();
	if(opened && enable) {
		fprintf(fp, "GameState,%d:%d:%d,", local_time->tm_hour, local_time->tm_min, local_time->tm_sec);
		fprintf(fp, "%d", game_state);
		fprintf(fp, "\n");
	}
}

int LogWriter::separate(void)
{
	if(opened && enable) {
		fprintf(fp, "\n---\n");
	}
	return 0;
}

void LogWriter::setEnable(bool benable)
{
	enable = benable;
}

void LogWriter::openFile(char *filename)
{
	closeFile();
	fp = fopen(filename, "w");
	if(fp) {
		opened = true;
		enable = true;
	}
}

void LogWriter::printVersionInfo(void)
{
	constexpr int MAJOR_VERSION = 2;
	constexpr int MINOR_VERSION = 0;
	fprintf(fp, "Game Monitor, version: %d.%d\n", MAJOR_VERSION, MINOR_VERSION);
}

void LogWriter::closeFile(void)
{
	if(opened) {
		fclose(fp);
	}
	enable = false;
	opened = false;
}

