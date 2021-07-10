#include <iostream>

#include "hashsimulator.h"


/* Test Hash Functions with Chi-squared test, Avalanche test, FillFactor test
 * At first, register hash functions, indexing methods, and names
 * then push the keys and start the test */


using namespace std;

///////////////////////////////////////////////////////////////////////////
// Register Hash Functions, Function's Name, Function's Indexing methods
///////////////////////////////////////////////////////////////////////////

// Hash Functions
extern void CustomHash(const void* key, int len, uint32_t seed, void* out);
extern void MurmurHash3(const void* key, int len, uint32_t seed, void* out);

// Indexing Methods
extern int DivIndexing(int bincount, void* out);
extern int BitIndexing(int bincount, void* out);

// Hash Function pointer list
// Index is same with HID
static void (*HashList[])(const void* key, int len, uint32_t seed, void* out) =
{
    MurmurHash3,
    CustomHash,
};

// Hash Function's name list
// Index is same with HID
static const char* HashNameList[] =
{
    "MurmurHash3",
    "CustomHash",
};

// Indexing method list
// Index is same with HID
static int (*IndexingList[])(int bincount, void* out) =
{
    BitIndexing,
    BitIndexing,
};

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////
// Hash Simulator's member functions
///////////////////////////////////////////////////////////////////////////

// HashSimulator's initializer
HashSimulator::HashSimulator(HID* HIDList, int HIDCount, int bincount, uint32_t seed)
{
    // Initialize
    this->HIDList = HIDList;
    this->HIDCount = HIDCount;
    this->bins = new int[bincount];
    this->bincount = bincount;
    this->seed = seed;

    for (int i = 0; i < bincount; i++) {
        this->bins[i] = 0;
    }

    // Show the progress
    for (int i = 0; i < HIDCount; i++) {
        int hIndex = HIDList[i];

        cout << HashNameList[hIndex] <<" is ready..." << endl;
    }
    cout << endl;
}

// Add the key's pointer to the simulator
void HashSimulator::AddKey(void *keyptr, int length)
{
    // If capacity of keyset is insufficient, increase it
    if (this->capacity == this->keycount) {
        // Increase capacity
        this->capacity *= 2;

        // Reallocate array with new capacity
        void** tmp = new void*[this->capacity];
        int* tmplen = new int[this->capacity];

        // Copy the keyset
        for (int i = 0; i < this->capacity; i++) {
            tmp[i] = this->keyset[i];
            tmplen[i] = this->lengthset[i];
        }

        // delete original arrays
        if (keyset) {
            delete[] this->keyset;
            delete[] this->lengthset;
        }

        this->keyset = tmp;
        this->lengthset = tmplen;
    }

    // Push keyptr
    this->keyset[this->keycount] = keyptr;

    // Push length
    this->lengthset[this->keycount] = length;

    // Increase key count
    this->keycount++;
}

// Do the test, print the results
void HashSimulator::Test()
{
    // Test start
    for (int i = 0; i < this->HIDCount; i++) {
        // Fill the bins
        HashingStart(this->HIDList[i]);

        ChiSquaredTest(this->HIDList[i]);
        AvalancheTest(this->HIDList[i]);
        FillFactorTest(this->HIDList[i]);

        // Initialize the bins
        HashingFinish(this->HIDList[i]);
    }
}

// Fill the bins
void HashSimulator::HashingStart(HID hid)
{
    // Hash code
    void* out = 0;

    // Index number
    int index = -1;

    // speed
    double hashingTime = 0;

    // Make hash code array
#if HASH_CODE_SIZE == 32
    this->hashcodeSet = new uint32_t[this->keycount];
#elif HASH_CODE_SIZE == 128
    this->hashcodeSet = new uint128_t[this->keycount];
#endif

    // Show the progress
    cout << HashNameList[hid] << "'s hashing is started..." << endl;

    // Speed check
    // 스피드체크해야함
    for (int i = 0; i < this->keycount; i++) {
        // Get the hash code
        HashList[hid](this->keyset[i], this->lengthset[i], this->seed, out);

        if (out == 0) {
            continue;
        }

        // Push the result
#if HASH_CODE_SIZE == 32
        this->hashcodeSet[i] = *(uint32_t*)(out);
#elif HASH_CODE_SIZE == 128
        this->hashcodeSet[i] = *(uint128_t*)(out);
#endif

        // Get the index
        index = IndexingList[hid](this->bincount, out);

        // Increase bin
        this->bins[index]++;
    }

    cout << HashNameList[hid] << "'s hashing is over" << endl;
    cout << "Speed : " << hashingTime << endl << endl;
}

// Chi-squared test
// Get the p-value and print it
void HashSimulator::ChiSquaredTest(HID hid)
{
    // p-value
    double pValue = 0;

    cout << HashNameList[hid] << "'s Chi-squared test is started..." << endl;

    cout << "p-value is " << pValue << endl << endl;
}

// Avalanche test
// print the possiblitiy
// For all keyset,
// pick a key, change the bit from 0 to the last
// Check the flipped bit of output hash code(32bit) with xor
void HashSimulator::AvalancheTest(HID hid)
{
    cout << HashNameList[hid] << "'s Avalanche test is started..." << endl;

    for (int i = 0; i < this->keycount; i++) {

    }
}

// FillFactor test
// print the FillFactor
void HashSimulator::FillFactorTest(HID hid)
{
    // FillFactor
    double f = 0;
    double b = 0;

    cout << HashNameList[hid] << "'s FillFactor test is started..." << endl;

    for (int i = 0; i < this->bincount; i++) {
        b += this->bins[i] * this->bins[i];
    }
    f = (double)(this->keycount * this->keycount) / b;

    cout << "FillFactor is " << f << endl << endl;
}

// Hashing is over
// Initialize results
void HashSimulator::HashingFinish(HID hid)
{
    // Empty the bins
    for (int i = 0; i < this->bincount; i++) {
        this->bins[i] = 0;
    }

    cout << HashNameList[hid] << "'s test is over..." << endl << endl;
}
