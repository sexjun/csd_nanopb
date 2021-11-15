cmake_minimum_required(VERSION 3.0.0)
project(cds_nanopb VERSION 0.1.0)

include(CTest)
enable_testing()

option(USING_NANOPB "使用nanopb库" ON)
if(USING_NANOPB)
    set(NANOPB_SRC 
        ./nanopb/pb_common.c
        ./nanopb/pb_encode.c
        ./nanopb/cds_nanopb_info.pb.c
        ./nanopb/pb_decode.c)

    set(NANOPB_INCLUDE
        ./nanopb/pb.h
        ./nanopb/pb_common.h
        ./nanopb/pb_encode.h
        ./nanopb/cds_nanopb_info.pb.h
        ./nanopb/pb_decode.h)
    
    include_directories("./nanopb/")
    include_directories("./include")
    aux_source_directory("./src" CDS_SRC)
endif()

add_executable(cds_nanopb main.cpp ${NANOPB_SRC} ${CDS_SRC})


set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
