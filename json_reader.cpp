#include "json_reader.h"

void JsonReader::ParseStops(trans_ctl::TransportCatalogue& catalogue, const json::Document& document)
{
	const auto& base_requests_ = document.GetRoot().AsDict().at("base_requests").AsArray();
	for (const auto& base_request : base_requests_) {
		if (base_request.AsDict().at("type").AsString() == "Stop") {
			trans_ctl::Stop stop;
			stop.name = base_request.AsDict().at("name").AsString();
			stop.coordinates = { base_request.AsDict().at("latitude").AsDouble(), base_request.AsDict().at("longitude").AsDouble() };
			catalogue.AddStop(std::move(stop));
		}
	}
}

void JsonReader::ParseStopsLength(trans_ctl::TransportCatalogue& catalogue, const json::Document& document)
{
	const auto& base_requests_ = document.GetRoot().AsDict().at("base_requests").AsArray();
	for (const auto& base_request : base_requests_) {
		if (base_request.AsDict().at("type").AsString() == "Stop") {
			std::string_view name_first_stop = base_request.AsDict().at("name").AsString();
			const auto& road_distances_ = base_request.AsDict().at("road_distances").AsDict();
			for (const auto& [name_second_stop, distance] : road_distances_) {
				catalogue.SetStopsLength(catalogue.FindStop(name_first_stop), catalogue.FindStop(name_second_stop), distance.AsDouble());
			}
		}
	}
}

void JsonReader::ParseBus(trans_ctl::TransportCatalogue& catalogue, const json::Document& document)
{
	const auto& base_requests_ = document.GetRoot().AsDict().at("base_requests").AsArray();
	for (const auto& base_request : base_requests_) {
		if (base_request.AsDict().at("type").AsString() == "Bus") {
			trans_ctl::Bus bus_;
			bus_.name = base_request.AsDict().at("name").AsString();
			bus_.isCircleRoute = base_request.AsDict().at("is_roundtrip").AsBool();
			const auto& stops_ = base_request.AsDict().at("stops").AsArray();
			for (const auto& stop_ : stops_) {
				bus_.stops.push_back(catalogue.FindStop(stop_.AsString()));
			}
			catalogue.AddBus(std::move(bus_));
		}
	}
}

svg::Color JsonReader::ParseColor(const json::Node& color) const
{
	svg::Color color_;

	if (color.IsString()) {
		color_ = color.AsString();
	}
	else if (color.IsArray() && color.AsArray().size() == 3) {
		const auto& rgb = color.AsArray();
		color_ = svg::Rgb{ static_cast<uint8_t>(rgb[0].AsInt()), 
			               static_cast<uint8_t>(rgb[1].AsInt()), 
			               static_cast<uint8_t>(rgb[2].AsInt()) };
	}
	else if (color.IsArray() && color.AsArray().size() == 4) {
		const auto& rgba = color.AsArray();
		color_ = svg::Rgba{ static_cast<uint8_t>(rgba[0].AsInt()), 
			                static_cast<uint8_t>(rgba[1].AsInt()), 
			                static_cast<uint8_t>(rgba[2].AsInt()), 
							rgba[3].AsDouble() };
	}

	return color_;
}

render::RenderSettings JsonReader::ParseRenderSettings(const json::Document& document) const
{
	render::RenderSettings render_settings_;
	const auto& settings_ = document.GetRoot().AsDict().at("render_settings").AsDict();

	render_settings_.size.height = settings_.at("height").AsDouble();
	render_settings_.size.width = settings_.at("width").AsDouble();
	render_settings_.padding = settings_.at("padding").AsDouble();
	render_settings_.line_width = settings_.at("line_width").AsDouble();
	render_settings_.stop_radius = settings_.at("stop_radius").AsDouble();
	render_settings_.bus_label_font_size = settings_.at("bus_label_font_size").AsInt();
	render_settings_.bus_label_offset.dx = settings_.at("bus_label_offset").AsArray()[0].AsDouble();
	render_settings_.bus_label_offset.dy = settings_.at("bus_label_offset").AsArray()[1].AsDouble();
	render_settings_.stop_label_font_size = settings_.at("stop_label_font_size").AsInt();
	render_settings_.stop_label_offset.dx = settings_.at("stop_label_offset").AsArray()[0].AsDouble();
	render_settings_.stop_label_offset.dy = settings_.at("stop_label_offset").AsArray()[1].AsDouble();
	render_settings_.underlayer_color = ParseColor(settings_.at("underlayer_color"));
	render_settings_.underlayer_width = settings_.at("underlayer_width").AsDouble();
	const auto& palette_ = settings_.at("color_palette").AsArray();
	for (const auto& color_ : palette_) {
		render_settings_.color_palette.push_back(ParseColor(color_));
	}

	return render_settings_;
}

std::string JsonReader::ParseSerializationSettings(const json::Document& document)
{
	return document.GetRoot().AsDict().at("serialization_settings").AsDict().at("file").AsString();
}

void JsonReader::ReadJSON(trans_ctl::TransportCatalogue& catalogue, const json::Document& document)
{
	ParseStops(catalogue, document);
	ParseStopsLength(catalogue, document);
	ParseBus(catalogue, document);
}

