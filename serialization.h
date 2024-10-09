#pragma once

#include "domain.h"
#include "transport_catalogue.h"
#include "json_reader.h"

#include <transport_catalogue.pb.h>
#include <unordered_map>
#include <string>
#include <variant>

namespace serialize {

	class Serializer {
	public:
		explicit Serializer (const trans_ctl::TransportCatalogue& catalogue) : catalogue_(catalogue) {}
		void SerializeTransportCatalogue();
		void SerializeRenderSettings(const JsonReader& json_reader, const json::Document& document);
		void SerializeRouterSettings(const JsonReader& json_reader, const json::Document& document);
		void SerializeGraph(const graph::DirectedWeightedGraph<double>& graph);
		void SerializeRouter(const transport_router::TRouter& router);
		void SaveTo(std::ostream& output);

	private:
		trans_catalogue_serialize::TransportCatalogue serialized_catalogue_;
		const trans_ctl::TransportCatalogue& catalogue_;
		std::unordered_map<std::string, uint32_t> stop_to_id;

		void SerializeStops();
		void SerializeStopDistances();
		void SerializeBusses();
	};

	class Deserializer {
	public:
		trans_ctl::TransportCatalogue DeserializeTransportCatalogue(std::istream& input);
		render::RenderSettings DeserializeRenderSettings();
		transport_router::Routing_settings DeserializeRouterSettings();
		graph::DirectedWeightedGraph<double> DeserializeGraph();
		transport_router::TRouter DeserializeRouter(trans_ctl::TransportCatalogue& catalogue);

	private:
		trans_catalogue_serialize::TransportCatalogue serialized_catalogue_;
		trans_ctl::TransportCatalogue catalogue_;
		std::unordered_map<uint32_t, std::string> id_to_stop;

		void DeserializeStops(std::istream& input);
		void DeserializeStopDistances();
		void DeserializeBusses();
	};

	svg_serialize::Color SerializeColor(const svg::Color& color);
	svg::Color DeserializeColor(const svg_serialize::Color& color);
}

