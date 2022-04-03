#ifndef SETTING_DIALOG_H
#define SETTING_DIALOG_H

#include <QDialog>
#include <QSlider>
#include <QLabel>
#include <QPushButton>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QDialogButtonBox>

class SettingDialog : public QDialog
{
	Q_OBJECT
public:
	SettingDialog(QWidget *parent = 0);
	void setDefaultParameters(const int, const int, const int, const int);
signals:
	void fontSizeChanged(int);
	void displaySizeChanged(int);
	void robotMarkerSizeChanged(int);
	void robotMarkerLineWidthChanged(int);
private:
	QSlider *font_size_slider;
	QSlider *display_size_slider;
	QSlider *robot_marker_size_slider;
	QSlider *robot_marker_line_width_slider;
	QPushButton *button;
	QLabel *font_size_label;
	QLabel *display_size_label;
	QLabel *robot_marker_size_label;
	QLabel *robot_marker_line_width_label;
	QGridLayout *slider_layout;
	QVBoxLayout *main_layout;
	QDialogButtonBox *buttonBox;
private slots:
	void fontSizeValueChanged(int value);
	void displaySizeValueChanged(int value);
	void robotMarkerSizeValueChanged(int);
	void robotMarkerLineWidthValueChanged(int);
};

#endif // SETTING_DIALOG_H

