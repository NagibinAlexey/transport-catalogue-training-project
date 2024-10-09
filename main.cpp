#include "json_reader.h"
#include "serialization.h"

#include <fstream>
#include <iostream>
#include <string_view>

using namespace std::literals;

json::Document LoadJSON(std::istream& input) {
    return json::Load(input);
}

void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        PrintUsage();
        return 1;
    }

    const std::string_view mode(argv[1]);

    if (mode == "make_base"sv) {

        trans_ctl::TransportCatalogue catalogue;
        auto doc = LoadJSON(std::cin);
        JsonReader json_reader;
        json_reader.ReadJSON(catalogue, doc);

        std::string filename = json_reader.ParseSerializationSettings(doc);
        std::ofstream ostrm(filename, std::ios::binary);
        serialize::Serializer serializer(catalogue);
        serializer.SerializeTransportCatalogue();
        serializer.SerializeRenderSettings(json_reader, doc);
        serializer.SerializeRouterSettings(json_reader, doc);

        transport_router::TRouter router(json_reader.ParseRoutingSettings(doc), catalogue);
        router.Build();
        serializer.SerializeGraph(router.GetGraph());
        serializer.SerializeRouter(router);

        serializer.SaveTo(ostrm);

    }
    else if (mode == "process_requests"sv) {

        auto doc = LoadJSON(std::cin);
        JsonReader json_reader;
        std::string filename = json_reader.ParseSerializationSettings(doc);
        std::ifstream istrm(filename, std::ios::binary);
        serialize::Deserializer deserializer;
        trans_ctl::TransportCatalogue catalogue = deserializer.DeserializeTransportCatalogue(istrm);

        renderer::MapRenderer map_renderer(deserializer.DeserializeRenderSettings());

        graph::DirectedWeightedGraph graph = deserializer.DeserializeGraph();
        transport_router::TRouter router(deserializer.DeserializeRouter(catalogue));
        router.ConnectGraph(graph);

        RequestHandler request_handler(catalogue, map_renderer, router);

        map_renderer.SetBusesColors(request_handler.GetBusesColors());
        
        json_reader.PrintStats(doc, request_handler, std::cout);
    }
    else {
        PrintUsage();
        return 1;
    }
}