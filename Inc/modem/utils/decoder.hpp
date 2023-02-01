//
// Created by paul on 1/23/23.
//

#ifndef PHONE_DECODER_HPP
#define PHONE_DECODER_HPP

#include <QByteArray>
#include <QString>
#include <vector>

class Decoder {
public:
    static QString decode7Bit(const QString &data);

    static QString decodeUCS2(const QString &data);

private:
    static std::string hexCharToBin(char c);

    static std::vector<std::string> hexStrToBinStr(const std::string &hex);
};


#endif //PHONE_DECODER_HPP
