#include "json_reader.h"

namespace transport_guide {

namespace json_reader {

static svg::Color ParseColor(const json::Node& color_node){
    if (color_node.IsString()){
        return color_node.AsString();
    } else {
        json::Array color_array = color_node.AsArray();
        if (color_array.size() == 3){
            return (svg::Rgb(color_array[0].AsInt(), color_array[1].AsInt(), color_array[2].AsInt()));
        } else {
            return (svg::Rgba(color_array[0].AsInt(), color_array[1].AsInt(), color_array[2].AsInt(), color_array[3].AsDouble()));
        }
    }    
}     

map_renderer::RenderSettings parseRenderSettings(const json::Dict& render_settings){
    map_renderer::RenderSettings settings;

    settings.width = render_settings.at("width").AsDouble();
    settings.height = render_settings.at("height").AsDouble();

    settings.padding = render_settings.at("padding").AsDouble();
    settings.line_width = render_settings.at("line_width").AsDouble();
    settings.stop_radius = render_settings.at("stop_radius").AsDouble();

    settings.bus_label_font_size = render_settings.at("bus_label_font_size").AsInt();
    json::Array offset_array = render_settings.at("bus_label_offset").AsArray();
    settings.bus_label_offset = svg::Point(offset_array[0].AsDouble(), offset_array[1].AsDouble());

    settings.stop_label_font_size = render_settings.at("stop_label_font_size").AsInt();
    offset_array = render_settings.at("stop_label_offset").AsArray();
    settings.stop_label_offset = svg::Point(offset_array[0].AsDouble(), offset_array[1].AsDouble());

    const auto& color_node = render_settings.at("underlayer_color");
    settings.underlayer_color = ParseColor(color_node);

    settings.underlayer_width = render_settings.at("underlayer_width").AsDouble();

    const json::Array& palette_colors = render_settings.at("color_palette").AsArray();
    settings.color_palette.resize(palette_colors.size());
    for (size_t i = 0; i < palette_colors.size(); i++){
        settings.color_palette[i] = ParseColor(palette_colors[i]);
    }
    return settings;
}

static DistanceMap GetDistanceToStops(const json::Dict& distance_to_stops){ 
    DistanceMap result;
    for (auto& [stop, distance] : distance_to_stops){
        result.emplace(stop, distance.AsInt());
    }
    return result;
}

ParsedStopQuery parseStopRequest(const json::Dict& stop_request){ 
    ParsedStopQuery result;
    result.name = stop_request.at("name").AsString();
    result.coordinates.lat = stop_request.at("latitude").AsDouble();
    result.coordinates.lng = stop_request.at("longitude").AsDouble();
    result.distance_to_stops = GetDistanceToStops(stop_request.at("road_distances").AsDict());
    return result;
}

static std::vector<std::string_view> parseStopsArray(const json::Array& stops){ 
    std::vector<std::string_view> result;
    for (const auto& stop : stops){
        result.push_back(stop.AsString());
    }
    return result;
}

ParsedBusQuery parseBusRequest(const json::Dict& bus_request){ 
    ParsedBusQuery result;
    result.name = bus_request.at("name").AsString();
    result.stops_on_route = parseStopsArray(bus_request.at("stops").AsArray());
    result.is_cycled = bus_request.at("is_roundtrip").AsBool();
    return result;
}

RoutingSettings parseRoutingSettings(const json::Dict& routing_settings){
    RoutingSettings result;
    result.bus_wait_time = routing_settings.at("bus_wait_time").AsInt();
    result.bus_velocity = routing_settings.at("bus_velocity").AsDouble();
    return result;
}

void updateCatalogue(const json::Array& requests_vector, transport_guide::TransportCatalogue& catalogue){
    std::vector<int> bus_query_positions;   
    for (size_t i = 0; i < requests_vector.size(); i++){
        const json::Dict& input_request = requests_vector[i].AsDict();
        if (input_request.at("type").AsString() == "Bus"){
            bus_query_positions.push_back(i);
        } else if (input_request.at("type").AsString() == "Stop"){
            auto [name_temp, coordinates, distance_to_stops_temp] = parseStopRequest(input_request);
            catalogue.AddStop(name_temp, coordinates, std::move(distance_to_stops_temp));
        }
    }
    for (size_t i = 0; i < bus_query_positions.size(); i++){
        auto [bus_name_temp, is_cycled, stops_on_route] = parseBusRequest(requests_vector[bus_query_positions[i]].AsDict());
            catalogue.AddRoute(bus_name_temp, is_cycled, std::move(stops_on_route));
    }
}

bool StatParser::isValidRequest(const json::Dict& request, QueryType type) const {
    if (type == QueryType::STOP){
        if (catalogue_){
            return catalogue_->IsStopListed(request.at("name").AsString());
        } else {
            return proto_stops_map_->count(request.at("name").AsString());
        }
    } 
    if (type == QueryType::BUS){
        if (catalogue_){
            return catalogue_->IsBusListed(request.at("name").AsString());
        } else {
            return proto_buses_map_->count(request.at("name").AsString());
        }
    }
    if (type == QueryType::MAP){
        return true;
    }
    // query type ROUTE will be checked for validity later
    return true;
}

