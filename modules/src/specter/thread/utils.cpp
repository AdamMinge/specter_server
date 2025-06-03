/* ----------------------------------- Local -------------------------------- */
#include "specter/thread/invoke.h"
#include "specter/thread/utils.h"
/* -------------------------------------------------------------------------- */

namespace specter::invoke {

bool setProperty(
  QObject *object, const char *property_name, const QVariant &new_value) {
  return InvokeInObjectThread(
    object, qOverload<const char *, const QVariant &>(&QObject::setProperty),
    property_name, new_value);
}

QVariant getProperty(const QObject *object, const char *property_name) {
  return InvokeInObjectThread(object, &QObject::property, property_name);
}

QString getObjectName(const QObject *object) {
  return InvokeInObjectThread(
    object, [object]() { return object->objectName(); });
}

QString getClassName(const QObject *object) {
  return InvokeInObjectThread(
    object, [object]() { return object->metaObject()->className(); });
}

}// namespace specter::invoke
