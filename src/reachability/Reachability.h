#ifndef VDSPROJECT_REACHABILITY_H
#define VDSPROJECT_REACHABILITY_H

#include <vector>

#include "ReachabilityInterface.h"
#include "../ManagerInterface.h"

namespace ClassProject {

    class Reachability : public ReachabilityInterface {
    private:
        std::vector<BDD_ID> currentStateVars;   // Current state bits (s0, s1, ...)
        std::vector<BDD_ID> nextStateVars;      // Next state bits (s0', s1', ...)
        std::vector<BDD_ID> inputVars;          // Input bits (x, ...)

        BDD_ID transitionRelation;  // Tau (s, x, s')
        BDD_ID initialState;        // Characteristic Function of initial state

    public:

        explicit Reachability(unsigned int stateSize, unsigned int inputSize = 0);  // Constructor

        const std::vector<BDD_ID> &getStates() const override;
        const std::vector<BDD_ID> &getInputs() const override;
        bool isReachable(const std::vector<bool> &stateVector) override;
        int stateDistance(const std::vector<bool> &stateVector) override;
        void setTransitionFunctions(const std::vector<BDD_ID> &transitionFunctions) override;
        void setInitState(const std::vector<bool> &stateVector) override;

    };

}
#endif
