#pragma once

#include "domain.h"
#include "transport_catalogue.h"
#include "router.h"
#include <unordered_map>
#include <vector>
#include <iterator>
#include <algorithm>
#include <memory>

namespace transport_router {

struct EdgeIdtoBus {
	std::string_view bus_name;
	std::string_view stop_name;
	int span_count = 0;
};

class TRouter {
public:
	TRouter(Routing_settings routing_settings, trans_ctl::TransportCatalogue& catalogue) :
		routing_settings_(routing_settings), catalogue_(catalogue), 
		waiting_stops_ids({}), stops_ids({}), id_to_bus_stop({}) {}

	TRouter(Routing_settings routing_settings, trans_ctl::TransportCatalogue& catalogue,
		std::unordered_map<std::string_view, size_t> waiting_stops_ids, std::unordered_map<std::string_view, size_t> stops_ids, 
		std::unordered_map<size_t, EdgeIdtoBus> id_to_bus_stop) :
		routing_settings_(routing_settings), catalogue_(catalogue), 
		waiting_stops_ids(waiting_stops_ids), stops_ids(stops_ids), id_to_bus_stop(id_to_bus_stop) {}

	void Build();
	void ConnectGraph(graph::DirectedWeightedGraph<double>& graph);
	std::optional<graph::Router<double>::RouteInfo> BuildRoute(const std::string_view& from, const std::string_view& to) const;
	graph::DirectedWeightedGraph<double> GetGraph() const { return graph_; }
	EdgeIdtoBus GetEdgeInfo(size_t id) const { return id_to_bus_stop.at(id); }
	std::unordered_map<std::string_view, size_t> GetWaitingStopsIds() const { return waiting_stops_ids; }
	std::unordered_map<std::string_view, size_t> GetStopsIds() const { return stops_ids; }
	std::unordered_map<size_t, EdgeIdtoBus> GetEdgeIdMap() const { return id_to_bus_stop; }

private:
	Routing_settings routing_settings_;
	trans_ctl::TransportCatalogue& catalogue_;
	graph::DirectedWeightedGraph<double> graph_;
	std::unordered_map<std::string_view, size_t> waiting_stops_ids;
	std::unordered_map<std::string_view, size_t> stops_ids;
	std::unordered_map<size_t, EdgeIdtoBus> id_to_bus_stop;
	std::unique_ptr<graph::Router<double>> router_ptr_ = nullptr;

	void AddStopsToGraph(graph::DirectedWeightedGraph<double>& graph);
	void AddRoutesToGraph(graph::DirectedWeightedGraph<double>& graph);
	void AddCircleRoute(std::string_view bus_name, std::vector<trans_ctl::Stop*> stops, graph::DirectedWeightedGraph<double>& graph);
};

} // namespace transport_router

