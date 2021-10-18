cmake_minimum_required(VERSION 2.6)

PROJECT(newplat)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11 -Os -Wall -Werror -Wmaybe-uninitialized -fbounds-check")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -Os -Wall -Werror -D_GNU_SOURCE -D__USE_XOPEN -Wmissing-declarations")
set(CMAKE_VERBOSE_MAKEFILE ON)

INCLUDE_DIRECTORIES(include)

add_definitions(-DADVANCED_SENSING)

aux_source_directory(./ code)
ADD_EXECUTABLE(newplat ${code})
TARGET_LINK_LIBRARIES(newplat 
	mosquitto 
	pthread 
	dl 
	cjson 
	ubox 
	uci 
	ubus 
	crypto
	)

INSTALL(TARGETS newplat
	RUNTIME DESTINATION sbin
)
