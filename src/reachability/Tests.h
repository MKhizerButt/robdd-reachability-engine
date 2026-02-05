#ifndef VDSPROJECT_REACHABILITY_TESTS_H
#define VDSPROJECT_REACHABILITY_TESTS_H

#include <gtest/gtest.h>
#include "Reachability.h"

using namespace ClassProject;

struct ReachabilityTest : testing::Test {

    std::unique_ptr<ClassProject::ReachabilityInterface> fsm2 = std::make_unique<ClassProject::Reachability>(2);
    std::vector<BDD_ID> stateVars2 = fsm2->getStates();
    std::vector<BDD_ID> transitionFunctions;
};

// 1. Test Constructor Exceptions
TEST_F(ReachabilityTest, ConstructorZeroSize) {
    // Should throw if stateSize is 0
    EXPECT_THROW(ClassProject::Reachability(0), std::runtime_error);
}

// 2. Test setTransitionFunctions Exceptions
TEST_F(ReachabilityTest, TransitionFunctionsErrors) {
    //Wrong Size (not enough functions)
    std::vector<BDD_ID> tooFew = {fsm2->True()};
    EXPECT_THROW(fsm2->setTransitionFunctions(tooFew), std::runtime_error);

    // Wrong Size (Too many functions)
    std::vector<BDD_ID> tooMany = {fsm2->True(), fsm2->True(), fsm2->True()};
    EXPECT_THROW(fsm2->setTransitionFunctions(tooMany), std::runtime_error);

    // To cover the Unknown ID
    // We create a fake ID that is definitely larger than the uniqueTable size
    BDD_ID hugeID = 999999;
    std::vector<BDD_ID> badIDs = {fsm2->True(), hugeID};
    EXPECT_THROW(fsm2->setTransitionFunctions(badIDs), std::runtime_error);
}

// 3. Test setInitState Exceptions
TEST_F(ReachabilityTest, SetInitStateErrors) {
    // same with too small
    std::vector<bool> smallVec = {false};
    EXPECT_THROW(fsm2->setInitState(smallVec), std::runtime_error);

    //too large
    std::vector<bool> largeVec = {false, false, true};
    EXPECT_THROW(fsm2->setInitState(largeVec), std::runtime_error);
}

// 4. Test isReachable Exceptions
TEST_F(ReachabilityTest, StateDistanceErrors) {
    // if Vector is too small
    std::vector<bool> smallVec = {false};
    EXPECT_THROW(fsm2->stateDistance(smallVec), std::runtime_error);
    EXPECT_THROW(fsm2->isReachable(smallVec), std::runtime_error);

    // another Case where Vector is too large
    std::vector<bool> largeVec = {false, false, true};
    EXPECT_THROW(fsm2->stateDistance(largeVec), std::runtime_error);
    EXPECT_THROW(fsm2->isReachable(largeVec), std::runtime_error);
}

// 5. Original Functional Test (Kept for sanity check)
TEST_F(ReachabilityTest, HowTo_Example) {
    BDD_ID s0 = stateVars2.at(0);
    BDD_ID s1 = stateVars2.at(1);

    transitionFunctions.push_back(fsm2->neg(s0));
    transitionFunctions.push_back(fsm2->neg(s1));
    fsm2->setTransitionFunctions(transitionFunctions);

    fsm2->setInitState({false,false});

    ASSERT_TRUE(fsm2->isReachable({false, false}));
    ASSERT_FALSE(fsm2->isReachable({false, true}));
    ASSERT_FALSE(fsm2->isReachable({true, false}));
    ASSERT_TRUE(fsm2->isReachable({true, true}));
}

// 3-bit Synchronous Counter FSM with an Input (Enable) signal:
TEST_F(ReachabilityTest, FSM_3Bit_Counter_With_Input) { /* NOLINT */
    // 1. Initialize FSM with 3 State Bits (s0, s1, s2) and 1 Input Bit (enable)
    // s0 is LSB, s2 is MSB
    auto fsm3 = std::make_unique<ClassProject::Reachability>(3, 1);

    std::vector<BDD_ID> states = fsm3->getStates();
    std::vector<BDD_ID> inputs = fsm3->getInputs();

    BDD_ID s0 = states.at(0);
    BDD_ID s1 = states.at(1);
    BDD_ID s2 = states.at(2);
    BDD_ID enable = inputs.at(0);


    // 2. Define Transition Functions for a 3-bit saturating counter
    // Next state logic:
    // If enable=0: Hold state (s' = s)
    // If enable=1: Increment state until 111, then hold at 111

    // For no-wrap 3-bit counter implementation.
    BDD_ID at_max = fsm3->and2(s2, fsm3->and2(s1, s0));
    BDD_ID inc = fsm3->and2(enable, fsm3->neg(at_max));

    BDD_ID s0_next = fsm3->xor2(s0, inc);
    BDD_ID carry0  = fsm3->and2(s0, inc);
    BDD_ID s1_next = fsm3->xor2(s1, carry0);
    BDD_ID carry1  = fsm3->and2(s1, carry0);
    BDD_ID s2_next = fsm3->xor2(s2, carry1);

    std::vector<BDD_ID> transitionFunctions = {s0_next, s1_next, s2_next};
    fsm3->setTransitionFunctions(transitionFunctions);

    // 3. Set Initial State to 0 (000)
    fsm3->setInitState({false, false, false});

    // 4. Test Reachability and State Distance
    
    // State 0 (000) -> Distance 0 (Initial State)
    EXPECT_TRUE(fsm3->isReachable({false, false, false}));
    EXPECT_EQ(fsm3->stateDistance({false, false, false}), 0);

    // State 1 (001) -> Distance 1
    // (Requires enable=1 for 1 cycle)
    EXPECT_TRUE(fsm3->isReachable({true, false, false})); // LSB=1, MSB=0
    EXPECT_EQ(fsm3->stateDistance({true, false, false}), 1);

    // State 3 (011) -> Distance 3
    EXPECT_TRUE(fsm3->isReachable({true, true, false}));
    EXPECT_EQ(fsm3->stateDistance({true, true, false}), 3);

    // State 7 (111) -> Distance 7
    // (Requires counting up 0->1->2->3->4->5->6->7)
    EXPECT_TRUE(fsm3->isReachable({true, true, true}));
    EXPECT_EQ(fsm3->stateDistance({true, true, true}), 7);

    // Verify Hash Table Exploitation (Implicit)
    // The stateDistance function loops. If we ask for a state reachable in 7 steps,
    // the internal loop runs 7 times. 
    // If we run it again for a closer state, or the same state, the transitions
    // are strictly deterministic and BDD operations cached.
    
    // Check "Hold" functionality via Unreachable test? 
    // In this specific counter, all 2^3=8 states are reachable.

    // Let's create an unreachable scenario by changing Init State
    // If we start at 000, and Reset logic was present, we might test that.
    // Instead, let's test a case where we start at 2 (010).
    // The counter only counts UP. So 1 (001) should be unreachable (distance -1).
    
    fsm3->setInitState({false, true, false}); // Start at 2 (010)
    EXPECT_FALSE(fsm3->isReachable({true, false, false})); // Target 1 (001)
    EXPECT_EQ(fsm3->stateDistance({true, false, false}), -1);

    
    // Target 3 (011) should be reachable in 1 step (2 -> 3)
    EXPECT_EQ(fsm3->stateDistance({true, true, false}), 1);
}

#endif
