cmake_minimum_required(VERSION 3.10)
project(VDSProject_test CXX C)

# Find GoogleTest
find_package(GTest REQUIRED)

# 1. Define the Manager library using the files in the parent directory
# This creates the "Manager" target that the linker was looking for
add_library(Manager STATIC ../Manager.cpp ../Manager.h ../ManagerInterface.h)

# 2. Define your reachability executable
add_executable(VDSProject_reachability
        main_test.cpp
        Reachability.cpp
        Reachability.h
        Tests.h
        ReachabilityInterface.h
)

# 3. Link everything
# Now the linker will find "Manager" because we defined it above
target_link_libraries(VDSProject_reachability
        PRIVATE
        Manager
        GTest::gtest
        GTest::gtest_main
        pthread
)