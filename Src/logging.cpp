//
// Created by Andrew Yaroshevych on 02.02.2023.
//

#include "logging.hpp"
#include "modem/utils/cache_manager.hpp"
#include <QFile>
#include <QIODevice>
#include <QTextStream>

void logOutputHandler(QtMsgType type, [[maybe_unused]] const QMessageLogContext &context, const QString &msg) {
    QFile logsFile(LOGS_FILEPATH);
    logsFile.open(QIODevice::WriteOnly | QIODevice::Append);
    QTextStream logsStream(&logsFile);
    QByteArray localMsg = msg.toLocal8Bit();

    QString txt;

    switch (type) {
        case QtDebugMsg:
            txt = MESSAGE_QSTRING("debug", localMsg);
            break;
        case QtInfoMsg:
            txt = MESSAGE_QSTRING("info", localMsg);
            break;
        case QtWarningMsg:
            txt = MESSAGE_QSTRING("warning", localMsg);
            break;
        case QtCriticalMsg:
            txt = MESSAGE_QSTRING("critical", localMsg);
            break;
        case QtFatalMsg:
            txt = MESSAGE_QSTRING("fatal", localMsg);
            abort();
    }

    logsStream << txt << Qt::endl;
}
