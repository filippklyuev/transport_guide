#pragma once
#include <transport_catalogue.pb.h>
#include <fstream>
#include <filesystem>
#include <string>
#include <optional>
#include "transport_catalogue.h"
#include "domain.h"
#include "geo.h"

namespace transport_guide {

void updateProtoWithStops(const TransportCatalogue::StopMap& stop_map, catalogue_proto::TransportCatalogue& proto_catalogue);

//void updateProtoWithBuses(const TransportCatalogue::BusMap& bus_map, catalogue_proto::TransportCatalogue& proto_catalogue);

void SerializeTransportCatalogue(const std::filesystem::path filename,
									const TransportCatalogue& catalogue);

catalogue_proto::TransportCatalogue createProtoCatalogue(const TransportCatalogue& catalogue);

} // namespace transport_guide