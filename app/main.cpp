#include <stdio.h>
#include <stdarg.h>
#include <algorithm>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <iostream>
#include <iomanip>
#include <inttypes.h>

#include "../lib/basic_8bit.h"
#include "../lib/platform.h"
#include "rans.h"
#include "../utils/panic.h"
#include "../lib/symbol_stats.h"
#include "../utils/fileUtil.h"

using namespace std;

// this is a function which deals with giving error and exit for this

void populateCummulativeFreq2Symbol(uint8_t *cummulativeFreq2Symbol, const SymbolStats &stats, uint32_t prob_scale)
{
    for (int s = 0; s < 256; s++)
    {
        uint32_t start = stats.commulativeFrequency[s];
        uint32_t end = stats.commulativeFrequency[s + 1];
        std::fill(cummulativeFreq2Symbol + start, cummulativeFreq2Symbol + end, s);
    }
}

void ransEncode(uint8_t *outputBuffer, size_t maxOutputSize, size_t inputSize, uint8_t *inputArray, encoderSymbol encodingSymbols[256], uint8_t *&rans_begin)
{
    printf("rANS encode:\n");
    for (int run = 0; run < 5; run++)
    {
        double start_time = timer();
        uint64_t enc_start_time = __rdtsc();

        state rans;
        initialiseEncoderState(&rans);

        uint8_t *ptr = outputBuffer + maxOutputSize;

        for (size_t i = inputSize - 1; i != (size_t)-1; i--)
        {
            int s = inputArray[i];
            getSymbolFromEncoder(&rans, &ptr, &encodingSymbols[s]);
            if (i == 0)
                break;
        }
        encoderFlush(&rans, &ptr);
        rans_begin = ptr;

        uint64_t enc_clocks = __rdtsc() - enc_start_time;
        double enc_time = timer() - start_time;

        std::cout << enc_clocks << " clocks, "
                  << std::fixed << std::setprecision(1) << (1.0 * enc_clocks / inputSize)
                  << " clocks/symbol ("
                  << std::setw(5) << (1.0 * inputSize / (enc_time * 1048576.0))
                  << " MiB/s)\n";
    }
}

void ransDecode(uint8_t *rans_begin, size_t inputSize, uint8_t cummulativeFreq2Symbol[16384], const uint32_t prob_bits, uint8_t *decodingBytes, decoderSymbol decodingSymbols[256])
{
    for (int run = 0; run < 5; run++)
    {
        double start_time = timer();
        uint64_t dec_start_time = __rdtsc();

        state rans;
        uint8_t *ptr = rans_begin;
        initialiseDecoderState(&rans, &ptr);

        for (size_t i = 0; i < inputSize; i++)
        {
            uint32_t s = cummulativeFreq2Symbol[getCFforDecodingSymbol(&rans, prob_bits)];
            decodingBytes[i] = (uint8_t)s;
            decoderWithSymbolTable(&rans, &ptr, &decodingSymbols[s], prob_bits);
        }

        uint64_t dec_clocks = __rdtsc() - dec_start_time;
        double dec_time = timer() - start_time;
        std::cout << dec_clocks << " clocks, "
                  << std::fixed << std::setprecision(1) << (1.0 * dec_clocks / inputSize)
                  << " clocks/symbol ("
                  << std::setprecision(1) << (1.0 * inputSize / (dec_time * 1048576.0))
                  << " MiB/s)\n";
    }
}

void checkResults(uint8_t *inputArray, uint8_t *decodingBytes, size_t inputSize)
{
    if (memcmp(inputArray, decodingBytes, inputSize) == 0)
        printf("decode ok!\n");
    else
        printf("ERROR: bad decoder!\n");
}

int main()
{
    size_t inputSize;
    uint8_t *inputArray = readFile("book1.tex", &inputSize);

    static const uint32_t prob_bits = 14;
    static const uint32_t prob_scale = 1 << prob_bits;

    SymbolStats stats;
    stats.calculateFrequency(inputArray, inputSize);
    stats.normaliseFrequency(prob_scale);

    // getCummulativefrequency from
    uint8_t cummulativeFreq2Symbol[prob_scale];
    populateCummulativeFreq2Symbol(cummulativeFreq2Symbol, stats, prob_scale);

    static size_t maxOutputSize = 32 << 20; // 32MB
    uint8_t *outputBuffer = new uint8_t[maxOutputSize];
    uint8_t *decodingBytes = new uint8_t[inputSize];
    memset(decodingBytes, 0xcc, inputSize);

    uint8_t *rans_begin;

    encoderSymbol encodingSymbols[256];
    decoderSymbol decodingSymbols[256];

    for (int i = 0; i < 256; i++)
    {
        encodingSymbolInitialise(&encodingSymbols[i], stats.commulativeFrequency[i], stats.commulativeFrequency[i + 1] - stats.commulativeFrequency[i], prob_bits);
        decodingSymbolInitialise(&decodingSymbols[i], stats.commulativeFrequency[i], stats.commulativeFrequency[i + 1] - stats.commulativeFrequency[i]);
    }

    // try rANS encode
    ransEncode(outputBuffer, maxOutputSize, inputSize, inputArray, encodingSymbols, rans_begin);

    printf("rANS: %d bytes\n", (int)(outputBuffer + maxOutputSize - rans_begin));

    // try rANS decode
    ransDecode(rans_begin, inputSize, cummulativeFreq2Symbol, prob_bits, decodingBytes, decodingSymbols);

    // check decode results
    checkResults(inputArray, decodingBytes, inputSize);
}
