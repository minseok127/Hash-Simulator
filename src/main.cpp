#include <iostream>

#include "../include/hashsimulator.h"

using namespace std;

int main()
{
    HID hFuncs[1] = {HID_MURMUR3};

    HashSimulator h(hFuncs, 1, 13, 0x1234);

    uint32_t testDatas[10000];
    for (int i = 1; i <= 10000; i++) {
        testDatas[i - 1] = i*2;
        h.AddKey(testDatas + (i - 1), sizeof(uint32_t));
    }

    h.Test();

    return 0;
}
