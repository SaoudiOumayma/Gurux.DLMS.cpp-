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

// GuruxDLMSClientExample.cpp : Defines the entry point for the Gurux DLMS Client example.
#if defined(_WIN32) || defined(_WIN64)//Windows
#include "../include/getopt.h"
#endif
#include "../include/communication.h"

static void ShowHelp()
{
    printf("GuruxDlmsSample reads data from the DLMS/COSEM device.\r\n");
    printf("GuruxDlmsSample -h [Meter IP Address] -p [Meter Port No] -c 16 -s 1 -r SN\r\n");
    printf(" -h \t host name or IP address.\r\n");
    printf(" -p \t port number or name (Example: 1000).\r\n");
    printf(" -S [COM1:9600:8None1]\t serial port.\r\n");
    printf(" -i \t IEC is a start protocol.\r\n");
    printf(" -a \t Authentication (None, Low, High, HighMd5, HighSha1, HighGmac, HighSha256).\r\n");
    printf(" -P \t Password for authentication.\r\n");
    printf(" -c \t Client address. (Default: 16)\r\n");
    printf(" -s \t Server address. (Default: 1)\r\n");
    printf(" -n \t Server address as serial number.\r\n");
    printf(" -f \t Formula to create address from serial number. (Default: SN%%10000+1000)\r\n");
    printf(" -r [sn, ln]\t Short name or Logican Name (default) referencing is used.\r\n");
    printf(" -w \t WRAPPER profile is used. HDLC is default.\r\n");
    printf(" -t Trace messages.\r\n");
    printf(" -g \"0.0.1.0.0.255:1; 0.0.1.0.0.255:2\" Get selected object(s) with given attribute index.\r\n");
    printf(" -C \t Security Level. (None, Authentication, Encrypted, AuthenticationEncryption)\r\n");
    printf(" -v \t Invocation counter data object Logical Name. Ex. 0.0.43.1.1.255\r\n");
    printf(" -I \t Auto increase invoke ID\r\n");
    printf(" -o \t Cache association view to make reading faster. Ex. -o C:\\device.xml\r\n");
    printf(" -T \t System title that is used with chiphering. Ex -D 4775727578313233\r\n");
    printf(" -A \t Authentication key that is used with chiphering. Ex -D D0D1D2D3D4D5D6D7D8D9DADBDCDDDEDF\r\n");
    printf(" -B \t Block cipher key that is used with chiphering. Ex -D 000102030405060708090A0B0C0D0E0F\r\n");
    printf(" -D \t Dedicated key that is used with chiphering. Ex -D 00112233445566778899AABBCCDDEEFF\r\n");
    printf("Example:\r\n");
    printf("Read LG device using TCP/IP connection.\r\n");
    printf("GuruxDlmsSample -r SN -c 16 -s 1 -h [Meter IP Address] -p [Meter Port No]\r\n");
    printf("Read LG device using serial port connection.\r\n");
    printf("GuruxDlmsSample -r SN -c 16 -s 1 -sp COM1 -i\r\n");
    printf("Read Indian device using serial port connection.\r\n");
    printf("GuruxDlmsSample -S COM1 -c 16 -s 1 -a Low -P [password]\r\n");
}