 QueryType StatParser::defineRequestType(std::string_view type) const {
    // std::cout << "defineRequestType " << type << '\n';  
    if (type == "Stop"){
        return QueryType::STOP;
    } else if (type == "Bus"){
        return QueryType::BUS;
    } else if (type == "Map"){
        return QueryType::MAP;
    } else {
        return QueryType::ROUTE;
    }
}

svg::Document StatParser::getSvgDoc() const {
    map_renderer::MapRenderer renderer(*catalogue_, settings_);
    return renderer.GetSvgDocument();
}

static std::string SvgToStr(svg::Document doc){
    std::stringstream strm;
    doc.Render(strm);
    return std::move(strm.str());
}

static void HandleError(const json::Dict& request, json::Builder& builder){
        builder.StartDict()
               .Key("request_id").Value(json::Node(static_cast<int>(request.at("id").AsInt())))
               .Key("error_message").Value(json::Node(static_cast<std::string>("not found")))
        .EndDict();    
}

void StatParser::parseRouteRequest(const json::Dict& request, json::Builder& builder){
    if (!router_manager_){
        router_manager_ = std::make_unique<router::TransportRouter>(*catalogue_, routing_settings_);
    }
    std::optional<router::RouteInfo> result = router_manager_->GetRouteInfo(request.at("from").AsString(), request.at("to").AsString());
    if (!result.has_value()){
        HandleError(request, builder);
    } else {
        const auto& route_edges = result->route_edges;
        const double time = result->overall_time;
        builder.StartDict()
               .Key("request_id").Value(json::Node(static_cast<int>(request.at("id").AsInt())))
               .Key("items")
                    .StartArray();
                    for (const router::EdgeInfo* edge : route_edges){
                         builder.StartDict()
                                .Key("stop_name").Value(json::Node(static_cast<std::string>(edge->from_stop)))
                                .Key("time").Value(json::Node(static_cast<double>(router_manager_->getWaitWeight())))
                                .Key("type").Value(json::Node(static_cast<std::string>("Wait")))
                         .EndDict()
                         .StartDict()
                                .Key("bus").Value(json::Node(static_cast<std::string>(edge->bus_name)))
                                .Key("span_count").Value(json::Node(static_cast<int>(edge->span)))
                                .Key("time").Value(json::Node(static_cast<double>(edge->weight - router_manager_->getWaitWeight())))
                                .Key("type").Value(json::Node(static_cast<std::string>("Bus")))
                         .EndDict();
                    }
                    builder.EndArray()
               .Key("total_time").Value(json::Node(static_cast<double>(time)))
        .EndDict();       
    }

}

void StatParser::parseStopRequest(const json::Dict& request, json::Builder& builder) const {
    std::string_view name = request.at("name").AsString();
    // if (catalogue_ != nullptr){
    const info::Stop& stop_info = catalogue_->GetStopInfo(name);
    // } else {
    //     const info::Stop stop_info = GetStopInfoFromProto(name);
    // }

    builder.StartDict()
           .Key("request_id").Value(json::Node(static_cast<int>(request.at("id").AsInt())))
           .Key("buses").StartArray();
           for (const auto& bus_name_node : request_handler::getPassingBuses(stop_info)){
                        builder.Value(bus_name_node);
           }
           builder.EndArray()
    .EndDict();    
}

void StatParser::parseStopRequestProto(const json::Dict& request, json::Builder& builder) const {
    std::string name = request.at("name").AsString();
    const catalogue_proto::Stop& stop_info = proto_catalogue_->stop(proto_stops_map_->at(name));
    builder.StartDict()
           .Key("request_id").Value(json::Node(static_cast<int>(request.at("id").AsInt())))
           .Key("buses").StartArray();
           // std::cout << stop_info.bus_index_size() << " size of bus_array\n";
           for (int i = 0; i < stop_info.bus_index_size(); i++){
                        // std::cout << i << '\n';
                        builder.Value(json::Node(static_cast<std::string>(proto_catalogue_->bus(stop_info.bus_index(i)).name())));
           }
           builder.EndArray()
    .EndDict();
}

void StatParser::parseBusRequest(const json::Dict& request, json::Builder& builder) const {
    std::string_view name = request.at("name").AsString();
    // if (catalogue_ != nullptr){
        const info::Bus& bus_info = catalogue_->GetBusInfo(name);
    // } else {
    //     const info::Bus bus_info = GetBusInfoFromProto(name);
    // }

    builder.StartDict()
           .Key("request_id").Value(json::Node(static_cast<int>(request.at("id").AsInt())))
           .Key("curvature").Value(json::Node(static_cast<double>(bus_info.curvature)))
           .Key("route_length").Value(json::Node(static_cast<int>(bus_info.factial_route_length)))
           .Key("stop_count").Value(json::Node(static_cast<int>(bus_info.getStopsCount())))
           .Key("unique_stop_count").Value(json::Node(static_cast<int>(bus_info.getUniqueStopsCount())))
    .EndDict();
}

void StatParser::parseBusRequestProto(const json::Dict& request, json::Builder& builder) const {
    std::string name = request.at("name").AsString();
    const catalogue_proto::Bus& bus_info = proto_catalogue_->bus(proto_buses_map_->at(name));
    builder.StartDict()
           .Key("request_id").Value(json::Node(static_cast<int>(request.at("id").AsInt())))
           .Key("curvature").Value(json::Node(static_cast<double>(bus_info.curvature())))
           .Key("route_length").Value(json::Node(static_cast<int>(bus_info.factual_route_length())))
           .Key("stop_count").Value(json::Node(static_cast<int>(bus_info.is_cycled()
                                                              ? bus_info.stop_index_size()
                                                              : bus_info.stop_index_size() * 2 - 1)))
           .Key("unique_stop_count").Value(json::Node(static_cast<int>(bus_info.unique_stops_count())))
    .EndDict();
}

void StatParser::parseMapRequest(const json::Dict& request, json::Builder& builder) const {
    builder.StartDict()
           .Key("request_id").Value(json::Node(static_cast<int>(request.at("id").AsInt())))
           .Key("map").Value(json::Node(static_cast<std::string>(SvgToStr(getSvgDoc()))))
    .EndDict();   
}

void StatParser::initializeProtoMap(QueryType request_type){
    switch (request_type){
        case QueryType::STOP : {
            if (!proto_stops_map_.has_value()){
                proto_stops_map_.emplace(std::unordered_map<std::string_view, int>{});
                for (int i = 0; i < proto_catalogue_->stop_size(); i++){
                    proto_stops_map_->emplace(proto_catalogue_->stop(i).name(), i);
                }
            }
            break ;
        }
        case QueryType::BUS : {
            if (!proto_buses_map_.has_value()){
                proto_buses_map_.emplace(std::unordered_map<std::string_view, int>{});
                for (int i = 0; i < proto_catalogue_->bus_size(); i++){
                    proto_buses_map_->emplace(proto_catalogue_->bus(i).name(), i);
                }
            }
            break;
        }

    }
}

void StatParser::parseSingleStatRequest(const json::Dict& request, json::Builder& builder){
    QueryType request_type = defineRequestType(request.at("type").AsString());
    if (catalogue_ == nullptr){
        initializeProtoMap(request_type);
    }
    if (!isValidRequest(request, request_type)){
        HandleError(request, builder);
        return;   
    }
    switch (request_type){
        case QueryType::STOP : {       
            catalogue_ != nullptr ? parseStopRequest(request, builder) : parseStopRequestProto(request, builder);
            break;
        }
        case QueryType::BUS : {     
            catalogue_ != nullptr ? parseBusRequest(request, builder) : parseBusRequestProto(request, builder);
            break;
        }
        case QueryType::MAP : {
            // parseMapRequest(request, builder);
            break;
        }
        case QueryType::ROUTE : {
            // parseRouteRequest(request, builder);
            break;
        }
    }
}

json::Document StatParser::parseStatArray(const json::Array& requests_vector) {
    json::Builder builder;
    builder.StartArray();
    // int i =0;
        for (const auto& request : requests_vector){
            // std::cout << i << '\n';
            parseSingleStatRequest(request.AsDict(), builder);
            // i++;
        }
        builder.EndArray();
    return json::Document(builder.Build());
}


} // namespace json_reader

} // namespace transport_guide