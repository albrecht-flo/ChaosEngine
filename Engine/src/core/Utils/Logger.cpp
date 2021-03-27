#include "Logger.h"

#include <spdlog/fmt/ostr.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/ringbuffer_sink.h>

std::shared_ptr<spdlog::logger> Logger::engineLogger;

void Logger::Init(LogLevel level) {
    if (engineLogger != nullptr) {
        LOG_WARN("[Logger] Logger already initialized! Skipping init");
        return;
    }
    std::vector<spdlog::sink_ptr> logSinks;
    logSinks.emplace_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
    logSinks.emplace_back(std::make_shared<spdlog::sinks::ringbuffer_sink_mt>(1024));
    logSinks[0]->set_pattern("%^%n [%T] %v%$");
    logSinks[1]->set_pattern("%^%n [%T] %v%$");

    engineLogger = std::make_shared<spdlog::logger>("ChaosEngine", begin(logSinks), end(logSinks));
    spdlog::register_logger(engineLogger);
    switch (level) {
        case LogLevel::Debug: {
            engineLogger->set_level(spdlog::level::debug);
            engineLogger->flush_on(spdlog::level::debug);
            break;
        }
        case LogLevel::Info: {
            engineLogger->set_level(spdlog::level::info);
            engineLogger->flush_on(spdlog::level::info);
            break;
        }
        case LogLevel::Warn: {
            engineLogger->set_level(spdlog::level::warn);
            engineLogger->flush_on(spdlog::level::warn);
            break;
        }
        case LogLevel::Error: {
            engineLogger->set_level(spdlog::level::err);
            engineLogger->flush_on(spdlog::level::err);
            break;
        }
    }

}

std::vector<std::string> Logger::GetLogBuffer() {
    auto &ringSink = dynamic_cast<spdlog::sinks::ringbuffer_sink_mt &>(*(engineLogger->sinks()[1]));
    return ringSink.last_formatted();
}
