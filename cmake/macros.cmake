# ----------------------------------------------------------------------- #
# -------------- Define a macro that prepare target module -------------- #
# ----------------------------------------------------------------------- #
macro(__specter_prepare_module_target target)
  string(REPLACE "-" "_" NAME_UPPER "${target}")
  string(TOUPPER "${NAME_UPPER}" NAME_UPPER)
  set_target_properties(${target} PROPERTIES DEFINE_SYMBOL
                                             ${NAME_UPPER}_EXPORTS)

  if(BUILD_SHARED_LIBS)
    set_target_properties(${target} PROPERTIES DEBUG_POSTFIX -d)
  else()
    set_target_properties(${target} PROPERTIES DEBUG_POSTFIX -s-d)
    set_target_properties(${target} PROPERTIES RELEASE_POSTFIX -s)
  endif()

  set_target_properties(${target} PROPERTIES COMPILE_FEATURES cxx_std_20)
  set_target_properties(${target} PROPERTIES LINKER_LANGUAGE CXX)

  install(
    TARGETS ${target}
    EXPORT specterConfigExport
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT bin
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT lib
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT lib)

endmacro()
# ----------------------------------------------------------------------- # 
# ---- Define a macro that create proto libs and link it to target ------ #
# ----------------------------------------------------------------------- #
macro(__specter_add_proto_modules target)
  cmake_parse_arguments(THIS "" "" "PROTOS" ${ARGN})
  if(NOT "${THIS_UNPARSED_ARGUMENTS}" STREQUAL "")
    message(
      FATAL_ERROR
        "Extra unparsed arguments when calling specter_add_proto_modules: ${THIS_UNPARSED_ARGUMENTS}"
    )
  endif()

  find_package(protobuf CONFIG REQUIRED)
  find_package(gRPC CONFIG REQUIRED)

  foreach(PROTO_FILE ${THIS_PROTOS})
    get_filename_component(PROTO_FILENAME_WE ${PROTO_FILE} NAME_WE)
    get_filename_component(PROTO_IMPORT_DIRS ${PROTO_FILE} DIRECTORY)

    set(PROTO_TARGET ${PROTO_FILENAME_WE}_proto)
    set(PROTO_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated/${PROTO_TARGET})

    add_custom_target(
      ${PROTO_TARGET}_create_proto_dir ALL
      COMMAND ${CMAKE_COMMAND} -E make_directory ${PROTO_BINARY_DIR}
      COMMENT "Creating directory ${PROTO_BINARY_DIR}")

    add_library(${PROTO_TARGET} OBJECT ${PROTO_FILE})
    target_link_libraries(${PROTO_TARGET} PUBLIC protobuf::libprotobuf
                                                 gRPC::grpc++)
    target_include_directories(${PROTO_TARGET}
                               PUBLIC $<BUILD_INTERFACE:${PROTO_BINARY_DIR}/..>)
    add_dependencies(${PROTO_TARGET} ${PROTO_TARGET}_create_proto_dir)
    set_target_properties(${PROTO_TARGET} PROPERTIES POSITION_INDEPENDENT_CODE
                                                     TRUE)

    protobuf_generate(TARGET ${PROTO_TARGET} IMPORT_DIRS ${PROTO_IMPORT_DIRS}
                      PROTOC_OUT_DIR ${PROTO_BINARY_DIR})

    protobuf_generate(
      TARGET
      ${PROTO_TARGET}
      LANGUAGE
      grpc
      GENERATE_EXTENSIONS
      .grpc.pb.h
      .grpc.pb.cc
      PLUGIN
      "protoc-gen-grpc=\$<TARGET_FILE:gRPC::grpc_cpp_plugin>"
      IMPORT_DIRS
      ${PROTO_IMPORT_DIRS}
      PROTOC_OUT_DIR
      ${PROTO_BINARY_DIR})

    target_link_libraries(${target} PRIVATE ${PROTO_TARGET})

  endforeach()

