#-------------------------------------------------------------------------------
#	CMAKE SETTINGS
#-------------------------------------------------------------------------------

cmake_minimum_required(VERSION 2.8.12)

project("receiver")

set(LIBRARIES
)

if(WIN32)
    list(APPEND LIBRARIES
        wsock32
        ws2_32
    )
else()
    list(APPEND LIBRARIES
        pthread
    )
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
endif()


set(SOURCES
	CommonLibsAndFuncs.cpp
	OStreamer.cpp
	FileWriter.cpp
	)
	
set(HEADERS
	CommonSys.h
	CommonLibsAndFuncs.h
	OStreamer.h 
	FileWriter.h
	)

add_executable(receiver trafreceive.cpp)
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT receiver)

add_library(lib STATIC ${HEADERS} ${SOURCES})

target_link_libraries(receiver ${LIBRARIES})
target_link_libraries(receiver lib)
