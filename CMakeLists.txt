cmake_minimum_required(VERSION 3.30)
project(OCR_in_Cpp)

set(CMAKE_CXX_STANDARD 20)

# Znajdź pakiet OpenCV
find_package(OpenCV REQUIRED)

# Dodaj katalogi z plikami nagłówkowymi OpenCV
include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(OCR_in_Cpp main.cpp)

# Linkuj biblioteki OpenCV do swojego projektu
target_link_libraries(OCR_in_Cpp ${OpenCV_LIBS})