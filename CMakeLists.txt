cmake_minimum_required(VERSION 3.5)

project(ToMobile VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Core Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED Network Core COMPONENTS Widgets)

include_directories("./ffmpeg/win32/include")
link_directories("./ffmpeg/win32/lib")

include_directories("./")
set(PROJECT_SOURCES
    main.cpp

)
set(PROJECT_UI_SOURCES
    ui/maindialog.cpp ui/maindialog.h ui/maindialog.ui
    ui/titlebar.h ui/titlebar.cpp ui/titlebar.ui
    ui/videotitlebar.h ui/videotitlebar.cpp ui/videotitlebar.ui
    ui/videobottombar.h ui/videobottombar.cpp ui/videobottombar.ui
    ui/videoform.h ui/videoform.cpp ui/videoform.ui
    ui/leftform.h ui/leftform.cpp ui/leftform.ui
    ui/settingsform.h ui/settingsform.cpp ui/settingsform.ui
    ui/leftbutton.h ui/leftbutton.cpp ui/leftbutton.ui
    ui/devicelistform.h ui/devicelistform.cpp ui/devicelistform.ui
    ui/aboutform.h ui/aboutform.cpp ui/aboutform.ui
    ui/tabitemform.h ui/tabitemform.cpp ui/tabitemform.ui
    ui/previewwidget.h ui/previewwidget.cpp ui/previewwidget.ui
    ui/video_widget.h ui/video_widget.cpp
    ui/iconfont.h ui/iconfont.cpp
)

set(PROJECT_CORE_SOURCES
    core/common.h
    core/adbprocess.h core/adbprocess.cpp
    core/device.h core/device.cpp
    core/demuxer.h core/demuxer.cpp
    core/controller.h core/controller.cpp
    core/decoder.h core/decoder.cpp
    core/binary.h
    core/packet_sink.h core/packet_sink.cpp
    core/tcpsocket.h core/tcpsocket.cpp
    core/frame_buffer.h core/frame_buffer.cpp
    core/recorder.h core/recorder.cpp
    core/android/input.h core/android/keycodes.h
    core/control_msg.h core/control_msg.cpp
    core/inject_normal.h core/inject_normal.cpp
    core/input_events.h
    core/config.h core/config.cpp
    core/utils.h core/utils.cpp


)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(ToMobile
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET ToMobile APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(ToMobile SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(ToMobile
            ${PROJECT_SOURCES}
            ${PROJECT_UI_SOURCES}
            ${PROJECT_CORE_SOURCES}
            res.qrc

        )
    endif()
endif()

target_link_libraries(ToMobile PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Network avcodec avformat avutil swresample)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.ToMobile)
endif()
set_target_properties(ToMobile PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS ToMobile
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(ToMobile)
endif()
