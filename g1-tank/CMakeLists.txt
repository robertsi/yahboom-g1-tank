file(GLOB PUBLIC_HEADER_LIST CONFIGURE_DEPENDS "include/g1-tank/*.hpp" "include/g1-tank/*.h") 
file(GLOB SRC_LIST CONFIGURE_DEPENDS "*.cpp" "*.c") 



set (PROJECT_FILES 
  ${PUBLIC_HEADER_LIST}
  ${SRC_LIST})

include(CMakePrintHelpers)
cmake_print_variables(PROJECT_FILES PUBLIC_HEADER_LIST SRC_LIST)

add_executable(g1-tank ${PROJECT_FILES})

# target_link_libraries(g1-tank PRIVATE "${LGPIO}" gpiozero)
target_link_libraries(g1-tank PRIVATE "${LGPIO}")

target_include_directories(g1-tank PUBLIC "./include")