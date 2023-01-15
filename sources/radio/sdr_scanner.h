#pragma once

#include <algorithms/spectrogram.h>
#include <config.h>
#include <network/data_controller.h>
#include <performance_logger.h>
#include <radio/recorder.h>
#include <radio/sdr_device.h>

#include <map>
#include <memory>
#include <optional>
#include <thread>

class SdrScanner {
 public:
  SdrScanner(const Config& config, const std::vector<UserDefinedFrequencyRange>& ranges, std::unique_ptr<SdrDevice>&& device, Mqtt& mqtt);
  ~SdrScanner();

  bool isRunning() const;

 private:
  void startStream(const FrequencyRange& frequencyRange, bool runForever);
  void readSamples(const FrequencyRange& frequencyRange);

  const Config& m_config;
  std::unique_ptr<SdrDevice> m_device;
  DataController m_dataController;
  Recorder m_recorder;
  PerformanceLogger m_performanceLogger;
  std::atomic_bool m_isRunning;
  std::unique_ptr<std::thread> m_thread;
};
