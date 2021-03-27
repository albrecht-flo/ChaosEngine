#include "Logger.h"

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/ostream_sink.h>

std::shared_ptr<spdlog::logger> Logger::engineLogger;
//std::unique_ptr<std::ostringstream> Logger::streamLogger;

void Logger::Init(LogLevel level) {
//    streamLogger = std::make_unique<std::ostringstream>(std::ios_base::ate);
    std::vector<spdlog::sink_ptr> logSinks;
    logSinks.emplace_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
//    logSinks.emplace_back(std::make_shared<spdlog::sinks::ostream_sink_st>(*streamLogger));
    logSinks[0]->set_pattern("%^%n [%T] %v%$");
//    logSinks[1]->set_pattern("%^%n [%T] %v%$");

    engineLogger = std::make_shared<spdlog::logger>("ChaosEngine", begin(logSinks), end(logSinks));
    spdlog::register_logger(engineLogger);
    spdlog::enable_backtrace(128);

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
