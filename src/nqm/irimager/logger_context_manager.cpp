#include "./logger_context_manager.hpp"

LoggerContextManager::~LoggerContextManager() = default;

void LoggerContextManager::start() {
  if (!logger_) logger_ = std::make_unique<Logger>();
}

void LoggerContextManager::stop() { logger_ = nullptr; }
