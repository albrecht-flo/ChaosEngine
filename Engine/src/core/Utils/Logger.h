#pragma once

// This ignores all warnings raised inside External headers
#pragma warning(push, 0)

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>

#pragma warning(pop)

enum class LogLevel {
    Debug, Info, Warn, Error
};

class Logger {
public:
    static void Init(LogLevel level = LogLevel::Info);

    static void I(const std::string &tag, const std::string &message) {
        engineLogger->info("[{0}]: {1}", tag, message);
    }

    static void D(const std::string &tag, const std::string &message) {
        engineLogger->debug("[{0}]: {1}", tag, message);
    }

    static void W(const std::string &tag, const std::string &message) {
        engineLogger->warn("[{0}]: {1}", tag, message);
    }

    static void E(const std::string &tag, const std::string &message) {
        engineLogger->error("[{0}]: {1}", tag, message);
    }

    static void C(const std::string &tag, const std::string &message) {
        engineLogger->critical("[{0}]: {1}", tag, message);
    }

    static spdlog::logger &GetLogger() { return *engineLogger; }

private:
    static std::shared_ptr<spdlog::logger> engineLogger;
};


#define LOG_INFO(...) ::Logger::GetLogger().info(__VA_ARGS__)
#define LOG_DEBUG(...) ::Logger::GetLogger().debug(__VA_ARGS__)
#define LOG_WARN(...) ::Logger::GetLogger().warn(__VA_ARGS__)
#define LOG_ERROR(...) ::Logger::GetLogger().error(__VA_ARGS__)
#define LOG_CRITICAL(...) ::Logger::GetLogger().critical(__VA_ARGS__)



