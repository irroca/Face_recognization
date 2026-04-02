// ============================================================
// test_pipeline.cpp — 管道-过滤器架构单元测试
// ============================================================
// 验证：
//   1. Pipeline 能正确串联多个 Filter
//   2. Filter 处理顺序的正确性
//   3. 短路机制（某个 Filter 失败时终止管道）
//   4. 空管道的处理
// ============================================================

#include "pipeline/IFilter.h"
#include "pipeline/Pipeline.h"
#include "core/Types.h"
#include <cassert>
#include <iostream>
#include <memory>
#include <vector>

using namespace smart_classroom;

// ============================================================
// 测试用 Mock Filter：记录处理顺序
// ============================================================
class MockFilter : public IFilter {
public:
    MockFilter(const std::string& name, bool shouldSucceed,
               std::vector<std::string>& log)
        : name_(name), shouldSucceed_(shouldSucceed), log_(log) {}

    bool process(VideoFrame& frame) override {
        log_.push_back(name_);
        return shouldSucceed_;
    }

    std::string name() const override { return name_; }

private:
    std::string name_;
    bool shouldSucceed_;
    std::vector<std::string>& log_;
};

// ============================================================
// 测试用例
// ============================================================

void test_empty_pipeline() {
    std::cout << "[TEST] Empty pipeline... ";
    Pipeline pipeline;
    VideoFrame frame;
    assert(pipeline.execute(frame) == true);
    assert(pipeline.filterCount() == 0);
    std::cout << "PASSED\n";
}

void test_single_filter() {
    std::cout << "[TEST] Single filter... ";
    std::vector<std::string> log;
    Pipeline pipeline;
    pipeline.addFilter(std::make_unique<MockFilter>("F1", true, log));

    VideoFrame frame;
    assert(pipeline.execute(frame) == true);
    assert(log.size() == 1);
    assert(log[0] == "F1");
    std::cout << "PASSED\n";
}

void test_pipeline_order() {
    std::cout << "[TEST] Pipeline execution order... ";
    std::vector<std::string> log;
    Pipeline pipeline;
    pipeline.addFilter(std::make_unique<MockFilter>("Decode", true, log));
    pipeline.addFilter(std::make_unique<MockFilter>("Preprocess", true, log));
    pipeline.addFilter(std::make_unique<MockFilter>("Detect", true, log));
    pipeline.addFilter(std::make_unique<MockFilter>("Recognize", true, log));
    pipeline.addFilter(std::make_unique<MockFilter>("Draw", true, log));
    pipeline.addFilter(std::make_unique<MockFilter>("Encode", true, log));

    VideoFrame frame;
    assert(pipeline.execute(frame) == true);
    assert(log.size() == 6);
    assert(log[0] == "Decode");
    assert(log[1] == "Preprocess");
    assert(log[2] == "Detect");
    assert(log[3] == "Recognize");
    assert(log[4] == "Draw");
    assert(log[5] == "Encode");
    std::cout << "PASSED\n";
}

void test_pipeline_short_circuit() {
    std::cout << "[TEST] Pipeline short-circuit on failure... ";
    std::vector<std::string> log;
    Pipeline pipeline;
    pipeline.addFilter(std::make_unique<MockFilter>("F1", true, log));
    pipeline.addFilter(std::make_unique<MockFilter>("F2_FAIL", false, log));
    pipeline.addFilter(std::make_unique<MockFilter>("F3", true, log));

    VideoFrame frame;
    assert(pipeline.execute(frame) == false);
    assert(log.size() == 2);  // F3 不应被执行
    assert(log[0] == "F1");
    assert(log[1] == "F2_FAIL");
    std::cout << "PASSED\n";
}

void test_pipeline_filter_count() {
    std::cout << "[TEST] Pipeline filter count... ";
    std::vector<std::string> log;
    Pipeline pipeline;
    assert(pipeline.filterCount() == 0);
    pipeline.addFilter(std::make_unique<MockFilter>("F1", true, log));
    assert(pipeline.filterCount() == 1);
    pipeline.addFilter(std::make_unique<MockFilter>("F2", true, log));
    assert(pipeline.filterCount() == 2);
    pipeline.clear();
    assert(pipeline.filterCount() == 0);
    std::cout << "PASSED\n";
}

int main() {
    std::cout << "========================================\n";
    std::cout << "Pipeline-Filter Architecture Tests\n";
    std::cout << "========================================\n";

    test_empty_pipeline();
    test_single_filter();
    test_pipeline_order();
    test_pipeline_short_circuit();
    test_pipeline_filter_count();

    std::cout << "========================================\n";
    std::cout << "All pipeline tests PASSED!\n";
    std::cout << "========================================\n";
    return 0;
}
