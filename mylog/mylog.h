#ifndef MY_LOG_H_
#define MY_LOG_H_

// Under DEBUG mode, we will log all from trace to error.
// Under RELEASE mode, we will only log from info to error.
#ifdef _DEBUG
  #define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
#else
  #define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_INFO  // Change this if want to use SetLogLevel().
#endif

#include <memory>
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>

#include <QString>
#include <QStringList>

// Must be called before any logging.
// @param[in] path log file name prefix.
int InitLog(const char *filename);

// Set current log level.
// 0: trace; 1: debug; 2: info; 3: warn; 4: error.
void SetLogLevel(int level);

inline std::ostream &operator<<(std::ostream &os, const QString &c)
{
    return os << c.toStdString();
}

inline std::ostream &operator<<(std::ostream &os, const QStringList &c)
{
    return os << "[" << c.join(", ") << "]";
}

#if !defined(SPD_NO_LOG)
#define L_TRACE(...) SPDLOG_TRACE(__VA_ARGS__)
#define L_DEBUG(...) SPDLOG_DEBUG(__VA_ARGS__)
#define L_INFO(...)  SPDLOG_INFO(__VA_ARGS__)
#define L_WARN(...)  SPDLOG_WARN(__VA_ARGS__)
#define L_ERROR(...) SPDLOG_ERROR(__VA_ARGS__)
#define L_FATAL(...) SPDLOG_CRITICAL(__VA_ARGS__)

#endif  // #if !defined(SPD_NO_LOG)

#endif
