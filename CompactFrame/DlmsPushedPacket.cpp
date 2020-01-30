#include "stdafx.h"
#include "DlmsPushedPacket.h"
#include "DatagramStreamParser.h"

size_t DlmsPushPacket::parseLengthField(IDatagramStreamParser& parser)
{
    auto b1 = parser.getNextUint8();

    if(b1 <= 0x80)
    {
        return b1;
    }

    if(b1 == 0x81)
    {
        return parser.getNextUint8();
    }

    else if(b1 == 0x82)
    {
        return parser.getNextBigEndianUint16();
    }

    else if(b1 == 0x84)
    {
        return parser.getNextBigEndianUint32();
    }

    else
    {
        throw std::runtime_error(fmt::format("invalid length for BER encoded field ({})", b1));
    }
}

std::string DlmsPushPacket::parseEncodedField(IDatagramStreamParser& parser)
{
    const auto length = parseLengthField(parser);
    std::string retval;
    retval.reserve(length);

    for(size_t i = 0; i < length; i++)
    {
        retval.push_back(parser.getNextUint8());
    }

    return retval;
}

void DlmsPushPacket::parse(const std::string& packetData)
{
    this->m_apdu = packetData;

    DatagramStreamParser parser(this->getApdu());
    this->m_version = parser.getNextBigEndianUint16();
    this->m_source = parser.getNextBigEndianUint16();
    this->m_destination = parser.getNextBigEndianUint16();
    this->m_packetLength = parser.getNextBigEndianUint16();

    const auto bytesParsedSoFar = parser.getTotalBytesProcessedSoFar();
    const auto expectedDataSize = this->m_packetLength + bytesParsedSoFar;
    const auto actualDataSize = this->getApdu().size();

    if(expectedDataSize != actualDataSize)
    {
        throw std::runtime_error(fmt::format("expected packet length of {} based on wrapper length field of {} but actual data size is {}",
                                            expectedDataSize, this->m_packetLength, actualDataSize));
    }

    const auto tag = parser.getNextUint8();

    if(tag != 0xDB)  // general-glo-ciphering
    {
        throw std::runtime_error(fmt::format("expected DLMS command general-glo-ciphering (219) for pushed data notification but got {}", tag));
    }

    this->m_systemTitle = parseEncodedField(parser);

    auto length = parseLengthField(parser);
    auto remainder = parser.getRemainingUnprocessedBytes();

    if(length != remainder)
    {
        throw std::runtime_error(fmt::format("invalid packet, expected {} bytes but only got {}", length, remainder));
    }

    auto securityControlByte = parser.getNextUint8();

    switch(securityControlByte)
    {
        case 0x10:
            this->m_securityControl = SecurityControl::AUTH_ONLY;
            break;

        case 0x20:
            this->m_securityControl = SecurityControl::ENCRYPTION_ONLY;
            break;

        case 0x30:
            this->m_securityControl = SecurityControl::AUTH_ENCRYPTION;
            break;

        default:
            this->m_securityControl = SecurityControl::NONE;
    }

    this->m_frameCounter = parser.getNextBigEndianUint32();
}

CGXReplyData DlmsPushPacket::getDecryptedData(const std::string& encryption, const std::string& authentication)
{
    using eams::convert::to;

    CGXDLMSSecureClient cl(true, this->getDestination(), this->getSource(), DLMS_AUTHENTICATION_NONE, NULL, DLMS_INTERFACE_TYPE_WRAPPER);

    CGXByteBuffer bb(to<int>(this->getApdu().size()));
    bb.AddString(this->getApdu());

    CGXReplyData data;
    data.Clear();

    CGXReplyData notify;
    notify.Clear();

    CGXByteBuffer authKey(to<int>(authentication.size()));
    authKey.Clear();
    authKey.AddString(authentication);

    CGXByteBuffer blockKey(to<int>(encryption.size()));
    blockKey.Clear();
    blockKey.AddString(encryption);

    cl.GetCiphering()->SetAuthenticationKey(authKey);
    cl.GetCiphering()->SetBlockCipherKey(blockKey);

    int ret{ 0 };

    if((ret = cl.GetData(bb, data, notify)) != 0 && ret != DLMS_ERROR_CODE_FALSE)
    {
        throw std::runtime_error(fmt::format("Failed to parsed pushed data with error {}", ret));
    }

    return data;
}