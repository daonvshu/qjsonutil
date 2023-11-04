cmake_minimum_required(VERSION 3.12)

project(qjsonutil_test)

set(CMAKE_AUTOMOC ON)
SET(CMAKE_INCLUDE_CURRENT_DIR ON)

#查找qt模块
find_package(Qt5Test REQUIRED)

enable_testing(true)
#设置编译源文件
add_executable(${PROJECT_NAME}
    dataconverttest.cpp
  )
add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME})

#链接库
target_link_libraries(${PROJECT_NAME} Qt5::Test)