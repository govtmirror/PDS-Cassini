cmake_minimum_required(VERSION 2.8)

PROJECT(PropellerDetector)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wno-deprecated -fpermissive -O3")

include_directories(".")
set(CMAKE_EXE_LINKER_FLAGS "-lpthread -pthread")


add_executable(PropellerTrain train.cpp propellerdetect.cpp helper.cpp labelparser.cpp indextabparser.cpp pdsimagereader.cpp pdsimageparameters.cpp CSV/CSVread.cpp CSV/CSVwrite.cpp CSV/strerror.cpp CSV/libcsv.c)

add_executable(PropellerTest test.cpp propellerdetect.cpp helper.cpp labelparser.cpp indextabparser.cpp pdsimagereader.cpp pdsimageparameters.cpp CSV/CSVread.cpp CSV/CSVwrite.cpp CSV/strerror.cpp CSV/libcsv.c)


add_executable(PropellerStats stats.cpp propellerdetect.cpp helper.cpp labelparser.cpp indextabparser.cpp pdsimagereader.cpp pdsimageparameters.cpp CSV/CSVread.cpp CSV/CSVwrite.cpp CSV/strerror.cpp CSV/libcsv.c)





if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    # require at least gcc 4.8
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.9)
        message(FATAL_ERROR "GCC version must be at least 4.9!")
    endif()

endif()

