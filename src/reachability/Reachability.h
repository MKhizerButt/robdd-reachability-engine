// Reachability.h
#ifndef VDSPROJECT_REACHABILITY_H
#define VDSPROJECT_REACHABILITY_H

#include "ReachabilityInterface.h"
#include <vector>
#include <stdexcept>

namespace ClassProject {

    class Reachability : public ReachabilityInterface {
    private:
        // Variables exposed to the user
        std::vector<BDD_ID> stateVars;
        std::vector<BDD_ID> inputVars;

        // Internal "shadow" variables for next state (s') logic
        // These are never returned to the user.
        std::vector<BDD_ID> nextStateVars;

        // System definition
        std::vector<BDD_ID> transitionFunctions; // delta functions
        std::vector<bool> initialStates;

        // Caching the reachable space
        // layers[i] stores the set of states reachable in exactly i steps (and not sooner).
        // The full reachable set c_R is the union of all layers.
        std::vector<BDD_ID> layers;

        // Helper: Computes the Transition Relation (tau)
        // Formula: tau = AND( s'_i XNOR delta_i ) for all i
        BDD_ID computeTransitionRelation();

        // Helper: Performs existential quantification for a specific variable
        // Formula: exists x. f = f|x=1 + f|x=0
        BDD_ID quantification(BDD_ID f, BDD_ID var);

        // Helper: Performs existential quantification for a set of variables
        BDD_ID quantifySet(BDD_ID f, const std::vector<BDD_ID>& vars);

        // Main algorithm: Symbolic Traversal
        // Populates the 'layers' vector until a fixed point is reached.
        void computeReachableStates();

    public:
        // Constructor [cite: 21]
        explicit Reachability(unsigned int stateSize, unsigned int inputSize = 0);

        ~Reachability() override = default;

        const std::vector<BDD_ID> &getStates() const override;
        const std::vector<BDD_ID> &getInputs() const override;

        // [cite: 5]
        bool isReachable(const std::vector<bool> &stateVector) override;

        // [cite: 8]
        int stateDistance(const std::vector<bool> &stateVector) override;

        // [cite: 20]
        void setTransitionFunctions(const std::vector<BDD_ID> &transitionFunctions) override;

        // [cite: 25]
        void setInitState(const std::vector<bool> &stateVector) override;
    };
}

#endif //VDSPROJECT_REACHABILITY_H