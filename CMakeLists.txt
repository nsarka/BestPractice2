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
            qt/MainWindow.cpp
            qt/MainWindow.h
            qt/ProgressDialog.cpp
            qt/ProgressDialog.h
            qt/main.cpp
        )

        set_target_properties(bestpractice_qt PROPERTIES AUTOMOC ON)

        target_link_libraries(bestpractice_qt PRIVATE Qt6::Widgets Qt6::Multimedia bestpractice_core)
        target_include_directories(bestpractice_qt PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")

        add_custom_command(TARGET bestpractice_qt POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${CMAKE_CURRENT_SOURCE_DIR}/qt/help.html"
                "$<TARGET_FILE_DIR:bestpractice_qt>/help.html"
        )

        get_target_property(QT_QMAKE_EXECUTABLE Qt6::qmake IMPORTED_LOCATION)
        get_filename_component(QT_BIN_DIR "${QT_QMAKE_EXECUTABLE}" DIRECTORY)
        find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${QT_BIN_DIR}")

        if(WINDEPLOYQT_EXECUTABLE)
            set(BESTPRACTICE_PACKAGE_DIR "${CMAKE_CURRENT_BINARY_DIR}/package")
            add_custom_target(bestpractice_package_stage
                COMMAND ${CMAKE_COMMAND} -E rm -rf "${BESTPRACTICE_PACKAGE_DIR}"
                COMMAND ${CMAKE_COMMAND} -E make_directory "${BESTPRACTICE_PACKAGE_DIR}"
                COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:bestpractice_qt>"
                    "${BESTPRACTICE_PACKAGE_DIR}/BestPractice.exe"
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${CMAKE_CURRENT_SOURCE_DIR}/qt/help.html"
                    "${BESTPRACTICE_PACKAGE_DIR}/help.html"
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${CMAKE_CURRENT_SOURCE_DIR}/license.txt"
                    "${BESTPRACTICE_PACKAGE_DIR}/license.txt"
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${CMAKE_CURRENT_SOURCE_DIR}/readme.txt"
                    "${BESTPRACTICE_PACKAGE_DIR}/readme.txt"
                COMMAND "${WINDEPLOYQT_EXECUTABLE}" --no-translations --compiler-runtime
                    "${BESTPRACTICE_PACKAGE_DIR}/BestPractice.exe"
                DEPENDS bestpractice_qt
                COMMENT "Staging the self-contained BestPractice application"
            )

            find_program(MAKENSIS_EXECUTABLE makensis
                HINTS "C:/Program Files/NSIS" "C:/Program Files (x86)/NSIS")
            if(MAKENSIS_EXECUTABLE)
                add_custom_target(bestpractice_installer
                    COMMAND "${MAKENSIS_EXECUTABLE}"
                        "/DSTAGE_DIR=${BESTPRACTICE_PACKAGE_DIR}"
                        "/DOUTPUT_FILE=${CMAKE_CURRENT_BINARY_DIR}/BestPractice-1.03-Setup.exe"
                        "${CMAKE_CURRENT_SOURCE_DIR}/bp.nsi"
                    DEPENDS bestpractice_package_stage
                    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
                    COMMENT "Building the English BestPractice installer"
                )
            else()
                message(STATUS "NSIS was not found; build bestpractice_package_stage now and install NSIS to enable bestpractice_installer.")
            endif()
        endif()

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
