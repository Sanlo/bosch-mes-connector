set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Network LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Network
                                                       LinguistTools)

set(TS_FILES MesSever_zh_CN.ts)

set(PROJECT_SOURCES main.cpp messerver.cpp messerver.h messerver.ui ${TS_FILES})

qt_add_executable(MesSever MANUAL_FINALIZATION ${PROJECT_SOURCES})
qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})

target_link_libraries(MesSever PRIVATE Qt${QT_VERSION_MAJOR}::Widgets
                                       Qt${QT_VERSION_MAJOR}::Network)

set_target_properties(MesSever PROPERTIES WIN32_EXECUTABLE TRUE)

if(QT_VERSION_MAJOR EQUAL 6)
  qt_finalize_executable(MesSever)
endif()
