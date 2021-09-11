#include "input_reader.h"

using namespace transport_guide;
// using namespace transport_guide::input; // не смог скомпилировать с двумя namespace

QueryType transport_guide::DefineQueryType(const std::string& query){ // Если здесь убрать явное указание перестает компилиться, не понимаю почему
    if (query[0] == 'S'){
        return QueryType::STOP;
    } else {
        return QueryType::BUS;
    }
}

std::vector<Query> input::GetQueries(bool is_for_output){ // если добавить using namespace transport_guide::input та же ошибка undefined reference, так что пришлось везде оставить явное указание пространств имен
    int number_of_queries = read::LineWithNumber();
    std::vector<Query> queries(number_of_queries);
    for (int i = 0; i < number_of_queries; i++){
        queries[i].query = read::Line();
        queries[i].type = DefineQueryType(queries[i].query);
        if (is_for_output){
            queries[i].name = output::detail::GetName(queries[i].query);
        }
    }
    return queries;
}

std::string input::read::Line() {
    std::string s;
    std::getline(std::cin, s);
    return s;
}

int input::read::LineWithNumber() {
    int result;
    std::cin >> result;
    Line();
    return result;
}

DistanceMap input::parse::getStopDistances(std::string_view distance_to_stops_query){
    //7500m to Rossoshanskaya ulitsa, 1800m to Biryusinka, 2400m to Universam //reference stop_str
    int64_t first, last;
    first = 0;
    std::unordered_map<std::string_view, int> stop_distances;
    std::string_view to_stop;
    int meters;
    while (true){
        last = distance_to_stops_query.find('m', first);
        meters = std::stoi(std::string(distance_to_stops_query.substr(first, last - first)));
        first = last + 5; // skipping "m to " to the the first letter of the to_stop name
        last = distance_to_stops_query.find(',', first);
        if (last == distance_to_stops_query.npos){
            to_stop = distance_to_stops_query.substr(first);
            break ;
        } else {
            to_stop = distance_to_stops_query.substr(first, last - first);
            first = last + 2; // skipping two symbols ", " to the first digit of next distance
            stop_distances.emplace(std::make_pair(to_stop, meters));
        }
    }
    stop_distances.emplace(std::make_pair(to_stop, meters));
    return stop_distances;
}

input::ParsedStopQuery input::parse::parseStopQuery(std::string_view stop_query){
    //Stop Biryulyovo Zapadnoye: 55.574371, 37.6517, 7500m to Rossoshanskaya ulitsa, 1800m to Biryusinka, 2400m to Universam //reference stop_str
    int64_t first, last;
    input::ParsedStopQuery parsed_query;

    // парсим название остановки
        first = stop_query.find(' ') + 1; // First letter of the bus stop (after skipping "Stop ")
        last = stop_query.find(':'); // Last letter of the bus stop (befor ":")
        parsed_query.name = stop_query.substr(first, last - first);

    // парсим широту
        first = last + 2; // First digit of the lattitude after skipping ": "
        last = stop_query.find(','); // Last digit of the lattidude is before ","
        parsed_query.lattitude = std::stod(std::string(stop_query.substr(first, last - first)));

    // парсим долготу
        first = last + 2; // First digit of the longtitude after skipping ", ";
        last = stop_query.find(',', first); // Last digit is either before "," or is the end of the string
        if (last == stop_query.npos){
            parsed_query.longtitude = std::stod(std::string(stop_query.substr(first)));
        } else {
            parsed_query.longtitude = std::stod(std::string(stop_query.substr(first, last - first)));
        }

    // парсим дистанцю до остановок
        if (last != stop_query.npos){
            parsed_query.distance_to_stops = input::parse::getStopDistances(stop_query.begin() + last + 2); 
        }

    return parsed_query;
}

DistanceMap input::detail::InsertSvsAndGetUpdatedMap(TransportCatalogue& catalogue, DistanceMap temp_map){
    DistanceMap result;
    for (auto [stop_name_temp, distance] : temp_map){
        result.emplace(std::make_pair(catalogue.GetSVFromInsertedName(stop_name_temp, QueryType::STOP), distance));
    }
    return result;
}

void input::parse::detail::updateBackRoute(const std::vector<info::Stop*>& stops_vec, info::Bus& result){
    for (int i = stops_vec.size() - 2; i >= 0; i--){
        if (stops_vec[i + 1]->distance_to_stops.count(stops_vec[i]->getName())){
            result.factial_route_length += stops_vec[i + 1]->distance_to_stops.at(stops_vec[i]->getName());
        } else {
            result.factial_route_length += stops_vec[i]->distance_to_stops.at(stops_vec[i + 1]->getName());
        }
    }
    result.geo_route_length *= 2;
}

