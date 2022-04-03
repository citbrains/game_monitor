#!/bin/bash -xe

PROJECT='GameMonitor.pro'

QMAKE='qmake'
echo $($QMAKE --version)
$QMAKE -project -o $PROJECT
echo 'QMAKE_CXXFLAGS += --std=c++11' >> $PROJECT
echo 'QT += network widgets' >> $PROJECT

if [ ! -d build ]; then
	mkdir build
fi
echo 'DESTDIR = build' >> $PROJECT
echo 'OBJECTS_DIR = build/obj' >> $PROJECT
echo 'MOC_DIR = build/moc' >> $PROJECT
echo 'RCC_DIR = build/rcc' >> $PROJECT
echo 'UI_DIR = build/ui' >> $PROJECT

$QMAKE
make all -j8

if [ ! -d log ]; then
	mkdir log
fi
if [ ! -d videos ]; then
	mkdir videos
fi

