//
// Copyright 2018 Honeywell Smart Energy.  All Rights Reserved
//

#pragma once
#include <string>

// provides a stream-oriented (stateful) interface for extracting data types from a binary stream.
// primary use case is parsing datagrams received from devices over the network instead of using
// the Metering SDK's MMeter implementation.
//
// std::string datagram = getNextDatagram();
// IDatagramStreamParser* parser = getParser(datagram);
// unsigned long value1 = parser->getNextLittleEndianUint16();
// unsigned long value2 = parser->getNextLittleEndianUint16();
// etc.
//
// the implementing object must keep track of where it is in the datagram (internal state).  parsing
// is sequential, from the beginning of the buffer to the end.
//
// it is not possible to go backward or reset the state.
//
// a DatagramEof exception will be thrown if the parser would run off the end of the buffer and
// the item being requested will not be returned.  in this case the internal state (where in the
// buffer the next item will come from) is not disturbed.
//
class IDatagramStreamParser
{
public:
    virtual ~IDatagramStreamParser() noexcept = default;

    virtual std::string getRemainingBuffer() = 0;

    virtual void skipBytes(size_t numberOfBytesToSkip) = 0;

    virtual size_t getTotalDatagramSize() = 0;
    virtual size_t getTotalBytesProcessedSoFar() = 0;
    virtual size_t getRemainingUnprocessedBytes() = 0;

    virtual uint8_t getNextUint8() = 0;
    virtual int8_t getNextInt8() = 0;

    virtual uint16_t getNextLittleEndianUint16() = 0;
    virtual int16_t getNextLittleEndianInt16() = 0;

    virtual uint32_t getNextLittleEndianUint32() = 0;
    virtual int32_t getNextLittleEndianInt32() = 0;

    virtual int64_t getNextLittleEndianInt48() = 0;

    virtual int16_t getNextBigEndianInt16() = 0;
    virtual uint16_t getNextBigEndianUint16() = 0;
    virtual uint32_t getNextBigEndianUint32() = 0;
    virtual uint64_t getNextBigEndianUint40() = 0;
};
