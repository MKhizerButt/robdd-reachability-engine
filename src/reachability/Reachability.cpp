#include "Reachability.h"
#include <iostream>

namespace ClassProject {
    Reachability::Reachability(unsigned int stateSize, unsigned int inputSize) : ReachabilityInterface(
        stateSize, inputSize) {
        if (stateSize == 0) {
            throw std::runtime_error("State size cannot be zero");
        }

        // Create variables for currentState(s), nextState(s') and input(x)
        for (unsigned int i = 0; i < stateSize; i++) {
            currentStateVars.push_back(createVar("s" + std::to_string(i)));
            nextStateVars.push_back(createVar("s" + std::to_string(i) + "'"));
        }

        for (unsigned int i = 0; i < inputSize; i++) {
            inputVars.push_back(createVar("x" + std::to_string(i)));
        }

        initialState = True(); // Characteristic function
        for (BDD_ID s: currentStateVars) {
            initialState = and2(initialState, neg(s));
            //default initial state, all bits are assumed to be set to false, function = 1
        }

        transitionRelation = True();

        //  Performing Conjunction (AND) of XNORs; nxt state var should be same as output of current state
        for (BDD_ID i = 0; i < stateSize; i++) {
            BDD_ID equivalence = xnor2(currentStateVars[i], nextStateVars[i]);
            transitionRelation = and2(equivalence, transitionRelation);
        }
    }

    const std::vector<BDD_ID> &Reachability::getStates() const {
        return currentStateVars;
    }

    const std::vector<BDD_ID> &Reachability::getInputs() const {
        return inputVars;
    }

    void Reachability::setInitState(const std::vector<bool> &stateVector) {
        if (stateVector.size() != currentStateVars.size()) {
            throw std::runtime_error("Size mismatch");
        }

        // Initial state Characteristic function
        initialState = True();
        for (BDD_ID i = 0; i < currentStateVars.size(); i++) {
            if (stateVector[i]) {
                initialState = and2(initialState, currentStateVars[i]);
            } else {
                initialState = and2(initialState, neg(currentStateVars[i]));
            }
        }
    }

    void Reachability::setTransitionFunctions(const std::vector<BDD_ID> &transitionFunctions) {
        if (transitionFunctions.size() != currentStateVars.size()) {
            throw std::runtime_error("Size mismatch");
        }

        transitionRelation = True();
        //  Performing Conjunction (AND) of XNORs; nxt state var should be same as output of current state
        for (BDD_ID i = 0; i < currentStateVars.size(); i++) {
            BDD_ID equivalence = xnor2(transitionFunctions[i], nextStateVars[i]);
            transitionRelation = and2(equivalence, transitionRelation);
        }
    }

    int Reachability::stateDistance(const std::vector<bool> &stateVector) {
        BDD_ID target = True();
        for (size_t i = 0; i < currentStateVars.size(); i++) {
            if (stateVector[i]) {
                target = and2(target, currentStateVars[i]);
            } else {
                target = and2(target, neg(currentStateVars[i]));
            }
        }

        // 'CR': Current Reachable states. Starts with just Initial State.
        BDD_ID CR = initialState;

        // 'visited': All states we have ever seen.
        BDD_ID visited = CR;

        int distance = 0;

        // Loop until CR holds. If it is False, no new states
        while (CR != False()) {
            if (and2(target, CR) != False()) {
                // Does target exists in CR
                // fixed point reached. CR is now the symbolic representation of the set of reachable states
                return distance;
            }

            // Image Computation

            // Conjunction of CR and Tau (s, x, s')
            BDD_ID temp = and2(CR, transitionRelation);

            // Quantify out Current State (s) and Inputs (x). CoFactor (from Manager) is equivalent
            // Quantify States (s0, s1, ...)
            for (auto &var: currentStateVars) {
                temp = or2(coFactorTrue(temp, var), coFactorFalse(temp, var));
            }

            // Quantify Inputs (x, ...)
            for (auto &var: inputVars) {
                temp = or2(coFactorTrue(temp, var), coFactorFalse(temp, var));
            }
            // temp: img(s'). Consists of only next states, described using s'

            // For the next iteration s' needs to be replaced with s by equality mapping
            BDD_ID mapping = True();
            for (size_t i = 0; i < currentStateVars.size(); i++) {
                mapping = and2(mapping, xnor2(currentStateVars[i], nextStateVars[i]));
            }

            BDD_ID temp2 = and2(temp, mapping);

            // Quantify next state vars (s')
            for (auto &var: nextStateVars) {
                temp2 = or2(coFactorTrue(temp2, var), coFactorFalse(temp2, var));
            }

            // temp2: img(s). All sets reachable in next step

            // Save only new states. If already visited no new states, else img(s)
            BDD_ID next_CR = ite(visited, False(), temp2);

            CR = next_CR;
            visited = or2(visited, next_CR); // Add new states to visited list
            distance++;
        }

        return -1; // Target not reachable
    }

    bool Reachability::isReachable(const std::vector<bool> &stateVector) {
        // If distance is not -1, given state is in the reachable state set
        return stateDistance(stateVector) != -1;
    }
}
