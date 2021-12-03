#pragma once
#include <transport_catalogue.pb.h>
#include <fstream>
#include <filesystem>
#include <string>
#include <iostream>
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

TransportCatalogue DeserializeTransportCatalogue(const std::filesystem::path& filename);

void parseStopsFromProto(const catalogue_proto::TransportCatalogue& proto_catalogue, TransportCatalogue& catalogue);

void parseBusesFromProto(const catalogue_proto::TransportCatalogue& proto_catalogue, TransportCatalogue& catalogue);

std::vector<std::string_view> getStopsVector(const catalogue_proto::Bus& bus,const catalogue_proto::TransportCatalogue& proto_catalogue);

} // namespace transport_guide