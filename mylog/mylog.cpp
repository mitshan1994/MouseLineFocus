#include "mylog.h"
#include "spdlog/sinks/stdout_color_sinks.h" // or "../stdout_sinks.h" if no colors needed
#include "spdlog/sinks/daily_file_sink.h"

#include <iostream>
#include <string>
#include <boost/filesystem.hpp>

static bool g_bInited = false;

// Create directory, return 0 if success.
static int CreateDir(const char* dirPath)
{
    try {
      boost::filesystem::create_directories(dirPath);
    } catch (std::exception e) {
    	return -1;
    }

    return 0;
}

int InitLog(const char * filename)
{
    // If inited, just return;
    if (g_bInited) {
        return 0;
    }

    // Create directory
    // Now can only support create one directory, if it's
    //     ./dir/nonexist1/nonexist2/aaa.log, then it will fail.
    do {
        std::string strFilename = filename;
        // replace all '\' with '/'
        for (auto it = strFilename.begin(); it != strFilename.end(); ++it) {
            if (*it == '\\') {
                *it = '/';
            }
        }
        auto pos = strFilename.find_last_of('/');
        if (std::string::npos == pos) {
            break;
        }
        std::string strDirPath = strFilename.substr(0, pos);
        CreateDir(strDirPath.c_str());
    } while (0);

    try {
        spdlog::level::level_enum level = spdlog::level::info;
#ifdef _DEBUG
        level = spdlog::level::trace;
#endif

        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        //console_sink->set_level(level);
        console_sink->set_pattern("[%T.%e] [%^%L%$] [%t] [%s:%#;%!] %v");

        auto daily_file_sink = std::make_shared<spdlog::sinks::daily_file_sink_mt>(filename, 0, 0);
        //daily_file_sink->set_level(level);
        daily_file_sink->set_pattern("[%T.%e] [%^%L%$] [%t] [%s:%#;%!] %v");

        auto logger = std::make_shared<spdlog::logger>("multi_sink",
            spdlog::sinks_init_list({ console_sink, daily_file_sink }));
        logger->flush_on(spdlog::level::trace);
        logger->set_level(level);
        spdlog::set_default_logger(logger);
    } catch (const spdlog::spdlog_ex& ex) {
        std::cout << "Log initialization failed: " << ex.what() << std::endl;
    }

    g_bInited = true;

    return 0;
}

void SetLogLevel(int level)
{
    if (level < spdlog::level::trace || level > spdlog::level::err) {
        L_ERROR("Invalid log level: {}", level);
        return;
    }

    spdlog::default_logger()->set_level(spdlog::level::level_enum(level));
}