#if defined(_WIN32) || defined(_WIN64)//Windows
int _tmain(int argc, _TCHAR* argv[])
#else
int main(int argc, char* argv[])
#endif
{
    {
#if defined(_WIN32) || defined(_WIN64)//Windows
        //Use user locale settings. This is causing errors in some Linux distros. Example Fedora
        std::locale::global(std::locale(""));
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        {
            // Tell the user that we could not find a usable WinSock DLL.
            return 1;
        }
#endif
        int ret;
        //Remove trace file if exists.
        remove("trace.txt");
        remove("LogFile.txt");
        GX_TRACE_LEVEL trace = GX_TRACE_LEVEL_INFO;
        bool useLogicalNameReferencing = true;
        int clientAddress = 16, serverAddress = 1;
        DLMS_AUTHENTICATION authentication = DLMS_AUTHENTICATION_NONE;
        DLMS_INTERFACE_TYPE interfaceType = DLMS_INTERFACE_TYPE_HDLC;
        DLMS_SECURITY security = DLMS_SECURITY_NONE;
        char* password = NULL;
        char* p, * p2, * readObjects = NULL;
        int index, a, b, c, d, e, f;
        int opt = 0;
        int port = 0;
        int serialNumber = -1;
        char* address = NULL;
        char* serialPort = NULL;
        bool iec = false;
        bool autoIncreaseInvokeID = false;
        char* invocationCounter = NULL;
        char* outputFile = NULL;
        char* systemTitle = NULL;
        char* authenticationKey = NULL;
        char* blockCipherKey = NULL;
        char* dedicatedKey = NULL;
        char* formula = (char *)"SN%10000+1000";

        while ((opt = getopt(argc, argv, "h:p:c:s:r:iIt:a:wP:g:S:n:C:f:v:o:T:A:B:D:")) != -1)
        {
            switch (opt)
            {
            case 'w':
                interfaceType = DLMS_INTERFACE_TYPE_WRAPPER;
                break;
            case 'r':
                if (strcmp("sn", optarg) == 0)
                {
                    useLogicalNameReferencing = false;
                }
                else if (strcmp("ln", optarg) == 0)
                {
                    useLogicalNameReferencing = 1;
                }
                else
                {
                    printf("Invalid reference option.\n");
                    return 1;
                }
                break;
            case 'h':
                //Host address.
                address = optarg;
                break;
            case 't':
                //Trace.
                if (strcmp("Error", optarg) == 0)
                    trace = GX_TRACE_LEVEL_ERROR;
                else  if (strcmp("Warning", optarg) == 0)
                    trace = GX_TRACE_LEVEL_WARNING;
                else  if (strcmp("Info", optarg) == 0)
                    trace = GX_TRACE_LEVEL_INFO;
                else  if (strcmp("Verbose", optarg) == 0)
                    trace = GX_TRACE_LEVEL_VERBOSE;
                else  if (strcmp("Off", optarg) == 0)
                    trace = GX_TRACE_LEVEL_OFF;
                else
                {
                    printf("Invalid trace option '%s'. (Error, Warning, Info, Verbose, Off)", optarg);
                    return 1;
                }
                break;
            case 'p':
                //Port.
                port = atoi(optarg);
                break;
            case 'P':
                password = optarg;
                break;
            case 'i':
                //IEC.
                iec = 1;
                break;
            case 'I':
                // AutoIncreaseInvokeID.
                autoIncreaseInvokeID = true;
                break;
            case 'C':
                if (strcmp("None", optarg) == 0)
                {
                    security = DLMS_SECURITY_NONE;
                }
                else if (strcmp("Authentication", optarg) == 0)
                {
                    security = DLMS_SECURITY_AUTHENTICATION;
                }
                else if (strcmp("Encryption", optarg) == 0)
                {
                    security = DLMS_SECURITY_ENCRYPTION;
                }
                else if (strcmp("AuthenticationEncryption", optarg) == 0)
                {
                    security = DLMS_SECURITY_AUTHENTICATION_ENCRYPTION;
                }
                else
                {
                    printf("Invalid Ciphering option '%s'. (None, Authentication, Encryption, AuthenticationEncryption)", optarg);
                    return 1;
                }
                break;
            case 'T':
                systemTitle = optarg;
                break;
            case 'A':
                authenticationKey = optarg;
                break;
            case 'B':
                blockCipherKey = optarg;
                break;
            case 'D':
                dedicatedKey = optarg;
                break;
            case 'o':
                outputFile = optarg;
                break;
            case 'v':
                invocationCounter = optarg;
#if defined(_WIN32) || defined(_WIN64)//Windows
                if ((ret = sscanf_s(optarg, "%d.%d.%d.%d.%d.%d", &a, &b, &c, &d, &e, &f)) != 6)
#else
                if ((ret = sscanf(optarg, "%d.%d.%d.%d.%d.%d", &a, &b, &c, &d, &e, &f)) != 6)
#endif
                {
                    ShowHelp();
                    return 1;
                }
                break;
            case 'g':
                //Get (read) selected objects.
                p = optarg;
                do
                {
                    if (p != optarg)
                    {
                        ++p;
                    }
#if defined(_WIN32) || defined(_WIN64)//Windows
                    if ((ret = sscanf_s(p, "%d.%d.%d.%d.%d.%d:%d", &a, &b, &c, &d, &e, &f, &index)) != 7)
#else
                    if ((ret = sscanf(p, "%d.%d.%d.%d.%d.%d:%d", &a, &b, &c, &d, &e, &f, &index)) != 7)
#endif
                    {
                        ShowHelp();
                        return 1;
                    }
                } while ((p = strchr(p, ',')) != NULL);
                readObjects = optarg;
                break;
            case 'f':
                formula = optarg;
                break;
            case 'S':
                serialPort = optarg;
                break;
            case 'a':
                if (strcmp("None", optarg) == 0)
                {
                    authentication = DLMS_AUTHENTICATION_NONE;
                }
                else if (strcmp("Low", optarg) == 0)
                {
                    authentication = DLMS_AUTHENTICATION_LOW;
                }
                else if (strcmp("High", optarg) == 0)
                {
                    authentication = DLMS_AUTHENTICATION_HIGH;
                }
                else if (strcmp("HighMd5", optarg) == 0)
                {
                    authentication = DLMS_AUTHENTICATION_HIGH_MD5;
                }
                else if (strcmp("HighSha1", optarg) == 0)
                {
                    authentication = DLMS_AUTHENTICATION_HIGH_SHA1;
                }
                else if (strcmp("HighGmac", optarg) == 0)
                {
                    authentication = DLMS_AUTHENTICATION_HIGH_GMAC;
                }
                else if (strcmp("HighSha256", optarg) == 0)
                {
                    authentication = DLMS_AUTHENTICATION_HIGH_SHA256;
                }
                else
                {
                    printf("Invalid Authentication option. (None, Low, High, HighMd5, HighSha1, HighGmac, HighSha256)\n");
                    return 1;
                }
                break;
            case 'c':
                clientAddress = atoi(optarg);
                break;
            case 's':
                serverAddress = atoi(optarg);
                break;
            case 'n':
                serialNumber = atoi(optarg);
                break;
            case '?':
            {
                if (optarg != nullptr) {
                    if (optarg[0] == 'c') {
                        printf("Missing mandatory client option.\n");
                    }
                    else if (optarg[0] == 's') {
                        printf("Missing mandatory server option.\n");
                    }
                    else if (optarg[0] == 'h') {
                        printf("Missing mandatory host name option.\n");
                    }
                    else if (optarg[0] == 'p') {
                        printf("Missing mandatory port option.\n");
                    }
                    else if (optarg[0] == 'r') {
                        printf("Missing mandatory reference option.\n");
                    }
                    else if (optarg[0] == 'a') {
                        printf("Missing mandatory authentication option.\n");
                    }
                    else if (optarg[0] == 'S') {
                        printf("Missing mandatory Serial port option.\n");
                    }
                    else if (optarg[0] == 'g') {
                        printf("Missing mandatory OBIS code option.\n");
                    }
                    else if (optarg[0] == 'C') {
                        printf("Missing mandatory Ciphering option.\n");
                    }
                    else if (optarg[0] == 'v') {
                        printf("Missing mandatory invocation counter logical name option.\n");
                    }
                    else if (optarg[0] == 'T') {
                        printf("Missing mandatory system title option.");
                    }
                    else if (optarg[0] == 'A') {
                        printf("Missing mandatory authentication key option.");
                    }
                    else if (optarg[0] == 'B') {
                        printf("Missing mandatory block cipher key option.");
                    }
                    else if (optarg[0] == 'D') {
                        printf("Missing mandatory dedicated key option.");
                    }
                }
                else
                {
                    ShowHelp();
                    return 1;
                }
            }
            break;
            default:
                ShowHelp();
                return 1;
            }
        }
        if (serialNumber > 0) 
        {
            serverAddress = CGXDLMSClient::GetServerAddress(serialNumber, formula);
        }
        CGXDLMSSecureClient cl(useLogicalNameReferencing, clientAddress, serverAddress, authentication, password, interfaceType);
        cl.GetCiphering()->SetSecurity(security);
        cl.SetAutoIncreaseInvokeID(autoIncreaseInvokeID);
        CGXByteBuffer bb;
        if (systemTitle != NULL)
        {
            bb.Clear();
            bb.SetHexString(systemTitle);
            cl.GetCiphering()->SetSystemTitle(bb);
        }
        if (authenticationKey != NULL)
        {
            bb.Clear();
            bb.SetHexString(authenticationKey);
            cl.GetCiphering()->SetAuthenticationKey(bb);
        }
        if (blockCipherKey != NULL)
        {
            bb.Clear();
            bb.SetHexString(blockCipherKey);
            cl.GetCiphering()->SetBlockCipherKey(bb);
        }
        if (dedicatedKey != NULL)
        {
            bb.Clear();
            bb.SetHexString(dedicatedKey);
            cl.GetCiphering()->SetDedicatedKey(bb);
        }
        CGXCommunication comm(&cl, 5000, trace, invocationCounter);

        if (port != 0 || address != NULL)
        {
            if (port == 0)
            {
                printf("Missing mandatory port option.\n");
                return 1;
            }
            if (address == NULL)
            {
                printf("Missing mandatory host option.\n");
                return 1;
            }
            if ((ret = comm.Connect(address, port)) != 0)
            {
                printf("Connect failed %s.\r\n", CGXDLMSConverter::GetErrorMessage(ret));
                return 1;
            }
        }
        else if (serialPort != NULL)
        {
            if ((ret = comm.Open(serialPort, iec)) != 0)
            {
                if (ret == DLMS_ERROR_CODE_RECEIVE_FAILED)
                {
                    printf("Failed to receive reply for IEC. Check is DLMS connection already established.\r\n");
                }
                else
                {
                    printf("Serial port open failed %d.\r\n", ret);
                    return 1;
                }
            }
        }
        else
        {
            printf("Missing mandatory connection information for TCP/IP or serial port connection.\n");
            return 1;
        }

        if (readObjects != NULL)
        {
            bool read = false;
            if (outputFile != NULL)
            {
                if ((ret = cl.GetObjects().Load(outputFile)) == 0)
                {
                    ret = 0;
                    read = true;
                }
            }
            ret = comm.InitializeConnection();
            if (ret == 0 && !read)
            {
                ret = comm.GetAssociationView();
            }
            if (ret == 0)
            {
                std::string str;
                std::string value;
                char buff[200];
                p = readObjects;
                do
                {
                    if (p != readObjects)
                    {
                        ++p;
                    }
                    str.clear();
                    p2 = strchr(p, ':');
                    ++p2;
#if defined(_WIN32) || defined(_WIN64)//Windows
                    sscanf_s(p2, "%d", &index);
#else
                    sscanf(p2, "%d", &index);
#endif
                    str.append(p, p2 - p);
                    CGXDLMSObject* obj = cl.GetObjects().FindByLN(DLMS_OBJECT_TYPE_ALL, str);
                    if (obj != NULL)
                    {
                        value.clear();
                        if ((ret = comm.Read(obj, index, value)) != DLMS_ERROR_CODE_OK)
                        {
#if _MSC_VER > 1000
                            sprintf_s(buff, 200, "Error! Index: %d %s\r\n", index, CGXDLMSConverter::GetErrorMessage(ret));
#else
                            sprintf(buff, "Error! Index: %d read failed: %s\r\n", index, CGXDLMSConverter::GetErrorMessage(ret));
#endif
                            comm.WriteValue(GX_TRACE_LEVEL_ERROR, buff);
                            //Continue reading.
                        }
                        else
                        {
#if _MSC_VER > 1000
                            sprintf_s(buff, 100, "Object: %s%d Value: ", index);
#else
                            sprintf(buff, "Object: %s%d Value: ", str.data(), index);
#endif
                            comm.WriteValue(trace, buff);
                            comm.WriteValue(trace, value.c_str());
                            comm.WriteValue(trace, "\r\n");
                        }
                    }
                    else
                    {
#if _MSC_VER > 1000
                        snprintf_s(buff, 100, "Unknown object: %s", str.c_str());
#else
                        snprintf(buff, 100, "Unknown object: %s", str.c_str());
#endif
                        str = buff;
                        comm.WriteValue(GX_TRACE_LEVEL_ERROR, str);
                    }
                } while ((p = strchr(p, ',')) != NULL);
                //Close connection.
                comm.Close();
                if (outputFile != NULL && ret == 0)
                {
                    ret = cl.GetObjects().Save(outputFile);
                }
            }
        }
        else {
            ret = comm.ReadAll(outputFile);
        }
        //Close connection.
        comm.Close();
    }
#if defined(_WIN32) || defined(_WIN64)//Windows
    WSACleanup();
#if _MSC_VER > 1400
    _CrtDumpMemoryLeaks();
#endif
#endif
    return 0;
}

