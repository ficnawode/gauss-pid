cmake_minimum_required(VERSION 3.10)
project(GAUSPID CXX)
if(NOT DEFINED CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 20)
endif()
set(PROJECT_VERSION 1.0)

macro(add_target_property _target _prop _value)
   get_target_property(_oldprop "${_target}" ${_prop})
   if(NOT _oldprop)
      set_target_properties("${_target}" PROPERTIES ${_prop} "${_value}")
   else(NOT _oldprop)
      set_target_properties("${_target}" PROPERTIES ${_prop} "${_oldprop} ${_value}")
   endif(NOT _oldprop)
endmacro(add_target_property)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
message(STATUS "Using C++${CMAKE_CXX_STANDARD}")

# by default build optimized code with debug symbols
if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE RELWITHDEBINFO)
endif ()

# in DEBUG mode make verbose Makefile
if (CMAKE_BUILD_TYPE MATCHES DEBUG)
    set(CMAKE_VERBOSE_MAKEFILE ON)
endif ()

# set(CMAKE_CXX_FLAGS_DEBUG "-O0 -ggdb -g -DDEBUG -D__DEBUG -Wall")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -ggdb -DDEBUG -D__DEBUG -Wall")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELEASE} -ggdb")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -ftree-vectorize -ffast-math -DNODEBUG")
message(STATUS "Using CXX flags for ${CMAKE_BUILD_TYPE}: ${CMAKE_CXX_FLAGS_${CMAKE_BUILD_TYPE}}")
list(APPEND CMAKE_PREFIX_PATH $ENV{ROOTSYS})

find_package(AnalysisTree REQUIRED)
find_package(ROOT REQUIRED RIO)

if (ROOT_FOUND)
    message(STATUS "Using ROOT: ${ROOT_VERSION} <${ROOT_CONFIG}>")
    include_directories(${CMAKE_SOURCE_DIR} ${ROOT_INCLUDE_DIR} ${ROOT_INCLUDE_DIRS})
    include(${ROOT_USE_FILE})
endif (ROOT_FOUND)

include_directories(${CMAKE_SOURCE_DIR} ${AnalysisTree_INCLUDE_DIR})

#************
set(SOURCES
  src/GAUSPIDFit1D.cpp
  src/GAUSPIDFit2D.cpp
    src/fit.cpp
  )

set(HEADERS
  src/name_helpers.hpp
  src/GAUSPIDFit1D.hpp
  src/GAUSPIDFit2D.hpp
  )

add_library(GAUSPID SHARED ${SOURCES} G__GAUSPID.cxx)
        
if(ROOT_FOUND)
  ROOT_GENERATE_DICTIONARY(G__GAUSPID ${HEADERS} LINKDEF src/GAUSPIDLinkDef.h OPTIONS "-DDO_TPCCATRACKER_EFF_PERFORMANCE")
  include_directories(${CMAKE_SOURCE_DIR})
  target_link_libraries(GAUSPID ${ROOT_LIBRARIES} AnalysisTreeBase AnalysisTreeInfra)
endif(ROOT_FOUND)  
add_target_property(GAUSPID COMPILE_FLAGS "-DDO_TPCCATRACKER_EFF_PERFORMANCE")

find_package(Boost 1.78 REQUIRED COMPONENTS json)
target_link_libraries(GAUSPID Boost::boost Boost::json)

add_executable(gauss_fit src/fit.cpp)
add_executable(gauss_infer src/infer.cpp)
add_dependencies(gauss_fit GAUSPID)
add_dependencies(gauss_infer GAUSPID)
add_target_property(gauss_fit COMPILE_FLAGS "-DDO_TPCCATRACKER_EFF_PERFORMANCE")
add_target_property(gauss_infer COMPILE_FLAGS "-DDO_TPCCATRACKER_EFF_PERFORMANCE")
target_link_libraries(gauss_fit GAUSPID)
target_link_libraries(gauss_infer GAUSPID)

install(TARGETS GAUSPID EXPORT GAUSPIDTargets
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
        INCLUDES DESTINATION include
        )

install(
        FILES
        ${HEADERS}
        DESTINATION
        include
        COMPONENT
        Devel
)

set(PCM_FILE_NAME libGAUSPID)

install(
        FILES
        "${CMAKE_CURRENT_BINARY_DIR}/${PCM_FILE_NAME}_rdict.pcm"
        DESTINATION
        lib
        OPTIONAL
)

install (TARGETS gauss_fit RUNTIME DESTINATION bin)
install (TARGETS gauss_infer RUNTIME DESTINATION bin)
#************

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/GAUSPID/GAUSPIDConfigVersion.cmake"
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY AnyNewerVersion
)

export(EXPORT GAUSPIDTargets
        FILE "${CMAKE_CURRENT_BINARY_DIR}/GAUSPID/GAUSPIDTargets.cmake")

message(STATUS "GAUSPIDTargets: ${GAUSPIDTargets}")
        
set(ConfigPackageLocation lib/cmake/GAUSPID)

set(GAUSPID_INCLUDE_DIR "include")
set(GAUSPID_LIBRARY_DIR "lib")

configure_package_config_file(
        src/GAUSPIDConfig.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/GAUSPID/GAUSPIDConfig.cmake
        INSTALL_DESTINATION ${ConfigPackageLocation}
        PATH_VARS GAUSPID_INCLUDE_DIR GAUSPID_LIBRARY_DIR
)

install(EXPORT GAUSPIDTargets
        FILE
        GAUSPIDTargets.cmake
        DESTINATION
        ${ConfigPackageLocation}
        )

install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/GAUSPID/GAUSPIDConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/GAUSPID/GAUSPIDConfigVersion.cmake"
        DESTINATION
        ${ConfigPackageLocation}
        COMPONENT
        Devel
        )
