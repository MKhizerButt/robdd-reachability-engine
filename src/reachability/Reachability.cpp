// Reachability.cpp
#include "Reachability.h"

#include <iostream>

using namespace ClassProject;

Reachability::Reachability(unsigned int stateSize, unsigned int inputSize) : ReachabilityInterface(stateSize, inputSize) {
    if (stateSize == 0) {
        throw std::runtime_error("State size cannot be zero."); // [cite: 8]
    }

    // 1. Create variables for current state (s) and next state (s') [cite: 21]
    // We create them in pairs (s0, s'0, s1, s'1...) to optimize BDD ordering naturally.
    for (unsigned int i = 0; i < stateSize; i++) {
        stateVars.push_back(createVar("s" + std::to_string(i)));
        nextStateVars.push_back(createVar("s'" + std::to_string(i)));
    }

    // Create input variables
    for (unsigned int i = 0; i < inputSize; i++) {
        inputVars.push_back(createVar("i" + std::to_string(i)));
    }

    // Default Initialization:
    // Transition function is Identity (s' = s)
    // Initial state is all False (0,0,...)
    initialStates.resize(stateSize, false);
    transitionFunctions = stateVars; // Identity
}

const std::vector<BDD_ID> &Reachability::getStates() const {
    return stateVars;
}

const std::vector<BDD_ID> &Reachability::getInputs() const {
    return inputVars;
}

// [cite: 31, 33, 34]
// Helper to perform existential quantification: exists var. f
BDD_ID Reachability::quantification(BDD_ID f, BDD_ID var) {
    // exists x. f = coFactorTrue(f, x) OR coFactorFalse(f, x)
    BDD_ID cfTrue = coFactorTrue(f, var);
    BDD_ID cfFalse = coFactorFalse(f, var);
    return or2(cfTrue, cfFalse);
}

// Helper to quantify out a whole vector of variables (iteratively)
BDD_ID Reachability::quantifySet(BDD_ID f, const std::vector<BDD_ID>& vars) {
    BDD_ID result = f;
    for (BDD_ID var : vars) {
        result = quantification(result, var);
    }
    return result;
}

// [cite: 24]
// Compute the transition relation tau
// tau = AND_i ( (s'_i XNOR delta_i) )
BDD_ID Reachability::computeTransitionRelation()  {
    BDD_ID tau = True();

    for (size_t i = 0; i < stateVars.size(); ++i) {
        // s'_i == delta_i  <==>  NOT (s'_i XOR delta_i) <==> s'_i XNOR delta_i
        BDD_ID s_prime = nextStateVars[i];
        BDD_ID delta = transitionFunctions[i];

        BDD_ID equality = xnor2(s_prime, delta);

        tau = and2(tau, equality);
    }
    return tau;
}

// [cite: 16] The Symbolic Traversal Algorithm
void Reachability::computeReachableStates() {
    layers.clear();

    // 1. Compute characteristic function of initial state (c_S) [cite: 25, 28]
    // c_S = (s0 == init0) * (s1 == init1) ...
    BDD_ID c_R_it = True();
    for (size_t i = 0; i < stateVars.size(); ++i) {
        BDD_ID bit_rep;
        if (initialStates[i]) {
            bit_rep = stateVars[i]; // s_i
        } else {
            bit_rep = neg(stateVars[i]); // !s_i
        }
        c_R_it = and2(c_R_it, bit_rep);
    }

    // Layer 0 is the initial state
    layers.push_back(c_R_it);
    BDD_ID c_R = c_R_it; // Total reachable set so far

    // Compute Relation Tau once
    BDD_ID tau = computeTransitionRelation();

    // Loop until fixed point
    while (true) {
        // [cite: 31] Compute Image in terms of Next State Variables (s')
        // img(s') = exists_s exists_i (c_R_it * tau)
        BDD_ID temp1 = and2(c_R_it, tau);

        // Quantify out current states (s)
        BDD_ID temp2 = quantifySet(temp1, stateVars);

        // Quantify out inputs (i) [cite: 46]
        BDD_ID img_s_prime = quantifySet(temp2, inputVars);

        // [cite: 35] Convert img(s') to img(s) (Renaming)
        // Formula: img(s) = exists_s' ( (s == s') * img(s') )
        BDD_ID mapping = True();
        for(size_t i=0; i<stateVars.size(); i++) {
            BDD_ID pair_equality = xnor2(stateVars[i], nextStateVars[i]);
            mapping = and2(mapping, pair_equality);
        }

        BDD_ID map_and_img = and2(mapping, img_s_prime);
        BDD_ID img_s = quantifySet(map_and_img, nextStateVars);

        // Calculate ONLY the NEW states found in this iteration
        // new_states = img_s AND NOT(c_R)
        // (We do this to properly handle stateDistance layers)
        BDD_ID new_states = and2(img_s, neg(c_R));

        // Fixed point check: If no new states are found, we are done
        if (new_states == False()) {
            break;
        }

        // Add new layer
        layers.push_back(new_states);

        // Update total reachable set [cite: 40]
        c_R = or2(c_R, new_states);

        // Update iterator for next step (we only expand from the frontier)
        c_R_it = new_states;
    }
}

