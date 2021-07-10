#ifndef HASHSIMULATOR_H
#define HASHSIMULATOR_H

#include <stdint.h>

#include "hashlist.h"
#include "hashcodesize.h"

typedef struct hashcode128
{
    uint32_t h1;
    uint32_t h2;
    uint32_t h3;
    uint32_t h4;
} uint128_t;

class HashSimulator
{
public:
    HashSimulator(HID* HIDList, int HIDCount, int bincount, uint32_t seed);
    void AddKey(void* keyptr, int length); // Add key to the key set

    void Test(); // Start the test, print results

private:
    HID* HIDList = 0; // Arrasy of hash funcitons
    int HIDCount = 0; // The number of hash functions

    uint32_t seed;

    void** keyset = 0; // Array of key pointers
    int keycount = 0; // The number of keys
    int capacity = 0; // Capcity of keyset
    int* lengthset = 0; // Array of key's length

#if HASH_CODE_SIZE == 32
    uint32_t* hashcodeSet = 0; // Array of hash code
#elif HASH_CODE_SIZE == 128
    uint128_t* hashcodeSet = 0; // Array of hash code, 4 uint32_t is one hash code
#endif

    int* bins = 0; // bins
    int bincount = 0; // The number of bins

    // Test
    void HashingStart(HID hid); // Hash the keys

    void ChiSquaredTest(HID hid); // Chi-squared test
    void AvalancheTest(HID hid); // Avalanche test
    void FillFactorTest(HID hid); // FillFactor test

    void HashingFinish(HID hid); // Initialize bins
};

#endif // HASHSIMULATOR_H
