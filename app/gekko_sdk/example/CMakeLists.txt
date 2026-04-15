CMAKE_MINIMUM_REQUIRED(VERSION 3.0)
SET(CMAKE_SYSTEM_NAME Linux)

SET(CMAKE_CROSS_COMPILER "mips-linux-gnu-")

SET(CMAKE_C_COMPILER "${CMAKE_CROSS_COMPILER}gcc")
SET(CMAKE_CXX_COMPILER "${CMAKE_CROSS_COMPILER}g++")

project(sample)

SET(CMAKE_C_FLAGS "-EL -O3 -march=mips32r2 -funroll-all-loops -flax-vector-conversions")
SET(CMAKE_CXX_FLAGS "-EL -O3 -std=c++11 -march=mips32r2 -funroll-all-loops -flax-vector-conversions")

SET(GEKKO_SDK_PATH "${PROJECT_SOURCE_DIR}/../install/")
SET(ISVP_SDK_PATH "${PROJECT_SOURCE_DIR}/../../sdk-lv3/")


INCLUDE_DIRECTORIES(${GEKKO_SDK_PATH}/include)
INCLUDE_DIRECTORIES(${ISVP_SDK_PATH}/include)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/)

SET(BIN_SRCS sample-Encoder-video.c sample-common.c)

SET(DEP_LIB_DIRS ${GEKKO_SDK_PATH}/lib/uclibc)
SET(SDK_LIB_DIRS ${ISVP_SDK_PATH}/lib/uclibc)
SET(DEP_GLIB_DIRS ${GEKKO_SDK_PATH}/lib/glibc)
SET(SDK_GLIB_DIRS ${ISVP_SDK_PATH}/lib/glibc)

SET(sample_dep_shared_uclibc
    ${SDK_LIB_DIRS}/libalog.a
    ${SDK_LIB_DIRS}/libimp.a
    ${DEP_LIB_DIRS}/libdrivers.so
    ${DEP_LIB_DIRS}/libgekko.so
    stdc++
	rt
	pthread
	m
	dl
)

SET(sample_dep_static_uclibc
    ${SDK_LIB_DIRS}/libalog.a
    ${SDK_LIB_DIRS}/libimp.a
    -Wl,-whole-archive ${DEP_LIB_DIRS}/libgekko.a -Wl,-no-whole-archive
    ${DEP_LIB_DIRS}/libdrivers.a
    stdc++
	rt
	pthread
	m
	dl
)

SET(sample_dep_shared_glibc
    ${SDK_GLIB_DIRS}/libalog.a
    ${SDK_GLIB_DIRS}/libimp.a
    ${DEP_GLIB_DIRS}/libdrivers.so
    ${DEP_GLIB_DIRS}/libgekko.so
    stdc++
	rt
	pthread
	m
	dl
)

SET(sample_dep_static_glibc
    ${SDK_GLIB_DIRS}/libalog.a
    ${SDK_GLIB_DIRS}/libimp.a
    -Wl,-whole-archive ${DEP_LIB_DIRS}/libgekko.a -Wl,-no-whole-archive
    ${DEP_GLIB_DIRS}/libdrivers.a
    stdc++
	rt
	pthread
	m
	dl
)

#######################generate aisp_3dnr bin ###################################
ADD_EXECUTABLE(bin_sample_shared_uclibc ${BIN_SRCS})
ADD_EXECUTABLE(bin_sample_static_uclibc ${BIN_SRCS})
ADD_EXECUTABLE(bin_sample_shared_glibc ${BIN_SRCS})
ADD_EXECUTABLE(bin_sample_static_glibc ${BIN_SRCS})

TARGET_LINK_LIBRARIES(
  bin_sample_shared_uclibc
  ${sample_dep_shared_uclibc}
  )
TARGET_LINK_LIBRARIES(
  bin_sample_static_uclibc
  ${sample_dep_static_uclibc}
  )

TARGET_LINK_LIBRARIES(
  bin_sample_shared_glibc
  ${sample_dep_shared_glibc}
  )
TARGET_LINK_LIBRARIES(
  bin_sample_static_glibc
  ${sample_dep_static_glibc}
  )

SET_TARGET_PROPERTIES(bin_sample_shared_uclibc PROPERTIES
    OUTPUT_NAME "sample_shared_uclibc"
    LINK_FLAGS "-muclibc"
    COMPILE_FLAGS "-muclibc"
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin 
)

SET_TARGET_PROPERTIES(bin_sample_static_uclibc PROPERTIES
    OUTPUT_NAME "sample_static_uclibc"
    LINK_FLAGS "-muclibc"
    COMPILE_FLAGS "-muclibc"
	RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

SET_TARGET_PROPERTIES(bin_sample_shared_glibc PROPERTIES
    OUTPUT_NAME "sample_shared_glibc"
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin 
)

SET_TARGET_PROPERTIES(bin_sample_static_glibc PROPERTIES
    OUTPUT_NAME "sample_static_glibc"
	RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)


