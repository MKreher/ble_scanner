#include "test_c_w_cpp.h"
#include "test_c_w_cpp___WRAPPER.h"

extern "C" {
    TestCWithCpp* newTestCWithCpp() {
        return new TestCWithCpp();
    }

    uint8_t TestCWithCpp_doSomething(TestCWithCpp* v, uint16_t timeout) {
        return v->doSomething(timeout);
    }

    void deleteMyClass(TestCWithCpp* v) {
        delete v;
    }
}