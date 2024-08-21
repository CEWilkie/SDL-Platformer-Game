cmake_minimum_required(VERSION 3.27)
project(CPP_ProgrammingProject)

set(SDL2_PATH "C:/Libs/SDL2-2.28.5")
set(SDL2_IMAGE_PATH "C:/Libs/SDL2_image-2.8.2")
set(SDL2_TTF_PATH "C:/Libs/SDL2_ttf-2.20.2")

include_directories(
        "${SDL2_PATH}/include"
        "${SDL2_IMAGE_PATH}/include"
        "${SDL2_TTF_PATH}/include")

set(CMAKE_CXX_STANDARD 14)

add_executable(CPP_ProgrammingProject
        src/main.cpp
        src/Track.h
        src/Track.cpp
        src/GameCore.cpp
        src/Player.h
        src/Player.cpp
        src/GameCore.h
        src/TrackBonusItems.h
        src/TrackBonusItems.cpp)

target_link_libraries(
        ${PROJECT_NAME}
        "${SDL2_PATH}/lib/x64/SDL2.lib"
        "${SDL2_IMAGE_PATH}/lib/x64/SDL2_image.lib"
        "${SDL2_TTF_PATH}/lib/x64/SDL2_ttf.lib"
)
