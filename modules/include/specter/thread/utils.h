#ifndef SPECTER_THREAD_UTILS_H
#define SPECTER_THREAD_UTILS_H

/* --------------------------------- Standard ------------------------------- */
#include <QObject>
#include <QVariant>
/* -------------------------------------------------------------------------- */

namespace specter::invoke {

bool setProperty(
  QObject *object, const char *property_name, const QVariant &new_value);
QVariant getProperty(const QObject *object, const char *property_name);

QString getObjectName(const QObject *object);
QString getClassName(const QObject *object);

}// namespace specter::invoke

#endif// SPECTER_THREAD_UTILS_H