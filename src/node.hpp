#pragma once

#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include "nodesoup/nodesoup.hpp"
#include "eBiome.hpp"
#include "biomeBag.hpp"

class node
{
    std::string sName;
    int32_t nID = 0;
    int32_t level = 0;
    eBiome biome = eBiome::ocean;
    std::vector<node> children;
    std::unordered_map<std::string, size_t> childPosition;

    // Static registries
    static std::unordered_map<int32_t, node*> idToNode;
    static std::unordered_map<int32_t, node*> positionToNode;
    static int32_t nextID;
	static BiomeBag biomeBag;

    void printRecursive(int indent, bool isLast) const;
    void collectNodesAtLevel(int32_t targetLevel, std::vector<node*>& result);
    void buildAdjacency(nodesoup::adj_list_t& graph, std::map<int32_t, size_t>& idToIndex);


public:
    explicit node(const std::string& name, int32_t level);
    node& setID(int32_t id);
    int32_t getID() const;
    int32_t getLvl() const;
    std::vector<node*> getNodesAtLevel(int32_t targetLevel);
    node* getItemAtRelativeLevel(int32_t offset);
    node* findLeafAtLevel(int32_t targetLevel);
    node* findLeafAtLevelRelative(int32_t targetLevel);
    const std::vector<node>& getChildren() const;
    std::string& getName();
    bool hasChildren() const;
    node& operator[](const std::string& name);
    node& setBiome(eBiome b);
    node& assignRandomBiome();
    eBiome getBiome() const;
    uint32_t getBiomeColor() const;

    nodesoup::adj_list_t buildGraph();
    node& bulkPopulate(int32_t lvl, int count);
    void bulkPopulateRelative(int32_t offset, int count);

    // Registry access
    static node* getNodeByID(int32_t id);
    static node* getNodeByPosition(int32_t pos);
    static const std::unordered_map<int32_t, node*>& getIDRegistry();
    static const std::unordered_map<int32_t, node*>& getPositionRegistry();
    
    void print() const;
};