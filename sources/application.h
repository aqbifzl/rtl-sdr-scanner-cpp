#pragma once

#include <arg_config.h>
#include <config.h>
#include <file_config.h>
#include <network/mqtt.h>
#include <network/remote_controller.h>
#include <scanner.h>

#include <nlohmann/json.hpp>

class Application {
 public:
  Application(nlohmann::json& tmpJson, const ArgConfig& argConfig);
  ~Application();

  bool reload() const;

 private:
  bool m_reload;
  const ArgConfig& m_argConfig;
  nlohmann::json& m_tmpJson;
  const nlohmann::json m_fileJson;
  const FileConfig m_fileConfig;
  const Config m_config;

  Mqtt m_mqtt;
  RemoteController m_remoteController;
  std::vector<std::unique_ptr<Scanner>> m_scanners;
};