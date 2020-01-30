#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include "spdlog/spdlog.h"
#include "convert.h"
#include "cxxopts.hpp"

#include <iostream>
#include <iomanip>
#include <stdexcept>
#include <string>
#include <sstream>

#include "INIReader.h"
#include "../development/include/GXDLMSCompactData.h"
#include "../development/include/GXBytebuffer.h"
#include "../development/include/GXDLMSNotify.h"
#include "../development/include/GXCipher.h"
#include "../development/include/GXDLMSSecureClient.h"
#include "../development/include/GXHelpers.h"
#include "../development/include/GXReplyData.h"
#include "../development/include/GXDLMSData.h"