endmacro()
# ----------------------------------------------------------------------- #
# ---------- Define a macro that helps add headers only module ---------- #
# ----------------------------------------------------------------------- #
macro(specter_add_headers_only_module target)

  cmake_parse_arguments(THIS "" "" "SOURCES;DEPENDS" ${ARGN})
  if(NOT "${THIS_UNPARSED_ARGUMENTS}" STREQUAL "")
    message(
      FATAL_ERROR
        "Extra unparsed arguments when calling specter_add_headers_only_module: ${THIS_UNPARSED_ARGUMENTS}"
    )
  endif()

  add_library(${target} INTERFACE ${THIS_SOURCES})
  add_library(specter::${target} ALIAS ${target})

  __specter_prepare_module_target(${target})

  if(THIS_DEPENDS)
    target_link_libraries(${target} INTERFACE ${THIS_DEPENDS})
  endif()

  if(NOT BUILD_SHARED_LIBS)
    target_compile_definitions(${target} INTERFACE "specter_STATIC")
  endif()

endmacro()
# ----------------------------------------------------------------------- #
# ----------------- Define a macro that helps add module ---------------- #
# ----------------------------------------------------------------------- #
macro(specter_add_module target)
  cmake_parse_arguments(
    THIS
    ""
    ""
    "SOURCES;PROTOS;DEPENDS;DEPENDS_PRIVATE;PRECOMPILE_HEADERS;PRECOMPILE_PRIVATE_HEADERS"
    ${ARGN})
  if(NOT "${THIS_UNPARSED_ARGUMENTS}" STREQUAL "")
    message(
      FATAL_ERROR
        "Extra unparsed arguments when calling specter_add_module: ${THIS_UNPARSED_ARGUMENTS}"
    )
  endif()

  add_library(${target} ${THIS_SOURCES})
  add_library(specter::${target} ALIAS ${target})

  __specter_prepare_module_target(${target})

  if(THIS_PROTOS)
    __specter_add_proto_modules(${target} PROTOS ${THIS_PROTOS})
  endif()

  if(THIS_DEPENDS)
    target_link_libraries(${target} PUBLIC ${THIS_DEPENDS})
  endif()

  if(THIS_DEPENDS_PRIVATE)
    target_link_libraries(${target} PRIVATE ${THIS_DEPENDS_PRIVATE})
  endif()

  if(THIS_PRECOMPILE_HEADERS)
    target_precompile_headers(${target} PUBLIC ${THIS_PRECOMPILE_HEADERS})
  endif()

  if(THIS_PRECOMPILE_PRIVATE_HEADERS)
    target_precompile_headers(${target} PRIVATE
                              ${THIS_PRECOMPILE_PRIVATE_HEADERS})
  endif()

  if(NOT BUILD_SHARED_LIBS)
    target_compile_definitions(${target} PUBLIC "specter_STATIC")
  endif()

endmacro()
# ----------------------------------------------------------------------- #
# --------------- Define a macro that helps export modules -------------- #
# ----------------------------------------------------------------------- #
function(specter_export_modules)

  if(BUILD_SHARED_LIBS)
    set(config_name "shared")
  else()
    set(config_name "static")
  endif()

  set(current_dir "${SPECTER_SOURCE_DIR}/cmake")
  set(targets_config_filename "specter-${config_name}-targets.cmake")
  set(config_package_location ${CMAKE_INSTALL_LIBDIR}/cmake/specter)

  include(CMakePackageConfigHelpers)
  write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/specter-config-version.cmake"
    VERSION ${SPECTER_VERSION_MAJOR}.${SPECTER_VERSION_MINOR}
    COMPATIBILITY SameMajorVersion)

  export(EXPORT specterConfigExport
         FILE "${CMAKE_CURRENT_BINARY_DIR}/${targets_config_filename}")

  configure_package_config_file(
    "${current_dir}/specter-config.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/specter-config.cmake"
    INSTALL_DESTINATION "${config_package_location}")
  configure_package_config_file(
    "${current_dir}/specter-config-dependencies.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/specter-config-dependencies.cmake"
    INSTALL_DESTINATION "${config_package_location}")

  install(
    EXPORT specterConfigExport
    FILE ${targets_config_filename}
    NAMESPACE specter::
    DESTINATION ${config_package_location})

  install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/specter-config.cmake
          ${CMAKE_CURRENT_BINARY_DIR}/specter-config-dependencies.cmake
          ${CMAKE_CURRENT_BINARY_DIR}/specter-config-version.cmake
    DESTINATION ${config_package_location})

  install(
    DIRECTORY ${SPECTER_SOURCE_DIR}/modules/include
    DESTINATION .
    COMPONENT devel
    FILES_MATCHING
    PATTERN "*.hpp"
    PATTERN "*.inl"
    PATTERN "*.h")

