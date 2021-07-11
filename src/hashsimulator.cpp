#include <assert.h>
#include <chrono>
#include <iostream>

#include "../include/hashsimulator.h"

/* Test Hash Functions with Chi-squared test, Avalanche test, FillFactor test
 * At first, register hash functions, indexing methods, and names
 * then push the keys and start the test */

using namespace std;

///////////////////////////////////////////////////////////////////////////
// Register Hash Functions, Function's Name, Function's Indexing methods
///////////////////////////////////////////////////////////////////////////

// Hash Functions
extern void MurmurHash3_x86_32(const void* key, int len, uint32_t seed, void* out);
extern void CustomHash_32(const void* key, int len, uint32_t seed, void* out);

// Indexing Methods
extern int DivIndexing(int bincount, void* out);

// Hash Function pointer list
// Index is same with HID
static void (*HashList[])(const void* key, int len, uint32_t seed, void* out) =
{
    MurmurHash3_x86_32,     // [HID_MURMUR3]
    CustomHash_32,          // [HID_CUSTOM]
};

// Hash Function's name list
// Index is same with HID
static const char* HashNameList[] =
{
    "MurmurHash3",          // [HID_MURMUR3]
    "Custom",               // [HID_CUSTOM]
};

// Indexing method list
// Index is same with HID
static int (*IndexingList[])(int bincount, void* out) =
{
    DivIndexing,            // [HID_MURMUR3]
    DivIndexing,            // [HID_CUSTOM]
};

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////
// Hash Simulator's member functions
///////////////////////////////////////////////////////////////////////////

// HashSimulator's initializer
HashSimulator::HashSimulator(HID* HIDList, int HIDCount, int binCount, uint32_t seed)
{
    // Get the hash id list
    this->HIDList = HIDList;
    this->HIDCount = HIDCount;

    // make bins
    this->bins = new int[binCount];
    this->binCount = binCount;

    // Empty bins
    for (int i = 0; i < binCount; i++) {
        this->bins[i] = 0;
    }

    // Get seed for hashing
    this->seed = seed;

    // Make key set, length set
    // first capacity is 1
    this->keySet = new void*[this->capacity];
    this->lengthSet = new int[this->capacity];

    // Set flip count array for avalanche test
    for (int i = 0; i < HASH_CODE_SIZE; i++) {
        this->flipCount[i] = 0;
    }

    // Show the progress
    for (int i = 0; i < HIDCount; i++) {
        cout << HashNameList[HIDList[i]] <<" is ready..." << endl;
    }
    cout << endl;
}

// Destroyer
HashSimulator::~HashSimulator()
{
    delete[] this->bins;
    delete[] this->keySet;
    delete[] this->lengthSet;
}

