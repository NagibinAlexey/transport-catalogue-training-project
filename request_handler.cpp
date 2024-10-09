#include "request_handler.h"

trans_ctl::Bus* RequestHandler::GetBus(const std::string_view& bus_name) const
{
	return db_.FindBus(bus_name);
}

std::unordered_map<std::string_view, trans_ctl::Bus*> RequestHandler::GetAllBuses() const
{
	return db_.GetAllRoutes();
}

trans_ctl::BusStat* RequestHandler::GetBusStats(const std::string_view& bus_name)
{
	return db_.ExecuteBusRequest(GetBus(bus_name));
}

std::map<trans_ctl::Bus*, svg::Color, trans_ctl::BusCmp> RequestHandler::GetBusesColors()
{
	std::map<trans_ctl::Bus*, svg::Color, trans_ctl::BusCmp> map_;

	const auto& buses_map = GetAllBuses();
	for (const auto& [bus_name, bus_ptr] : buses_map) {
		if (!db_.BusIsEmpty(bus_ptr)) {
			map_[bus_ptr];
		}
	}

	size_t i = 0;
	size_t colors_count = GetRenderSettings().color_palette.size();
	for (auto& [bus, color] : map_) {
		if (i > colors_count - 1) { i = 0; }
		map_[bus] = GetRenderSettings().color_palette[i];
		++i;
	}

	return map_;
}

trans_ctl::Stop* RequestHandler::GetStop(const std::string_view& stop_name) const
{
	return db_.FindStop(stop_name);
}

const std::set<trans_ctl::Bus*, trans_ctl::BusCmp> RequestHandler::GetBusesByStop(const std::string_view& stop_name) const
{
	return db_.ExecuteStopRequest(db_.FindStop(stop_name));
}

const render::RenderSettings RequestHandler::GetRenderSettings() const
{
	return renderer_.GetSettings();
}

svg::Document RequestHandler::RenderMap()
{
	return renderer_.Render();
}

std::optional<graph::Router<double>::RouteInfo> RequestHandler::FindRoute(const std::string_view& from, const std::string_view& to) const
{
	return router_.BuildRoute(from, to);
}

graph::DirectedWeightedGraph<double> RequestHandler::GetGraph() const
{
	return router_.GetGraph();
}

transport_router::EdgeIdtoBus RequestHandler::GetEdgeInfo(size_t id) const
{
	return router_.GetEdgeInfo(id);
}

