cmake_minimum_required(VERSION 3.5)

project(reactor)

set(SOURCE_DIR
  ${CMAKE_SOURCE_DIR}/Reactor
)

set(SOURCE_FILE
  ${SOURCE_DIR}/main_timer.cc
  ${SOURCE_DIR}/Timer.cc
  ${SOURCE_DIR}/TimerQueue.cc
  ${SOURCE_DIR}/Channel.cc
  ${SOURCE_DIR}/Poller.cc
  ${SOURCE_DIR}/EventLoop.cc
)

add_executable(${PROJECT_NAME} ${SOURCE_FILE})

target_include_directories(${PROJECT_NAME}
  PRIVATE
  ${SOURCE_DIR}
)

# should add the static lib source of muduo
