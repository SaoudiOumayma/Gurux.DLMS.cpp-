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

#include "../include/GXDLMSContextType.h"
#include "../include/GXDLMSVariant.h"
#include "../include/GXHelpers.h"

std::string CGXDLMSContextType::GetConformance()
{
    return m_Conformance;
}
void CGXDLMSContextType::SetConformance(std::string value)
{
    m_Conformance = value;
}

int CGXDLMSContextType::GetMaxPduSize()
{
    return m_MaxReceivePduSize;
}
void CGXDLMSContextType::SetMaxReceivePduSize(int value)
{
    m_MaxReceivePduSize = value;
}

int CGXDLMSContextType::GetMaxSendPpuSize()
{
    return m_MaxSendPpuSize;
}
void CGXDLMSContextType::SetMaxSendPpuSize(int value)
{
    m_MaxSendPpuSize = value;
}

int CGXDLMSContextType::GetDlmsVersionNumber()
{
    return m_DlmsVersionNumber;
}
void CGXDLMSContextType::SetDlmsVersionNumber(int value)
{
    m_DlmsVersionNumber = value;
}
int CGXDLMSContextType::GetQualityOfService()
{
    return m_QualityOfService;
}
void CGXDLMSContextType::SetQualityOfService(int value)
{
    m_QualityOfService = value;
}

CGXByteBuffer& CGXDLMSContextType::GetCypheringInfo()
{
    return m_CypheringInfo;
}

void CGXDLMSContextType::SetCypheringInfo(CGXByteBuffer& value)
{
    m_CypheringInfo = value;
}

std::string CGXDLMSContextType::ToString()
{
    CGXByteBuffer bb;
    bb.AddString(m_Conformance.c_str());
    bb.SetInt8(' ');
    bb.AddIntAsString(m_MaxReceivePduSize);
    bb.SetInt8(' ');
    bb.AddIntAsString(m_MaxSendPpuSize);
    bb.SetInt8(' ');
    bb.AddIntAsString(m_DlmsVersionNumber);
    bb.SetInt8(' ');
    bb.AddIntAsString(m_QualityOfService);
    bb.SetInt8(' ');
    bb.Set(&m_CypheringInfo, 0, ULONG_MAX);
    return bb.ToString();
}