cmake_minimum_required(VERSION 3.14)
project("RXExt" LANGUAGES C CXX)

set(CMAKE_CONFIGURATION_TYPES "Debug" "Release")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
  add_compile_options(/GR- /GL /Os /W3 /MP /wd4100)
  add_link_options($<$<CONFIG:RELEASE>:/LTCG>)
  add_compile_options($<$<CONFIG:RELEASE>:/MT>)
  add_compile_definitions(_CRT_SECURE_NO_WARNINGS _WINSOCK_DEPRECATED_NO_WARNINGS)
else()
  add_compile_options(-Wall -Wpedantic -Wno-unknown-pragmas)
endif()

if(NOT DEFINED RX_DIR)
  set(RX_DIR ${CMAKE_CURRENT_LIST_DIR}/RX)
endif()
set(RXEXT_INCLUDES_DIR ${CMAKE_CURRENT_LIST_DIR}/include)
set(EXTENSIONS_DIR ${CMAKE_CURRENT_LIST_DIR}/extensions)
set(LIBS_DIR ${CMAKE_CURRENT_LIST_DIR}/libs)

macro("rx_extension" project)
  set(PROJECT ${project})
  set(PROJECT_DIR ${EXTENSIONS_DIR}/${PROJECT})

  file(GLOB_RECURSE sources ${PROJECT_DIR}/*.cpp ${PROJECT_DIR}/*.c ${PROJECT_DIR}/*.h ${PROJECT_DIR}/*.lua)
  file(GLOB_RECURSE headers ${RXEXT_INCLUDES_DIR}/*.h)
  file(GLOB_RECURSE lib_sources ${LIBS_DIR}/*.cpp)
  file(GLOB_RECURSE lib_headers ${LIBS_DIR}/*.h)
  add_library(${PROJECT} SHARED ${sources} ${headers} ${lib_sources} ${lib_headers})
  source_group(TREE ${CMAKE_CURRENT_LIST_DIR} FILES ${headers} ${lib_sources} ${lib_headers})
  source_group(TREE ${PROJECT_DIR} FILES ${sources})
  
  target_include_directories(${PROJECT} PRIVATE . ${RXEXT_INCLUDES_DIR} ${LIBS_DIR})
  
  set_target_properties(${PROJECT} PROPERTIES
      RUNTIME_OUTPUT_DIRECTORY "$<$<CONFIG:Release>:${RX_DIR}/modules>$<$<CONFIG:Debug>:${RX_DIR}/modules>")

  if(RX_RUN_PIXERA)
    set_target_properties(${PROJECT} PROPERTIES
      VS_DEBUGGER_COMMAND "${RX_DIR}/../Pixera.exe"
      VS_DEBUGGER_WORKING_DIRECTORY "${RX_DIR}/..")
  else()
    set_target_properties(${PROJECT} PROPERTIES
      VS_DEBUGGER_COMMAND "${RX_DIR}/RX.exe"
      VS_DEBUGGER_WORKING_DIRECTORY "${RX_DIR}"
      VS_DEBUGGER_COMMAND_ARGUMENTS "-script=${PROJECT_DIR}/run.lua")
  endif()
endmacro()

#--------------------------------------------------------------------------

if(NOT OMIT_SAMPLE_PROJECTS)
  rx_extension("ExtSampleCPU")
  
  rx_extension("ExtSampleShader")
  
  rx_extension("ExtSampleVK")
    target_compile_definitions(${PROJECT} PRIVATE VK_USE_PLATFORM_WIN32_KHR _CRT_SECURE_NO_WARNINGS)
    target_include_directories(${PROJECT} PRIVATE ${PROJECT_DIR}/libs/vulkan/include ${PROJECT_DIR}/src/piglit)
    target_link_directories(${PROJECT} PRIVATE ${PROJECT_DIR}/libs/vulkan/win64)
    target_link_libraries(${PROJECT} PRIVATE vulkan-1.lib)
    
  rx_extension("ExtNDI")
    target_include_directories(${PROJECT} PRIVATE ${PROJECT_DIR}/libs/NDI/Include)
    target_link_directories(${PROJECT} PRIVATE ${PROJECT_DIR}/libs/NDI/Lib/x64)
    target_link_libraries(${PROJECT} PRIVATE Processing.NDI.Lib.x64.lib)
  
  rx_extension("ExtNotch")
    target_include_directories(${PROJECT} PRIVATE ${PROJECT_DIR}/libs)
    target_sources(${PROJECT} PRIVATE ${PROJECT_DIR}/libs/notch/NotchBlock.cpp)
    target_link_libraries(${PROJECT} PRIVATE d3d11.lib dxgi.lib opengl32.lib)
    
  rx_extension("ExtSpout")
    target_include_directories(${PROJECT} PRIVATE ${PROJECT_DIR}/libs)
    target_link_libraries(${PROJECT} PRIVATE d3d11.lib dxgi.lib opengl32.lib)
  
  rx_extension("ExtSampleD3D")
    target_link_libraries(${PROJECT} d3d11.lib dxgi.lib d3dcompiler.lib)
  
  # make last project active
  set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT})
endif()
