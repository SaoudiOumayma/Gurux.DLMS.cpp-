#include "stdafx.h"

//----------------------------------------------------------------------
char eams::convert::HexChar(unsigned hexNumber)
{
    //ASSERT( hexNumber < 16 );

    if (hexNumber < 10)
    {
        return hexNumber + char{ 48 };
    }

    return hexNumber - 10 + char{ 65 };
}

//----------------------------------------------------------------------
std::string eams::convert::ByteToHex(unsigned char byte)
{
    char hex[3];

    hex[0] = HexChar(byte >> 4);
    hex[1] = HexChar(byte & 0xF);
    hex[2] = 0;

    return hex;
}

//----------------------------------------------------------------------
std::string eams::convert::FormatHex(const void* buffer, size_t size)
{
    std::string result;

    if(size == 0)
    {
        return result;
    }

    const auto bytes = static_cast<const unsigned char*>(buffer);

    result = ByteToHex(bytes[0]);

    for(size_t i = 1; i < size; ++i)
    {
        result += " ";
        result += ByteToHex(bytes[i]);
    }

    return result;
}

//----------------------------------------------------------------------
std::string eams::convert::BinToHex(const std::string& bin)
{
    std::string result;
    auto size = bin.size();
    auto buffer = bin.data();

    if(size == 0)
    {
        return result;
    }

    const auto bytes = reinterpret_cast<const unsigned char*>(buffer);

    result = ByteToHex(bytes[0]);

    for(size_t i = 1; i < size; ++i)
    {
        result += ByteToHex(bytes[i]);
    }

    return result;
}

//----------------------------------------------------------------------
std::string eams::convert::FormatHex(const std::string& byteString)
{
    return FormatHex(byteString.data(), byteString.size());
}

//----------------------------------------------------------------------
std::string eams::convert::HexToDec(const std::string& hexStr)
{
    std::stringstream ss;

    if(hexStr.empty())
    {
        return ss.str();
    }

    const unsigned sizeOfInt2 = sizeof(int) * 2;

    if(hexStr.size() > sizeOfInt2)
    {
        throw std::runtime_error("HexToDec: Conversion size error");
    }

    if(!(ss << std::hex << hexStr))
    {
        throw std::runtime_error("HexToDec: Conversion 1 error");
    }

    int intVal = 0;

    if(!(ss >> intVal))
    {
        throw std::runtime_error("HexToDec: Conversion 2 error");
    }

    ss.clear();
    ss.str("");
    ss << std::dec;

    if(!(ss << intVal))
    {
        throw std::runtime_error("HexToDec: Conversion 3 error");
    }

    return ss.str();
}

//--------------------------------------------------------------------
std::string eams::convert::HexToBin(const std::string& hex)
{
    std::string bin;
    bin.resize(hex.size() / 2);

    char tmp[3];
    tmp[2] = 0;

    char* dummy = nullptr;

    for(size_t i = 0; i < hex.size(); i += 2)
    {
        tmp[0] = hex[i];
        tmp[1] = hex[i + 1];

        unsigned x = strtol(tmp, &dummy, 16);
        bin[i / 2] = x;
    }

    return bin;
}

std::time_t eams::convert::strToTime_t(const std::string& str, const std::string& format)
{
    std::tm tme{0, 0, 0, 0, 0, 0, 0, 0, 0};

    std::istringstream ss(str);
    ss >> std::get_time(&tme, format.c_str());
    tme.tm_isdst = -1; // let system figure out dst

    return _mkgmtime(&tme); // mktime() returns a gmt representation, use _mkgmtime to avoid such a conversion
}
