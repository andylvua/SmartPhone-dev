//
// Created by Andrew Yaroshevych on 26.01.2023.
//

#include "modem/media_types.hpp"

Message::Message(QString number, QString dateTime, QString message,
                 enum messageDirection messageDirection) {
    this->number = std::move(number);
    this->dateTime = std::move(dateTime);
    this->message = std::move(message);
    this->messageDirection = messageDirection;
}

Contact::Contact() = default;

Contact::Contact(QString name, QString number) {
    this->name = std::move(name);
    this->number = std::move(number);
}

bool Contact::hasValue() const {
    return !name.isEmpty() && !number.isEmpty();
}