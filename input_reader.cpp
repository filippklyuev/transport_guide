#include "input_reader.h"

namespace transport_guide { // решил сделтать неймспейсы по гугловским правилам, столкнулся с большим количеством сложностей при использовании using namespace transport_guide::input

namespace input {

QueryType DefineQueryType(const std::string& query){ 
    if (query[0] == 'S'){
        return QueryType::STOP;
    } else {
        return QueryType::BUS;
    }
}

std::vector<Query> GetQueries(std::istream& in){
    int number_of_queries = read::LineWithNumber(in);
    std::vector<Query> queries(number_of_queries);
    for (int i = 0; i < number_of_queries; i++){
        queries[i].query = read::Line(in);
        queries[i].type = DefineQueryType(queries[i].query);
    }
    return queries;
}

std::string read::Line(std::istream& in) {
    std::string s;
    std::getline(in, s);
    return s;
}

int read::LineWithNumber(std::istream& in) {
    int result;
    in >> result;
    Line(in);
    return result;
}

// Не стал делать отдельную отчипывающую функцию, потому что вроде как и так отщипываю ненужное начало string_view в одну строчку

DistanceMap parse::getStopDistances(std::string_view distance_to_stops_query){
    //7500m to Rossoshanskaya ulitsa, 1800m to Biryusinka, 2400m to Universam //reference stop_str
    DistanceMap stop_distances;
    std::string_view to_stop;
    int meters;
    while (true){
        int64_t last = distance_to_stops_query.find('m');
        meters = std::stoi(std::string(distance_to_stops_query.substr(0, last)));
        distance_to_stops_query = distance_to_stops_query.begin() + last + 5; // skipping "m to " to the the first letter of the to_stop name 
        last = distance_to_stops_query.find(',');
        if (last == distance_to_stops_query.npos){
            to_stop = distance_to_stops_query;
            break ;
        } else {
            to_stop = distance_to_stops_query.substr(0, last);
            distance_to_stops_query = distance_to_stops_query.begin() + last + 2; // skipping two symbols ", " to the first digit of next distance
            stop_distances.emplace(std::make_pair(to_stop, meters));
        }
    }
    stop_distances.emplace(std::make_pair(to_stop, meters));
    return stop_distances;
}

ParsedStopQuery parse::parseStopQuery(std::string_view stop_query){
    //Stop Biryulyovo Zapadnoye: 55.574371, 37.6517, 7500m to Rossoshanskaya ulitsa, 1800m to Biryusinka, 2400m to Universam //reference stop_str
    ParsedStopQuery parsed_query;
    // парсим название остановки
        parsed_query.name = detail::GetName(stop_query.begin() + 5); // First letter of the bus stop (after skipping "Stop ");
        stop_query = stop_query.begin() + 7 + parsed_query.name.size(); // skipping 2ws + stop_name.size() +"Stop";
    // парсим широту
        int64_t last = stop_query.find(','); // Last digit of the lattidude is before ","
        parsed_query.lattitude = std::stod(std::string(stop_query.substr(0, last)));
        stop_query = stop_query.begin() + last + 2; //First digit of the longtitude after skipping ", " and lattitude.size();
    // парсим долготу
        last = stop_query.find(','); // Last digit is either before "," or is the end of the string
        if (last == stop_query.npos){
            parsed_query.longtitude = std::stod(std::string(stop_query));
        } else {
            parsed_query.longtitude = std::stod(std::string(stop_query.substr(0, last)));
            // парсим дистанцю до остановок
            parsed_query.distance_to_stops = getStopDistances(stop_query.begin() + last + 2); 
        }

    return parsed_query;
}

DistanceMap detail::InsertSvsAndGetNewMap(TransportCatalogue& catalogue, DistanceMap temp_map){
    DistanceMap result;
    for (auto [stop_name_temp, distance] : temp_map){
        result.emplace(std::make_pair(catalogue.InsertNameSV(stop_name_temp, QueryType::STOP), distance));
    }
    return result;
}

void updateCatalogue(TransportCatalogue& catalogue,const std::vector<Query>& input_queries){
    std::vector<int> positions_of_bus_queries;
    for (int i = 0; i < input_queries.size(); i++){
        if (input_queries[i].type == QueryType::STOP){
            auto [stop_name_temp, lat, lng, distance_to_stops_temp] = parse::parseStopQuery(input_queries[i].query);
            std::string_view stop_name = catalogue.InsertNameSV(stop_name_temp, QueryType::STOP);
            catalogue.AddStop(stop_name);
            catalogue.GetStopInfo(stop_name).setName(stop_name).setCoordinates({lat, lng}).setDistanceToStops(detail::InsertSvsAndGetNewMap(catalogue, std::move(distance_to_stops_temp)));
        } else {
            positions_of_bus_queries.push_back(i);
        }
    }
    for (int i = 0; i < positions_of_bus_queries.size(); i++){
        auto [bus_name_temp, is_cycled, stops_on_route_temp] = parse::parseBusQuery(input_queries[positions_of_bus_queries[i]].query);
        std::string_view bus_name = catalogue.InsertNameSV(bus_name_temp, QueryType::BUS);
        catalogue.AddRoute(bus_name);
        catalogue.GetRouteInfo(bus_name).setName(bus_name).setIsCycled(is_cycled).setStopsAndDistance(catalogue, std::move(stops_on_route_temp)).updatePassingBus();   
    }
}

std::string_view parse::detail::GetName(std::string_view stop_query){
    uint64_t pos_last;
    pos_last = stop_query.find(':'); // Last letter of the bus stop (befor ":")
    return std::string_view(stop_query.data(), pos_last);
}

ParsedBusQuery parse::parseBusQuery(std::string_view bus_query){
    // Bus 828: Biryulyovo Zapadnoye > Universam > Rossoshanskaya ulitsa > Biryulyovo Zapadnoye // reference bus query
    ParsedBusQuery parsed_query;
    const char separator = detail::DefineSeparator(bus_query);
    int64_t last;

    //Парсим название автобуса
        parsed_query.name = detail::GetName(bus_query.begin() + 4); // skipping "Bus ";
    bus_query = bus_query.begin() + parsed_query.name.size() + 6;  // skipping "Bus" + name.size() + 2ws;
    //Парсим остановки
    while (true) {
        last = bus_query.find(separator);
        if (last == bus_query.npos){
            parsed_query.stops_on_route.push_back(bus_query);
            break ;
        }
        parsed_query.stops_on_route.push_back(bus_query.substr(0, last - 1)); // finding stop name -1 ws before separator;
        bus_query = bus_query.begin() + last + 2; // skipping separator + 1ws to find first character of the stop name
    }
    (separator == '>') ? (parsed_query.is_cycled = true) : (parsed_query.is_cycled = false);
    return parsed_query;
}

char parse::detail::DefineSeparator(std::string_view bus_query){
    char separator;
    if (bus_query.rfind('>') != bus_query.npos){
        separator = '>';
    } else {
        separator = '-';
    }
    return separator;
}

} // namespace input

} //namespace transport_guide