#include "Manager.h"
#include <algorithm>

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

        // We iterate through every node currently in the manager.
        for (const auto &node : nodes) {
            if (node.label == label) {
                // If we find a match in labels, the result will be the currently existing ID:
                return node.id;
            }
        }
        // Below happens whenever we didn't find a match in the currently existing nodes' labels.
        BDD_ID new_id = nodes.size();
        nodes.push_back({new_id, TRUE_ID, FALSE_ID, new_id, label});
        return new_id;
    }


    std::string Manager::getTopVarName(const BDD_ID &root) {
        BDD_ID topVarId = nodes[root].topVar;
        return nodes[topVarId].label;
    }

    BDD_ID Manager::ite(BDD_ID i, BDD_ID t, BDD_ID e) {

        // For Terminal Cases
        if (i == TRUE_ID)   return t;
        if (i == FALSE_ID)  return e;
        if (t == TRUE_ID && e == FALSE_ID)  return i;
        if (t == e) return t;

        // For Recursive Cases
        BDD_ID topI = isConstant(i) ? 99 : topVar(i);
        BDD_ID topT = isConstant(t) ? 99 : topVar(t);
        BDD_ID topE = isConstant(e) ? 99 : topVar(e);

        BDD_ID top = std::min({topI, topT, topE});

        // Calculate Cofactors
        BDD_ID i_high;
        BDD_ID i_low;
        if (!isConstant(i) && topVar(i) == top) {
            i_high = nodes[i].high;
            i_low = nodes[i].low;
        }

        BDD_ID t_high;
        BDD_ID t_low;
        if (!isConstant(t) && topVar(t) == top) {
            t_high = nodes[t].high;
            t_low = nodes[t].low;
        }

        BDD_ID e_high;
        BDD_ID e_low;
        if (!isConstant(e) && topVar(e) == top) {
            e_high = nodes[e].high;
            e_low = nodes[e].low;
        }

        // Recursion (highSuccessor & lowSuccessor)
        BDD_ID r_high = ite(i_high, t_high, e_high);
        BDD_ID r_low = ite(i_low, t_low, e_low);

        // Reduction
        if (r_high == r_low) return r_high;

        // Check if this node already exists, eliminate isomorphic sub-graphs
        for (size_t k = 0; k < nodes.size(); k++) {
            if (nodes[k].high == r_high && nodes[k].low == r_low && nodes[k].topVar == top) {
                return k;
            }
        }

        // Create new node if not found
        BDD_ID new_id = nodes.size();
        nodes.push_back({new_id, r_high, r_low, top, ""});
        return new_id;
    }

    BDD_ID Manager::coFactorTrue(BDD_ID f, BDD_ID x) {
        // Return Constant if f == constant
        if (isConstant(f)) {
            return f;
        }

        // Return highSuccessor if topVar matches
        if (topVar(f) == x) {
            return coFactorTrue(f);
        }
        // return the id of f if x != topVar
        else
            return f;
    }

    BDD_ID Manager::coFactorFalse(BDD_ID f, BDD_ID x) {
        // Return Constant if f == constant
        if (isConstant(f)) {
            return f;
        }

        // Return lowSuccessor if topVar matches
        if (topVar(f) == x) {
            return coFactorFalse(f);
        }
        // return the id of f if x != topVar
        else
            return f;
    }

    BDD_ID Manager::coFactorTrue(BDD_ID f) { return nodes[f].high; }
    BDD_ID Manager::coFactorFalse(BDD_ID f){ return nodes[f].low; }

    BDD_ID Manager::and2(BDD_ID a, BDD_ID b) { return ite(a, b, FALSE_ID); }
    BDD_ID Manager::or2(BDD_ID a, BDD_ID b)  { return neg(ite(neg(a), neg(b), FALSE_ID)); }
    BDD_ID Manager::xor2(BDD_ID a, BDD_ID b) { return or2(and2(neg(a),b),and2(a,neg(b))); }
    BDD_ID Manager::neg(BDD_ID a)            { return ite(a, FALSE_ID, TRUE_ID); }
    BDD_ID Manager::nand2(BDD_ID a, BDD_ID b){ return neg(and2(a, b)); }
    BDD_ID Manager::nor2(BDD_ID a, BDD_ID b) { return neg(or2(a, b)); }
    BDD_ID Manager::xnor2(BDD_ID a, BDD_ID b){ return neg(xor2(a, b)); }

    void Manager::findNodes(const BDD_ID &root, std::set<BDD_ID> &nodes_of_root) {

        // Check for UniqueIDs
        if (nodes_of_root.find(root) != nodes_of_root.end()) return;

        // Set: nodes are sorted in the ascending order by default
        nodes_of_root.insert(root);

        // Stops at the Terminal/Leaf nodes
        if (isConstant(root)) {
            return;
        }

        // Recursively check for reachable nodes by looking at successors
       findNodes(nodes[root].high, nodes_of_root);
       findNodes(nodes[root].low, nodes_of_root);

    }

    void Manager::findVars(const BDD_ID &root, std::set<BDD_ID> &vars_of_root) {

        // Finding all reachable nodes using findNodes
        std::set<BDD_ID> all_reachable_nodes;
        findNodes(root, all_reachable_nodes);

        // Extract the variables from those nodes
        for (BDD_ID node_id : all_reachable_nodes) {

            // Excluding Terminal/Leaf nodes
            if (!isConstant(node_id)) {

                BDD_ID var_id = topVar(node_id);

                vars_of_root.insert(var_id);
            }
        }
    }

    void Manager::visualizeBDD(std::string filepath, BDD_ID &root) {}
}
#include "Manager.h"