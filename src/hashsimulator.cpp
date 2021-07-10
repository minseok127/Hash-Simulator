#include <iostream>
#include <math.h>

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
HashSimulator::HashSimulator(HID* HIDList, int HIDCount, int binCount, uint32_t seed)
{
    // Initialize
    this->HIDList = HIDList;
    this->HIDCount = HIDCount;
    this->bins = new int[binCount];
    this->binCount = binCount;
    this->seed = seed;

    for (int i = 0; i < binCount; i++) {
        this->bins[i] = 0;
    }

    for (int i = 0; i < HASH_CODE_SIZE; i++) {
        this->flipCount[i] = 0;
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
    if (this->capacity == this->keyCount) {
        // Increase capacity
        this->capacity *= 2;

        // Reallocate array with new capacity
        void** tmp = new void*[this->capacity];
        int* tmplen = new int[this->capacity];

        // Copy the keyset
        for (int i = 0; i < this->capacity; i++) {
            tmp[i] = this->keySet[i];
            tmplen[i] = this->lengthSet[i];
        }

        // delete original arrays
        if (keySet) {
            delete[] this->keySet;
            delete[] this->lengthSet;
        }

        this->keySet = tmp;
        this->lengthSet = tmplen;
    }

    // Push keyptr
    this->keySet[this->keyCount] = keyptr;

    // Push length
    this->lengthSet[this->keyCount] = length;

    // Increase key count
    this->keyCount++;
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
    this->outputSet = new uint32_t[this->keyCount];
#elif HASH_CODE_SIZE == 128
    this->outputSet = new uint128_t[this->keyCount];
#endif

    // Show the progress
    cout << HashNameList[hid] << "'s hashing is started..." << endl;

    // Speed check
    // 스피드체크해야함
    for (int i = 0; i < this->keyCount; i++) {
        // Get the hash code
        HashList[hid](this->keySet[i], this->lengthSet[i], this->seed, out);

        if (out == 0) {
            continue;
        }

        // Push the result
#if HASH_CODE_SIZE == 32
        this->outputSet[i] = *(uint32_t*)(out);
#elif HASH_CODE_SIZE == 128
        this->outputSet[i] = *(uint128_t*)(out);
#endif

        // Get the index
        index = IndexingList[hid](this->binCount, out);

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

    // Expected bin
    double expectedPerBin = (double)this->keyCount / this->binCount;

    // Chi-squared value
    double chiValue = 0;
    double diff = 0;

    cout << HashNameList[hid] << "'s Chi-squared test is started..." << endl;

    // Get the chi-squared value
    for (int i = 0; i < this->binCount; i++) {
        diff = this->bins[i] - expectedPerBin;
        chiValue += diff * diff / expectedPerBin; // sum of (real - expected)^2 / expected
    }

    // Get the p-value with "keycount" Degree of freedom

    cout << "p-value is " << pValue << endl << endl;
}

// Avalanche test
// print the possiblitiy
// For all keyset,
// pick a key, change the bit from 0 to the last
// Check the flipped bit of output hash code with xor

static uchar flipTable[8] = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};
inline __attribute__ ((always_inline)) void Flip(uchar* target, int n)
{
    *(target + n / 8) ^= flipTable[n % 8]; // Assume target is pointing 0, 0 1 2 ... like big-endian
}

inline __attribute__ ((always_inline)) bool IsBitSet(uchar* target, int n)
{
    return *(target + n / 8) & flipTable[n % 8]; // Assume target is pointing 0, 0 1 2 ... like big-endian
}

void HashSimulator::AvalancheTest(HID hid)
{
    int count = 0;
    void* key = 0;

#if HASH_CODE_SIZE == 32
    uint32_t originalOutput;
    uint32_t newOutput;
    uint32_t checkCode;
#elif HASH_CODE_SIZE == 128
    uint128_t originalOutput;
    uint128_t newOutput;
    uint128_t IsBitSet;
#endif

    cout << HashNameList[hid] << "'s Avalanche test is started..." << endl;

    // For all keys
    for (int i = 0; i < this->keyCount; i++) {
        // original output will be compared
        originalOutput = this->outputSet[i];

        // Copy original key
        key = malloc(this->lengthSet[i]);
        memcpy(key, this->keySet[i], this->lengthSet[i]);

        // Flip the key's all bits, 0 to length
        for (int j = 0; j < this->lengthSet[i]; j++) {
            // Flip jth bit
            Flip((uchar*)key, j);

            // Get the new hash code
            HashList[hid](key, this->lengthSet[i], this->seed, &newOutput);

            // (original) xor (new)
            // to check changed bit
            // If bit is changed, xor will set the bit to 1
            checkCode = originalOutput ^ newOutput;

            // Check the changed bit
            for (int k = 0; k < HASH_CODE_SIZE; k++) {
                if (IsBitSet((uchar*)&checkCode, k)) { // Is kth bit is 1?
                    this->flipCount[k]++; // this bit is changed, increase flip count
                }
            }

            // Flip back
            Flip((uchar*)key, j);
            count++; // total flipped count
        }

        // free copied key
        free(key);
    }

    // Print the possiblity of each bits
    for (int i = 0; i < HASH_CODE_SIZE; i++) {
        cout << "bit" << HASH_CODE_SIZE - (i + 1) << " : " << (double)this->flipCount[i] / count << endl;
    }

    cout << "Avalanche test is over" << endl << endl;
}

// FillFactor test
// print the FillFactor
void HashSimulator::FillFactorTest(HID hid)
{
    // FillFactor
    double f = 0;
    double b = 0;

    cout << HashNameList[hid] << "'s FillFactor test is started..." << endl;

    for (int i = 0; i < this->binCount; i++) {
        b += this->bins[i] * this->bins[i];
    }
    f = (double)(this->keyCount * this->keyCount) / b; // kk / nrr

    cout << "FillFactor is " << f << endl << endl;
}

// Hashing is over
// Initialize results
void HashSimulator::HashingFinish(HID hid)
{
    // Empty the bins
    for (int i = 0; i < this->binCount; i++) {
        this->bins[i] = 0;
    }

    cout << HashNameList[hid] << "'s test is over..." << endl << endl;
}
