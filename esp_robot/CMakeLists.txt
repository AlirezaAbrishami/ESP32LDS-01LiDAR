cmake_minimum_required(VERSION 2.8.3)
project(esp_robot)

find_package(catkin REQUIRED COMPONENTS
  roscpp
  sensor_msgs
)

catkin_package(
        INCLUDE_DIRS include
        CATKIN_DEPENDS roscpp sensor_msgs
)
include_directories(
# include
  ${catkin_INCLUDE_DIRS}
)

add_executable(laser_publisher src/laser_publisher.cpp)
add_dependencies(laser_publisher ${${PROJECT_NAME}_EXPORTED_TARGETS} ${catkin_EXPORTED_TARGETS})
target_link_libraries(laser_publisher ${catkin_LIBRARIES})

