//
// --------------------------------------------------------------------------
//  Gurux Ltd
//
//
//
// Filename:        $HeadURL$
//
// Version:         $Revision$,
//                  $Date$
//                  $Author$
//
// Copyright (c) Gurux Ltd
//
//---------------------------------------------------------------------------
//
//  DESCRIPTION
//
// This file is a part of Gurux Device Framework.
//
// Gurux Device Framework is Open Source software; you can redistribute it
// and/or modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; version 2 of the License.
// Gurux Device Framework is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU General Public License for more details.
//
// More information of Gurux products: http://www.gurux.org
//
// This code is licensed under the GNU General Public License v2.
// Full text may be retrieved at http://www.gnu.org/licenses/gpl-2.0.txt
//---------------------------------------------------------------------------

#include "../include/communication.h"

CGXCommunication::CGXCommunication(CGXDLMSClient* pParser, int wt, bool trace) :
    m_WaitTime(wt), m_Parser(pParser),
    m_socket(-1), m_Trace(trace)
{
#if defined(_WIN32) || defined(_WIN64)//Windows includes
    ZeroMemory(&m_osReader, sizeof(OVERLAPPED));
    ZeroMemory(&m_osWrite, sizeof(OVERLAPPED));
    m_hComPort = INVALID_HANDLE_VALUE;
    m_osReader.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    m_osWrite.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
#endif
}

CGXCommunication::~CGXCommunication(void)
{
    Close();
}

//Close connection to the meter.
int CGXCommunication::Close()
{
    int ret;
    std::vector<CGXByteBuffer> data;
    CGXReplyData reply;
    if ((ret = m_Parser->DisconnectRequest(data)) != 0 ||
            (ret = ReadDataBlock(data, reply)) != 0)
    {
        //Show error but continue close.
    }
    if (m_hComPort != INVALID_HANDLE_VALUE)
    {
#if defined(_WIN32) || defined(_WIN64)//Windows includes
        CloseHandle(m_hComPort);
        m_hComPort = INVALID_HANDLE_VALUE;
        CloseHandle(m_osReader.hEvent);
        CloseHandle(m_osWrite.hEvent);
#else
        close(m_hComPort);
#endif
    }
    if (m_socket != -1)
    {
#if defined(_WIN32) || defined(_WIN64)//Windows includes
        closesocket(m_socket);
#else
        close(m_socket);
#endif
        m_socket = -1;
    }
    return ret;
}

//Make TCP/IP connection to the meter.
int CGXCommunication::Connect(const char* pAddress, unsigned short Port)
{
    Close();
    //create socket.
    m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (m_socket == -1)
    {
        assert(0);
        return ERROR_CODES_INVALID_PARAMETER;
    }
    sockaddr_in add;
    add.sin_port = htons(Port);
    add.sin_family = AF_INET;
    add.sin_addr.s_addr = inet_addr(pAddress);
    //If address is give as name
    if(add.sin_addr.s_addr == INADDR_NONE)
    {
        hostent *Hostent = gethostbyname(pAddress);
        if (Hostent == NULL)
        {
#if defined(_WIN32) || defined(_WIN64)//Windows includes
            int err = WSAGetLastError();
#else
            int err = h_errno;
#endif
            Close();
            return err;
        };
        add.sin_addr = *(in_addr*)(void*)Hostent->h_addr_list[0];
    };

    //Connect to the meter.
    int ret = connect(m_socket, (sockaddr*)&add, sizeof(sockaddr_in));
    if (ret == -1)
    {
        return ERROR_CODES_INVALID_PARAMETER;
    };
    return ERROR_CODES_OK;
}

#if defined(_WIN32) || defined(_WIN64)//Windows
int CGXCommunication::GXGetCommState(HANDLE hWnd, LPDCB dcb)
{
    ZeroMemory(dcb, sizeof(DCB));
    dcb->DCBlength = sizeof(DCB);
    if (!GetCommState(hWnd, dcb))
    {
        DWORD err = GetLastError(); //Save occured error.
        if (err == 995)
        {
            COMSTAT comstat;
            unsigned long RecieveErrors;
            if (!ClearCommError(hWnd, &RecieveErrors, &comstat))
            {
                return GetLastError();
            }
            if (!GetCommState(hWnd, dcb))
            {
                return GetLastError(); //Save occured error.
            }
        }
        else
        {
            //If USB to serial port converters do not implement this.
            if (err != ERROR_INVALID_FUNCTION)
            {
                return err;
            }
        }
    }
    return ERROR_CODES_OK;
}

