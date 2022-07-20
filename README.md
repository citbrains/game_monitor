[![Build Status](https://travis-ci.org/SatoshiShimada/game_monitor.svg?branch=master)](https://travis-ci.org/SatoshiShimada/game_monitor)

# Game Monitor

![Screen Image](https://github.com/SatoshiShimada/game_monitor/wiki/figures/screen_image3.png)

The robot state viewer via robot communication.  
It is for CIT Brains' robot "Accelite" and "GankenKun".  
This application supports Linux, Mac OS and Windows.  
If you find a bug please let me know by opening an issue in this repository.  
Please check Wiki for more information.  

Functions:

- Display
    - Game state (from official GameController)
        - Game time
        - Secondary time (kick-off, time-out, etc.)
        - Score
    - Robot state (from robot communication)
        - Robot position and reliability
        - Current strategy
        - Detected ball position and reliability
        - Detected goal post positions
        - Voltage of the motor
        - Temperature of the motor
- Logging and play the status of the game.

## Requirements

<details>
<summary>Qt (Version: 5.3 or greater)</summary>
URL: https://www.qt.io/

### インストール法
`sudo apt install qtbase5-dev qttools5-dev-tools qt5-default`
</details>
<details>
<summary>protocol buf</summary>
URL: https://developers.google.com/protocol-buffers

### インストール法
```bash
wget "https://github.com/protocolbuffers/protobuf/releases/download/v21.1/protobuf-all-21.1.tar.gz" -O protobuf-all-21.1.tar.gz
tar -zxvf protobuf-all-21.1.tar.gz
cd protobuf-3.21.1
./configure
make -j$(nproc) # $(nproc) ensures it uses all cores for compilation
make check -j$(nproc)
sudo make install
sudo ldconfig # refresh shared library cache.
```
</details>

<details>
<summary>boost</summary>
URL: https://www.boost.org/

### インストール法
`sudo apt install libboost-all-dev`
</details>

<details>
<summary>sdl 1.2</summary>
URL: 

### インストール法
`sudo apt install libsdl1.2-dev`
</details>

## How to build

### Linux (Ubuntu 18.04)

1. Install libraries.

上記参照

2. Build application.  
2.1 homeに`citbrains_humanoid`をcloneする  
https  
`git clone https://github.com/citbrains/game_monitor.git`  
ssh  
`git@github.com:citbrains/game_monitor.git`  
2.2 game_monitorを任意の場所にclone  
https  
`git clone https://github.com/citbrains/game_monitor.git`  
ssh  
`git clone git@github.com:citbrains/game_monitor.git`  
2.3 game_monitorに移動  
2.4 support_new_infoshareブランチに移動  
`git checkout support_new_infoshare`  
2.5 buildする  
```bash
mkdir build
cd build
cmake .. #build方法は例(ninjaでも可能)
make -jX install #Xは自分のスレッド数
```

3. 起動方法  
`./start_GM.sh`

## License

MIT License (see `LICENSE` file).

