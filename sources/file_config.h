#pragma once

#include <radio/help_structures.h>
#include <utils/serializers.h>

#include <nlohmann/json.hpp>
#include <vector>

struct IgnoredFrequency {
  Frequency frequency;
  Frequency bandwidth;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(IgnoredFrequency, frequency, bandwidth)

struct OutputConfig {
  bool color_log_enabled;
  std::string console_log_level;
  std::string file_log_level;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(OutputConfig, color_log_enabled, console_log_level, file_log_level)

struct PositionConfig {
  std::string latitude;
  std::string longitude;
  int altitude;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(PositionConfig, latitude, longitude, altitude)

struct RecordingConfig {
  Frequency min_sample_rate;
  std::chrono::milliseconds min_time_ms;
  std::chrono::milliseconds max_noise_time_ms;
  Frequency step;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(RecordingConfig, min_sample_rate, min_time_ms, max_noise_time_ms, step)

struct FileConfig {
  std::string api_key;
  std::vector<Device> devices;
  std::vector<IgnoredFrequency> ignored_frequencies;
  OutputConfig output;
  PositionConfig position;
  RecordingConfig recording;
  int workers;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(FileConfig, api_key, devices, ignored_frequencies, output, position, recording, workers)