int CGXCommunication::GXSetCommState(HANDLE hWnd, LPDCB DCB)
{
    if (!SetCommState(hWnd, DCB))
    {
        DWORD err = GetLastError(); //Save occured error.
        if (err == 995)
        {
            COMSTAT comstat;
            unsigned long RecieveErrors;
            if (!ClearCommError(hWnd, &RecieveErrors, &comstat))
            {
                return GetLastError();
            }
            if (!SetCommState(hWnd, DCB))
            {
                return GetLastError();
            }
        }
        else
        {
            //If USB to serial port converters do not implement this.
            if (err != ERROR_INVALID_FUNCTION)
            {
                return err;
            }
        }
    }
    return ERROR_CODES_OK;
}

#endif //Windows

int CGXCommunication::Read(unsigned char eop, CGXByteBuffer& reply)
{
#if defined(_WIN32) || defined(_WIN64)//Windows
    unsigned long RecieveErrors;
    COMSTAT comstat;
    DWORD bytesRead = 0;
#else //If Linux.
    unsigned short bytesRead = 0;
    int ret;
#endif
    int pos;
    unsigned long cnt = 1;
    bool bFound = false;
    int lastReadIndex = 0;
    do
    {
#if defined(_WIN32) || defined(_WIN64)//Windows
        //We do not want to read byte at the time.
        if (!ClearCommError(m_hComPort, &RecieveErrors, &comstat))
        {
            return ERROR_CODES_SEND_FAILED;
        }
        bytesRead = 0;
        cnt = 1;
        //Try to read at least one byte.
        if (comstat.cbInQue > 0)
        {
            cnt = comstat.cbInQue;
        }
        //If there is more data than can fit to buffer.
        if (cnt > RECEIVE_BUFFER_SIZE)
        {
            cnt = RECEIVE_BUFFER_SIZE;
        }
        if (!ReadFile(m_hComPort, m_Receivebuff, cnt, &bytesRead, &m_osReader))
        {
            DWORD nErr = GetLastError();
            if (nErr != ERROR_IO_PENDING)
            {
                return ERROR_CODES_RECEIVE_FAILED;
            }
            //Wait until data is actually read
            if (::WaitForSingleObject(m_osReader.hEvent, m_WaitTime) != WAIT_OBJECT_0)
            {
                return ERROR_CODES_RECEIVE_FAILED;
            }
            if (!GetOverlappedResult(m_hComPort, &m_osReader, &bytesRead, TRUE))
            {
                return ERROR_CODES_RECEIVE_FAILED;
            }
        }
#else
        //Get bytes available.
        ret = ioctl(m_hComPort, FIONREAD, &cnt);
        //If driver is not supporting this functionality.
        if (ret < 0)
        {
            cnt = RECEIVE_BUFFER_SIZE;
        }
        else if (cnt == 0)
        {
            //Try to read at least one byte.
            cnt = 1;
        }
        //If there is more data than can fit to buffer.
        if (cnt > RECEIVE_BUFFER_SIZE)
        {
            cnt = RECEIVE_BUFFER_SIZE;
        }
        bytesRead = read(m_hComPort, m_Receivebuff, cnt);
        if (bytesRead == 0xFFFF)
        {
            //If wait time has elapsed.
            if (errno == EAGAIN)
            {
                return ERROR_CODES_RECEIVE_FAILED;
            }
            //If connection is closed.
            else if (errno == EBADF)
            {
                return ERROR_CODES_RECEIVE_FAILED;
            }
            else
            {
                return ERROR_CODES_RECEIVE_FAILED;
            }
        }
#endif
        reply.AddRange(m_Receivebuff, bytesRead);
        //Note! Some USB converters can return true for ReadFile and Zero as bytesRead.
        //In that case wait for a while and read again.
        if (bytesRead == 0)
        {
#if defined(_WIN32) || defined(_WIN64)//Windows
            Sleep(100);
#else
            usleep(100000);
#endif
            continue;
        }
        if (reply.GetSize() > 5)
        {
            //Some optical strobes can return extra bytes.
            for(pos = reply.GetSize() - 1; pos != lastReadIndex; --pos)
            {
                if(reply.GetData()[pos] == eop)
                {
                    bFound = true;
                    break;
                }
            }
            lastReadIndex = pos;
        }
    }
    while(!bFound);
    return ERROR_CODES_OK;
}

