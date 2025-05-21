# ----------------------------------------------------------------------- #
# ----------- Define a macro that helps defining an option -------------- #
# ----------------------------------------------------------------------- #
macro(specter_set_option var default type docstring)
  if(NOT DEFINED ${var})
    set(${var} ${default})
  endif()
  set(${var}
      ${${var}}
      CACHE ${type} ${docstring} FORCE)
endmacro()
# ----------------------------------------------------------------------- #
# ---------------------------- Detect the OS ---------------------------- #
# ----------------------------------------------------------------------- #
if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
  set(SPECTER_OS_WINDOWS TRUE)
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
  set(SPECTER_OS_LINUX TRUE)
else()
  message(FATAL_ERROR "Unsupported operating system or environment")
endif()
# ----------------------------------------------------------------------- #
# ---------------------------- Define options --------------------------- #
# ----------------------------------------------------------------------- #
specter_set_option(
  BUILD_SHARED_LIBS
  TRUE
  BOOL
  "TRUE to build SPECTER dependencies as shared libraries, FALSE to build it as static libraries"
)
specter_set_option(
  SPECTER_DEPRECATED_WARNINGS TRUE BOOL
  "FALSE to disable deprecated warning, TRUE to enable depracated warning")
specter_set_option(SPECTER_BUILD_DOCUMENTATION FALSE BOOL
                 "TRUE to build the documentation, FALSE to ignore them")
specter_set_option(SPECTER_ENABLE_CLANG_TIDY FALSE BOOL
                 "TRUE to enable clang tidy, FALSE to ignore them")
specter_set_option(SPECTER_ENABLE_CPPCHECK FALSE BOOL
                 "TRUE to enable cppcheck, FALSE to ignore them")
# ----------------------------------------------------------------------- #
# -------------------------- Set other options -------------------------- #
# ----------------------------------------------------------------------- #
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${SPECTER_SOURCE_DIR}/cmake/modules)
# ----------------------------------------------------------------------- #
