cmake_minimum_required(VERSION 3.20)

project(BestPracticeModern LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(bestpractice_core STATIC
    buffer.cpp
    CWSOLA.cpp
    IIRButter.cpp
    SampleData.cpp
    SecondOrderSection.cpp
    TKaraokeProcessor.cpp
    TQMFBank.cpp
    TResampler.cpp
    TReader.cpp
)

target_include_directories(bestpractice_core PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

if(MSVC)
    target_compile_options(bestpractice_core PRIVATE /W4 /permissive-)
else()
    target_compile_options(bestpractice_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(bestpractice_core_smoke
    tools/core_smoke.cpp
)

target_link_libraries(bestpractice_core_smoke PRIVATE bestpractice_core)

option(BESTPRACTICE_BUILD_QT_FORMS "Build the modern Qt Widgets forms when Qt 6 is available" ON)

if(BESTPRACTICE_BUILD_QT_FORMS)
    find_package(Qt6 QUIET COMPONENTS Widgets Multimedia)

    if(Qt6_FOUND)
        add_executable(bestpractice_qt
            qt/AudioEngine.cpp
            qt/AudioEngine.h
            qt/AboutDialog.cpp
            qt/AboutDialog.h
            qt/LanguageDialog.cpp
            qt/LanguageDialog.h
            qt/MainWindow.cpp
            qt/MainWindow.h
            qt/ProgressDialog.cpp
            qt/ProgressDialog.h
            qt/main.cpp
        )

        set_target_properties(bestpractice_qt PROPERTIES AUTOMOC ON)

        target_link_libraries(bestpractice_qt PRIVATE Qt6::Widgets Qt6::Multimedia bestpractice_core)
        target_include_directories(bestpractice_qt PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")

        if(MSVC)
            target_compile_options(bestpractice_qt PRIVATE /W4 /permissive-)
        else()
            target_compile_options(bestpractice_qt PRIVATE -Wall -Wextra -Wpedantic)
        endif()

        add_executable(bestpractice_audio_engine_smoke
            qt/AudioEngine.cpp
            qt/AudioEngine.h
            tools/audio_engine_smoke.cpp
        )
        set_target_properties(bestpractice_audio_engine_smoke PROPERTIES AUTOMOC ON)
        target_link_libraries(bestpractice_audio_engine_smoke PRIVATE Qt6::Multimedia bestpractice_core)
        target_include_directories(bestpractice_audio_engine_smoke PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
    else()
        message(STATUS "Qt 6 Widgets and Multimedia were not found; skipping bestpractice_qt. Install those Qt 6 modules and reconfigure to build the modern app.")
    endif()
endif()
