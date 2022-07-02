#!/bin/bash

echo $#
echo $1
game_monitor_dir="$PWD"
if [ $# -ne 1 ] ;then
	echo "ブランチ名を引数に指定してください"
	echo "masterでいい場合はエンターを押してください"

	read -p "ok? (Y/n): " yn
	case "$yn" in [nN]*) echo abort ; exit ;; *) branch_name="master"; ;; esac

	#read -p "Hit enter: " yn
	#echo $yn
	#if [ ! "$yn" = "\n" ];then
	#	echo "in"
	#	exit 1
	#else
	#	branch_name=$1
	#fi
else
	branch_name=$1
fi
cd ~/citbrains_humanoid

git pull

git checkout origin ${branch_name}

git pull origin ${branch_name}

cd $game_monitor_dir/build
make install -j$(nproc)
cd ..

./build/game_monitor