//Open serial port.
int CGXCommunication::Open(const char* port, bool iec, int maxBaudrate)
{
    Close();
    CGXByteBuffer reply;
    unsigned short baudRate;
    int ret, len, pos;
    unsigned char ch;
    //In Linux serial port name might be very long.
    char buff[50];
#if defined(_WIN32) || defined(_WIN64)
    DCB dcb = {0};
    unsigned long sendSize = 0;
#if _MSC_VER > 1000
    sprintf_s(buff, 50, "\\\\.\\%s", port);
#else
    sprintf(buff, "\\\\.\\%s", port);
#endif
    //Open serial port for read / write. Port can't share.
    m_hComPort = CreateFileA(buff,
                             GENERIC_READ | GENERIC_WRITE, 0, NULL,
                             OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
    if (m_hComPort == INVALID_HANDLE_VALUE)
    {
        return ERROR_CODES_INVALID_PARAMETER;
    }
    dcb.DCBlength = sizeof(DCB);
    dcb.fBinary = 1;
    dcb.fOutX = dcb.fInX = 0;
    //Abort all reads and writes on Error.
    dcb.fAbortOnError = 1;
    if (iec)
    {
        dcb.BaudRate = 300;
        dcb.ByteSize = 7;
        dcb.StopBits = ONESTOPBIT;
        dcb.Parity = EVENPARITY;
    }
    else
    {
        dcb.BaudRate = 9600;
        dcb.ByteSize = 8;
        dcb.StopBits = ONESTOPBIT;
        dcb.Parity = NOPARITY;
    }
    if ((ret = GXSetCommState(m_hComPort, &dcb)) != 0)
    {
        return ERROR_CODES_INVALID_PARAMETER;
    }
#else //#if defined(__LINUX__)
    struct termios options;
    // read/write | not controlling term | don't wait for DCD line signal.
    m_hComPort = open(port, O_RDWR | O_NOCTTY /*| O_NONBLOCK*/);  // with O_NONBLOCK flag only one byte is being sent
    if(m_hComPort == -1) // if open is unsuccessful.
    {
        printf("Failed to Open port.\r");
        return ERROR_CODES_INVALID_PARAMETER;
    }
    else
    {
        if(!isatty(m_hComPort))
        {
            printf("Failed to Open port. This is not a serial port.\r");
            return ERROR_CODES_INVALID_PARAMETER;
        }

        if ((ioctl(m_hComPort, TIOCEXCL) == -1))
        {
            printf("Failed to Open port. Exclusive access denied.\r");
            return ERROR_CODES_INVALID_PARAMETER;
        }

        memset(&options, 0, sizeof(options));
        options.c_iflag = 0;
        options.c_oflag = 0;
        if (iec)
        {
            options.c_cflag |= PARENB;
            options.c_cflag &= ~PARODD;
            options.c_cflag &= ~CSTOPB;
            options.c_cflag &= ~CSIZE;
            options.c_cflag |= CS7;
            //Set Baud Rates
            cfsetospeed (&options, B300);
            cfsetispeed (&options, B300);
        }
        else
        {
            // 8n1, see termios.h for more information
            options.c_cflag = CS8 | CREAD | CLOCAL;
            /*
            options.c_cflag &= ~PARENB
            options.c_cflag &= ~CSTOPB
            options.c_cflag &= ~CSIZE;
            options.c_cflag |= CS8;
            */
            //Set Baud Rates
            cfsetospeed(&options, B9600);
            cfsetispeed(&options, B9600);
        }
        options.c_lflag = 0;
        options.c_cc[VMIN] = 1;
        //How long we are waiting reply charachter from serial port.
        options.c_cc[VTIME] = m_WaitTime / 1000;
        //hardware flow control is used as default.
        //options.c_cflag |= CRTSCTS;
        if (tcsetattr(m_hComPort, TCSAFLUSH, &options) != 0)
        {
            printf("Failed to Open port. tcsetattr failed.\r");
            return ERROR_CODES_INVALID_PARAMETER;
        }
    }
#endif
    if (iec)
    {
#if _MSC_VER > 1000
        strcpy_s(buff, 50, "/?!\r\n");
#else
        strcpy(buff, "/?!\r\n");
#endif
        len = strlen(buff);
        if (m_Trace)
        {
            printf("\r\n<-");
            for(pos = 0; pos != len; ++pos)
            {
                printf("%.2X ", buff[pos]);
            }
            printf("\r\n");
        }
#if defined(_WIN32) || defined(_WIN64)
        ret = WriteFile(m_hComPort, buff, len, &sendSize, &m_osWrite);
        if (ret == 0)
        {
            DWORD err = GetLastError();
            //If error occurs...
            if (err != ERROR_IO_PENDING)
            {
                return ERROR_CODES_SEND_FAILED;
            }
            //Wait until data is actually sent
            WaitForSingleObject(m_osWrite.hEvent, INFINITE);
        }
#else //#if defined(__LINUX__)
        ret = write(m_hComPort, buff, len);
        if (ret != len)
        {
            return ERROR_CODES_SEND_FAILED;
        }
#endif
        //Read reply data.
        if (Read('\n', reply) != 0)
        {
            return ERROR_CODES_INVALID_PARAMETER;
        }
        if (m_Trace)
        {
            printf("-> %s\r\n", reply.ToHexString().c_str());
        }
        if (reply.GetUInt8(&ch) != 0 || ch != '/')
        {
            return ERROR_CODES_SEND_FAILED;
        }
        //Get used baud rate.
        if ((ret = reply.GetUInt8(4, &ch)) != 0)
        {
            return ERROR_CODES_SEND_FAILED;
        }
        switch (ch)
        {
        case '0':
#if defined(_WIN32) || defined(_WIN64)
            baudRate = 300;
#else
            baudRate = B300;
#endif
            break;
        case '1':
#if defined(_WIN32) || defined(_WIN64)
            baudRate = 600;
#else
            baudRate = B600;
#endif
            break;
        case '2':
#if defined(_WIN32) || defined(_WIN64)
            baudRate = 1200;
#else
            baudRate = B1200;
#endif
            break;
        case '3':
#if defined(_WIN32) || defined(_WIN64)
            baudRate = 2400;
#else
            baudRate = B2400;
#endif
            break;
        case '4':
#if defined(_WIN32) || defined(_WIN64)
            baudRate = 4800;
#else
            baudRate = B4800;
#endif
            break;
        case '5':
#if defined(_WIN32) || defined(_WIN64)
            baudRate = 9600;
#else
            baudRate = B9600;
#endif
            break;
        case '6':
#if defined(_WIN32) || defined(_WIN64)
            baudRate = 19200;
#else
            baudRate = B19200;
#endif
            break;
        default:
            return ERROR_CODES_INVALID_PARAMETER;
        }
        //Send ACK
        buff[0] = 0x06;
        //Send Protocol control character
        buff[1] = '2';// "2" HDLC protocol procedure (Mode E)
        buff[2] = (char) ch;
        buff[3] = '2';
        buff[4] = (char) 0x0D;
        buff[5] = 0x0A;
        len = 6;
        reply.Clear();
        if (m_Trace)
        {
            printf("\r\n<-");
            for(pos = 0; pos != len; ++pos)
            {
                printf("%.2X ", buff[pos]);
            }
            printf("\r\n");
        }
#if defined(_WIN32) || defined(_WIN64)//Windows
        ret = WriteFile(m_hComPort, buff, len, &sendSize, &m_osWrite);
        if (ret == 0)
        {
            int err = GetLastError();
            //If error occurs...
            if (err != ERROR_IO_PENDING)
            {
                printf("WriteFile %d\r\n", err);
                return ERROR_CODES_SEND_FAILED;
            }
            //Wait until data is actually sent
            WaitForSingleObject(m_osWrite.hEvent, INFINITE);
        }
#else //#if defined(__LINUX__)
        ret = write(m_hComPort, buff, len);
        if (ret != len)
        {
            return ERROR_CODES_SEND_FAILED;
        }
#endif
#if defined(_WIN32) || defined(_WIN64)//Windows
        //This sleep is in standard. Do not remove.
        Sleep(1000);
        dcb.BaudRate = baudRate;
        if ((ret = GXSetCommState(m_hComPort, &dcb)) != 0)
        {
            return ERROR_CODES_INVALID_PARAMETER;
        }
        printf("New baudrate %d\r\n", (int) dcb.BaudRate);
        len = 6;
        if ((ret = Read('\n', reply)) != 0)
        {
            printf("Read %d\r\n", ret);
            return ERROR_CODES_INVALID_PARAMETER;
        }
        dcb.ByteSize = 8;
        dcb.StopBits = ONESTOPBIT;
        dcb.Parity = NOPARITY;
        if ((ret = GXSetCommState(m_hComPort, &dcb)) != 0)
        {
            return ERROR_CODES_INVALID_PARAMETER;
        }
#else
        //This sleep is in standard. Do not remove.
        usleep(1000000);
        // 8n1, see termios.h for more information
        options.c_cflag=CS8|CREAD|CLOCAL;
        //Set Baud Rates
        cfsetospeed (&options, baudRate);
        cfsetispeed (&options, baudRate);
        if (tcsetattr(m_hComPort, TCSAFLUSH, &options) != 0)
        {
            printf("Failed to Open port. tcsetattr failed.\r");
            return ERROR_CODES_INVALID_PARAMETER;
        }
#endif
    }
    return ERROR_CODES_OK;
}

//Initialize connection to the meter.
int CGXCommunication::InitializeConnection()
{
    TRACE1("InitializeConnection\r\n");
    std::vector<CGXByteBuffer> data;
    CGXReplyData reply;
    int ret = 0;
    //Get meter's send and receive buffers size.
    if ((ret = m_Parser->SNRMRequest(data)) != 0 ||
            (ret = ReadDataBlock(data, reply)) != 0 ||
            (ret = m_Parser->ParseUAResponse(reply.GetData())) != 0)
    {
        TRACE("SNRMRequest failed %d.\r\n", ret);
        return ret;
    }
    reply.Clear();
    if ((ret = m_Parser->AARQRequest(data)) != 0 ||
            (ret = ReadDataBlock(data, reply)) != 0 ||
            (ret = m_Parser->ParseAAREResponse(reply.GetData())) != 0)
    {
        if (ret == ERROR_CODES_APPLICATION_CONTEXT_NAME_NOT_SUPPORTED)
        {
            TRACE1("Use Logical Name referencing is wrong. Change it!\r\n");
            return ret;
        }
        TRACE("AARQRequest failed %d.\r\n", ret);
        return ret;
    }
    return ERROR_CODES_OK;
}

// Read DLMS Data frame from the device.
int CGXCommunication::ReadDLMSPacket(CGXByteBuffer& data, CGXReplyData& reply)
{
    int ret;
    CGXByteBuffer bb;
    std::string tmp;
    if (data.GetSize() == 0)
    {
        return ERROR_CODES_OK;
    }
    Now(tmp);
    tmp = "<- " + tmp;
    tmp += "\t" + data.ToHexString();
    if (m_Trace)
    {
        printf("%s\r\n", tmp.c_str());
    }
    GXHelpers::Write("trace.txt", tmp + "\r\n");
    int len = data.GetSize();
    if (m_hComPort != INVALID_HANDLE_VALUE)
    {
#if defined(_WIN32) || defined(_WIN64)//Windows includes
        DWORD sendSize = 0;
        BOOL bRes = ::WriteFile(m_hComPort, data.GetData(), len, &sendSize, &m_osWrite);
        if (!bRes)
        {
            DWORD err = GetLastError();
            //If error occurs...
            if (err != ERROR_IO_PENDING)
            {
                return ERROR_CODES_SEND_FAILED;
            }
            //Wait until data is actually sent
            ::WaitForSingleObject(m_osWrite.hEvent, INFINITE);
        }
#else
        ret = write(m_hComPort, data.GetData(), len);
        if (ret != len)
        {
            return ERROR_CODES_SEND_FAILED;
        }
#endif
    }
    else if (send(m_socket, (const char*) data.GetData(), len, 0) == -1)
    {
        //If error has occured
        return ERROR_CODES_SEND_FAILED;
    }
    // Loop until whole DLMS packet is received.
    while ((ret = m_Parser->GetData(bb, reply)) == ERROR_CODES_FALSE)
    {
        if (m_hComPort != INVALID_HANDLE_VALUE)
        {
            if(Read(0x7E, bb) != 0)
            {
                return ERROR_CODES_SEND_FAILED;
            }
        }
        else
        {
            len = RECEIVE_BUFFER_SIZE;
            if ((ret = recv(m_socket, (char*) m_Receivebuff, len, 0)) == -1)
            {
                return ERROR_CODES_RECEIVE_FAILED;
            }
            bb.AddRange(m_Receivebuff, ret);
        }
    }
    tmp = "";
    Now(tmp);
    tmp = "-> " + tmp;
    tmp += "\t" + bb.ToHexString();
    if (m_Trace)
    {
        printf("%s\r\n", tmp.c_str());
    }
    GXHelpers::Write("trace.txt", tmp);
    return ret;
}

int CGXCommunication::ReadDataBlock(std::vector<CGXByteBuffer>& data, CGXReplyData& reply)
{
    //If ther is no data to send.
    if (data.size() == 0)
    {
        return ERROR_CODES_OK;
    }
    int ret;
    CGXByteBuffer bb;
    //Send data.
    for(std::vector<CGXByteBuffer>::iterator it = data.begin(); it != data.end(); ++it)
    {
        //Send data.
        if ((ret = ReadDLMSPacket(*it, reply)) != ERROR_CODES_OK)
        {
            return ret;
        }
        while (reply.IsMoreData())
        {
            bb.Clear();
            if ((ret = m_Parser->ReceiverReady(reply.GetMoreData(), bb)) != 0)
            {
                return ret;
            }
            if ((ret = ReadDLMSPacket(bb, reply)) != ERROR_CODES_OK)
            {
                return ret;
            }
        }
    }
    return ERROR_CODES_OK;
}

//Get Association view.
int CGXCommunication::GetObjects(CGXDLMSObjectCollection& objects)
{
    TRACE1("GetAssociationView\r\n");
    int ret;
    std::vector<CGXByteBuffer> data;
    CGXReplyData reply;
    if ((ret = m_Parser->GetObjectsRequest(data)) != 0 ||
            (ret = ReadDataBlock(data, reply)) != 0 ||
            (ret = m_Parser->ParseObjects(reply.GetData(), objects, false)) != 0)
    {
        TRACE("GetObjects failed %d.\r\n", ret);
        return ret;
    }
    return ERROR_CODES_OK;
}

//Update SN or LN access list.
int CGXCommunication::UpdateAccess(CGXDLMSObject* pObject, CGXDLMSObjectCollection& Objects)
{
    CGXDLMSVariant data;
    int ret = Read(pObject, 2, data);
    if (ret != ERROR_CODES_OK)
    {
        return ret;
    }
    if (data.vt != DLMS_DATA_TYPE_ARRAY)
    {
        return ERROR_CODES_INVALID_PARAMETER;
    }
    for(std::vector<CGXDLMSVariant>::iterator obj = data.Arr.begin(); obj != data.Arr.end(); ++obj)
    {
        if (obj->vt != DLMS_DATA_TYPE_STRUCTURE || obj->Arr.size() != 4)
        {
            return ERROR_CODES_INVALID_PARAMETER;
        }
        CGXDLMSVariant& access_rights = obj->Arr[3];
        if (access_rights.vt != DLMS_DATA_TYPE_STRUCTURE || access_rights.Arr.size() != 2)
        {
            return ERROR_CODES_INVALID_PARAMETER;
        }
        OBJECT_TYPE type = (OBJECT_TYPE) obj->Arr[0].uiVal;
        // unsigned char version = obj->Arr[1].bVal;
        std::string ln;
        GXHelpers::GetLogicalName(obj->Arr[2].byteArr, ln);
        CGXDLMSObject* pObj = Objects.FindByLN(type, ln);
        if (pObj != NULL)
        {
            //Attribute access.
            for(std::vector<CGXDLMSVariant>::iterator it = access_rights.Arr[0].Arr.begin();
                    it != access_rights.Arr[0].Arr.end(); ++it)
            {
                unsigned char id = it->Arr[0].bVal;
                ACCESSMODE access = (ACCESSMODE) it->Arr[1].bVal;
                pObj->SetAccess(id, access);
            }
            //Method access.
            for(std::vector<CGXDLMSVariant>::iterator it = access_rights.Arr[1].Arr.begin();
                    it != access_rights.Arr[1].Arr.end(); ++it)
            {
                unsigned char id = it->Arr[0].bVal;
                METHOD_ACCESSMODE access = (METHOD_ACCESSMODE) it->Arr[1].bVal;
                pObj->SetMethodAccess(id, access);
            }
        }
    }
    return ERROR_CODES_OK;
}

//Read selected object.
int CGXCommunication::Read(CGXDLMSObject* pObject, int attributeIndex, CGXDLMSVariant& value)
{
    value.Clear();
    int ret;
    std::vector<CGXByteBuffer> data;
    CGXReplyData reply;
    //Get meter's send and receive buffers size.
    CGXDLMSVariant name = pObject->GetName();
    if ((ret = m_Parser->Read(name, pObject->GetObjectType(), attributeIndex, data)) != 0 ||
            (ret = ReadDataBlock(data, reply)) != 0 ||
            (ret = m_Parser->UpdateValue(*pObject, attributeIndex, reply.GetValue())) != 0)
    {
        return ret;
    }
    //Update data type.
    DLMS_DATA_TYPE type;
    if ((ret = pObject->GetDataType(attributeIndex, type)) != 0)
    {
        return ret;
    }
    if (type == DLMS_DATA_TYPE_NONE)
    {
        type = reply.GetValue().vt;
        if ((ret = pObject->SetDataType(attributeIndex, type)) != 0)
        {
            return ret;
        }
    }
    //Get read value as string.
    //Note! This is for example. It's faster if you handle read COSEM object directly.
    std::vector<std::string> values;
    pObject->GetValues(values);
    value = values[attributeIndex - 1];
    return ERROR_CODES_OK;
}

//Write selected object.
int CGXCommunication::Write(CGXDLMSObject* pObject, int attributeIndex, CGXDLMSVariant& value)
{
    int ret;
    std::vector<CGXByteBuffer> data;
    CGXReplyData reply;
    //Get meter's send and receive buffers size.
    if ((ret = m_Parser->Write(pObject, attributeIndex, value, data)) != 0 ||
            (ret = ReadDataBlock(data, reply)) != 0)
    {
        return ret;
    }
    return ERROR_CODES_OK;
}

int CGXCommunication::Method(CGXDLMSObject* pObject, int attributeIndex, CGXDLMSVariant& value)
{
    int ret;
    std::vector<CGXByteBuffer> data;
    CGXReplyData reply;
    //Get meter's send and receive buffers size.
    if ((ret = m_Parser->Method(pObject, attributeIndex, value, data)) != 0 ||
            (ret = ReadDataBlock(data, reply)) != 0)
    {
        return ret;
    }
    return ERROR_CODES_OK;
}

int CGXCommunication::ReadRowsByRange(CGXDLMSProfileGeneric* pObject, struct tm* start, struct tm* end, CGXDLMSVariant& rows)
{
    rows.Clear();
    int ret;
    std::vector<CGXByteBuffer> data;
    CGXReplyData reply;
    if ((ret = m_Parser->ReadRowsByRange(pObject, start, end, data)) != 0 ||
            (ret = ReadDataBlock(data, reply)) != 0 ||
            (ret = m_Parser->UpdateValue(*pObject, 2, reply.GetValue())) != 0)
    {
        return ret;
    }
    //Get rows value as string.
    //Note! This is for example. It's faster if you handle read COSEM object directly.
    std::vector<std::string> values;
    pObject->GetValues(values);
    rows = values[2 - 1];
    return ERROR_CODES_OK;
}

int CGXCommunication::ReadRowsByEntry(CGXDLMSProfileGeneric* pObject, unsigned int index, unsigned int count, CGXDLMSVariant& rows)
{
    rows.Clear();
    int ret;
    std::vector<CGXByteBuffer> data;
    CGXReplyData reply;
    if ((ret = m_Parser->ReadRowsByEntry(pObject, index, count, data)) != 0 ||
            (ret = ReadDataBlock(data, reply)) != 0 ||
            (ret = m_Parser->UpdateValue(*pObject, 2, reply.GetValue())) != 0)
    {
        return ret;
    }
    //Get rows value as string.
    //Note! This is for example. It's faster if you handle read COSEM object directly.
    std::vector<std::string> values;
    pObject->GetValues(values);
    rows = values[2 - 1];
    return ERROR_CODES_OK;
}
