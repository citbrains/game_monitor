cmake_minimum_required(VERSION 3.1)

project(game_monitor)

if(CMAME_COMPILER_IS_GNUCXX)
	add_definitions(-std=c++11)
endif()
set(CMAKE_CXX_STANDARD 11)

STRING(APPEND new_infoshare_path "/home/citbrains/citbrains_humanoid/for2050/src/new_infoshare")
STRING(APPEND hpl_path "/home/citbrains/citbrains_humanoid/for2050/src/HPL")
STRING(APPEND hc_server_path "/home/citbrains/citbrains_humanoid/for2050/src/HCServer")

add_subdirectory(${new_infoshare_path} ./new_infoshare)

set(SRCS
	src/interface.cpp
	src/interface.h
	src/log_writer.cpp
	src/log_writer.h
	src/main.cpp
	src/pos_types.h
	src/new_udp_thread.cpp
	src/new_udp_thread.h
	src/aspect_ratio_pixmap_label.cpp
	src/field_space_manager.cpp
	src/field_space_manager.h
	src/gcreceiver.cpp
	src/gcreceiver.h
	src/game_state.cpp
	src/game_state.h
	src/setting_dialog.cpp
)

set(MOC_HEADERS
	src/interface.h
	src/new_udp_thread.h
	src/aspect_ratio_pixmap_label.h
	src/setting_dialog.h
)

include(FindSDL)
find_package(SDL REQUIRED)
if(NOT_SDL_FOUND)
    message(FATAL ERROR "SDL not found!!!")
endif(NOT_SDL_FOUND)

find_package(Qt5Widgets REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Network REQUIRED)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

set(QT_USE_QTMAIN true)
set(QT_USE_QTGUI true)
set(QT_USE_QTNETWORK true)
add_definitions(${QT_DEFINITIONS})
include_directories(${CMAKE_BINARY_DIR})

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING
		"Default build type: RelWithDebInfo" FORCE)
endif()

include_directories(src
	${new_infoshare_path}
	${new_infoshare_path}/socket
	${hpl_path}
	${CMAKE_CURRENT_BINARY_DIR}/new_infoshare
	${hc_server_path}
	)
add_executable(game_monitor ${SRCS} ${MOC_SRCS} ${RES_SORUCES})
target_link_libraries(game_monitor
	${QT_LIBRARIES}
	Qt5::Widgets
	Qt5::Network
	infoshareLib
)

message(STATUS "create log & videos directories")
file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/log)
file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/videos)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/new_infoshare/command_dict.dat ${CMAKE_CURRENT_BINARY_DIR}/new_infoshare/behavior_name_dict.dat
		DESTINATION ${CMAKE_CURRENT_BINARY_DIR} )