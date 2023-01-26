//
// Created by Andrew Yaroshevych on 26.01.2023.
//

#include "../../Inc/modem/media_types.hpp"

Message::Message(QString number, QString dateTime, QString message,
                 enum messageDirection messageDirection) {
    this->number = std::move(number);
    this->dateTime = std::move(dateTime);
    this->message = std::move(message);
    this->messageDirection = messageDirection;
}
