cmake_minimum_required(VERSION 3.16)
project(H_Passwd C)

set(CMAKE_C_STANDARD 11)

add_definitions(-DVERSION="0.0.1" -DVERSION_INFO="H-Password, now start.")
add_executable(H_Passwd main.c base64.c passwd.c argument.c)