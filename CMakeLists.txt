cmake_minimum_required(VERSION 3.10)
project(DisplayImage)

# Find OpenCV package
find_package(OpenCV REQUIRED)

# Include OpenCV headers
include_directories(${OpenCV_INCLUDE_DIRS})

# Add executable
add_executable(display_image main.cpp)

# Link OpenCV libraries
target_link_libraries(display_image ${OpenCV_LIBS})