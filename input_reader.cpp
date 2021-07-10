#include "input_reader.h"

using namespace transport_guide::input;

std::string read::Line() {
    std::string s;
    std::getline(std::cin, s);
    return s;
}

int read::LineWithNumber() {
    int result;
    std::cin >> result;
    Line();
    return result;
}

Queries::Type transport_guide::input::DefineQueryType(const std::string& query){
    if (query[0] == 'S'){
        return Queries::Type::STOP;
    } else {
        return Queries::Type::BUS;
    }
}

transport_guide::input::Queries transport_guide::input::GetQueriesByType(){
    int number_of_queries = read::LineWithNumber();
    Queries input_queries;
    for (int i = 0 ; i < number_of_queries; i++){
        std::string query = read::Line();
        Queries::Type query_type = DefineQueryType(query);
        if (query_type == Queries::Type::STOP){
            input_queries.stops_strings.push_back(std::move(query));
        } else {
            input_queries.buses_strings.push_back(std::move(query));
        }
    }
    return input_queries;
}

void to_catalogue::PutStops(transport_guide::TransportCatalogue& catalogue,const std::vector<std::string>& stops_strings){
    for (const auto& stop_query: stops_strings){
        catalogue.AddStop(parse::GetStopNameAndInfo(catalogue, stop_query));
    }
}

std::string_view parse::detail::GetStopNameSV(const std::string& stop_query){
    uint64_t pos_first, pos_last;
    pos_first = stop_query.find(' ') + 1; // First letter of the bus stop (after skipping "Stop ")
    pos_last = stop_query.find(':'); // Last letter of the bus stop (befor ":")
    return std::string_view(stop_query.data() + pos_first, pos_last - pos_first);
}

void to_catalogue::PutBuses(transport_guide::TransportCatalogue& catalogue,const std::vector<std::string>& buses_strings){
    for (const auto& bus_query: buses_strings){
        catalogue.AddRoute(parse::GetBusNameAndRoute(catalogue, bus_query));
    }
}

std::pair<std::string_view, transport_guide::info::Stop> parse::GetStopNameAndInfo (transport_guide::TransportCatalogue& catalogue, const std::string& stop_query){
    //Stop Biryulyovo Zapadnoye: 55.574371, 37.6517, 7500m to Rossoshanskaya ulitsa, 1800m to Biryusinka, 2400m to Universam //reference stop_str
    int64_t pos_first, pos_last;
    pos_first = stop_query.find(' ') + 1; // First letter of the bus stop (after skipping "Stop ")
    pos_last = stop_query.find(':'); // Last letter of the bus stop (befor ":")
    std::string_view stop_name = catalogue.GetSVFromInsertedStopName(stop_query.substr(pos_first, pos_last - pos_first));
    pos_first = pos_last + 2; // First digit of the lattitude after skipping ": "
    pos_last = stop_query.find(','); // Last digit of the lattidude is before ","
    transport_guide::info::Stop stop_info;
    stop_info.coordinates.lat = std::stod(stop_query.substr(pos_first, pos_last - pos_first));
    pos_first = pos_last + 2; // First digit of the longtitude after skipping ", ";
    pos_last = stop_query.find(',', pos_first); // Last digit is either before "," or is the end of the string
    if (pos_last == stop_query.npos){
        stop_info.coordinates.lng = std::stod(stop_query.substr(pos_first));
        return std::make_pair(stop_name, stop_info);
    }
    stop_info.coordinates.lng = std::stod(stop_query.substr(pos_first, pos_last - pos_first));
    stop_info.distance_to_stops = GetStopDistances(catalogue, stop_name, stop_query, pos_last);
    return std::make_pair(stop_name, stop_info);;
}

std::unordered_map<std::string_view, int> parse::GetStopDistances(transport_guide::TransportCatalogue& catalogue, std::string_view stop_name, const std::string& stop_query, uint64_t pos_last){
    //Stop Biryulyovo Zapadnoye: 55.574371, 37.6517, 7500m to Rossoshanskaya ulitsa, 1800m to Biryusinka, 2400m to Universam //reference stop_str
    int64_t pos_first = pos_last + 2; // first digit of distance after skipping ", "
    std::unordered_map<std::string_view, int> stop_distances;
    std::string_view to_stop;
    int meters;
    while (true){
        pos_last = stop_query.find('m', pos_first);
        meters = std::stoi(stop_query.substr(pos_first, pos_last - pos_first));
        pos_first = pos_last + 5; // skipping "m to " to the the first letter of the to_stop name
        pos_last = stop_query.find(',', pos_first);
        if (pos_last == stop_query.npos){
            to_stop = catalogue.GetSVFromInsertedStopName(stop_query.substr(pos_first));
            break ;
        } else {
            to_stop = catalogue.GetSVFromInsertedStopName(stop_query.substr(pos_first, pos_last - pos_first));
            pos_first = pos_last + 2; // skipping two symbols ", " to the first digit of next distance
            stop_distances.emplace(std::make_pair(to_stop, meters));
        }
    }
    stop_distances.emplace(std::make_pair(to_stop, meters));
    return stop_distances;
}

std::pair<std::string_view, transport_guide::info::Bus> parse::GetBusNameAndRoute(transport_guide::TransportCatalogue& catalogue, const std::string& bus_query){
    // Bus 828: Biryulyovo Zapadnoye > Universam > Rossoshanskaya ulitsa > Biryulyovo Zapadnoye // reference bus query
    const char separator = parse::detail::DefineSeparator(bus_query);
    int64_t pos_last = bus_query.find(':', 4); // finding the end of the bus name after 4 symbols "Bus "
    std::string_view bus_name = catalogue.GetSVFromInsertedBusName(bus_query.substr(4, pos_last - 4));
    int64_t pos_first = pos_last + 2; // skipping ": " to find first character of the stop name
    transport_guide::info::Bus bus_info;
    while (true) {
        pos_last = bus_query.find(separator, pos_first);
        if (pos_last == bus_query.npos){
            std::string_view last_stop = catalogue.GetSVFromInsertedBusName(bus_query.substr(pos_first));
            catalogue.AddDistanceToStop(last_stop, bus_info);
            catalogue.AddBusToStop(bus_name, last_stop);
            break ;
        }
        std::string_view stop = catalogue.GetSVFromInsertedBusName(bus_query.substr(pos_first, pos_last - (pos_first + 1)));
        catalogue.AddDistanceToStop(stop, bus_info);
        catalogue.AddBusToStop(bus_name, stop);
        pos_first = pos_last + 2; // skipping "'separator' " to find first character of the stop name
    }
    (separator == '>') ? (bus_info.is_cycled = true) : (bus_info.is_cycled = false);
    if (bus_info.is_cycled == false){
        bus_info.factial_route_length += catalogue.CalculateBackRoute(bus_info);
        bus_info.geo_route_length *= 2;
    }
    return std::make_pair(bus_name, bus_info);
}

char parse::detail::DefineSeparator(const std::string& bus_query){
    char separator;
    if (bus_query.rfind('>') != bus_query.npos){
        separator = '>';
    } else {
        separator = '-';
    }
    return separator;
}
