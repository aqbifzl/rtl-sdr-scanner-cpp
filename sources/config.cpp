#include "config.h"

#include <config_migrator.h>
#include <logger.h>
#include <radio/sdr_device_reader.h>
#include <utils/utils.h>

#include <fstream>
#include <regex>

constexpr auto LABEL = "config";

spdlog::level::level_enum parseLogLevel(const std::string& level) {
  if (level == "trace")
    return spdlog::level::level_enum::trace;
  else if (level == "debug")
    return spdlog::level::level_enum::debug;
  else if (level == "info")
    return spdlog::level::level_enum::info;
  else if (level == "warn" || level == "warning")
    return spdlog::level::level_enum::warn;
  else if (level == "err" || level == "error")
    return spdlog::level::level_enum::err;
  else if (level == "critical")
    return spdlog::level::level_enum::critical;
  return spdlog::level::level_enum::off;
}

Config::Config(const nlohmann::json& json, const ArgConfig& argConfig) : m_id(!argConfig.id.empty() ? argConfig.id : randomHex(8)), m_json(json), m_argConfig(argConfig), m_fileConfig(json) {}

Config Config::loadFromFile(const std::string& path, const ArgConfig& argConfig) {
  std::ifstream stream(path);
  if (stream) {
    try {
      auto json = nlohmann::json::parse(stream);
      ConfigMigrator::update(json);
      SdrDeviceReader::scanSoapyDevices(json);
      ConfigMigrator::sort(json);
      return Config(json, argConfig);
    } catch (const nlohmann::json::parse_error& exception) {
      throw std::runtime_error(fmt::format("can not parse config file, invalid json format: {}", path));
    }
  } else {
    throw std::runtime_error(fmt::format("can not parse config file, file not found: {}", path));
  }
}

void Config::saveToFile(const std::string& path, const nlohmann::json& json) {
  std::ofstream stream(path);
  if (stream) {
    auto tmp = json;
    SdrDeviceReader::clearDevices(tmp);
    stream << std::setw(4) << tmp << std::endl;
  } else {
    Logger::warn(LABEL, "save new config failed");
  }
}

nlohmann::json Config::hideSensitiveData(const nlohmann::json& json) {
  nlohmann::json config(json);
  try {
    if (!config["api_key"].empty()) {
      config["api_key"] = "******";
    }
    std::regex regex(R"(^(\d+)\.\d+)");
    config["position"]["latitude"] = std::regex_replace(config["position"]["latitude"].get<std::string>(), regex, "$1.********");
    config["position"]["longitude"] = std::regex_replace(config["position"]["longitude"].get<std::string>(), regex, "$1.********");
  } catch (const std::exception& exception) {
    Logger::warn(LABEL, "hide sensitive data exception: {}", colored(RED, "{}", exception.what()));
  }
  return config;
}

nlohmann::json Config::json() const { return m_json; }
std::string Config::mqtt() const { return fmt::format("{}@{}", m_argConfig.mqttUser, m_argConfig.mqttUrl); };

std::string Config::getId() const { return m_id; }
std::vector<Device> Config::devices() const { return m_fileConfig.devices; }

bool Config::isColorLogEnabled() const { return m_fileConfig.output.color_log_enabled; }
spdlog::level::level_enum Config::consoleLogLevel() const { return parseLogLevel(m_fileConfig.output.console_log_level); }
spdlog::level::level_enum Config::fileLogLevel() const { return parseLogLevel(m_fileConfig.output.file_log_level); }

std::vector<FrequencyRange> Config::ignoredRanges() const {
  std::vector<FrequencyRange> ranges;
  for (const auto& range : m_fileConfig.ignored_frequencies) {
    ranges.emplace_back(range.frequency - range.bandwidth / 2, range.frequency + range.bandwidth / 2);
  }
  return ranges;
}
int Config::recordersCount() const {
  const auto max_workers = static_cast<int>(std::thread::hardware_concurrency() / 2);
  const auto workers = std::max(0, std::min(m_fileConfig.workers, max_workers));
  return workers == 0 ? max_workers : workers;
}
Frequency Config::recordingBandwidth() const { return m_fileConfig.recording.min_sample_rate; }
std::chrono::milliseconds Config::recordingMinTime() const { return m_fileConfig.recording.min_time_ms; }
std::chrono::milliseconds Config::recordingTimeout() const { return m_fileConfig.recording.max_noise_time_ms; }
Frequency Config::recordingTuningStep() const { return m_fileConfig.recording.step; }

std::string Config::mqttUrl() const { return m_argConfig.mqttUrl; }
std::string Config::mqttUsername() const { return m_argConfig.mqttUser; }
std::string Config::mqttPassword() const { return m_argConfig.mqttPassword; }

std::string Config::apiKey() const { return m_fileConfig.api_key; }
std::string Config::latitude() const { return m_fileConfig.position.latitude; }
std::string Config::longitude() const { return m_fileConfig.position.longitude; }
int Config::altitude() const { return m_fileConfig.position.altitude; }