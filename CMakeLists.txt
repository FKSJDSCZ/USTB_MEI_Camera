cmake_minimum_required(VERSION 3.16)
project(MEI)

set(CMAKE_CXX_STANDARD 20)

set(WITH_CUDA ON)
set(WITH_OPENVINO OFF)
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
    file(GLOB CUDA_LIBS /usr/local/cuda/lib64/*.so)
    # tensorrt
    include_directories(/home/ywh/TensorRT-8.6.1.6/include)
    file(GLOB TENSORRT_LIBS /home/ywh/TensorRT-8.6.1.6/lib/*.so)
    # cvcuda & nvcv_types
    include_directories(/home/ywh/cvcuda/include)
    file(GLOB CVCUDA_LIBS /home/ywh/cvcuda/lib/x86_64-linux-gnu/*.so)
    # libtorch
    #    include_directories(/home/ywh/libtorch/include)
    #    include_directories(/home/ywh/libtorch/include/torch/csrc/api/include)
    #    file(GLOB TORCH_LIBS /home/ywh/libtorch/lib/*.so)
    # torchvision
    #    include_directories(/home/ywh/torchvision/include)
    #    file(GLOB TORCHVISION_LIBS /home/ywh/torchvision/lib/*.so)
    # add definition
    add_definitions(-DWITH_CUDA)
    message("Using cuda")
elseif (WITH_OPENVINO)
    # openvino
    set(OpenVINO_DIR /home/ywh/openvino/runtime/cmake)
    find_package(OpenVINO REQUIRED)
    include_directories(/home/ywh/openvino/runtime/include)
    # add definition
    add_definitions(-DWITH_OPENVINO)
    message("Using openvino")
endif ()

# add definitions
if (GRAPHIC_DEBUG)
    add_definitions(-DGRAPHIC_DEBUG)
endif ()

if (WITH_SERIAL)
    add_definitions(-DWITH_SERIAL)
endif ()

# MEI
include_directories(include)
file(GLOB_RECURSE SRC_FILES src/*.cpp)
add_executable(${PROJECT_NAME}
        ${SRC_FILES}
        main.cpp
        #        test.cpp
        #        rename.cpp
)

target_link_libraries(${PROJECT_NAME}
        ${OpenCV_LIBS}
        realsense2
        pthread
)

if (WITH_CUDA)
    target_link_libraries(${PROJECT_NAME}
            ${CUDA_LIBS}
            ${TENSORRT_LIBS}
            ${CVCUDA_LIBS}
            #            ${TORCH_LIBS}
            #            ${TORCHVISION_LIBS}
    )
elseif (WITH_OPENVINO)
    target_link_libraries(${PROJECT_NAME}
            openvino::runtime
    )
endif ()
