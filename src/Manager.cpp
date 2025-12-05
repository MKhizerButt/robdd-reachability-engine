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
        // Initialise with defaults
        BDD_ID i_high = i, i_low = i;
        BDD_ID t_high = t, t_low = t;
        BDD_ID e_high = e, e_low = e;

        // Override if there is dependency
        if (!isConstant(i) && topVar(i) == top) {
            i_high = nodes[i].high;
            i_low = nodes[i].low;
        }

        if (!isConstant(t) && topVar(t) == top) {
            t_high = nodes[t].high;
            t_low = nodes[t].low;
        }

        if (!isConstant(e) && topVar(e) == top) {
            e_high = nodes[e].high;
            e_low = nodes[e].low;
        }

        // Recursion
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
        return 0;
    }

    BDD_ID Manager::coFactorFalse(BDD_ID f, BDD_ID x) {
        return 0;
    }

    BDD_ID Manager::coFactorTrue(BDD_ID f) { return 0; }
    BDD_ID Manager::coFactorFalse(BDD_ID f){ return 0; }

    BDD_ID Manager::and2(BDD_ID a, BDD_ID b) { return 0; }
    BDD_ID Manager::or2(BDD_ID a, BDD_ID b)  { return 0; }
    BDD_ID Manager::xor2(BDD_ID a, BDD_ID b) { return 0; }
    BDD_ID Manager::neg(BDD_ID a)            { return 0; }
    BDD_ID Manager::nand2(BDD_ID a, BDD_ID b){ return 0; }
    BDD_ID Manager::nor2(BDD_ID a, BDD_ID b) { return 0; }
    BDD_ID Manager::xnor2(BDD_ID a, BDD_ID b){ return 0; }

    void Manager::findNodes(const BDD_ID &root, std::set<BDD_ID> &nodes_of_root) {

    }

    void Manager::findVars(const BDD_ID &root, std::set<BDD_ID> &vars_of_root) {

    }

    void Manager::visualizeBDD(std::string filepath, BDD_ID &root) {}
}
#include "Manager.h"
