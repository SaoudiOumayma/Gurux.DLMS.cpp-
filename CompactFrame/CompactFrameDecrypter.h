#pragma once
#include <string>
#include <memory>

class CompactFrameDecrypter
{
public:
    void LoadSecurityInfo(const std::string& iniFileName = "dlms.ini");

    std::string DataNotificationToCompactFrame(const std::string& dataNotificationPdu);

    std::string getAuthKey() const;

    std::string getBlockKey() const;

private:
    std::string m_blockKey;
    std::string m_authKey;
    std::string m_systemTitle;
    long m_frameCounter{ 0L };
    long m_source{ 0L };
    long m_destination{ 0L };
};