void input::parse::detail::updateDistance(const std::vector<info::Stop*>& stops_vec, info::Bus& result){
    if (stops_vec.size() == 1){
        return ;
    }
    int size = stops_vec.size();
    std::string_view last_stop_name = stops_vec.back()->getName();
    result.geo_route_length += geo::ComputeDistance(stops_vec[size - 2]->coordinates, stops_vec.back()->coordinates);

    if (stops_vec[size - 2]->distance_to_stops.count(last_stop_name)){
        result.factial_route_length += stops_vec[size - 2]->distance_to_stops.at(last_stop_name);
    } else {
        result.factial_route_length += stops_vec.back()->distance_to_stops.at(stops_vec[size-2]->getName());
    }
}

void input::parse::updateBusInfo(TransportCatalogue& catalogue, std::vector<std::string_view> stops_on_route_temp, bool is_cycled, info::Bus& bus_info){ // making catalogue nonconst to call nonconst getStopInfo
    for (auto stop : stops_on_route_temp){
        bus_info.stops.push_back(&(catalogue.GetStopInfo(stop)));
        bus_info.unique_stops.insert(bus_info.stops.back()->getName());
        input::parse::detail::updateDistance(bus_info.stops, bus_info);
    }
    if (!is_cycled){
        input::parse::detail::updateBackRoute(bus_info.stops, bus_info);
    }
}

void input::detail::updatePassingBus(info::Bus* bus_info, std::vector<info::Stop*>& stops_vec){
    for (auto& stop : stops_vec){
        stop->passing_buses.insert(bus_info);
    }
}

void input::updateCatalogue(TransportCatalogue& catalogue,const std::vector<Query>& input_queries){
    std::vector<int> positions_of_bus_queries;
    for (int i = 0; i < input_queries.size(); i++){
        if (input_queries[i].type == QueryType::STOP){
            auto [stop_name_temp, lat, lng, distance_to_stops_temp] = input::parse::parseStopQuery(input_queries[i].query);
            std::string_view stop_name = catalogue.GetSVFromInsertedName(stop_name_temp, QueryType::STOP);
            catalogue.AddStop(stop_name);
            info::Stop& stop_info = catalogue.GetStopInfo(stop_name);
            stop_info.name = stop_name;
            stop_info.coordinates = {lat, lng};
            stop_info.distance_to_stops = input::detail::InsertSvsAndGetUpdatedMap(catalogue, std::move(distance_to_stops_temp));
        } else {
            positions_of_bus_queries.push_back(i);
        }
    }
    for (int i = 0; i < positions_of_bus_queries.size(); i++){
        auto [bus_name_temp, is_cycled, stops_on_route_temp] = input::parse::parseBusQuery(input_queries[positions_of_bus_queries[i]].query);

        std::string_view bus_name = catalogue.GetSVFromInsertedName(bus_name_temp, QueryType::BUS);
        catalogue.AddRoute(bus_name);
        info::Bus& bus_info = catalogue.GetRouteInfo(bus_name);
        input::parse::updateBusInfo(catalogue, std::move(stops_on_route_temp), is_cycled, bus_info);
        bus_info.name = bus_name;
        bus_info.is_cycled = is_cycled;
        input::detail::updatePassingBus(&bus_info, bus_info.stops);
    }
}

std::string_view input::parse::detail::GetStopNameSV(const std::string& stop_query){
    uint64_t pos_first, pos_last;
    pos_first = stop_query.find(' ') + 1; // First letter of the bus stop (after skipping "Stop ")
    pos_last = stop_query.find(':'); // Last letter of the bus stop (befor ":")
    return std::string_view(stop_query.data() + pos_first, pos_last - pos_first);
}

input::ParsedBusQuery input::parse::parseBusQuery(std::string_view bus_query){
    // Bus 828: Biryulyovo Zapadnoye > Universam > Rossoshanskaya ulitsa > Biryulyovo Zapadnoye // reference bus query
    input::ParsedBusQuery parsed_query;
    const char separator = parse::detail::DefineSeparator(bus_query);
    int64_t first, last;

    //Парсим название автобуса
        last = bus_query.find(':', 4); // finding the end of the bus name after 4 symbols "Bus "
        parsed_query.name = bus_query.substr(4, last - 4);
    
    //Парсим остановки
    first = last + 2; // skipping ": " to find first character of the stop name
    while (true) {
        last = bus_query.find(separator, first);
        if (last == bus_query.npos){
            std::string_view last_stop = bus_query.substr(first);
            parsed_query.stops_on_route.push_back(last_stop);
            break ;
        }
        std::string_view stop = bus_query.substr(first, last - (first + 1));
        parsed_query.stops_on_route.push_back(stop);
        first = last + 2; // skipping "'separator' " to find first character of the stop name
    }
    (separator == '>') ? (parsed_query.is_cycled = true) : (parsed_query.is_cycled = false);
    return parsed_query;
}

char input::parse::detail::DefineSeparator(std::string_view bus_query){
    char separator;
    if (bus_query.rfind('>') != bus_query.npos){
        separator = '>';
    } else {
        separator = '-';
    }
    return separator;
}
