# ============================================================
# 测试 CMakeLists.txt
# ============================================================

# 使用 Google Test（若 GTest 不可用则使用简单的 assert 测试）
find_package(GTest QUIET)

if(GTest_FOUND)
    include(GoogleTest)
    set(TEST_LIBS smart_classroom_lib GTest::gtest GTest::gtest_main)
else()
    message(STATUS "GTest not found, building tests with simple main()")
    set(TEST_LIBS smart_classroom_lib)
endif()

# 管道-过滤器架构测试
add_executable(test_pipeline test_pipeline.cpp)
target_link_libraries(test_pipeline PRIVATE ${TEST_LIBS})
add_test(NAME PipelineTest COMMAND test_pipeline)

# 策略模式测试
add_executable(test_strategy test_strategy.cpp)
target_link_libraries(test_strategy PRIVATE ${TEST_LIBS})
add_test(NAME StrategyTest COMMAND test_strategy)

# 观察者模式测试
add_executable(test_observer test_observer.cpp)
target_link_libraries(test_observer PRIVATE ${TEST_LIBS})
add_test(NAME ObserverTest COMMAND test_observer)
