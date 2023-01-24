//
// Created by paul on 1/23/23.
//

#ifndef PHONE_7BITDECODER_H
#define PHONE_7BITDECODER_H

#include <QByteArray>
#include <QString>
#include <vector>

class Decoder {

public:
    static QString decode7Bit(const QString &data);
    static QString decodeUcs2(const QString &data);
private:
    static const std::string _hexChartoBin(char c);
    static const std::vector<std::string> _hexStrtoBinStr(const std::string &hex);
};


#endif //PHONE_7BITDECODER_H
