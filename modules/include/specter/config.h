/* -------------------------------------------------------------------------- */
#ifndef SPECTER_CONFIG_H
#define SPECTER_CONFIG_H
/* -------------------------------------------------------------------------- */
/* ---------------------------- Define SPECTER version ----------------------- */
/* -------------------------------------------------------------------------- */
// clang-format off

#define SPECTER_VERSION_MAJOR 0
#define SPECTER_VERSION_MINOR 0
#define SPECTER_VERSION_PATCH 1
#define SPECTER_VERSION 0.0.1
#define SPECTER_VERSION_STR "0.0.1"

// clang-format on
/* -------------------------------------------------------------------------- */
/* ----------------------- Identify the operating system -------------------- */
/* -------------------------------------------------------------------------- */
#if defined(_WIN32)
#define SPECTER_OS_WINDOWS
#elif defined(__linux__)
#define SPECTER_OS_LINUX
#else
#error This operating system is not supported by SPECTER library
#endif
/* -------------------------------------------------------------------------- */
/* ------------------------ Define a portable debug macro ------------------- */
/* -------------------------------------------------------------------------- */
#if !defined(NDEBUG)
#define SPECTER_DEBUG
#endif
/* -------------------------------------------------------------------------- */
/* ----------------------- Define a export/import dll macro ----------------- */
/* -------------------------------------------------------------------------- */
#if !defined(SPECTER_STATIC)

#if defined(SPECTER_OS_WINDOWS)

#define SPECTER_API_EXPORT __declspec(dllexport)
#define SPECTER_API_IMPORT __declspec(dllimport)

#ifdef _MSC_VER

#pragma warning(disable : 4251)

#endif

#else

#define SPECTER_API_EXPORT __attribute__((__visibility__("default")))
#define SPECTER_API_IMPORT __attribute__((__visibility__("default")))

#endif

#else

#define SPECTER_API_EXPORT
#define SPECTER_API_IMPORT

#endif
/* -------------------------------------------------------------------------- */
/* -------------------- Define helpers to set items as deprecated ----------- */
/* -------------------------------------------------------------------------- */
#if !defined(SPECTER_DEPRECATED_WARNINGS)
#define SPECTER_DEPRECATED [[deprecated]]
#else
#define SPECTER_DEPRECATED
#endif
/* -------------------------------------------------------------------------- */
#endif  // SPECTER_CONFIG_H
/* -------------------------------------------------------------------------- */
