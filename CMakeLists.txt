cmake_minimum_required(VERSION 3.16)
project(MEI)

set(CMAKE_CXX_STANDARD 17)

set(WITH_CUDA OFF)
set(WITH_OPENVINO ON)
set(GRAPHIC_DEBUG ON)
set(WITH_SERIAL OFF)

# realsense
include_directories(/usr/include)

# opencv
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

if (WITH_CUDA)
    # cuda & cudnn
    include_directories(/usr/local/cuda/include)
    # tensorrt
    include_directories(/home/ywh/TensorRT-8.6.1.6/include)
    # add definition
    add_definitions(-DWITH_CUDA)
    message("using cuda and tensorrt")
elseif (WITH_OPENVINO)
    # openvino
    set(OpenVINO_DIR /home/ywh/openvino/runtime/cmake)
    find_package(OpenVINO REQUIRED)
    include_directories(/home/ywh/openvino/runtime/include)
    # add definition
    add_definitions(-DWITH_OPENVINO)
    message("using openvino")
else ()
    message(FATAL_ERROR "No cuda libs or openvino libs was found")
endif ()

# MEI
if (GRAPHIC_DEBUG)
    add_definitions(-DGRAPHIC_DEBUG)
endif ()

if (WITH_SERIAL)
    add_definitions(-DWITH_SERIAL)
endif ()

include_directories(include)
file(GLOB_RECURSE SRC_FILES src/*.cpp)
add_executable(${PROJECT_NAME}
        ${SRC_FILES}
        main.cpp
)

target_link_libraries(${PROJECT_NAME}
        ${OpenCV_LIBS}
        realsense2
)

if (WITH_CUDA)
    target_link_libraries(${PROJECT_NAME}
            cudart
            nvinfer
    )
elseif (WITH_OPENVINO)
    target_link_libraries(${PROJECT_NAME}
            openvino::runtime
    )
endif ()