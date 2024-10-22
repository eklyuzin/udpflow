# It's not part of Android Studio Project


cmake_minimum_required(VERSION 3.22.1)

set(CMAKE_CXX_STANDARD 17)

project(UdpFlowCApp)

set(UDPFLOW_LIBRARY_ONLY ON)
add_subdirectory(cpp)
add_subdirectory(cpp-console-app)
