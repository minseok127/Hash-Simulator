#include <iostream>

#include "hashsimulator.h"

using namespace std;

int main()
{
    HID hFuncs[1] = {HID_MURMUR3};

    HashSimulator h(hFuncs, 1, 31, 0x1234);

    uint32_t testDatas[1000];
    for (int i = 1; i <= 1000; i++) {
        testDatas[i - 1] = i * 2654435761;
        h.AddKey(testDatas + (i - 1), sizeof(uint32_t));
    }

    h.Test();

    return 0;
}
