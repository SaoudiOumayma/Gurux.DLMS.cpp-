#include "stdafx.h"
#include "CompactFrameEncrypter.h"

using eams::convert::HexToBin;
using eams::convert::BinToHex;

std::string CompactFrameEncrypter::getSystemTitle() const
{
    return BinToHex(m_systemTitle);
}

std::string CompactFrameEncrypter::getAuthKey() const
{
    return BinToHex(m_authKey);
}

std::string CompactFrameEncrypter::getBlockKey() const
{
    return BinToHex(m_blockKey);
}

void CompactFrameEncrypter::LoadSecurityInfo(const std::string& iniFileName)
{
    INIReader rdr(iniFileName);

    if (rdr.ParseError() != 0) 
    {
        throw std::runtime_error("failed to load config file");
    }

    this->m_blockKey = HexToBin(rdr.Get("ciphering", "block_key", "00000000000000000000000000000000"));
    spdlog::debug(fmt::format("block-key: {}", getBlockKey()));

    this->m_authKey = HexToBin(rdr.Get("ciphering", "auth_key", "00000000000000000000000000000000"));
    spdlog::debug(fmt::format("auth-key: {}", getAuthKey()));

    this->m_systemTitle = HexToBin(rdr.Get("encoding", "system_title", "0000000000000000"));
    spdlog::debug(fmt::format("system-title: {}", getSystemTitle()));

    this->m_frameCounter = rdr.GetInteger("encoding", "frame_counter", 0L);
    spdlog::debug(fmt::format("frame-counter: {}", getFrameCounter()));

    this->m_source = rdr.GetInteger("encoding", "wrapper-source", 0L);
    spdlog::debug(fmt::format("wrapper-client-address: {}", getSource()));

    this->m_destination = rdr.GetInteger("encoding", "wrapper-destination", 0L);
    spdlog::debug(fmt::format("wrapper-server-address: {}", getDestination()));
}

// silly class needed because the SetCipher() method is protected in CGXDLMSNotify for some reason
//
class Notify : public CGXDLMSNotify
{
public:
    Notify(CGXCipher* cipher, bool ln, int logical, int physical, DLMS_INTERFACE_TYPE type) : CGXDLMSNotify(ln, logical, physical, type)
    {
        this->SetCipher(cipher);
    }

};

std::string CompactFrameEncrypter::CompactFrameToDataNotification(const std::string& plainTextCompactFrameBuffer)
{
    
    auto bin = HexToBin(plainTextCompactFrameBuffer);

    spdlog::debug(fmt::format("compact-frame-id: {}", static_cast<unsigned char>(bin[0])));
    spdlog::debug(fmt::format("compact-frame-data: {}", plainTextCompactFrameBuffer));

    unsigned char* bytes = new unsigned char[bin.size()];
    ::memcpy(bytes, bin.data(), bin.size());
    CGXDLMSVariant inner(bytes, bin.size(), DLMS_DATA_TYPE_OCTET_STRING);
    delete[] bytes;
    
    CGXDLMSVariant outer;
    outer.Arr.push_back(inner);
    outer.vt = DLMS_DATA_TYPE_STRUCTURE;
    
    CGXByteBuffer buf;
    GXHelpers::SetData(buf, DLMS_DATA_TYPE_STRUCTURE, outer);
    
    CGXByteBuffer blockKey;
    blockKey.Set(m_blockKey.data(), m_blockKey.size());

    CGXByteBuffer authKey;
    authKey.Set(m_authKey.data(), m_authKey.size());

    CGXByteBuffer systemTitle;
    systemTitle.Set(m_systemTitle.data(), m_systemTitle.size());

    auto cipher = std::make_unique<CGXCipher>(systemTitle);
    cipher->SetBlockCipherKey(blockKey);
    cipher->SetAuthenticationKey(authKey);
    cipher->SetFrameCounter(m_frameCounter);
    cipher->SetSecurity(DLMS_SECURITY_AUTHENTICATION_ENCRYPTION);

    auto notify = std::make_unique<Notify>(cipher.get(), true, this->m_source, this->m_destination, DLMS_INTERFACE_TYPE_WRAPPER);
    notify->SetInvokeID(0);
    notify->SetServiceClass(DLMS_SERVICE_CLASS_UN_CONFIRMED);
    notify->SetPriority(DLMS_PRIORITY_NORMAL);
    notify->SetUseLogicalNameReferencing(true);

    std::vector<CGXByteBuffer> reply;
    notify->GenerateDataNotificationMessages(nullptr, buf, reply);

    if (reply.size() != 1)
    {
        throw std::runtime_error("failed to generate data notification message");
    }

    auto msg = reply[0];
    auto ptr = reinterpret_cast<char*>(msg.GetData());
    auto pdu = BinToHex(std::string(ptr, msg.GetSize()));
    spdlog::debug(fmt::format("dlms-pdu: {}", pdu));
    return pdu;
}

