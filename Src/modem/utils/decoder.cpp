#include "modem/utils/decoder.hpp"
#include <string>
#include <vector>
#include <bitset>
#include <codecvt>
#include <locale>
#include <QString>

std::string Decoder::hexCharToBin(char c)
{
    switch(toupper(c))
    {
        case '0': return "0000";
        case '1': return "0001";
        case '2': return "0010";
        case '3': return "0011";
        case '4': return "0100";
        case '5': return "0101";
        case '6': return "0110";
        case '7': return "0111";
        case '8': return "1000";
        case '9': return "1001";
        case 'A': return "1010";
        case 'B': return "1011";
        case 'C': return "1100";
        case 'D': return "1101";
        case 'E': return "1110";
        case 'F': return "1111";
        default: return "0000";
    }
}

std::vector<std::string> Decoder::hexStrToBinStr(const std::string &hex) {
    std::vector<std::string> bin;
    for (unsigned i = 0; i != hex.length() / 2; ++i)
        bin.push_back(hexCharToBin(hex[2 * i]) + hexCharToBin(hex[2 * i + 1]));
    return bin;
}

QString Decoder::decode7Bit(const QString &encoded) {
    QString decoded = "";
    std::string message = encoded.toStdString();

    std::vector<std::string> messageInBin = hexStrToBinStr(message);
    std::vector<std::string> decodedMessage;
    std::string temp;

    for (size_t i = 0; i < messageInBin.size(); i++) {
        if (temp.size() == 7) {
            decodedMessage.push_back(temp);
            temp = "";
        }
        decodedMessage.push_back(messageInBin[i].substr((i) % 7 + 1) + temp);
        temp = messageInBin[i].substr(0, i % 7 + 1);
    }

    for (const auto & i : decodedMessage) {
        std::bitset<7> b(i);
        decoded += char(b.to_ulong());
    }

    return decoded;
}

QString Decoder::decodeUCS2(const QString &encoded) {
    std::string hexString = encoded.toStdString();
    std::string unicodeLiteral;
    for (size_t i = 0; i < hexString.length(); i += 4) {
        std::string hex = hexString.substr(i, 4);
        unicodeLiteral += "\\u" + hex;
    }

    std::wstring ws;
    ws.reserve(unicodeLiteral.size());

    for (size_t i = 0; i < unicodeLiteral.size();) {
        char ch = unicodeLiteral[i];
        i++;

        if ((ch == '\\') && (i < unicodeLiteral.size()) && (unicodeLiteral[i] == 'u')) {
            auto wc = static_cast<wchar_t>(std::stoi(unicodeLiteral.substr(i + 1, 4),
                                                     nullptr,
                                                     16));
            i += 5;
            ws.push_back(wc);
        } else {
            ws.push_back(static_cast<wchar_t>(ch));
        }
    }

    std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
    std::string s = conv.to_bytes(ws);

    return QString::fromStdString(s);
}
