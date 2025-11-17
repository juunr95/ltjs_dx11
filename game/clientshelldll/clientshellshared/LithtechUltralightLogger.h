#pragma once
#include "Ultralight/Ultralight.h"
#include <fstream>
#include <iostream>
#include <mutex>
#include <string>

using namespace ultralight;

class LithtechUltraligthLogger : public Logger {
public:
    LithtechUltraligthLogger(const std::string& filename = "ultralight.log")
        : log_file_(filename, std::ios::out | std::ios::app) {
        if (!log_file_.is_open()) {
            std::cerr << "[FileLogger] Falha ao abrir o arquivo de log: " << filename << std::endl;
        }
    }

    ~LithtechUltraligthLogger() override {
        if (log_file_.is_open())
            log_file_.close();
    }

    static LithtechUltraligthLogger* create(const std::string& filename = "ultralight.log") {
        return new LithtechUltraligthLogger(filename);
	}

    void LogMessage(LogLevel log_level, const String& message) override {
        std::lock_guard<std::mutex> lock(mutex_);
        std::string level = LevelToString(log_level);
        std::string text = message.utf8().data();

        std::string formatted = "[" + level + "] " + text + "\n";

        // Escreve no arquivo
        if (log_file_.is_open())
            log_file_ << formatted;

        // Exibe no console
        std::cout << formatted;
    }

private:
    std::ofstream log_file_;
    std::mutex mutex_;

    std::string LevelToString(LogLevel level) {
        switch (level) {
        case LogLevel::Error: return "ERROR";
        case LogLevel::Warning: return "WARNING";
        case LogLevel::Info: return "INFO";
        default: return "UNKNOWN";
        }
    }
};
