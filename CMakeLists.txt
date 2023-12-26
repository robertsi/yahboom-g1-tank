cmake_minimum_required(VERSION 3.13.0)
project(yahboom-tank-pi VERSION 0.1.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Weffc++")

# Let's ensure -std=c++xx instead of -std=g++xx
set(CMAKE_CXX_EXTENSIONS OFF)

# Let's nicely support folders in IDEs
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

include(CTest)
enable_testing()

# find_library(LGPIO liblgpio REQUIRED)
find_library(LGPIO NAMES liblgpio.so REQUIRED)
# find_library(LGPIO NAMES libpigpio.so REQUIRED)
add_subdirectory ("g1-tank")
add_subdirectory ("gpiozero")

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
