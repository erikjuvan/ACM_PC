cmake_minimum_required(VERSION 3.0)
project(ADC_Sampler-PC)

set(CMAKE_CONFIGURATION_TYPES "Debug;Release") 
 
# Add user source files
file(GLOB SOURCES "src/*.cpp")
# Add user header files
file(GLOB HEADERS "include/*.h")

# Serial
set(SERIAL_ROOT_DIR "" CACHE PATH "Path to serial root directory")
find_library(SERIAL_LIBRARIES_RELEASE NAMES "serial" PATHS "${SERIAL_ROOT_DIR}/*" NO_DEFAULT_PATH)
find_library(SERIAL_LIBRARIES_DEBUG NAMES "serial-d" PATHS "${SERIAL_ROOT_DIR}/*" NO_DEFAULT_PATH)
set(SERIAL_LIBRARIES debug ${SERIAL_LIBRARIES_DEBUG} optimized ${SERIAL_LIBRARIES_RELEASE})
find_path(SERIAL_INCLUDE_DIR NAME "serial/serial.h" PATHS "${SERIAL_ROOT_DIR}/*" NO_DEFAULT_PATH)

# FFTW 3
set(FFTW3_ROOT_DIR "" CACHE PATH "Path to FFTW3 root directory")
find_library(FFTW3_LIBRARIES NAME "libfftw3-3" PATHS ${FFTW3_ROOT_DIR} NO_DEFAULT_PATH)
find_path(FFTW3_INCLUDE_DIR NAME "fftw3.h" PATHS ${FFTW3_ROOT_DIR} NO_DEFAULT_PATH)

link_directories(${SERIAL_LIB_DIR} ${FFTW3_LIB_DIR})
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})
target_include_directories(${PROJECT_NAME} PRIVATE include ${SERIAL_INCLUDE_DIR} ${FFTW3_INCLUDE_DIR})


if (WIN32)
target_link_libraries(${PROJECT_NAME} PRIVATE
	${SERIAL_LIBRARIES}
	${FFTW3_LIBRARIES})
endif ()
	
if (UNIX AND NOT APPLE)
target_link_libraries(${PROJECT_NAME} PRIVATE
	${SERIAL_LIBRARIES}
	${FFTW3_LIBRARIES})
endif ()

set(FFTW3_DLLS libfftw3-3.dll)

foreach(file_to_be_copied ${FFTW3_DLLS})
add_custom_command(
  TARGET ${PROJECT_NAME}
  POST_BUILD
  COMMAND ${CMAKE_COMMAND}
  ARGS -E copy_if_different ${FFTW3_ROOT_DIR}\\${FFTW3_DLLS}
  ${PROJECT_BINARY_DIR}/$<CONFIG>/${file_to_be_copied})
endforeach(file_to_be_copied) 

