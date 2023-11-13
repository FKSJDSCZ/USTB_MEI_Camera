cmake_minimum_required(VERSION 3.16)
project(MEI)

set(CMAKE_CXX_STANDARD 17)

# cuda & cudnn
include_directories(/usr/local/cuda/include)

# tensorrt
include_directories(/home/ywh/TensorRT-8.6.1.6/include)

# realsense
include_directories(/usr/include)

# opencv
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

# MEI
include_directories(include)
file(GLOB SRC_FILES src/*.cpp)
add_executable(MEI main.cpp)

target_link_libraries(MEI ${OpenCV_LIBS} realsense2 nvinfer cudart)
#set_target_properties(MEI PROPERTIES CUDA_SEPARABLE_COMPILATION ON)
