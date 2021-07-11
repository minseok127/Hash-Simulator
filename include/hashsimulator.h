#ifndef HASHSIMULATOR_H
#define HASHSIMULATOR_H

#include "hashlist.h"
#include "hashcodesize.h"
#include "types.h"

class HashSimulator
{
public:
    HashSimulator(HID* HIDList, int HIDCount, int binCount, uint32_t seed);
    ~HashSimulator();

    void AddKey(void* keyptr, int length); // Add key to the key set

    void Test(); // Start the test, print results

private:
    HID* HIDList = 0; // Arrasy of hash funcitons
    int HIDCount = 0; // The number of hash functions

    uint32_t seed;

    void** keySet = 0; // Array of key pointers
    int* lengthSet = 0; // Array of key's length
    int keyCount = 0; // The number of keys
    int capacity = 1; // Capcity of keyset

#if HASH_CODE_SIZE == 32
    uint32_t* outputSet = 0; // Array of hash code
#elif HASH_CODE_SIZE == 128
    uint128_t* outputSet = 0; // Array of hash code, 4 uint32_t is one hash code
#endif

    int flipCount[HASH_CODE_SIZE]; // Used in Avalanche test

    int* bins = 0; // bins
    int binCount = 0; // The number of bins

    // Test
    void HashingStart(HID hid); // Hash the keys

    void ChiSquaredTest(HID hid); // Chi-squared test
    void AvalancheTest(HID hid); // Avalanche test
    void FillFactorTest(HID hid); // FillFactor test

    void HashingFinish(HID hid); // Initialize bins
};

#endif // HASHSIMULATOR_H
