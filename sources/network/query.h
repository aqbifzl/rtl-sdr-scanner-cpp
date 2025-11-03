#pragma once

#include <radio/help_structures.h>
#include <utils/serializers.h>

struct SatellitesQuery {
  std::string latitude;
  std::string longitude;
  int altitude;
  std::string api_key;
  std::vector<Satellite> satellites;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(SatellitesQuery, latitude, longitude, altitude, api_key, satellites)

struct ScheduledTransmission {
  std::string name;
  std::chrono::seconds begin;
  std::chrono::seconds end;
  Frequency frequency;
  Frequency bandwidth;
  std::string modulation;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ScheduledTransmission, name, begin, end, frequency, bandwidth, modulation)
