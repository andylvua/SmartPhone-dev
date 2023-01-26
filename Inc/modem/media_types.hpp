//
// Created by Andrew Yaroshevych on 26.01.2023.
//

#ifndef PHONE_MEDIA_TYPES_HPP
#define PHONE_MEDIA_TYPES_HPP

#include <QString>
#include <QDateTime>
#include <utility>

using callResult_t = enum class callResult {
    CR_ANSWERED,
    CR_NO_ANSWER
};

using callDirection_t = enum class callDirection {
    CD_INCOMING,
    CD_OUTGOING
};

using messageDirection_t = enum class messageDirection {
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
    callResult_t callResult;
    callDirection_t callDirection;
};

struct Message {
    Message();
    Message(QString number, QString dateTime, QString message, messageDirection_t messageDirection);
    QString number;
    QString dateTime;
    QString message;
    messageDirection_t messageDirection;
};

#endif //PHONE_MEDIA_TYPES_HPP