Dict JsonReader::GetBusStats(const json::Dict& stat_request, RequestHandler& request_handler) const
{
	auto bus_stat = request_handler.GetBusStats(stat_request.at("name").AsString());
	if (bus_stat == nullptr) {
		Dict dict_node = json::Builder{}.StartDict().Key("request_id"s).Value(stat_request.at("id").AsInt())
													.Key("error_message"s).Value("not found"s).EndDict().Build().AsDict();
		return dict_node;
	}

	Dict dict_node = json::Builder{}.StartDict().Key("curvature"s).Value(static_cast<double>(bus_stat->route_length / bus_stat->route_distance))
												.Key("request_id"s).Value(stat_request.at("id").AsInt())
												.Key("route_length"s).Value(static_cast<double>(bus_stat->route_length))
												.Key("stop_count"s).Value(static_cast<int>(bus_stat->stops_count))
												.Key("unique_stop_count"s).Value(static_cast<int>(bus_stat->unique_stops_count))
												.EndDict().Build().AsDict();
	return dict_node;
}

Dict JsonReader::GetStopStats(const json::Dict& stat_request, RequestHandler& request_handler) const
{
	if (request_handler.GetStop(stat_request.at("name").AsString()) == nullptr) {
		Dict dict_node = json::Builder{}.StartDict().Key("request_id"s).Value(stat_request.at("id").AsInt())
													.Key("error_message"s).Value("not found"s)
													.EndDict().Build().AsDict();
		return dict_node;
	}
	Array stops{};
	auto stop_stat = request_handler.GetBusesByStop(stat_request.at("name").AsString());
	if (stop_stat.empty()) {
		Dict dict_node = json::Builder{}.StartDict().Key("buses"s).Value(stops)
													.Key("request_id"s).Value(stat_request.at("id").AsInt())
													.EndDict().Build().AsDict();
		return dict_node;
	}

	for (const auto& stop : stop_stat) { stops.push_back(stop->name); }
	Dict dict_node = json::Builder{}.StartDict().Key("buses"s).Value(stops)
												.Key("request_id"s).Value(stat_request.at("id").AsInt())
												.EndDict().Build().AsDict();
	return dict_node;
}

Dict JsonReader::GetRoute(const json::Dict& stat_request, RequestHandler& request_handler) const
{
	auto route = request_handler.FindRoute(stat_request.at("from").AsString(), stat_request.at("to").AsString());

	if (!route) {
		Dict dict_node = json::Builder{}.StartDict()
			.Key("request_id"s).Value(stat_request.at("id").AsInt())
			.Key("error_message"s).Value("not found"s)
			.EndDict().Build().AsDict();
		return dict_node;
	}

	Array items_;

	auto graph = request_handler.GetGraph();
	auto edges = route.value().edges;

	for (const auto& edge_id : edges) {
		auto [bus_name, stop_name, span_count] = request_handler.GetEdgeInfo(edge_id);
		if (bus_name == "waiting") {
			Dict dict_ = json::Builder{}.StartDict().Key("type"s).Value("Wait"s)
				.Key("stop_name"s).Value(request_handler.GetStop(stop_name)->name)
				.Key("time"s).Value(graph.GetEdge(edge_id).weight)
				.EndDict().Build().AsDict();
			items_.push_back(std::move(dict_));
		}
		else {
			Dict dict_ = json::Builder{}.StartDict().Key("type"s).Value("Bus"s)
				.Key("bus"s).Value(request_handler.GetBus(bus_name)->name)
				.Key("span_count"s).Value(span_count)
				.Key("time"s).Value(graph.GetEdge(edge_id).weight)
				.EndDict().Build().AsDict();
			items_.push_back(std::move(dict_));
		}
	}

	Dict dict_node = json::Builder{}.StartDict()
		.Key("request_id"s).Value(stat_request.at("id").AsInt())
		.Key("total_time").Value(route.value().weight)
		.Key("items").Value(items_)
		.EndDict().Build().AsDict();
	return dict_node;
}

Array JsonReader::GetStats(const json::Document& document, RequestHandler& request_handler) const
{
	Array stats_;
	const auto& stat_requests_ = document.GetRoot().AsDict().at("stat_requests").AsArray();
	for (const auto& stat_request : stat_requests_) {
		if (stat_request.AsDict().at("type").AsString() == "Bus") {
			stats_.push_back(GetBusStats(stat_request.AsDict(), request_handler));
		}
		if (stat_request.AsDict().at("type").AsString() == "Stop") {
			stats_.push_back(GetStopStats(stat_request.AsDict(), request_handler));
		}
		if (stat_request.AsDict().at("type").AsString() == "Map") {
			stats_.push_back(GetMap(document, request_handler));
		}
		if (stat_request.AsDict().at("type").AsString() == "Route") {
			stats_.push_back(GetRoute(stat_request.AsDict(), request_handler));
		}
	}
	return stats_;
}

void JsonReader::PrintStats(const json::Document& document, RequestHandler& request_handler, std::ostream& out)
{
	Document doc(GetStats(document, request_handler));
	json::Print(doc, out);
}

Dict JsonReader::GetMap(const json::Document& document, RequestHandler& request_handler) const
{
	int id;
	const auto& stat_requests_ = document.GetRoot().AsDict().at("stat_requests").AsArray();
	for (const auto& request : stat_requests_) {
		if (request.AsDict().at("type").AsString() == "Map") {
			id = request.AsDict().at("id").AsInt();
		}
	}

	std::ostringstream oss;
	request_handler.RenderMap().Render(oss);

	Dict dict_node = json::Builder{}.StartDict().Key("request_id"s).Value(id).Key("map"s).Value(oss.str()).EndDict().Build().AsDict();

	return dict_node;
}

transport_router::Routing_settings JsonReader::ParseRoutingSettings(const json::Document& document) const
{
	transport_router::Routing_settings routing_settings_;

	const auto& settings_ = document.GetRoot().AsDict().at("routing_settings").AsDict();

	routing_settings_.bus_velocity = settings_.at("bus_velocity").AsDouble();
	routing_settings_.bus_wait_time = settings_.at("bus_wait_time").AsInt();

	return routing_settings_;
}