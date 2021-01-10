#ifndef __TEST_C_W_CPP___WRAPPER_H
#define __TEST_C_W_CPP___WRAPPER_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct TestCWithCpp TestCWithCpp;

TestCWithCpp* newTestCWithCpp();

uint8_t TestCWithCpp_doSomething(TestCWithCpp* v, uint16_t timeout);

void deleteMyClass(TestCWithCpp* v);

#ifdef __cplusplus
}
#endif

#endif //__TEST_C_W_CPP___WRAPPER_H