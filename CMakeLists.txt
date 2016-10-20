cmake_minimum_required(VERSION 2.8)
project(revenant)
include(aux.cmake)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0 -ggdb3 -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_C_FLAGS_DEBUG} -O3")
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -std=c++14 -Wall -Wextra -ftemplate-depth=1024")
add_definitions(-DUNIX)
add_definitions(-DSSE=${SSE})
# デバッグビルドの際はGLibのデバッグモードをONにする
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	add_definitions(-D_GLIBCXX_DEBUG)
endif()

# windows依存ファイル
set(DEPENDANT_SRC_WIN dir_depWin.cpp)
# unix依存ファイル
set(DEPENDANT_SRC_UNIX dir_depLinux.cpp)
set(OS_DEPENDANT_SRC ${DEPENDANT_SRC_WIN} ${DEPENDANT_SRC_UNIX})
# 依存ファイル以外のソースファイルを収集
file(GLOB LIBSRC_FILE RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp)
list(REMOVE_ITEM LIBSRC_FILE ${OS_DEPENDANT_SRC})
# OS依存ファイルの設定
if(${WIN32})
	set(LIBSRC_FILE ${LIBSRC_FILE} ${DEPENDANT_SRC_WIN})
elseif(${UNIX})
	set(LIBSRC_FILE ${LIBSRC_FILE} ${DEPENDANT_SRC_UNIX})
else()
	message(FATAL_ERROR "couldn't detect OS type")
endif()

include_directories(
	./
	./frea
	./lubee
)
add_library(revenant SHARED ${LIBSRC_FILE})
set_target_properties(revenant PROPERTIES VERSION 1.0.0 SOVERSION 1)
install(TARGETS revenant LIBRARY DESTINATION lib)

if(NOT without-test)
	enable_testing()
	find_package(Threads REQUIRED)
	find_package(GTest REQUIRED)
	include_directories(
		${GTEST_INCLUDE_DIRS}
	)
	# testsディレクトリ以下のソースが対象
	aux_source_directory(tests TEST_SRC)
	foreach(SRC IN LISTS TEST_SRC)
		GetFileName(${SRC}, SRCNAME)
		AddTest(${SRCNAME} ${SRC})
	endforeach()
endif()
