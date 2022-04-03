#include "setting_dialog.h"

SettingDialog::SettingDialog(QWidget *parent) : QDialog(parent)
{
	buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	connect(buttonBox, SIGNAL(QDialogButtonBox::accepted), this, SLOT(QDialog::accept));
	connect(buttonBox, SIGNAL(QDialogButtonBox::rejected), this, SLOT(QDialog::reject));

	button = new QPushButton("Apply");
	font_size_label = new QLabel("font size of game state: ");
	font_size_slider = new QSlider(Qt::Horizontal);
	font_size_slider->setRange(1, 128);
	connect(font_size_slider, SIGNAL(valueChanged(int)), this, SLOT(fontSizeValueChanged(int)));
	display_size_label = new QLabel("display size: ");
	display_size_slider = new QSlider(Qt::Horizontal);
	display_size_slider->setRange(1, 300);
	connect(display_size_slider, SIGNAL(valueChanged(int)), this, SLOT(displaySizeValueChanged(int)));
	robot_marker_size_label = new QLabel("Robot marker size: ");
	robot_marker_size_slider = new QSlider(Qt::Horizontal);
	robot_marker_size_slider->setRange(1, 100);
	connect(robot_marker_size_slider, SIGNAL(valueChanged(int)), this, SLOT(robotMarkerSizeValueChanged(int)));
	robot_marker_line_width_label = new QLabel("Robot marker line width: ");
	robot_marker_line_width_slider = new QSlider(Qt::Horizontal);
	robot_marker_line_width_slider->setRange(1, 20);
	connect(robot_marker_line_width_slider, SIGNAL(valueChanged(int)), this, SLOT(robotMarkerLineWidthValueChanged(int)));

	slider_layout = new QGridLayout;
	slider_layout->addWidget(font_size_label, 1, 1);
	slider_layout->addWidget(font_size_slider, 1, 2);
	slider_layout->addWidget(display_size_label, 2, 1);
	slider_layout->addWidget(display_size_slider, 2, 2);
	slider_layout->addWidget(robot_marker_size_label, 3, 1);
	slider_layout->addWidget(robot_marker_size_slider, 3, 2);
	slider_layout->addWidget(robot_marker_line_width_label, 4, 1);
	slider_layout->addWidget(robot_marker_line_width_slider, 4, 2);
	main_layout = new QVBoxLayout;
	main_layout->addLayout(slider_layout);
	main_layout->addWidget(button);
	main_layout->addWidget(buttonBox);
	setLayout(main_layout);

	setMinimumSize(640, 480);
}

void SettingDialog::setDefaultParameters(const int game_state_font_size, const int display_minimum_height, const int robot_marker_size, const int robot_marker_line_width)
{
	font_size_slider->setValue(game_state_font_size);
	display_size_slider->setValue(display_minimum_height);
	robot_marker_size_slider->setValue(robot_marker_size);
	robot_marker_line_width_slider->setValue(robot_marker_line_width);
}

void SettingDialog::fontSizeValueChanged(int value)
{
	emit fontSizeChanged(value);
}

void SettingDialog::displaySizeValueChanged(int value)
{
	emit displaySizeChanged(value);
}

void SettingDialog::robotMarkerSizeValueChanged(int value)
{
	emit robotMarkerSizeChanged(value);
}

void SettingDialog::robotMarkerLineWidthValueChanged(int value)
{
	emit robotMarkerLineWidthChanged(value);
}

