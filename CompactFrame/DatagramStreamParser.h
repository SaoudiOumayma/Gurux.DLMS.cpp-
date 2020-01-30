//
// Copyright 2018 Honeywell Smart Energy.  All Rights Reserved
//

#pragma once
#include "IDatagramStreamParser.h"

#include <string>

class DatagramStreamParser : public IDatagramStreamParser
{
public:
    DatagramStreamParser(const std::string& buffer);

    std::string getRemainingBuffer() override;
    uint8_t getNextUint8() override;
    int8_t getNextInt8() override;
    size_t getTotalDatagramSize() override;
    size_t getTotalBytesProcessedSoFar() override;
    size_t getRemainingUnprocessedBytes() override;
    uint16_t getNextLittleEndianUint16() override;
    int16_t getNextLittleEndianInt16() override;
    uint32_t getNextLittleEndianUint32() override;
    int32_t getNextLittleEndianInt32() override;
    int64_t getNextLittleEndianInt48() override;
    int16_t getNextBigEndianInt16() override;
    uint16_t getNextBigEndianUint16() override;
    uint32_t getNextBigEndianUint32() override;
    uint64_t getNextBigEndianUint40() override;

    void skipBytes(size_t numberOfBytesToSkip) override;
    int32_t sumBytes();

private:
    void ensureRemaining(size_t count) const;

    std::string m_buffer;
    size_t m_offset;
};
