#pragma once
#include <map>
#include <memory>
#include <optional>
#include <string_view>
#include <sstream>
#include <unordered_map>
#include <utility>

#include "graph.h"
#include "router.h"
#include "json.h"
#include "svg.h"
#include "transport_catalogue.h"
#include "map_renderer.h"

namespace transport_guide {

namespace request_handler {

using BusInfo = transport_guide::info::Bus;
using StopInfo = transport_guide::info::Stop; 

json::Array getPassingBuses(const StopInfo& stop_info);

void printSvgDoc(std::ostream& out, const json::Document& doc_to_print);

} // namespace request handler

} // namespace transport_guide