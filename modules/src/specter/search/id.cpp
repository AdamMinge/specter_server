/* ----------------------------------- Local -------------------------------- */
#include "specter/search/id.h"
/* ------------------------------------ Qt ---------------------------------- */
#include <QJsonDocument>
#include <QJsonObject>
/* -------------------------------------------------------------------------- */

namespace specter {

/* ----------------------------------- ObjectId ----------------------------- */

ObjectId::ObjectId() : ObjectId(nullptr) {}

ObjectId::ObjectId(const QObject *data)
    : m_data(reinterpret_cast<qulonglong>(data)) {}

ObjectId::~ObjectId() = default;

ObjectId ObjectId::fromString(const QString &id) {
  auto object_id = ObjectId{};
  object_id.m_data = id.toULongLong();

  return object_id;
}

QString ObjectId::toString() const { return QString::number(m_data); }

bool ObjectId::operator==(const ObjectId &other) const {
  return m_data == other.m_data;
}

bool ObjectId::operator!=(const ObjectId &other) const {
  return m_data != other.m_data;
}

}// namespace specter
