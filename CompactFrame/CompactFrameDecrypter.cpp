#include "stdafx.h"
#include "CompactFrameDecrypter.h"
#include "DlmsPushedPacket.h"

using eams::convert::HexToBin;
using eams::convert::BinToHex;

std::string CompactFrameDecrypter::getAuthKey() const
{
    return BinToHex(m_authKey);
}

std::string CompactFrameDecrypter::getBlockKey() const
{
    return BinToHex(m_blockKey);
}

void CompactFrameDecrypter::LoadSecurityInfo(const std::string& iniFileName)
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
}

std::string CompactFrameDecrypter::DataNotificationToCompactFrame(const std::string& dataNotificationPdu)
{
    auto binPdu = HexToBin(dataNotificationPdu);

    DlmsPushPacket packet(binPdu);
    spdlog::debug(fmt::format("wrapper-client-address: {}", packet.getSource()));
    spdlog::debug(fmt::format("wrapper-server-address: {}", packet.getDestination()));
    spdlog::debug(fmt::format("system-title: {}", packet.getSystemTitle()));
    spdlog::debug(fmt::format("frame-counter: {}", packet.getFrameCounter()));
    spdlog::debug(fmt::format("dlms-pdu: {}", dataNotificationPdu));

    CGXDLMSSecureClient cl(
        true,
        packet.getSource(),
        packet.getDestination(),
        DLMS_AUTHENTICATION_NONE,
        NULL,
        DLMS_INTERFACE_TYPE_WRAPPER);

    CGXByteBuffer bb(binPdu.size());
    bb.AddString(binPdu);

    CGXReplyData data;
    data.Clear();

    CGXReplyData notify;
    notify.Clear();

    CGXByteBuffer authKey(m_authKey.size());
    authKey.Clear();
    authKey.AddString(m_authKey);

    CGXByteBuffer blockKey(m_blockKey.size());
    blockKey.Clear();
    blockKey.AddString(m_blockKey);

    cl.GetCiphering()->SetAuthenticationKey(authKey);
    cl.GetCiphering()->SetBlockCipherKey(blockKey);

    int ret{ 0 };

    if ((ret = cl.GetData(bb, data, notify)) != 0 && ret != DLMS_ERROR_CODE_FALSE)
    {
        throw std::runtime_error("failed to parse PDU");
    }

    auto values = data.GetValue().Arr;
    if (values.size() != 1U)
    {
        throw std::runtime_error("did not understand message format");
    }

    auto payload = values[0];
    auto size = payload.GetSize();

    CGXByteBuffer frame;
    payload.GetBytes(frame);

    char* p = reinterpret_cast<char*>(frame.GetData());

    auto retval = BinToHex(std::string(p, frame.GetSize()));
    spdlog::debug(fmt::format("compact-frame-id: {}", static_cast<unsigned char>(p[0])));
    spdlog::debug(fmt::format("compact-frame-data: {}", retval));

    return retval;
}