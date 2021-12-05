#include "serialization.h"

namespace transport_guide {	

void updateProtoWithStops(const TransportCatalogue::StopMap& stop_map, catalogue_proto::TransportCatalogue& proto_catalogue){

	proto_catalogue.mutable_stop()->Reserve(stop_map.size());
	for (int i = 0; i < stop_map.size(); i++){
		proto_catalogue.add_stop(); // Бред, но у protobuf::RepeatedPtrField не нашел адекватного способа сделать resize :(
	}
	for (const auto& [name, info] : stop_map){
		proto_catalogue.add_stop();
		catalogue_proto::Stop* stop = proto_catalogue.mutable_stop(info.id_);
		stop->set_name(std::string(name));
		stop->set_lattitude(info.coordinates.lat);
		stop->set_longtitude(info.coordinates.lng);
		for (const transport_guide::info::Bus* bus : info.passing_buses){
			stop->add_bus_index(bus->id_);
		}
		// auto& map = *proto_catalogue.mutable_stop_index();
		// map[std::string(name)] = info.id_;
		// (*proto_catalogue.stop_index())[std::string(name)] = info.id_;
	}
}

void updateProtoWithBuses(const TransportCatalogue::BusMap& bus_map, catalogue_proto::TransportCatalogue& proto_catalogue){
	proto_catalogue.mutable_bus()->Reserve(bus_map.size());
	for (int i = 0; i < bus_map.size(); i++){
		proto_catalogue.add_bus();
	}	
	for (const auto& [name, info] : bus_map){
		catalogue_proto::Bus* bus = proto_catalogue.mutable_bus(info.id_);
		bus->set_name(std::string(name));
		bus->set_factual_route_length(info.factial_route_length);
		bus->set_geo_route_length(info.geo_route_length);
		bus->set_curvature(info.curvature);
		bus->set_unique_stops_count(info.unique_stops.size());
		bus->set_is_cycled(info.is_cycled);
		for (const transport_guide::info::Stop* stop : info.stops){
			bus->add_stop_index(stop->id_);
		}
		// auto& map = *proto_catalogue.mutable_bus_index();
		// map[std::string(name)] = info.id_;
		// (*proto_catalogue.bus_index())[std::string(name)] = info.id_;
	}
}

catalogue_proto::TransportCatalogue createProtoCatalogue(const TransportCatalogue& catalogue){
	catalogue_proto::TransportCatalogue proto_catalogue;
	updateProtoWithStops(catalogue.GetStopsMap(), proto_catalogue);
	updateProtoWithBuses(catalogue.GetBusesMap(), proto_catalogue);
	// for (const auto stop : proto_catalogue.stop()){
	// 	std::cout << stop.name() <<'\n';
	// }
	// while (1){}
	return proto_catalogue;
}

void SerializeTransportCatalogue(const std::filesystem::path filename, 
									const TransportCatalogue& catalogue){
	catalogue_proto::TransportCatalogue proto_catalogue = createProtoCatalogue(catalogue);
	std::ofstream ofs(filename, std::ios::binary);
	proto_catalogue.SerializeToOstream(&ofs);
}

catalogue_proto::TransportCatalogue DeserializeCatalogue(const std::filesystem::path filename){
	std::ifstream ifs(filename);
	catalogue_proto::TransportCatalogue proto_catalogue;
	if (!proto_catalogue.ParseFromIstream(&ifs)){
		return {};
	}
	return proto_catalogue;
}

// json::Document ProtoStatParser::parseStatArray(const json::Array& requests_vector){
// 	json::Builder builder;
// 	builder.StatArray();
// 		for (const auto& request : requests_vector){

// 		}
// }

// TransportCatalogue DeserializeTransportCatalogue(const std::filesystem::path& filename){
// 	std::ifstream ifs(filename);
// 	catalogue_proto::TransportCatalogue proto_catalogue;
// 	TransportCatalogue catalogue;
// 	if (!proto_catalogue.ParseFromIstream(&ifs)){
// 		return catalogue;
// 	}
// 	parseStopsFromProto(proto_catalogue, catalogue);
// 	parseBusesFromProto(proto_catalogue, catalogue);
// 	return catalogue;
// }

// void parseStopsFromProto(const catalogue_proto::TransportCatalogue& proto_catalogue, TransportCatalogue& catalogue){
// 	for (int i = 0; i < proto_catalogue.stop_size(); i++){
// 		const catalogue_proto::Stop& stop = proto_catalogue.stop(i);
// 		catalogue.AddStop(stop.name(), geo::Coordinates{stop.lattitude(), stop.longtitude()}, getDistanceMap(stop, proto_catalogue));
// 	}
// }

// DistanceMap getDistanceMap(const catalogue_proto::Stop& stop, const catalogue_proto::TransportCatalogue& proto_catalogue){
// 	return {};
// }

// void parseBusesFromProto(const catalogue_proto::TransportCatalogue& proto_catalogue, TransportCatalogue& catalogue){
// 	for (int i = 0; i < proto_catalogue.bus_size(); i++){
// 		const catalogue_proto::Bus& bus = proto_catalogue.bus(i);
// 		catalogue.AddRoute(bus.name(), false, getStopsVector(bus, proto_catalogue));
// 	}
// }

// std::vector<std::string_view> getStopsVector(const catalogue_proto::Bus& bus, const catalogue_proto::TransportCatalogue& proto_catalogue){
// 	std::vector<std::string_view> stop_vector;
// 	for (int i = 0; i < bus.stop_index_size(); i++){
// 		stop_vector.push_back(proto_catalogue.stop(bus.stop_index(i)).name());
// 	}
// 	return stop_vector;
// }

} // namespace transport_guide