#include "transport_router.h"

namespace transport_router {

	void TRouter::Build() {

		graph::DirectedWeightedGraph<double> graph(catalogue_.GetStopsCount() * 2);

		AddStopsToGraph(graph);
		AddRoutesToGraph(graph);

		graph_ = std::move(graph);
		router_ptr_ = std::make_unique<graph::Router<double>>(graph_);
	}

	void TRouter::ConnectGraph(graph::DirectedWeightedGraph<double>& graph) {

		graph_ = std::move(graph);
		router_ptr_ = std::make_unique<graph::Router<double>>(graph_);
	}

	void TRouter::AddStopsToGraph(graph::DirectedWeightedGraph<double>& graph)
	{
		size_t id = 0;
		auto stops = catalogue_.GetAllStops();
		for (const auto& stop : stops) {
			//create stop & waiting stop
			stops_ids[stop.first] = id;
			waiting_stops_ids[stop.first] = id + 1;
			auto edge_id = graph.AddEdge({ id + 1, id, static_cast<double>(routing_settings_.bus_wait_time) });
			id_to_bus_stop[edge_id] = { "waiting", stop.first, 1 };
			id += 2;
		}
	}

	void TRouter::AddRoutesToGraph(graph::DirectedWeightedGraph<double>& graph)
	{
		auto routes = catalogue_.GetAllRoutes();

		for (const auto& [bus_name, bus_ptr] : routes) {
			AddCircleRoute(bus_name, bus_ptr->stops, graph);
			if (!bus_ptr->isCircleRoute) {
				std::vector<trans_ctl::Stop*> reverse_stops = { bus_ptr->stops.rbegin(), bus_ptr->stops.rend() };
				AddCircleRoute(bus_name, reverse_stops, graph);
			}
		}
	}

	void TRouter::AddCircleRoute(std::string_view bus_name, std::vector<trans_ctl::Stop*> stops, graph::DirectedWeightedGraph<double>& graph) {
		std::vector<double> times(stops.size() - 1);

		for (int i = 0; i < stops.size() - 1; ++i) {
			times[i] = 0.06 * catalogue_.GetStopsLength(stops[i], stops[i + 1]) / routing_settings_.bus_velocity;
		}

		for (int i = 1, j = 0; j < stops.size() - 1;) {
			auto first_stop_id = stops_ids.at(stops[j]->name);
			auto second_stop_id = waiting_stops_ids.at(stops[i]->name);
			double route_time = 0.0;
			for (int it = j; it < i; ++it) {
				route_time += times[it];
			}
			auto edge_id = graph.AddEdge({ first_stop_id, second_stop_id, route_time });
			id_to_bus_stop[edge_id] = { bus_name, std::string_view(stops[j]->name), i - j };
			++i;
			if (i == stops.size()) {
				++j;
				i = j + 1;
			}
		}
	}

	std::optional<graph::Router<double>::RouteInfo> TRouter::BuildRoute(const std::string_view& from, const std::string_view& to) const
	{
		auto from_id = waiting_stops_ids.at(from);
		auto to_id = waiting_stops_ids.at(to);

		return router_ptr_->BuildRoute(from_id, to_id);
	}
}