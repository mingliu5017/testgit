cmake_minimum_required(VERSION 3.2)

set(APP_TARGET_NAME aml_migu_demo)
set(AML_SUPPORT_LIB_NAME aml_support)

project(${APP_TARGET_NAME} C CXX)

set(PROJECT_OUTPUT $ENV{TARGET_OUTPUT_DIR})
set(PROJECT_SYSROOT ${PROJECT_OUTPUT}/host/usr/arm-linux-gnueabihf/sysroot)
set(PROJECT_TARGET_DIR ${PROJECT_OUTPUT}/target)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output/libs)

file(GLOB_RECURSE C_FILES_VENDOR		"./vendor/*.c") 
file(GLOB_RECURSE CXX_FILES_VENDOR		"./vendor/*.cpp") 
file(GLOB_RECURSE CC_FILES_VENDOR		"./vendor/*.cc") 


set(CMAKE_C_FLAGS    "-Wall -pipe -fPIC -std=gnu99 -O2 -fmessage-length=0")
set(CMAKE_CXX_FLAGS  "-Wall -pipe -fPIC -std=c++11 -O2 -fmessage-length=0")
set(CMAKE_BUILD_TYPE Release ON)

include_directories(
	${PROJECT_SYSROOT}/usr/include 
	${PROJECT_SYSROOT}/usr/include/glib-2.0
	${PROJECT_SYSROOT}/usr/lib/glib-2.0/include
	${PROJECT_SYSROOT}/usr/include/gstreamer-1.0
	${PROJECT_SYSROOT}/usr/include/json-c
	${PROJECT_SYSROOT}/usr/include/bluetooth
	${PROJECT_SYSROOT}/usr/include/dbus-1.0
	${PROJECT_SYSROOT}/usr/lib/dbus-1.0/include
	
	./vendor
	./vendor/homeserver
	./vendor/cloudphone
	./vendor/statuseventManage
	./vendor/key_event_handler
	./vendor/sai_voip_imp
	./thirdparty/
	./thirdparty/include
	./lib_src/aml_utils 
	./lib_src/aml_gstplayer 
	./lib_src/pcm_player_recorder 
	./lib_src/ledring
	./lib_src/key_event
	./lib_src/aml_core
	./lib_src/aml_volume
	./lib_src/network_cfg
	./lib_src/aml_btctl
	./lib_src/aml_downloader
	./lib_src/aml_tts
	./lib_src/aml_playermanager
)

link_directories(
	./thirdparty/lib/
	${PROJECT_SYSROOT}/usr/lib
	${PROJECT_SYSROOT}/usr/lib/gstreamer-1.0
	
	./thirdparty/sound_ai/lib
)

#编译so
file(GLOB_RECURSE AML_SUPPORT_LIB_SRC_C  "./lib_src/*.c")
file(GLOB_RECURSE AML_SUPPORT_LIB_SRC_CXX  "./lib_src/*.cpp")
file(GLOB_RECURSE AML_SUPPORT_LIB_SRC_CC  "./lib_src/*.cc")

add_library(${AML_SUPPORT_LIB_NAME} 
			SHARED 
			${AML_SUPPORT_LIB_SRC_C} ${AML_SUPPORT_LIB_SRC_CXX} ${AML_SUPPORT_LIB_SRC_CC}
)
target_link_libraries(${AML_SUPPORT_LIB_NAME}  libbootloader_message.a)


#设置链接so
set(LINK_LIBS
	sai_sdk sai_talk
	migu voipengine qlink   

	${AML_SUPPORT_LIB_NAME}
	gstaudio-1.0 gstreamer-1.0 curl ssl
	glib-2.0 gobject-2.0 gio-2.0 -ljson-c -lsqlite3 asound bluetooth
	
	pthread m rt dl   
)


#指定编译输出的执行文件和依赖的源文件
add_executable(${APP_TARGET_NAME} ${C_FILES_VENDOR} ${CXX_FILES_VENDOR} ${CC_FILES_VENDOR})
target_link_libraries(${APP_TARGET_NAME}  ${LINK_LIBS})

