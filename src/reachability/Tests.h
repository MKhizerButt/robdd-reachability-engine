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

#endif