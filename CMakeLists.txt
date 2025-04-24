cmake_minimum_required (VERSION 3.8)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

project("RLGymPPO_CPP_Example")

add_subdirectory(cpp-interface)

add_executable(RLGymPPO_CPP_Example "./examplemain.cpp" "./RLBotClient.cpp" "./RLBotClient.h")

# Set C++ version to 20 and enable PIC
set_target_properties(RLGymPPO_CPP_Example PROPERTIES 
    LINKER_LANGUAGE CXX
    CXX_STANDARD 20
    POSITION_INDEPENDENT_CODE ON
)

# Make sure RLGymPPO_CPP is going to build in the same directory as us
# Otherwise, we won't be able to import it at runtime
set(LIBRARY_OUTPUT_PATH "${CMAKE_BINARY_DIR}")
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}")

# Include RLBot
target_link_libraries(RLGymPPO_CPP_Example PRIVATE RLBotCPP)

# Include RLGymSim_PPO
add_subdirectory(RLGymPPO_CPP)
target_link_libraries(RLGymPPO_CPP_Example PRIVATE RLGymPPO_CPP)