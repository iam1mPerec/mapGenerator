#pragma once

#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include "nodesoup/nodesoup.hpp"

class node
{
    std::string sName;
    int32_t nID = 0;
    int32_t level = 0;
    std::vector<node> items;
    std::unordered_map<std::string, size_t> itemPointer;
    static int32_t nextID;

    void printRecursive(int indent, bool isLast) const;
    void collectNodesAtLevel(int32_t targetLevel, std::vector<const node*>& result) const;
    void buildAdjacency(nodesoup::adj_list_t& graph, std::map<int32_t, size_t>& idToIndex);


public:
    node();
    explicit node(const std::string& name, int32_t level);
    node& setID(int32_t id);
    int32_t getID() const;
    int32_t getLvl() const;
    std::vector<const node*> getNodesAtLevel(int32_t targetLevel) const;
    const std::vector<node>& getChildren() const;
    std::string& getName();
    bool hasChildren() const;
    node& operator[](const std::string& name);
    
    nodesoup::adj_list_t buildGraph();
    void bulkPopulate(int32_t lvl, int count);
    
    void print() const;
};