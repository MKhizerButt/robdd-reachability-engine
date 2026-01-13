#include "Manager.h"
#include <algorithm>

// for visualisation:
#include <fstream>
#include <iostream>
#include <set>

namespace ClassProject {
    Manager::Manager() {
        nodes.push_back({FALSE_ID, FALSE_ID, FALSE_ID, FALSE_ID, "False"});
        nodes.push_back({TRUE_ID, TRUE_ID, TRUE_ID, TRUE_ID, "True"});
    }

    const BDD_ID &Manager::True() { return TRUE_ID; }
    const BDD_ID &Manager::False() { return FALSE_ID; }
    bool Manager::isConstant(BDD_ID f) { return (f == TRUE_ID || f == FALSE_ID); }
    bool Manager::isVariable(BDD_ID x) { return (nodes[x].high == TRUE_ID && nodes[x].low == FALSE_ID); }
    BDD_ID Manager::topVar(BDD_ID f) { return nodes[f].topVar; }
    size_t Manager::uniqueTableSize() { return nodes.size(); }

    BDD_ID Manager::createVar(const std::string &label) {
        BDD_ID new_id = nodes.size();
        nodes.push_back({new_id, TRUE_ID, FALSE_ID, new_id, label});

        // Look for the Key in this struct '{}', and store value = new_id using the keyHasher as defined in Manager.h
        uniqueTable[{TRUE_ID, FALSE_ID, new_id}] = new_id;

        return new_id;
    }


    std::string Manager::getTopVarName(const BDD_ID &root) {
        BDD_ID topVarId = nodes[root].topVar;
        return nodes[topVarId].label;
    }

    BDD_ID Manager::ite(BDD_ID i, BDD_ID t, BDD_ID e) {
        // For Terminal Cases
        if (i == TRUE_ID) return t;
        if (i == FALSE_ID) return e;
        if (t == TRUE_ID && e == FALSE_ID) return i;
        if (t == e) return t;

        // For Computed Table entry. (From Bryant's ite algo. Prevents recalculating when recursing)
        ComputedKey key = {i, t, e};
        if (computedTable.count(key)) {
            return computedTable[key];
        }

        // For Recursive Cases
        BDD_ID top = topVar(i);

        if (!isConstant(t) && topVar(t) < top) {
            top = topVar(t);
        }

        if (!isConstant(e) && topVar(e) < top) {
            top = topVar(e);
        }

        // Calculate Cofactors (Removed intermediate variables)
        // Recursion (highSuccessor & lowSuccessor)
        BDD_ID r_high = ite(coFactorTrue(i, top), coFactorTrue(t, top), coFactorTrue(e, top));
        BDD_ID r_low = ite(coFactorFalse(i, top), coFactorFalse(t, top), coFactorFalse(e, top));

        // Reduction
        if (r_high == r_low) return r_high;

        // Create a new node
        UniqueKey uniqueKey = {r_high, r_low, top};

        // Check if it already exists
        if (uniqueTable.count(uniqueKey)) {
            BDD_ID R = uniqueTable[uniqueKey];

            // New node not created, save working in the computedTable for use in future recursion
            computedTable[key] = R;
            return R;
        }

        // When id is new, pushback in nodes (storage), uniqueTable (Canonicity) and computedTable (for future use in recursions)
        BDD_ID new_id = nodes.size();
        nodes.push_back({new_id, r_high, r_low, top, ""});

        uniqueTable[uniqueKey] = new_id;
        computedTable[key] = new_id;

        return new_id;
    }

    BDD_ID Manager::coFactorTrue(BDD_ID f, BDD_ID x) {
        // Return Constant if f == constant, topVar(f) > x condition makes sure we look only deeper and not above (no dependency)
        if (isConstant(f) || isConstant(x) || topVar(f) > x) {
            return f;
        }

        // Return highSuccessor if topVar matches
        if (topVar(f) == x) {
            return coFactorTrue(f);
        }
        // Find the node wrt x. (If it doesn't exist create it using ITE)

        BDD_ID T = coFactorTrue(nodes[f].high, x);
        BDD_ID E = coFactorTrue(nodes[f].low, x);

        return ite(topVar(f), T, E);
    }

    BDD_ID Manager::coFactorFalse(BDD_ID f, BDD_ID x) {
        // Return Constant if f == constant, topVar(f) > x condition makes sure we look only deeper and not above (no dependency)
        if (isConstant(f) || isConstant(x) || topVar(f) > x) {
            return f;
        }

        // Return lowSuccessor if topVar matches
        if (topVar(f) == x) {
            return coFactorFalse(f);
        }
        // Find the node wrt x. (If it doesn't exist create it using ITE)

        BDD_ID T = coFactorFalse(nodes[f].high, x);
        BDD_ID E = coFactorFalse(nodes[f].low, x);

        return ite(topVar(f), T, E);
    }

