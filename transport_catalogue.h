#pragma once

#include "geo.h"
#include "domain.h"
#include <string>
#include <string_view>
#include <deque>
#include <vector>
#include <set>
#include <unordered_map>
#include <algorithm>
#include <utility>

namespace trans_ctl {

	class TransportCatalogue {
	public:
		void AddStop(const Stop& stop);
		void SetStopsLength(Stop* first_stop, Stop* second_stop, double distance);
		Stop* FindStop(const std::string_view stop_name) const;
		void AddBus(const Bus& bus);
		Bus* FindBus(const std::string_view bus_name) const;
		double GetStopsLength(Stop* first_stop, Stop* second_stop) const;
		size_t GetStopsCount() const;
		std::unordered_map<std::string_view, Bus*> GetAllRoutes() const;
		std::unordered_map<std::string_view, Stop*> GetAllStops() const;
		std::unordered_map<std::pair<Stop*, Stop*>, double, StopsHasher> GetAllStopLengths() const;
		bool BusIsEmpty(Bus* bus) const;

		BusStat* ExecuteBusRequest(Bus* bus);
		std::set<Bus*, BusCmp> ExecuteStopRequest(Stop* stop) const;

		BusStat* CalcBusStat(Bus* route);

	private:
		std::deque<Stop> stops_;
		std::unordered_map<std::string_view, Stop*> stops_map_;
		std::unordered_map<Stop*, std::set<Bus*, BusCmp>> stops_info_;

		std::deque<Bus> routes_;
		std::unordered_map<std::string_view, Bus*> routes_info_;

		std::unordered_map<std::pair<Stop*, Stop*>, double, StopsHasher> segments_map_;
	};

	namespace calc {
		void RouteLength(TransportCatalogue& catalogue, Bus* route);
		void RouteDistance(Bus* route);
		void RouteStops(Bus* route);
		void RouteUniqueStops(Bus* route);
	}
}
