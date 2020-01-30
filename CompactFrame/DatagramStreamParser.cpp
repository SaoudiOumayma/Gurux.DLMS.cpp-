#include "stdafx.h"
#include "DatagramStreamParser.h"

DatagramStreamParser::DatagramStreamParser(const std::string& buffer) : m_buffer(buffer), m_offset(0)
{
}

std::string DatagramStreamParser::getRemainingBuffer()
{
    return this->m_buffer.substr(this->m_offset);
}

void DatagramStreamParser::ensureRemaining(size_t count) const
{
    if (this->m_offset >= this->m_buffer.size())
    {
        throw std::runtime_error(fmt::format("already past the end of a buffer, asking for {} more bytes", count));
    }

    if (this->m_buffer.size() - this->m_offset < count)
    {
        throw std::runtime_error(fmt::format(
            "{} bytes into a {} byte buffer, asking for {} bytes would overrun the buffer",
            this->m_offset, this->m_buffer.size(), count));
    }
}

uint8_t DatagramStreamParser::getNextUint8()
{
    size_t sizeOfDataType = 1;
    this->ensureRemaining(sizeOfDataType);
    unsigned long retval = 0;
    auto byte = static_cast<unsigned char>(this->m_buffer.at(this->m_offset));
    retval |= byte;
    this->m_offset += sizeOfDataType;
    return retval;
}

int8_t DatagramStreamParser::getNextInt8()
{
    size_t sizeOfDataType = 1;
    this->ensureRemaining(sizeOfDataType);
    auto retval = static_cast<int8_t>(this->m_buffer.at(this->m_offset));
    this->m_offset += sizeOfDataType;
    return retval;
}

size_t DatagramStreamParser::getTotalDatagramSize()
{
    return this->m_buffer.size();
}

size_t DatagramStreamParser::getTotalBytesProcessedSoFar()
{
    return this->m_offset;
}

size_t DatagramStreamParser::getRemainingUnprocessedBytes()
{
    return this->m_buffer.size() - this->m_offset;
}

uint16_t DatagramStreamParser::getNextLittleEndianUint16()
{
    size_t sizeOfDataType = 2;
    this->ensureRemaining(sizeOfDataType);

    auto lsb = this->getNextUint8();
    auto msb = this->getNextUint8();

    uint16_t result = msb;
    result <<= 8;
    result |= lsb;

    return result;
}

int16_t DatagramStreamParser::getNextLittleEndianInt16()
{
    size_t sizeOfDataType = 2;
    this->ensureRemaining(sizeOfDataType);

    auto lsb = this->getNextUint8();
    auto msb = this->getNextUint8();

    int16_t retval = msb;
    retval <<= 8;
    retval |= lsb;

    return retval;
}

uint32_t DatagramStreamParser::getNextLittleEndianUint32()
{
    size_t sizeOfDataType = 4;
    this->ensureRemaining(sizeOfDataType);

    uint32_t retval = 0;
    retval |= this->getNextUint8();
    retval |= static_cast<uint32_t>(this->getNextUint8()) << 8;
    retval |= static_cast<uint32_t>(this->getNextUint8()) << 16;
    retval |= static_cast<uint32_t>(this->getNextUint8()) << 24;

    return retval;
}

int32_t DatagramStreamParser::getNextLittleEndianInt32()
{
    size_t sizeOfDataType = 4;
    this->ensureRemaining(sizeOfDataType);

    int32_t retval = 0;
    retval |= this->getNextUint8();
    retval |= static_cast<int32_t>(this->getNextUint8()) << 8;
    retval |= static_cast<int32_t>(this->getNextUint8()) << 16;
    retval |= static_cast<int32_t>(this->getNextUint8()) << 24;

    return retval;
}

int64_t DatagramStreamParser::getNextLittleEndianInt48()
{
    size_t sizeOfDataType = 6;
    this->ensureRemaining(sizeOfDataType);

    int64_t retval = 0;
    retval |= this->getNextUint8();
    retval |= static_cast<uint64_t>(this->getNextUint8()) << 8;
    retval |= static_cast<uint64_t>(this->getNextUint8()) << 16;
    retval |= static_cast<uint64_t>(this->getNextUint8()) << 24;
    retval |= static_cast<uint64_t>(this->getNextUint8()) << 32;

    auto lastByte = this->getNextUint8();
    retval |= static_cast<uint64_t>(lastByte) << 40;

    if (lastByte & 0x80)
    {
        retval |= 0xFFFF000000000000;
    }

    return retval;
}

uint16_t DatagramStreamParser::getNextBigEndianUint16()
{
    size_t sizeOfDataType = 2;
    this->ensureRemaining(sizeOfDataType);

    auto lsb = this->getNextUint8();
    auto msb = this->getNextUint8();

    uint16_t retval = lsb;
    retval <<= 8;
    retval |= msb;

    return retval;
}

int16_t DatagramStreamParser::getNextBigEndianInt16()
{
    size_t sizeOfDataType = 2;
    this->ensureRemaining(sizeOfDataType);

    auto lsb = this->getNextUint8();
    auto msb = this->getNextUint8();

    int16_t retval = lsb;
    retval <<= 8;
    retval |= msb;

    return retval;
}

uint32_t DatagramStreamParser::getNextBigEndianUint32()
{
    size_t sizeOfDataType = 4;
    this->ensureRemaining(sizeOfDataType);

    uint32_t retval = 0;
    retval |= static_cast<uint32_t>(this->getNextUint8()) << 24;
    retval |= static_cast<uint32_t>(this->getNextUint8()) << 16;
    retval |= static_cast<uint32_t>(this->getNextUint8()) << 8;
    retval |= this->getNextUint8();

    return retval;
}

uint64_t DatagramStreamParser::getNextBigEndianUint40()
{
    size_t sizeOfDataType = 5;
    this->ensureRemaining(sizeOfDataType);

    uint64_t retval = 0;
    retval |= static_cast<uint64_t>(this->getNextUint8()) << 32;
    retval |= static_cast<uint64_t>(this->getNextUint8()) << 24;
    retval |= static_cast<uint64_t>(this->getNextUint8()) << 16;
    retval |= static_cast<uint64_t>(this->getNextUint8()) << 8;
    retval |= this->getNextUint8();

    return retval;

}

void DatagramStreamParser::skipBytes(size_t numberOfBytesToSkip)
{
    if (numberOfBytesToSkip)
    {
        this->ensureRemaining(numberOfBytesToSkip);
        this->m_offset += numberOfBytesToSkip;
    }
}

int32_t DatagramStreamParser::sumBytes()
{
    int32_t retval = 0;

    for (auto x : this->m_buffer)
    {
        retval += x;
    }

    return retval;
}