    BDD_ID Manager::coFactorTrue(BDD_ID f) { return nodes[f].high; }
    BDD_ID Manager::coFactorFalse(BDD_ID f) { return nodes[f].low; }

    BDD_ID Manager::and2(BDD_ID a, BDD_ID b) { return ite(a, b, FALSE_ID); }
    BDD_ID Manager::or2(BDD_ID a, BDD_ID b) { return ite(a, TRUE_ID, b); }
    BDD_ID Manager::xor2(BDD_ID a, BDD_ID b) { return or2(and2(neg(a), b), and2(a, neg(b))); }
    BDD_ID Manager::neg(BDD_ID a) { return ite(a, FALSE_ID, TRUE_ID); }
    BDD_ID Manager::nand2(BDD_ID a, BDD_ID b) { return neg(and2(a, b)); }
    BDD_ID Manager::nor2(BDD_ID a, BDD_ID b) { return neg(or2(a, b)); }
    BDD_ID Manager::xnor2(BDD_ID a, BDD_ID b) { return neg(xor2(a, b)); }

    void Manager::findNodes(const BDD_ID &root, std::set<BDD_ID> &nodes_of_root) {
        // Check for UniqueIDs
        if (nodes_of_root.insert(root).second) {
            //.second is the 2nd output of .insert() which is bool (true=new, false=already exists)

            if (isConstant(root)) {
                return;
            }

            findNodes(nodes[root].high, nodes_of_root);
            findNodes(nodes[root].low, nodes_of_root);
        } // else return, implicitly
    }

    void Manager::findVars(const BDD_ID &root, std::set<BDD_ID> &vars_of_root) {
        // Finding all reachable nodes using findNodes
        std::set<BDD_ID> all_reachable_nodes;
        findNodes(root, all_reachable_nodes);

        // Extract the variables from those nodes
        for (BDD_ID node_id: all_reachable_nodes) {
            // Excluding Terminal/Leaf nodes
            if (!isConstant(node_id)) {
                BDD_ID var_id = topVar(node_id);

                vars_of_root.insert(var_id);
            }
        }
    }


    void Manager::visualizeBDD(std::string filepath, BDD_ID &root) {
        std::ofstream outputFile(filepath);
        if (!outputFile.is_open()) {
            std::cerr << "Error: Unable to open file " << filepath << std::endl;
            return;
        }


        outputFile << "digraph BDD {" << std::endl;
        outputFile << "    rankdir=TB;" << std::endl;
        outputFile << "    node [shape=circle];" << std::endl;

        // To keep track of nodes we have already processed
        std::set<BDD_ID> visitedNodes;

        // We should have the track of nodes we have processed before! So we call a recursive visualization feom the root.
        visualizeNode(root, outputFile, visitedNodes);

        outputFile << "}" << std::endl;
        outputFile.close();
    }

    void Manager::visualizeNode(BDD_ID id, std::ostream &outputFile, std::set<BDD_ID> &visitedNodes) {
        //          First, we need to check whether the node is already visited; if not, do not process again
        if (visitedNodes.find(id) != visitedNodes.end()) {
            return;
        }

        //          We visited
        visitedNodes.insert(id);

        const auto &node = nodes[id];

        if (isConstant(id)) {
            // Terminal nodes (True/False) are usually drawn as boxes
            outputFile << "    " << id << " [label=\"" << node.label << "\", shape=box];" << std::endl;
            return;
        } else {
            //showing their label (a, b ,... )
            std::string topVarName = nodes[node.topVar].label;
            // outputFile << "    " << id << " [label=\"" << node.label << "\"];" << std::endl;
            outputFile << "    " << id << " [label=\"" << topVarName << "\"];" << std::endl;
        }

        // For edges:
        // Low child (False) -> Dotted Line
        outputFile << "    " << id << " -> " << node.low << " [style=dotted, label=\"0\"];" << std::endl;

        // High child (True) -> Solid Line
        outputFile << "    " << id << " -> " << node.high << " [style=solid, label=\"1\"];" << std::endl;

        // Recursively visualising children
        visualizeNode(node.low, outputFile, visitedNodes);
        visualizeNode(node.high, outputFile, visitedNodes);
    }
}
