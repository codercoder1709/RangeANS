#pragma once

#include <cstdint>
#include <stddef.h>
#include <cassert>

struct SymbolStats
{
    uint32_t frequencyArray[256];
    uint32_t commulativeFrequency[257];

    void calculateFrequency(uint8_t const *inputArray, size_t inputArraySize);
    void calculateCummulativeFrequency();
    void normaliseFrequency(uint32_t totalTarget);
};

// calculate the frequency for each symbol
void SymbolStats::calculateFrequency(uint8_t const *inputArray, size_t inputArraySize)
{
    for (int i = 0; i < 256; i++)
        frequencyArray[i] = 0;

    for (size_t i = 0; i < inputArraySize; i++)
        frequencyArray[inputArray[i]]++;
}

// calculate the cummulative frequency for each symbol
void SymbolStats::calculateCummulativeFrequency()
{
    commulativeFrequency[0] = 0;
    for (int i = 0; i < 256; i++)
        commulativeFrequency[i + 1] = commulativeFrequency[i] + frequencyArray[i];
}

void SymbolStats::normaliseFrequency(uint32_t totalTarget)
{
    // totalTarget must be greateer than 256 since the frequency beacuse the frequency for any symbol can't be zero
    assert(totalTarget >= 256);

    calculateCummulativeFrequency();
    uint32_t currentTotal = commulativeFrequency[256];

    // resample distribution based on cumulative frequencyArray and totalTarget
    for (int i = 1; i <= 256; i++)
        commulativeFrequency[i] = ((uint64_t)totalTarget * commulativeFrequency[i]) / currentTotal;

    // if the cummulativeFrequency of a symbol is changed to zero then we change some things accordingly
    for (int i = 0; i < 256; i++)
    {
        if (frequencyArray[i] && commulativeFrequency[i + 1] == commulativeFrequency[i])
        {
            int best_steal = -1;
            uint32_t best_freq = ~0u;

            // Find best symbol with minimum non-zero frequency to steal from
            for (int j = 0; j < 256; j++)
            {
                uint32_t freq = commulativeFrequency[j + 1] - commulativeFrequency[j];
                if (freq > 1 && freq < best_freq)
                {
                    best_freq = freq;
                    best_steal = j;
                }
            }

            assert(best_steal != -1);

            if (best_steal < i)
            {
                for (int j = best_steal + 1; j <= i; j++)
                    commulativeFrequency[j]--;
            }
            else
            {
                assert(best_steal > i);
                for (int j = i + 1; j <= best_steal; j++)
                    commulativeFrequency[j]++;
            }
        }
    }
}