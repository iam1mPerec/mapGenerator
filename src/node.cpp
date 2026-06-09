#include <iostream>
#include "node.hpp"

int32_t node::nextID = 0;

node::node()
    : sName("root")
{
    nID = nextID++;
}

node::node(const std::string& name, int32_t level)
    : sName(name),
    level(level)
{
    nID = nextID++;
}

node& node::setID(int32_t id)
{
    nID = id;
    return *this;
}

int32_t node::getID() const
{
    return nID;
}

int32_t node::getLvl() const
{
    return level;
}

void node::collectNodesAtLevel(int32_t targetLevel,
    std::vector<const node*>& result) const
{
    int32_t currentLevel = getLvl();

    if (currentLevel > targetLevel)
        return;

    if (currentLevel == targetLevel)
    {
        result.push_back(this);
        return;
    }

    for (const auto& child : items)
    {
        child.collectNodesAtLevel(targetLevel, result);
    }
}

void node::buildAdjacency(nodesoup::adj_list_t& graph)
{
    size_t currentId = getID();

    if (graph.size() <= currentId)
        graph.resize(currentId + 1);

    for (auto& child : items)
    {
        size_t childId = child.getID();

        if (graph.size() <= childId)
            graph.resize(childId + 1);

        graph[currentId].push_back(childId);
        graph[childId].push_back(currentId);

        child.buildAdjacency(graph);
    }
}

nodesoup::adj_list_t node::buildGraph()
{
    nodesoup::adj_list_t graph;
    buildAdjacency(graph);
    return graph;
}

void node::bulkPopulate(int32_t lvl, int count)
{
    // Get all nodes at the target level
    std::vector<const node*> nodesAtLevel = getNodesAtLevel(lvl);

    if (nodesAtLevel.empty())
        return;

    // Distribute count nodes across all nodes at lvl
    int baseCount = count / nodesAtLevel.size();
    int remainder = count % nodesAtLevel.size();

    for (size_t i = 0; i < nodesAtLevel.size(); ++i)
    {
        // Cast away const to access non-const getChildren
        node* mutableNode = const_cast<node*>(nodesAtLevel[i]);

        int nodesToAdd = baseCount + (i < remainder ? 1 : 0);

        for (int j = 0; j < nodesToAdd; ++j)
        {
            // Create child name as parentName.childIndex
            std::string childName = mutableNode->getName() + "." + std::to_string(mutableNode->getChildren().size() + 1);
            (*mutableNode)[childName];
        }
    }
}

std::vector<const node*> node::getNodesAtLevel(int32_t targetLevel) const
{
    std::vector<const node*> result;

    collectNodesAtLevel(targetLevel, result);

    return result;
}

const std::vector<node>& node::getChildren() const
{
    return items;
}

std::string& node::getName()
{
    return sName;
}

bool node::hasChildren() const
{
    return !items.empty();
}

node& node::operator[](const std::string& name)
{
    if (itemPointer.count(name) == 0)
    {
        itemPointer[name] = items.size();
        items.emplace_back(name, level + 1);
    }
    return items[itemPointer[name]];
}

void node::printRecursive(int indent, bool isLast) const
{
    // Print the current node with proper tree lines
    for (int i = 0; i < indent; ++i) {
        if (i == indent - 1)
            std::cout << (isLast ? "└── " : "├── ");
        else
            std::cout << "│   ";
    }

    if (indent == 0)
        std::cout << "● ";           // Root symbol
    else
        std::cout << "";

    std::cout << sName;

    if (nID != 0)
        std::cout << " [ID:" << nID << "]" << " [lvl:" << level << "]";

    std::cout << '\n';

    // Print children
    for (size_t i = 0; i < items.size(); ++i) {
        bool lastChild = (i == items.size() - 1);
        items[i].printRecursive(indent + 1, lastChild);
    }
}

void node::print() const
{
    printRecursive(0, true);
}