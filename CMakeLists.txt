cmake_minimum_required(VERSION 3.16)
project(H_Passwd C)

set(CMAKE_C_STANDARD 11)
set(WriteKey " " CACHE STRING INTERNAL)
set(UserName "SpacialMan" CACHE STRING INTERNAL)
set(KeyTips "There isn't tips." CACHE STRING INTERNAL)

string(COMPARE NOTEQUAL ${WriteKey} " " have_key)

if (have_key)
    add_definitions(-DINCLUDE_KEY="${WriteKey}" -DUserName="${UserName}" -DKeyTips="${KeyTips}")
    message(STATUS "WriteKey = " ${WriteKey})
    message(STATUS "UserName = " ${UserName})
    message(STATUS "KeyTips = " ${KeyTips})
endif()

add_definitions(-DVERSION="0.0.1" -DVERSION_INFO="H-Password, now start.")
add_executable(H_Passwd main.c base64.c passwd.c argument.c random_passwd.c)