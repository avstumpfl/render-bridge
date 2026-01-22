cmake_minimum_required(VERSION 3.14)
project(RXExt)

# optionally set directory of RX installation to deploy to
#set(RX_DIR "C:/Program Files/AV Stumpfl/Pixera/build_25-1_INTER_9/RX")

# optionally start Pixera relative to RX directory
#set(RX_RUN_PIXERA TRUE)

#set(OMIT_SAMPLE_PROJECTS true)

include(RenderBridge/CMakeLists.txt)
set(EXTENSIONS_DIR ${CMAKE_CURRENT_SOURCE_DIR})

#--------------------------------------------------------------------------

rx_extension("ExtTemplate")
  target_include_directories(${PROJECT} PRIVATE
    ${PROJECT_DIR}/src
  )
  target_link_directories(${PROJECT} PRIVATE
    ${PROJECT_DIR}/libs
  )

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT})
