#include <iostream>
#include <filesystem>
#include "node.hpp"
#include "mapRenderer.hpp"

int main() {
    node root("A", 0);
	root.assignRandomBiome();
    root["C"].assignRandomBiome().bulkPopulateRelative(0, 4);
    root["D"].assignRandomBiome().bulkPopulateRelative(0, 4);
    root["E"].assignRandomBiome().bulkPopulateRelative(0, 4);

    root["B"].assignRandomBiome()["F"].assignRandomBiome()["F.1"].assignRandomBiome().bulkPopulateRelative(0, 4);
    root["B"]["F"]["F.1"]["F.1.2"].assignRandomBiome().bulkPopulateRelative(0, 4);
	root["B"]["F"]["F.2"].assignRandomBiome()["F.2.2"].assignRandomBiome().bulkPopulateRelative(0, 4);
    root["B"]["F"]["F.2"]["F.2.3"].assignRandomBiome().bulkPopulateRelative(0, 4);

    root["B"]["G"].assignRandomBiome()["G.1"]["G.1.2"].assignRandomBiome().bulkPopulateRelative(0, 4);
    root["B"]["G"]["G.1"].assignRandomBiome()["G.1.1"].assignRandomBiome().bulkPopulateRelative(0, 4);
    root["B"]["G"]["G.2"].assignRandomBiome().bulkPopulateRelative(0, 4);

    root["C"]["C1"].assignRandomBiome().bulkPopulateRelative(0, 4);
    root["D"]["D1"].assignRandomBiome().bulkPopulateRelative(0, 4);
    root["E"]["E1"].assignRandomBiome().bulkPopulateRelative(0, 4);
    auto graph = root.buildGraph();

    // The graph's shape is fixed above, so its Kamada-Kawai layout never changes
    // between runs - compute it once and reuse the cached file from then on.
    MapRenderer renderer;
    renderer.setRenderPoints(true).setRenderLines(true);
    const std::string layoutCachePath = "layout.cache";
    if (!std::filesystem::exists(layoutCachePath)) {
        renderer.generateLayout(graph, layoutCachePath);
    }
    renderer.buildMap(graph, layoutCachePath);
}