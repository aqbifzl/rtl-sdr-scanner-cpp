#include "sdr_device_reader.h"

#include <config.h>
#include <logger.h>
#include <utils/utils.h>

#include <set>

constexpr auto LABEL = "config";

std::set<Frequency> getSampleRates(SoapySDR::Device* sdr) {
  std::set<Frequency> sampleRates;
  for (const auto value : sdr->listSampleRates(SOAPY_SDR_RX, 0)) {
    const auto sampleRate = static_cast<Frequency>(value);
    Logger::info(LABEL, "  supported sample rate: {}", formatFrequency(sampleRate));
    sampleRates.insert(sampleRate);
  }
  return sampleRates;
}

std::vector<nlohmann::json> getGains(SoapySDR::Device* sdr) {
  std::vector<nlohmann::json> gains;
  for (const auto& gain : sdr->listGains(SOAPY_SDR_RX, 0)) {
    const auto gainRange = sdr->getGainRange(SOAPY_SDR_RX, 0, gain);
    Logger::info(
        LABEL,
        "  supported gain: {}, min: {}, max: {}, step: {}",
        colored(GREEN, "{}", gain),
        colored(GREEN, "{}", gainRange.minimum()),
        colored(GREEN, "{}", gainRange.maximum()),
        colored(GREEN, "{}", gainRange.step()));
    gains.push_back({{"name", gain}, {"value", gainRange.maximum()}});
  }
  return gains;
}

void SdrDeviceReader::updateSoapyDevice(nlohmann::json& json, const SoapySDR::Kwargs args) {
  const auto serial = args.at("serial");
  const auto driver = args.at("driver");
  Logger::info(LABEL, "update device, driver: {}, serial: {}", colored(GREEN, "{}", driver), colored(GREEN, "{}", serial));

  SoapySDR::Device* sdr = SoapySDR::Device::make(args);
  if (sdr == nullptr) {
    throw std::runtime_error("open device failed");
  }

  json["driver"] = driver;

  const auto sampleRate = json.at("sample_rate").get<Frequency>();
  const auto sampleRates = getSampleRates(sdr);
  json["sample_rates"] = sampleRates;
  if (sampleRates.count(sampleRate) == 0) {
    json["sample_rate"] = getNearestElement(sampleRates, sampleRate);
  }

  SoapySDR::Device::unmake(sdr);
}

void SdrDeviceReader::createSoapyDevices(nlohmann::json& json, const SoapySDR::Kwargs args) {
  const auto serial = args.at("serial");
  const auto driver = args.at("driver");
  Logger::info(LABEL, "creating device, driver: {}, serial: {}", colored(GREEN, "{}", driver), colored(GREEN, "{}", serial));

  SoapySDR::Device* sdr = SoapySDR::Device::make(args);
  if (sdr == nullptr) {
    throw std::runtime_error("open device failed");
  }

  json["driver"] = driver;
  json["serial"] = serial;
  json["enabled"] = true;
  json["start_recording_level"] = DEFAULT_RECORDING_START_LEVEL;
  json["stop_recording_level"] = DEFAULT_RECORDING_STOP_LEVEL;

  const auto sampleRates = getSampleRates(sdr);
  json["sample_rates"] = sampleRates;

  json["ranges"] = nlohmann::json::array();
  auto addSampleRate = [&sampleRates, &json](Frequency start, Frequency stop, Frequency sampleRate) {
    if (json.at("ranges").empty() && sampleRates.count(sampleRate)) {
      json["ranges"].push_back({{"start", start}, {"stop", stop}});
      json["sample_rate"] = sampleRate;
      return true;
    } else {
      return false;
    }
  };

  addSampleRate(140000000, 160000000, 20480000);
  addSampleRate(140000000, 160000000, 20000000);
  addSampleRate(144000000, 146000000, 2048000);
  addSampleRate(144000000, 146000000, 2000000);
  addSampleRate(144000000, 146000000, 1024000);
  addSampleRate(144000000, 146000000, 1000000);
  addSampleRate(144000000, 146000000, *sampleRates.rbegin());

  json["gains"] = getGains(sdr);
  json["satellites"] = nlohmann::json::array();

  SoapySDR::Device::unmake(sdr);
}

void SdrDeviceReader::scanSoapyDevices(nlohmann::json& json) {
  Logger::info(LABEL, "scanning connected devices");
  const SoapySDR::KwargsList results = SoapySDR::Device::enumerate("remote=");
  Logger::info(LABEL, "found {} devices:", colored(GREEN, "{}", results.size()));

  for (auto& device : json.at("devices")) {
    device["driver"] = "";
    device["sample_rates"] = nlohmann::json::array();
  }
  for (uint32_t i = 0; i < results.size(); ++i) {
    try {
      auto& devices = json.at("devices");
      const auto serial = results[i].at("serial");
      const auto f = [serial](nlohmann::json& device) { return device.at("serial").get<std::string>() == serial; };
      const auto it = std::find_if(devices.begin(), devices.end(), f);
      if (it != devices.end()) {
        updateSoapyDevice(*it, results[i]);
      } else {
        nlohmann::json device;
        createSoapyDevices(device, results[i]);
        devices.push_back(device);
      }
    } catch (const std::exception& exception) {
      Logger::warn(LABEL, "scan device exception: {}", exception.what());
    }
  }
}

void SdrDeviceReader::clearDevices(nlohmann::json& json) {
  for (auto& device : json.at("devices")) {
    device.erase("driver");
    device.erase("sample_rates");
  }
}
