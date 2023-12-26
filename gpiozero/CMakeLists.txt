file(GLOB PUBLIC_HEADER_LIST CONFIGURE_DEPENDS "include/gpiozero/*.hpp" "include/g1-tank/*.h") 
file(GLOB SRC_LIST CONFIGURE_DEPENDS "*.cpp" "*.c") 

set (PROJECT_FILES 
  ${PUBLIC_HEADER_LIST}
  ${SRC_LIST})

add_library(gpiozero ${PROJECT_FILES})

target_link_libraries(gpiozero PRIVATE "${LGPIO}")

target_include_directories(gpiozero PUBLIC "./include")