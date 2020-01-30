#include "stdafx.h"
#include "CompactFrameEncrypter.h"
#include "CompactFrameDecrypter.h"
#include "spdlog/sinks/basic_file_sink.h"

int main_app(int argc, char* argv[])
{
    spdlog::set_level(spdlog::level::info);

    cxxopts::Options options(argv[0], "Deal with compact frames and DLMS PDUs");
    options.allow_unrecognised_options();
    options.add_options()
        ("file", "File name", cxxopts::value<std::string>()->default_value("dlms.ini"))
        ("action", "Action", cxxopts::value<std::string>())
        ("pdu", "DLMS PDU", cxxopts::value<std::string>())
        ("frame", "Compact frame buffer", cxxopts::value<std::string>())
        ("verbose", "Show more info", cxxopts::value<bool>());

    auto result = options.parse(argc, argv);
    auto file = result["file"].as<std::string>();

    auto action = result["action"].as<std::string>();
    auto verbose = result["verbose"].as<bool>();
    if (verbose)
    {
        spdlog::set_level(spdlog::level::debug);
    }

    std::string frame;
    std::string pdu;

    if (action == "extract")
    {
        CompactFrameDecrypter app;
        app.LoadSecurityInfo(file);

        pdu = result["pdu"].as<std::string>();
        frame = app.DataNotificationToCompactFrame(pdu);
        std::cout << frame << std::endl;
    }
    else if (action == "encode")
    {
        CompactFrameEncrypter app;
        app.LoadSecurityInfo(file);
        frame = result["frame"].as<std::string>();
        pdu = app.CompactFrameToDataNotification(frame);
        std::cout << pdu << std::endl;
    }
    else
    {
        throw std::runtime_error(fmt::format("unknown action {}", action));
    }

    return 0;
}

int main(int argc, char* argv[])
{
    try
    {
        return main_app(argc, argv);
    }
    catch (const std::exception & e)
    {
        spdlog::error(e.what());
    }
    return -1;
}

