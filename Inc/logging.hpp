//
// Created by Andrew Yaroshevych on 17.01.2023.
//

#ifndef PHONE_LOGGING_HPP
#define PHONE_LOGGING_HPP

#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/spdlog.h"
#include <QtGlobal>
#include <QDateTime>

#define MESSAGE_QSTRING(type, msg) QString("[%1] [%2] [Qt] %3") \
.arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz"), type, (msg).constData())

void logOutputHandler(QtMsgType type, [[maybe_unused]] const QMessageLogContext &context, const QString &msg);

#endif //PHONE_LOGGING_HPP