bool Reachability::isReachable(const std::vector<bool> &stateVector) {
    if (stateVector.size() != stateVars.size()) {
        throw std::runtime_error("State vector size mismatch.");
    }

    // Lazy computation: if layers are empty, we haven't computed reachability yet
    if (layers.empty()) {
        computeReachableStates();
    }

    // 1. Build BDD for the query state
    BDD_ID stateBDD = True();
    for (size_t i = 0; i < stateVars.size(); ++i) {
        BDD_ID bit = stateVector[i] ? stateVars[i] : neg(stateVars[i]);
        stateBDD = and2(stateBDD, bit);
    }

    // 2. Check overlap with Total Reachable Set
    // Since 'layers' contains disjoint sets of newly reached states,
    // we can iterate checks or compute the union.
    // Efficient way: Check if (State AND (Union of all layers)) != False

    // Construct full c_R from layers if needed, or iterate
    for(const auto& layer : layers) {
        if (and2(stateBDD, layer) != False()) {
            return true;
        }
    }
    return false;
}

int Reachability::stateDistance(const std::vector<bool> &stateVector) {
    if (stateVector.size() != stateVars.size()) {
        throw std::runtime_error("State vector size mismatch.");
    }

    if (layers.empty()) {
        computeReachableStates();
    }

    // Build BDD for the query state
    BDD_ID stateBDD = True();
    for (size_t i = 0; i < stateVars.size(); ++i) {
        BDD_ID bit = stateVector[i] ? stateVars[i] : neg(stateVars[i]);
        stateBDD = and2(stateBDD, bit);
    }

    // Iterate through layers to find the index (distance)
    for (int i = 0; i < layers.size(); ++i) {
        if (and2(stateBDD, layers[i]) != False()) {
            return i; // Distance is the layer index
        }
    }

    return -1; // Unreachable
}

void Reachability::setTransitionFunctions(const std::vector<BDD_ID> &transitionFunctions) {
    if (transitionFunctions.size() != stateVars.size()) {
        throw std::runtime_error("Transition function size mismatch.");
    }

    // Check if IDs are valid (simple check if they exist in manager)
    for(BDD_ID id : transitionFunctions) {
        if(id >= uniqueTableSize()) { // Assuming uniqueTableSize() available from Manager
             throw std::runtime_error("Unknown BDD ID in transition functions.");
        }
    }

    this->transitionFunctions = transitionFunctions;

    // Invalidate cache so next query re-computes
    layers.clear();
}

void Reachability::setInitState(const std::vector<bool> &stateVector) {
    if (stateVector.size() != stateVars.size()) {
        throw std::runtime_error("Initial state vector size mismatch.");
    }

    this->initialStates = stateVector;

    // Invalidate cache so next query re-computes
    layers.clear();
}