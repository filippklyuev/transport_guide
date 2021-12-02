#include "serialization.h"

namespace transport_guide {	

void updateProtoWithStops(const TransportCatalogue::StopMap& stop_map, catalogue_proto::TransportCatalogue& proto_catalogue){
	// proto_catalogue.stop_name.Resize(stops_map.size());
	proto_catalogue.stop.Resize(stop_map.size());
	for (const auto& [name, info] : stop_map){
		// proto_catalogue.set_stop_name(info.id_, std::string(name));
		catalogue_proto::Stop* stop = proto_catalogue.mutable_stop(info.id_);
		stop->set_name(std::string(name));
		stop->set_lattitude(info.coordinates.lat);
		stop->set_longtitude(info.coordinates.lng);
		for (const transport_guide::info::Bus* bus : info.passing_buses){
			stop->add_bus_index(bus->id_);
		}
	}
}

void updateProtoWithBuses(const TransportCatalogue::BusMap& bus_map, catalogue_proto::TransportCatalogue& proto_catalogue){
	proto_catalogue.bus.Resize(bus_map.size());
	for (const auto& [name, info] : bus_map){
		catalogue_proto::Bus* bus = proto_catalogue.mutable_bus(info.id_);
		bus->set_name(std::string(name));
		bus->set_factual_route_length(info.factial_route_length);
		bus->set_geo_route_length(info.geo_route_length);
		bus->set_curvature(info.curvature);
		bus->set_unique_stops_count(info.unique_stops.size());
		for (const transport_guide::info::Stop* stop : info.stops){
			bus->add_stop_index(stop->id_);
		}
	}
}

catalogue_proto::TransportCatalogue createProtoCatalogue(const transport_guide::TransportCatalogue& catalogue){
	catalogue_proto::TransportCatalogue proto_catalogue;
	updateProtoWithStops(catalogue.GetStopsMap(), proto_catalogue);
	updateProtoWithBuses(catalogue.GetBusesMap(), proto_catalogue);
	return proto_catalogue;
}

void SerializeTransportCatalogue(const std::filesystem::path filename, 
									const transport_guide::TransportCatalogue& catalogue){
	catalogue_proto::TransportCatalogue proto_catalogue = createProtoCatalogue(catalogue);
	ofstream ofs(filename, std::ios::binary);
	proto_catalogue.SerializeToOstream(&ofs);
}

} // namespace transport_guide