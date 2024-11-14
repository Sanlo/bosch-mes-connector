set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Network LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Network
                                                       LinguistTools)

set(TS_FILES MesConnector_zh_CN.ts)

# set(PROJECT_SOURCES main.cpp mesconnectorclient.cpp mesconnectorclient.h
# settings.cpp settings.h settings.ui mesconnectorclient.ui xopconwriter.h
# xopconwriter.cpp ${TS_FILES})

qt_add_executable(
  MesConnector
  MANUAL_FINALIZATION
  ${TS_FILES}
  main.cpp
  mesconnectorclient.cpp
  mesconnectorclient.h
  settings.cpp
  settings.h
  settings.ui
  mesconnectorclient.ui
  xopconwriter.h
  xopconwriter.cpp
  xopconreader.h
  xopconreader.cpp
  mscconnector.qrc)
qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})

target_link_libraries(MesConnector PRIVATE Qt${QT_VERSION_MAJOR}::Widgets
                                           Qt${QT_VERSION_MAJOR}::Network)

set_target_properties(MesConnector PROPERTIES WIN32_EXECUTABLE TRUE)

if(QT_VERSION_MAJOR EQUAL 6)
  qt_finalize_executable(MesConnector)
endif()
