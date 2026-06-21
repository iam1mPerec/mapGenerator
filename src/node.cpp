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

int32_t node::getPosition() const
{
    return position;
}

node& node::connect(node& other)
{
    childPosition[other.getName()] = children.size();
    children.push_back(other);

    return *this;
}

void node::collectNodesAtLevel(int32_t targetLevel,
    std::vector<node*>& result)
{
    int32_t currentLevel = getLvl();

    if (currentLevel > targetLevel)
        return;

    if (currentLevel == targetLevel)
    {
        result.push_back(this);
        return;
    }

    for (auto& child : children)
    {
        child.collectNodesAtLevel(targetLevel, result);
    }
}

void node::buildAdjacency(nodesoup::adj_list_t& graph, std::map<int32_t, size_t>& idToIndex)
{
    size_t currentId = getID();

    // Assign index if not already assigned
    if (idToIndex.find(currentId) == idToIndex.end())
        idToIndex[currentId] = idToIndex.size();

    size_t currentIndex = idToIndex[currentId];
	position = currentIndex;

    for (auto& child : children)
    {
        int32_t childId = child.getID();

        // Assign index if not already assigned
        if (idToIndex.find(childId) == idToIndex.end())
            idToIndex[childId] = idToIndex.size(); 

        size_t childIndex = idToIndex[childId];

        if (graph.size() <= std::max(currentIndex, childIndex))
            graph.resize(std::max(currentIndex, childIndex) + 1);

        graph[currentIndex].push_back(childIndex);
        graph[childIndex].push_back(currentIndex);

        child.buildAdjacency(graph, idToIndex);
    }
}

nodesoup::adj_list_t node::buildGraph()
{
    nodesoup::adj_list_t graph;
    std::map<int32_t, size_t> idToIndex;
    buildAdjacency(graph, idToIndex);
    return graph;
}

void node::bulkPopulate(int32_t lvl, int count)
{
    // Get all nodes at the target level
    std::vector<node*> nodesAtLevel = getNodesAtLevel(lvl);

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

void node::bulkPopulateRelative(int32_t offset, int count)
{
	std::cout << "offset: " << offset + getLvl() << "\n";
    bulkPopulate(getLvl() + offset, count);
}

std::vector<node*> node::getNodesAtLevel(int32_t targetLevel)
{
    std::vector<node*> result;

    collectNodesAtLevel(targetLevel, result);

    return result;
}

node* node::findLeafAtLevel(int32_t targetLevel)
{
    if (getLvl() > targetLevel)
        return nullptr;

    if (getLvl() == targetLevel)
        return hasChildren() ? nullptr : this;

    for (auto& child : children)
    {
        if (node* found = child.findLeafAtLevel(targetLevel))
            return found;
    }

    return nullptr;
}

node* node::findLeafAtLevelRelative(int32_t offset)
{
    return findLeafAtLevel(getLvl() + offset);
}

node* node::getItemAtRelativeLevel(int32_t offset)
{
    auto nodes = getNodesAtLevel(getLvl() + offset);

    if (nodes.empty())
        return nullptr;

    return nodes.front();
}

const std::vector<node>& node::getChildren() const
{
    return children;
}

std::string& node::getName()
{
    return sName;
}

bool node::isConnectedToOtherTree()
{
    if (children.empty())
        return false;

    // Get the first letter of the first child's name
    char firstLetterPrefix = getName()[0];

    // Check if any child has a different first letter
    for (auto& child : children)
    {
        if (child.getName()[0] != firstLetterPrefix)
            return true;  // Found child from different tree
    }

    return false;
}

bool node::hasChildren() const
{
    return !children.empty();
}

node& node::operator[](const std::string& name)
{
    if (childPosition.count(name) == 0)
    {
        childPosition[name] = children.size();
        children.emplace_back(name, level + 1);
    }
    return children[childPosition[name]];
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
    for (size_t i = 0; i < children.size(); ++i) {
        bool lastChild = (i == children.size() - 1);
        children[i].printRecursive(indent + 1, lastChild);
    }
}

void node::print() const
{
    printRecursive(0, true);
}