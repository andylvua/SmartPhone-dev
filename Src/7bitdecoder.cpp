#include <iostream>
#include <string>
#include <vector>
#include <bitset>
#include <QString>


const std::string hex_char_to_bin(char c)
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
    }
}

std::vector<std::string> hex_str_to_bin_str(const std::string& hex)
{
    std::vector<std::string> bin;
    for(unsigned i = 0; i != hex.length()/2; ++i)
        bin.push_back(hex_char_to_bin(hex[2*i]) + hex_char_to_bin(hex[2*i+1]));
    return bin;
}

QString decode(const QString& encoded){
    QString decoded = "";
    std::string message = encoded.toStdString();
    std::cin>> message;
    // CE3048 -> 11001110 00111000 01001000
    std::vector<std::string> message_in_bin = hex_str_to_bin_str(message);
    //decode message using 7-bit ASCII
    // 11001110 00111000 01001000 -> 1001110 1110001 0100000
    std::vector <std::string> decoded_message;
    std::string temp = "";
    for (int i=0; i < message_in_bin.size(); i++){
        if (temp.size() == 7){
            decoded_message.push_back(temp);
            temp = "";
        }
        decoded_message.push_back(message_in_bin[i].substr((i)%7+1)+temp);
        temp = message_in_bin[i].substr(0,i%7+1);

    }
    for (int i = 0; i < decoded_message.size(); i++){
        std::bitset<7> b(decoded_message[i]);
        decoded += char(b.to_ulong());
    }
    decoded += "\n";
    return decoded
}