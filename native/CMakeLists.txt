cmake_minimum_required(VERSION 3.14.0 FATAL_ERROR)

set(PROJECT_NAME jdk-21-crash)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(OUTPUT_DEBUG bin/debug)
set(OUTPUT_RELEASE bin/release)

project(${PROJECT_NAME} CXX C)

find_package(JNI)

if (JNI_FOUND)
    message(STATUS "JNI_INCLUDE_DIRS=${JNI_INCLUDE_DIRS}")
    message(STATUS "JNI_LIBRARIES=${JNI_LIBRARIES}")

    if (JNI_LIBRARIES MATCHES ".*jdk1\\.8.*")
        message("FOUND JAVA 1.8!")
    elseif (JNI_LIBRARIES MATCHES ".*java-8-openjdk-.*")
        message("FOUND JAVA 1.8!")
    elseif (JNI_LIBRARIES MATCHES ".*jdk-8.*")
        message("FOUND JAVA 1.8!")
    elseif (JNI_LIBRARIES MATCHES ".*zulu-8.jdk.*")
        message("FOUND JAVA 1.8!")
    else ()
        message(FATAL_ERROR "ERROR! Java 1.8 not detected, check your path!!! Newer Java versions will not work.")
    endif ()
endif ()

include_directories(.)
include_directories(${JNI_INCLUDE_DIRS})


set(HEADERS_FILES
        CustomJVMTI.h
        dllmain.h
        jvmagent.h
)

set(SRC_FILES
        CustomJVMTI.cpp
        dllmain.cpp
        jvmagent.cpp
)

source_group("Headers" FILES ${HEADERS_FILES} ${COMMON_HEADERS_FILES} ${COMMON_GRPC_HEADERS_FILES} ${TINY_PROC_LIB_HEADER_FILES})
source_group("Sources" FILES ${SRC_FILES} ${COMMON_SRC_FILES} ${COMMON_GRPC_SRC_FILES} ${TINY_PROC_LIB_SRC_FILES})

add_library(${PROJECT_NAME} SHARED
        ${HEADERS_FILES} ${SRC_FILES}
        ${COMMON_HEADERS_FILES} ${COMMON_SRC_FILES}
        ${COMMON_GRPC_HEADERS_FILES} ${COMMON_GRPC_SRC_FILES}
        ${TINY_PROC_LIB_HEADER_FILES} ${TINY_PROC_LIB_SRC_FILES}
)

if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    target_compile_definitions(${PROJECT_NAME} PRIVATE
            -DSLEEP
    )
endif ()

target_link_libraries(${PROJECT_NAME} ${JNI_LIBRARIES})
