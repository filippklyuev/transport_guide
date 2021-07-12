#pragma once
#include <iomanip>
#include <iostream>
#include <chrono>
#include <iterator>
#include <set>
#include <string>
#include <string_view>
#include <vector>
#include <utility>

#include "geo.h"
#include "transport_catalogue.h"

#define PROFILE_CONCAT_INTERNAL(X, Y) X##Y
#define PROFILE_CONCAT(X, Y) PROFILE_CONCAT_INTERNAL(X, Y)
#define UNIQUE_VAR_NAME_PROFILE PROFILE_CONCAT(profileGuard, LINE)
#define LOG_DURATION(x) LogDuration UNIQUE_VAR_NAME_PROFILE(x)
#define LOG_DURATION_STREAM(x,y) LogDuration UNIQUE_VAR_NAME_PROFILE(x,y)


class LogDuration {
public:
    // заменим имя типа std::chrono::steady_clock
    // с помощью using для удобства
    using Clock = std::chrono::steady_clock;

    LogDuration(const std::string& id, std::ostream& out = std::cerr)
        : id_(id), out_(out) {
    }

    ~LogDuration() {
        using namespace std::chrono;
        using namespace std::literals;

        const auto end_time = Clock::now();
        const auto dur = end_time - start_time_;
        out_ << "Operation time for " << id_ << ": "s << duration_cast<milliseconds>(dur).count() << " ms"s << std::endl;
    }

private:
    const std::string id_;
    const Clock::time_point start_time_ = Clock::now();
    std::ostream& out_;
};

namespace transport_guide {

enum class QueryType {
	STOP,
	BUS
};

QueryType DefineQueryType(const std::string& query);

namespace input {

struct Query {
	std::string query;
	transport_guide::QueryType type;
};

void ParseInput(transport_guide::TransportCatalogue& catalogue,const std::vector<Query>& input_queries);

std::vector<Query> GetQueries();

namespace read {

int LineWithNumber();

std::string Line();

} // namespace read

namespace parse {

namespace detail {

std::string_view GetStopNameSV(const std::string& stop_str);

char DefineSeparator(const std::string& bus_query);

} // namespace detail

std::pair<std::string_view, transport_guide::info::Stop> GetStopNameAndInfo (transport_guide::TransportCatalogue& catalogue, const std::string& stop_query);

std::unordered_map<std::string_view, int> GetStopDistances(transport_guide::TransportCatalogue& catalogue, const std::string& stop_query, uint64_t pos_last);

std::pair<std::string_view, transport_guide::info::Bus> GetBusNameAndRoute(transport_guide::TransportCatalogue& catalogue, const std::string& bus_query);
	
} //namespace parse

} // namespace input

} // namespace transport_guide