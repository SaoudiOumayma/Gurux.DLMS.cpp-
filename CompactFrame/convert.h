//
// Copyright 2018 Honeywell Smart Energy.  All Rights Reserved
//

#pragma once
#include <climits>
#include <ctime>
#include <stdexcept>
#include <string>
#include <sstream>

namespace eams
{
    namespace convert
    {
        // Converts binary input into string containing ASCII-hex representation.  Each
        // byte is converted into two ASCII characters ('0'-'F') and an ASCII space is
        // inserted between each pair of characters.  There is no trailing space.  All
        // Hex digits 'A'-'F' result in capital characters in the output.  Ex:
        //
        // FormatHex( "\x01\x34\fd", 3) == std::string("01 34 fd")
        //
        std::string  FormatHex(const void* buffer, size_t size);

        // calls FormatHex(const void*, size_t) with the contents and length of the
        // parameter
        //
        std::string  FormatHex(const std::string& byteString);

        // Converts an ASCII hex representation of a numeric value into an ASCII decimal
        // representation of the same value.  Ex:
        //
        // HexToDec("FE") == std::string("254")
        //
        std::string  HexToDec(const std::string& hexStr);

        char  HexChar(unsigned hexNumber);

        std::string ByteToHex(unsigned char byte);

        // convert ASCII hex into binary data
        //
        std::string HexToBin(const std::string& hex);

        // convert std::string("\x01\x02\x03",3) into std::string("010203")
        //
        std::string  BinToHex(const std::string& bin);


        // general purpose lexical converter (a poor man's boost::lexical_cast).  this function
        // can convert from/to any type that stringstream can insert/extract.
        //
        // usage is like this:
        //
        // long eams::convert::to<long>("12345")
        // std::string eams::covert::to<std::string>((long)12345);
        //
        template <typename T, typename F>
        T to(const F& f)
        {
            std::stringstream ss;
            ss << std::setprecision(10);
            ss << std::boolalpha;
            ss << f;
            T t;
            ss >> t;

            if(ss.fail())
            {
                throw std::runtime_error("conversion failed");
            }

            return t;
        }

        // specializations for converting to/from bools

        template <>
        inline long to<long, bool>(const bool& t)
        {
            return t ? 1 : 0;
        }

        template <>
        inline bool to<bool, long>(const long& l)
        {
            return (l == 0) ? false : true;
        }

        template <>
        inline bool to<bool, unsigned long>(const unsigned long& l)
        {
            return (l == 0) ? false : true;
        }

        template <>
        inline bool to<bool, unsigned char>(const unsigned char& l)
        {
            return (l == 0) ? false : true;
        }

        template <>
        inline unsigned long to<unsigned long, long>(const long& l)
        {
            if(l >= 0)
            {
                return static_cast<unsigned long>(l);
            }

            throw std::runtime_error("conversion to unsigned not possible");
        }

        template <>
        inline long to<long, __int64>(const __int64& l)
        {
            if(l <= LONG_MAX)
            {
                return static_cast<long>(l);
            }

            throw std::runtime_error("conversion to smaller size would lose information");
        }

        template <>
        inline long to<long, size_t>(const size_t& l)
        {
            if(l <= LONG_MAX)
            {
                return static_cast<long>(l);
            }

            throw std::runtime_error("conversion to smaller size would lose information");
        }

        template <>
        inline int to<int, size_t>(const size_t& l)
        {
            if(l <= INT_MAX)
            {
                return static_cast<int>(l);
            }

            throw std::runtime_error("conversion to smaller size would lose information");
        }

        template <>
        inline unsigned long to<unsigned long, size_t>(const size_t& l)
        {
            if(l <= ULONG_MAX)
            {
                return static_cast<unsigned long>(l);
            }

            throw std::runtime_error("conversion to smaller size would lose information");
        }

        template <>
        inline size_t to<size_t, __int64>(const __int64& l)
        {
            return static_cast<size_t>(l);
        }

        inline int toInt(size_t x)
        {
            if(x > INT_MAX)
            {
                return INT_MAX;
            }

            return (x & 0xFFFFFFFF);
        }

        inline long toLong(size_t x)
        {
            return static_cast<long>(x);
        }

        inline std::string toIso8601(time_t source)
        {
            struct tm dt
            {
                0, 0, 0, 0, 0, 0, 0, 0, 0
            };
            _gmtime64_s(&dt, &source);

            char ts_str[ 300 ] {0};

            sprintf_s(ts_str, "%04d-%02d-%02dT%02d:%02d:%02dZ",
                    dt.tm_year + 1900, dt.tm_mon + 1, dt.tm_mday,
                    dt.tm_hour, dt.tm_min, dt.tm_sec);
            return std::string(ts_str);
        }

        /**
            Converts a given timestamp string to time_t.
            The return value is number of seconds since epoch in /local/ time.

            @param str - the timestamp string
            @param format - optional, format to parse the given string
        */
        std::time_t strToTime_t(const std::string& str, const std::string& format = "%Y-%m-%d %H:%M:%S");
    };
};
