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
    int32_t position = 0;
    std::vector<node> children;
    std::unordered_map<std::string, size_t> childPosition;
    static int32_t nextID;

    void printRecursive(int indent, bool isLast) const;
    void collectNodesAtLevel(int32_t targetLevel, std::vector<node*>& result);
    void buildAdjacency(nodesoup::adj_list_t& graph, std::map<int32_t, size_t>& idToIndex);


public:
    node();
    explicit node(const std::string& name, int32_t level);
    node& setID(int32_t id);
    int32_t getID() const;
    int32_t getLvl() const;
    int32_t getPosition() const;
    std::vector<node*> getNodesAtLevel(int32_t targetLevel);
    node* getItemAtRelativeLevel(int32_t offset);
    node* findLeafAtLevel(int32_t targetLevel);
    node* findLeafAtLevelRelative(int32_t targetLevel);
    const std::vector<node>& getChildren() const;
    std::string& getName();
    bool isConnectedToOtherTree();
    node& connect(node& other);
    bool hasChildren() const;
    node& operator[](const std::string& name);
    
    nodesoup::adj_list_t buildGraph();
    void bulkPopulate(int32_t lvl, int count);
    void bulkPopulateRelative(int32_t offset, int count);
    
    void print() const;
};