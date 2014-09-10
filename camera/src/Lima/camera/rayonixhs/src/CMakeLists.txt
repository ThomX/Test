cmake_minimum_required(VERSION 2.8)
project(RayonixHsLimaPlugin)

set (BUILD_SHARED_LIBS true)

set(ROOT_DIR ${CMAKE_SOURCE_DIR}/..)
set(LIMA_DIR ${ROOT_DIR}/../Lima)

set(BOOST_INCLUDE_DIRS ${ROOT_DIR}/third-party/boost/include)
set(BOOST_LIB_DIRS ${ROOT_DIR}/third-party/boost/lib)
set(LIMA_INCLUDE_DIRS ${LIMA_DIR}/hardware/include ${LIMA_DIR}/common/include)
set(LIMA_LIB_DIRS ${LIMA_DIR}/build ${LIMA_DIR}/third-party/Processlib/build)
set(RAYONIXHS_LIB_DIRS ${ROOT_DIR}/sdk/lib64)
set(RAYONIXHS_INCLUDE_DIRS ${ROOT_DIR}/sdk/include/craydl ${ROOT_DIR}/include)

if (NOT EXISTS ${LIMA_DIR})
	message(FATAL_ERROR "Did not find main Lima source tree at ${LIMA_DIR}.")
endif()

################################################################################################################################################################################
#set (CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -g -DNO_LIMA_DEBUG") # we want -g for release
set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Wall -g -DNO_LIMA_DEBUG -DWITH_SPS_IMAGE") # we want -g for release

#set (CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0") #cmake auto adds "-g" for Debug
set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -O0 -DWITH_SPS_IMAGE") #cmake auto adds "-g" for Debug

set(ALL_INCLUDES ${BOOST_INCLUDE_DIRS} ${LIMA_INCLUDE_DIRS} ${RAYONIXHS_INCLUDE_DIRS})
set(ALL_LIBS ${RAYONIXHS_LIB_DIRS} ${LIMA_LIB_DIRS} ${BOOST_LIB_DIRS})
#message("All libs: ${ALL_LIBS}")

#separate_arguments(ALL_INCLUDES_SEP UNIX_COMMAND "${ALL_INCLUDES}")
#message(ALL_INCLUDES: ${ALL_INCLUDES_SEP})
#separate_arguments(ALL_LIBS_SEP UNIX_COMMAND "${ALL_LIBS}")
#message(ALL_LIBS: ${ALL_LIBS_SEP})

include_directories(${ALL_INCLUDES})
link_directories(${ALL_LIBS})

set(CPP_FILES RayonixHsBufferCtrlObj.cpp RayonixHsCamera.cpp  RayonixHsDetInfoCtrlObj.cpp  RayonixHsInterface.cpp
	      RayonixHsSyncCtrlObj.cpp FrameStatusCb.cpp RayonixHsBinCtrlObj.cpp)

add_library(RayonixHs ${CPP_FILES})

find_library(LimaLib limacore ${ALL_LIBS})
if (NOT EXISTS ${LimaLib})
   message(FATAL_ERROR "Could not find limacore lima library!")
else()
   message("Found LimaLib at ${LimaLib}")
endif (NOT EXISTS ${LimaLib})

find_library(ProcessLib processlib ${ALL_LIBS})
if (NOT EXISTS ${ProcessLib})
   message(FATAL_ERROR "Could not find processlib library!")
else()
   message("Found ProcessLib at ${ProcessLib}")
endif (NOT EXISTS ${ProcessLib})

#find_package(CUDA)

#if (CUDA_FOUND)
#	message("Found nVidia CUDA.  Using regular craydl library.")

find_library(craydlLib craydl ${ALL_LIBS})
if (NOT EXISTS ${craydlLib})
	message(FATAL_ERROR "Could not find craydl library!")
else()
	message("Found craydlLib at ${craydlLib}")
endif (NOT EXISTS ${craydlLib})

#else()
#	message("nVidia CUDA NOT found.  Using craydl_nocuda library for testing
#	purposes.")
#	find_library(craydlNoCudaLib craydl_nocuda ${ALL_LIBS})
#	if (NOT EXISTS ${craydlNoCudaLib})
#		message(FATAL_ERROR "Could not find craydl_nocuda library!")
#	else()
#		message("Found craydlNoCudaLib at ${craydlNoCudaLib}")
#	endif (NOT EXISTS ${craydlNoCudaLib})
#endif()

add_executable(test test.cpp)
target_link_libraries(test RayonixHs ${LimaLib} ${ProcessLib} ${SpecInstLib} ${craydlLib} ${craydlGpuLib} ${cbfLib} ${OPENSSL_LIBRARIES} gsl dl)

