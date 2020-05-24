cmake_minimum_required(VERSION 3.12.0)

project("Vulcro")
SET (SOURCE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/Vulcro/src")
SET (LIB_PATH "${CMAKE_CURRENT_SOURCE_DIR}/Lib")

OPTION (VULCRO_INCLUDE_GLM_SDL "Include GLM + SDL" TRUE)
OPTION (VULCRO_BUILD_SAMPLES "Build Samples" TRUE)

file(GLOB_RECURSE src_cpp
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    "${SOURCE_PATH}/*.cpp"
    "${SOURCE_PATH}/*.h"
)

foreach(source IN LISTS src_cpp)
    get_filename_component(source_path "${source}" PATH)
    string(REPLACE "/" "\\" source_path_msvc "${source_path}")
    source_group("${source_path_msvc}" FILES "${source}")
endforeach()

add_library(vulcro-lib ${src_cpp})

include_directories( ${SOURCE_PATH} )

if (VULCRO_INCLUDE_GLM_SDL)
include_directories( ${LIB_PATH} )
endif()


if(BUILD_SAMPLES)

    SET (SAMPLES_PATH "${CMAKE_CURRENT_SOURCE_DIR}/Samples")

    include_directories ("${SAMPLES_PATH}")
     
    macro(add_test_executable _testname _testfolder _testfile)
      add_executable (${_testname} "${SAMPLES_PATH}/${_testfolder}/${_testfile}")
      set_target_properties (${_testname} PROPERTIES FOLDER "Samples")
      target_link_libraries (${_testname} vulcro-lib)
      target_link_libraries(${_testname}  "${LIB_PATH}/vulkan-1.lib")
      target_link_libraries(${_testname}  "${LIB_PATH}/SDL2.lib")
      target_include_directories(${_testname} PUBLIC ${LIB_PATH})

      if (WIN32)
          set_target_properties(${_testname} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${SAMPLES_PATH}/${_testfolder}/")
      endif ()
    endmacro(add_test_executable)

    add_test_executable ("triangle" "Triangle" "main.cpp")
    add_test_executable ("grass" "Grass" "main.cpp")
    add_test_executable ("raytracing" "Raytracing" "main.cpp")


endif(BUILD_SAMPLES)