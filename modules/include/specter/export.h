#ifndef SPECTER_EXPORT_H
#define SPECTER_EXPORT_H

/* ------------------------------------ Qt ---------------------------------- */
#include "specter/config.h"
/* -------------------------------------------------------------------------- */

#if defined(SPECTER_EXPORTS)
#define LIB_SPECTER_API SPECTER_API_EXPORT
#else
#define LIB_SPECTER_API SPECTER_API_IMPORT
#endif

#endif// SPECTER_EXPORT_H
