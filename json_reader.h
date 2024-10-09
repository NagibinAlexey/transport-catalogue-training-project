#pragma once

#include "json_builder.h"
#include "request_handler.h"
#include <iostream>
#include <fstream>
#include <string>
#include <string_view>
#include <vector>
#include <utility>
#include <iomanip>

using namespace json;
using namespace std::literals;

class JsonReader {
	//cataloque content and statistics
public:
	void ReadJSON(trans_ctl::TransportCatalogue& catalogue, const json::Document& document);
	Array GetStats(const json::Document& document, RequestHandler& request_handler) const;
	void PrintStats(const json::Document& document, RequestHandler& request_handler, std::ostream& out);

private:
	void ParseStops(trans_ctl::TransportCatalogue& catalogue, const json::Document& document);
	void ParseStopsLength(trans_ctl::TransportCatalogue& catalogue, const json::Document& document);
	void ParseBus(trans_ctl::TransportCatalogue& catalogue, const json::Document& document);

	Dict GetBusStats(const json::Dict& stat_request, RequestHandler& request_handler) const;
	Dict GetStopStats(const json::Dict& stat_request, RequestHandler& request_handler) const;
	Dict GetRoute(const json::Dict& stat_request, RequestHandler& request_handler) const;

	//map 
public:
	render::RenderSettings ParseRenderSettings(const json::Document& document) const;
	std::string ParseSerializationSettings(const json::Document& document);
	Dict GetMap(const json::Document& document, RequestHandler& request_handler) const;

private:
	svg::Color ParseColor(const json::Node& color) const;

	//routing
public:
	transport_router::Routing_settings ParseRoutingSettings(const json::Document& document) const;
};