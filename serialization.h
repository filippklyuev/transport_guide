#include <transport_catalogue.pb.h>
#include <fstream>
#include <filesystem>
#include <string>
#include "transport_catalogue.h"

namespace transport_guide {

void updateProtoWithStops(const TransportCatalogue::StopMap& stop_map, catalogue_proto::TransportCatalogue& proto_catalogue);

void updateProtoWithBuses(const TransportCatalogue::BusMap& bus_map, catalogue_proto::TransportCatalogue& proto_catalogue);

void SerializeTransportCatalogue(const std::filesystem::path filename,
									const transport_guide::TransportCatalogue& catalogue);

catalogue_proto::TransportCatalogue createProtoCatalogue(const transport_guide::TransportCatalogue& catalogue);

} // namespace transport_guide