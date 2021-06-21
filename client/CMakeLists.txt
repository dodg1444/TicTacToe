cmake_minimum_required(VERSION 3.5)

project(client LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "-pthread -lboost_system")

add_executable(client drawing_device.h drawing_device.cpp tools.h client.h client.cpp main.cpp)

include(FetchContent)

set(FETCHCONTENT_UPDATES_DISCONNECTED TRUE)

FetchContent_Declare(ftxui
  GIT_REPOSITORY https://github.com/ArthurSonzogni/ftxui
  GIT_TAG 87a1d75bf1c1ee964e8eed4a6686544ae3b44116
)

FetchContent_Declare(fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt
  GIT_TAG 2cac8a9
)

FetchContent_MakeAvailable(ftxui)


target_link_libraries(client
  PRIVATE ftxui::screen
  PRIVATE ftxui::dom
  PRIVATE ftxui::component
)