endfunction()
# ----------------------------------------------------------------------- #
# -------------- Define a macro that helps add application -------------- #
# ----------------------------------------------------------------------- #
macro(__specter_add_executable target)

  cmake_parse_arguments(THIS "" "RESOURCES_DIR"
                        "SOURCES;DEPENDS;DEPENDS_PRIVATE" ${ARGN})
  if(NOT "${THIS_UNPARSED_ARGUMENTS}" STREQUAL "")
    message(
      FATAL_ERROR
        "Extra unparsed arguments when calling __specter_add_executable: ${THIS_UNPARSED_ARGUMENTS}"
    )
  endif()

  add_executable(${target} ${THIS_SOURCES})
  set_target_properties(${target} PROPERTIES DEBUG_POSTFIX -d)

  if(THIS_DEPENDS)
    target_link_libraries(${target} PUBLIC ${THIS_DEPENDS})
  endif()

  if(THIS_DEPENDS_PRIVATE)
    target_link_libraries(${target} PRIVATE ${THIS_DEPENDS_PRIVATE})
  endif()

  install(TARGETS ${target} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
                                    COMPONENT bin)

  if(THIS_RESOURCES_DIR)
    get_filename_component(THIS_RESOURCES_DIR "${THIS_RESOURCES_DIR}" ABSOLUTE)

    if(NOT EXISTS "${THIS_RESOURCES_DIR}")
      message(
        FATAL_ERROR
          "Given resources directory to install does not exist: ${THIS_RESOURCES_DIR}"
      )
    endif()
    install(DIRECTORY ${THIS_RESOURCES_DIR}
            DESTINATION ${CMAKE_INSTALL_PREFIX}/bin/${target})
  endif()

  set_target_properties(${target} PROPERTIES CXX_STANDARD 20)

endmacro()
# ----------------------------------------------------------------------- #
# ----------------- Define a macro that helps add tool ------------------ #
# ----------------------------------------------------------------------- #
macro(specter_add_application target)

  cmake_parse_arguments(THIS "" "RESOURCES_DIR"
                        "SOURCES;DEPENDS;DEPENDS_PRIVATE" ${ARGN})
  if(NOT "${THIS_UNPARSED_ARGUMENTS}" STREQUAL "")
    message(
      FATAL_ERROR
        "Extra unparsed arguments when calling specter_add_application: ${THIS_UNPARSED_ARGUMENTS}"
    )
  endif()

  __specter_add_executable(
    ${target}
    SOURCES
    ${THIS_SOURCES}
    DEPENDS
    ${THIS_DEPENDS}
    DEPENDS_PRIVATE
    ${THIS_DEPENDS_PRIVATE}
    RESOURCES_DIR
    ${THIS_RESOURCES_DIR})

  target_compile_definitions(${target} PUBLIC QT_NO_KEYWORDS)

endmacro()
# ----------------------------------------------------------------------- #
# ----------- Define a function that helps add static analysis ---------- #
# ----------------------------------------------------------------------- #
function(specter_static_analyzers)
  if(SPECTER_ENABLE_CLANG_TIDY)
    find_program(CLANGTIDY clang-tidy)
    if(CLANGTIDY)
      set(CMAKE_CXX_CLANG_TIDY ${CLANGTIDY}
                               -extra-arg=-Wno-unknown-warning-option)
    else()
      message(FATAL_ERROR "Clang-Tidy requested but executable not found")
    endif()
  endif()

  if(SPECTER_ENABLE_CPPCHECK)
    find_program(CPPCHECK cppcheck)
    if(CPPCHECK)
      set(CMAKE_CXX_CPPCHECK ${CPPCHECK} --suppress=missingInclude --enable=all
                             --inline-suppr --inconclusive --force)
    else()
      message(FATAL_ERROR "Cppcheck requested but executable not found")
    endif()
  endif()
endfunction()
# ----------------------------------------------------------------------- #