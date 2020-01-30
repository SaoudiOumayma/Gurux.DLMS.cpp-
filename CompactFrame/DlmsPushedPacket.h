#pragma once
#include "IDatagramStreamParser.h"
#include "../development/include/GXReplyData.h"
#include <string>

class DlmsPushPacket
{
public:
    DlmsPushPacket(const std::string& packetData)
    {
        this->parse(packetData);
    }

    unsigned getVersion() const
    {
        return m_version;
    }

    unsigned getSource() const
    {
        return m_source;
    }

    unsigned getDestination() const
    {
        return m_destination;
    }

    size_t getPacketLength() const
    {
        return m_packetLength;
    }

    std::string getSystemTitle() const
    {
        return m_systemTitle;
    }

    std::string getApdu() const
    {
        return m_apdu;
    }

    enum class SecurityControl
    {
        NONE, AUTH_ONLY, ENCRYPTION_ONLY, AUTH_ENCRYPTION
    };

    SecurityControl getSecurityControl() const
    {
        return m_securityControl;
    }

    unsigned getFrameCounter() const
    {
        return m_frameCounter;
    }

    CGXReplyData getDecryptedData(const std::string& encryptionKey, const std::string& authKey);


    static size_t parseLengthField(IDatagramStreamParser& parser);
    static std::string parseEncodedField(IDatagramStreamParser& parser);

private:
    void parse(const std::string& packetData);


    unsigned m_version{ 0U };
    unsigned m_source{ 0U };
    unsigned m_destination{ 0U };
    size_t m_packetLength{ 0U };
    std::string m_systemTitle;
    SecurityControl m_securityControl{ SecurityControl::NONE };
    unsigned m_frameCounter{ 0U };
    std::string m_apdu;
};