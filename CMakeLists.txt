cmake_minimum_required(VERSION 3.0)
project(ADC_Sampler-PC)

include_directories(include)

#Can manually add the sources using the set command as follows:
#set(SOURCES src/mainapp.cpp src/Student.cpp)
 
#However, the file(GLOB...) allows for wildcard additions:
file(GLOB SOURCES "src/*.cpp")
file(GLOB HEADERS "include/*.h")

# External library include directories
set(SERIAL_INCLUDE_DIR "" CACHE STRING "Path to serial include directory")
include_directories(${SERIAL_INCLUDE_DIR})

set(FFTW3_INCLUDE_DIR "" CACHE STRING "Path to FFTW3 include directory")
include_directories(${FFTW3_INCLUDE_DIR})

# External library lib directories
set(SERIAL_LIB_DIR "" CACHE STRING "Path to serial lib directory")
link_directories(${SERIAL_LIB_DIR})

set(FFTW3_LIB_DIR "" CACHE STRING "Path to FFTW3 lib directory")
link_directories(${FFTW3_LIB_DIR})

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

if (WIN32)
target_link_libraries(${PROJECT_NAME} debug serial-d.lib optimized serial.lib
	debug libfftw3-3.lib optimized libfftw3-3.lib)
endif ()

if (UNIX AND NOT APPLE)
target_link_libraries(${PROJECT_NAME} debug serial-d optimized serial
	debug fftw3 optimized fftw3)
endif ()
