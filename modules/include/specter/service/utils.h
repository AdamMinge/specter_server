#ifndef SPECTER_SERVICE_UTILS_H
#define SPECTER_SERVICE_UTILS_H

/* ----------------------------------- Proto -------------------------------- */
#include <specter_proto/specter.grpc.pb.h>
#include <specter_proto/specter.pb.h>
/* ------------------------------------ Qt ---------------------------------- */
#include <QObject>
#include <QVariant>
#include <QWidget>
/* ----------------------------------- Local -------------------------------- */
#include "specter/export.h"
#include "specter/search/id.h"
#include "specter/search/query.h"
/* -------------------------------------------------------------------------- */

namespace specter {

QVariant convertIntoVariant(const google::protobuf::Value &value);
google::protobuf::Value convertIntoValue(const QVariant &variant);

std::pair<grpc::Status, QObjectList> tryGetObjects(const ObjectQuery &query);

std::pair<grpc::Status, QObject *> tryGetSingleObject(const ObjectQuery &query);
std::pair<grpc::Status, QObject *> tryGetSingleObject(const ObjectId &query);

std::pair<grpc::Status, QWidget *> tryGetSingleWidget(const ObjectQuery &query);
std::pair<grpc::Status, QWidget *> tryGetSingleWidget(const ObjectId &query);

QPoint resolvePosition(QWidget *widget, const specter_proto::Offset &offset);
QPoint resolvePosition(QWidget *widget, const specter_proto::Anchor &anchor);

}// namespace specter

#endif// SPECTER_SERVICE_UTILS_H
