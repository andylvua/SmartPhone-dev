//
// Created by Andrew Yaroshevych on 26.01.2023.
//

#ifndef PHONE_MEDIA_TYPES_HPP
#define PHONE_MEDIA_TYPES_HPP

#include <QString>
#include <QDateTime>
#include <utility>

enum class callResult {
    CR_ANSWERED,
    CR_NO_ANSWER
};

enum class callDirection {
    CD_INCOMING,
    CD_OUTGOING
};

enum class messageDirection {
    MD_INCOMING,
    MD_OUTGOING
};

enum class ussdEncoding {
    UE_GSM7,
    UE_UCS2,
    UE_UNKNOWN
};

struct Call {
    QString number;
    QDateTime startTime;
    QDateTime endTime;
    callResult callResult;
    callDirection callDirection;
};

struct Message {
    Message();
    Message(QString number, QString dateTime, QString message, messageDirection messageDirection);
    QString number;
    QString dateTime;
    QString message;
    messageDirection messageDirection;
};

#endif //PHONE_MEDIA_TYPES_HPP
