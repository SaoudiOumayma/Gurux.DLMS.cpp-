#pragma once

#include <string>
#include <memory>


class CompactFrameEncrypter
{
public:
    void LoadSecurityInfo(const std::string& iniFileName = "dlms.ini");

    std::string CompactFrameToDataNotification(const std::string& plainTextCompactFrameBuffer);

    std::string getSystemTitle() const;

    std::string getAuthKey() const;

    std::string getBlockKey() const;
    
    long getSource() const
    {
        return m_source;
    }

    long getDestination() const
    {
        return m_destination;
    }

    long getFrameCounter() const
    {
        return m_frameCounter;
    }

private:
    std::string m_blockKey;
    std::string m_authKey;
    std::string m_systemTitle;
    long m_frameCounter{ 0L };
    long m_source{ 0L };
    long m_destination{ 0L };
};