// Add the key's pointer to the simulator
void HashSimulator::AddKey(void *keyptr, int length)
{
    // If capacity of keyset is insufficient, increase it(doubled)
    if (this->capacity == this->keyCount) {
        // Increase capacity
        this->capacity *= 2;

        // Reallocate array with new capacity
        void** tmp = new void*[this->capacity];
        int* tmplen = new int[this->capacity];

        // Copy the original
        for (int i = 0; i < this->keyCount; i++) {
            tmp[i] = this->keySet[i];
            tmplen[i] = this->lengthSet[i];
        }

        // delete original arrays
        if (this->keySet) {
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
    // For all hashes
    for (int i = 0; i < this->HIDCount; i++) {
        // Fill the bins
        this->HashingStart(this->HIDList[i]);

        // Test start
        this->ChiSquaredTest(this->HIDList[i]);
        this->AvalancheTest(this->HIDList[i]);
        this->FillFactorTest(this->HIDList[i]);

        // Destroy the bins
        this->HashingFinish(this->HIDList[i]);
    }
}

// Fill the bins
void HashSimulator::HashingStart(HID hid)
{
    // Index number
    int index = -1;

    // speed
    chrono::nanoseconds nano;

    // Make hash code array
#if HASH_CODE_SIZE == 32
    this->outputSet = new uint32_t[this->keyCount];
    uint32_t out = 0;
#elif HASH_CODE_SIZE == 128
    this->outputSet = new uint128_t[this->keyCount];
    uint128_t out = 0;
#endif

    // Show the progress
    cout << HashNameList[hid] << "'s hashing is started..." << endl;

    // Speed check
    chrono::system_clock::time_point start = chrono::system_clock::now();
    for (int i = 0; i < this->keyCount; i++) {
        // Get the hash code
        HashList[hid](this->keySet[i], this->lengthSet[i], this->seed, &out);
        assert(out != 0);

        // Push the result
        this->outputSet[i] = out;

        // Get the index
        index = IndexingList[hid](this->binCount, &out);
        assert(this->binCount - 1 >= index);

        // Increase bin
        this->bins[index]++;
    }
    chrono::system_clock::time_point end = chrono::system_clock::now();

    nano = end - start;

    cout << HashNameList[hid] << "'s hashing is over" << endl;
    cout << "Size of key set : " << this->keyCount << endl;
    cout << "Speed : " << nano.count() << "(ns)" << endl << endl;
}

// Chi-squared test
// Get the p-value and print it
void HashSimulator::ChiSquaredTest(HID hid)
{
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

    cout << "Chi-squared value : " << chiValue << endl;
    cout << "DOF : " << this->binCount << endl << endl;
}

// Avalanche test
// print the possiblitiy
// For all keyset,
// pick a key, change the bit from 0 to the last
// Check the flipped bit of output hash code with xor

static uint8_t flipTable[8] = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};
inline __attribute__ ((always_inline)) void Flip(uint8_t* target, int n)
{
    // target variable is pointing a byte
    // Assume bit array's index is increased from left to the right
    // 0 1 2 3 4 5 6 7 8 <= 1 byte, target is pointing 0bit
    // n / 8 is location of target byte
    // n % 8 is index of flipped bit
    *(target + n / 8) ^= flipTable[n % 8];
}

inline __attribute__ ((always_inline)) bool IsBitSet(uint8_t* target, int n)
{
    // target variable is pointing a byte
    // Assume bit array's index is increased from left to the right
    // 0 1 2 3 4 5 6 7 8 <= 1 byte, target is pointing 0bit
    // n / 8 is location of target byte
    // n % 8 is index of checked bit
    return *(target + n / 8) & flipTable[n % 8];
}

void HashSimulator::AvalancheTest(HID hid)
{
    int count = 0; // total flip count
    void* keyFrame = 0;

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
        keyFrame = malloc(this->lengthSet[i]);
        memcpy(keyFrame, this->keySet[i], this->lengthSet[i]);

        // Flip the key's all bits, 0 to length
        for (int j = 0; j < this->lengthSet[i]; j++) {
            // Flip jth bit
            Flip((uint8_t*)keyFrame, j);

            // Get the new hash code
            HashList[hid](keyFrame, this->lengthSet[i], this->seed, &newOutput);

            // (original) xor (new)
            // to check changed bit
            // If bit is changed, xor will set the bit to 1
            checkCode = originalOutput ^ newOutput;

            // Check the changed bit
            for (int k = 0; k < HASH_CODE_SIZE; k++) {
                if (IsBitSet((uint8_t*)&checkCode, k)) { // Is kth bit is 1?
                    this->flipCount[k]++; // this bit is changed, increase flip count
                }
            }

            // Flip back
            Flip((uint8_t*)keyFrame, j);
            count++; // total flipped count
        }

        // free copied key
        free(keyFrame);
    }

    // Print the possibility of each bits
    double avg = 0; // average possibility
    double p = 0;
    for (int i = 0; i < HASH_CODE_SIZE; i++) {
        // possibility
        p = (double)this->flipCount[i] / count;

        cout << "bit" << HASH_CODE_SIZE - (i + 1) << " : " << p << endl;

        avg += p;
    }
    avg /= HASH_CODE_SIZE;
    cout << "Average : " << avg << endl << endl;
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

    cout << 100 * (1 - f / this->binCount) << "% is wasted..." << endl << endl;
}

// Hashing is over
// Initialize results
void HashSimulator::HashingFinish(HID hid)
{
    // Empty the bins
    cout << "bins" << endl;
    for (int i = 0; i < this->binCount; i++) {
        cout << "bin" << i << " : " << this->bins[i] << " (expected : " << (double)this->keyCount / this->binCount << ") " << endl;
        this->bins[i] = 0;
    }
    cout << endl;

    // Delete output set
    delete[] this->outputSet;
    this->outputSet = 0;

    cout << HashNameList[hid] << "'s test is over..." << endl << endl;
}
