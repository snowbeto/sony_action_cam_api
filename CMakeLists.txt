cmake_minimum_required(VERSION 2.8)

project( Sony_APPI )

set(CMAKE_BUILD_TYPE  Release)

set(CMAKE_CXX_FLAGS "-std=c++0x")
#Find openCV 
find_package( OpenCV REQUIRED )

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)


include_directories (
    include
    /home/daniel/curlcpp/include/
    ${Boost_INCLUDE_DIR}
    ${OpenCV_INCLUDE_DIRS}
)

#create executable
add_executable( sony.o
		src/test.cpp
		src/sony_camera.cpp
)

target_link_libraries(sony.o -lcurlcpp -lcurl ${OpenCV_LIBS})
