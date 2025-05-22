# ----------------------------------------------------------------------- #
# ---------------------------- Set CMake Version ------------------------ #
# ----------------------------------------------------------------------- #
cmake_minimum_required(VERSION 3.11)
# ----------------------------------------------------------------------- #
# ---------------------------- Set files variable ----------------------- #
# ----------------------------------------------------------------------- #
set(source_root ${SPECTER_SOURCE_DIR}/modules/src/specter)
set(include_root ${SPECTER_SOURCE_DIR}/modules/include/specter)
set(resource_root ${SPECTER_SOURCE_DIR}/modules/resource/specter)
set(proto_root ${SPECTER_SOURCE_DIR}/proto/specter)

set(sources
    ${source_root}/inject.cpp
    ${source_root}/module.cpp
    ${source_root}/server/server.cpp
    ${source_root}/server/call.cpp
    ${source_root}/service/marker.cpp
    ${source_root}/service/recorder.cpp
    ${source_root}/service/object.cpp
    ${source_root}/service/utils.cpp
    ${source_root}/record/action.cpp
    ${source_root}/record/recorder.cpp
    ${source_root}/record/strategy.cpp
    ${source_root}/search/utils.cpp
    ${source_root}/search/query.cpp
    ${source_root}/search/searcher.cpp
    ${source_root}/search/strategy.cpp
    ${source_root}/observe/tree/action.cpp
    ${source_root}/observe/tree/observer.cpp
    ${source_root}/observe/property/action.cpp
    ${source_root}/observe/property/observer.cpp
    ${source_root}/mark/marker.cpp
    ${source_root}/mark/widget_marker.cpp
    ${source_root}/mark/widget_tooltip.cpp)

set(includes
    ${include_root}/config.h
    ${include_root}/export.h
    ${include_root}/module.h
    ${include_root}/server/service.h
    ${include_root}/server/server.h
    ${include_root}/server/call.h
    ${include_root}/service/marker.h
    ${include_root}/service/recorder.h
    ${include_root}/service/object.h
    ${include_root}/service/utils.h
    ${include_root}/record/action.h
    ${include_root}/record/recorder.h
    ${include_root}/record/strategy.h
    ${include_root}/search/utils.h
    ${include_root}/search/query.h
    ${include_root}/search/searcher.h
    ${include_root}/search/strategy.h
    ${include_root}/observe/tree/action.h
    ${include_root}/observe/tree/observer.h
    ${include_root}/observe/property/action.h
    ${include_root}/observe/property/observer.h
    ${include_root}/mark/marker.h
    ${include_root}/mark/widget_marker.h
    ${include_root}/mark/widget_tooltip.h)

set(resources ${resource_root}/specter.qrc)

set(protos ${proto_root}/specter.proto)
# ----------------------------------------------------------------------- #
# ------------------------- Qt libraries configuration ------------------ #
# ----------------------------------------------------------------------- #
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
# ----------------------------------------------------------------------- #
# -------------------------- Find external libraries -------------------- #
# ----------------------------------------------------------------------- #
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network)
# ----------------------------------------------------------------------- #
# -------------------------- Create plugin shared lib ------------------- #
# ----------------------------------------------------------------------- #
specter_add_module(
  specter
  SOURCES
  ${sources}
  ${includes}
  ${resources}
  PROTOS
  ${protos}
  DEPENDS
  Qt6::Core
  Qt6::Widgets
  Qt6::Network)

target_include_directories(
  specter
  PUBLIC $<BUILD_INTERFACE:${SPECTER_SOURCE_DIR}/modules/include>
  INTERFACE $<INSTALL_INTERFACE:include>
  PRIVATE ${SPECTER_SOURCE_DIR}/modules/src)
# ----------------------------------------------------------------------- #
