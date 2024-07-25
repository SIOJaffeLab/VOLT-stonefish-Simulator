add_definitions(-DDATA_DIR_PATH=\"${CMAKE_CURRENT_SOURCE_DIR}/Data/\")

find_package(data_tamer_cpp QUIET)


add_executable(VOLT_Sim_Test test_sim/main.cpp test_sim/UnderwaterTestApp.cpp test_sim/UnderwaterTestManager.cpp)
target_link_libraries(VOLT_Sim_Test VOLT_Sim_Lib)

