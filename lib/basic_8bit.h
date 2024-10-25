#ifndef basic_8bitFile
#define basic_8bitFile

#include <stdint.h>
#include <cassert>

constexpr uint32_t lowerBound = 1u << 23;

typedef uint32_t state;

static void initialiseEncoderState(state *st)
{
    *st = lowerBound;
}

static void normaliseEncoder(state *s, uint8_t **outputBuffer, uint32_t upperBound)
{
    // maximum value of state if it is exceeded oyte output is taken
    uint32_t x = *s;
    if (x > upperBound)
    {
        // output buffer ennd address
        uint8_t *ptr = *outputBuffer;
        do
        {
            // output a byte if s>upperBound
            *--ptr = (uint8_t)(x & 0xff);
            x >>= 8;
        } while (x > upperBound);
        // outputbuffer update to current location of pointer
        *outputBuffer = ptr;
    }
    *s = x;
}

// this function is used for encoding of message
static void encoder(state *s, uint8_t **outputBuffer, uint32_t start, uint32_t frequency, uint32_t scaleBits)
{
    // update the state x = ((x/frequency) << scaleBits ) + (x%frequency) + start;
    const uint32_t precision = 32;
    uint32_t reciprocal = ((1ull << precision) + frequency - 1) / frequency;
    uint64_t quotient = ((uint64_t)*s * reciprocal) >> precision;
    uint32_t remainder = *s - (quotient * frequency);
    *s = (quotient << scaleBits) + remainder + start;
}

// this function adds the remaining bits to output buffer
static void encoderFlush(state *s, uint8_t **outputBuffer)
{
    uint32_t x = *s;
    uint8_t *ptr = *outputBuffer;
    ptr -= 4;

    for (int i = 0; i < 4; i++)
        ptr[i] = (uint8_t)(x >> (i * 8));

    *outputBuffer = ptr;
}

// since the decoding part is "reverse" of the encoding part we will start decoding in reverse
static void initialiseDecoderState(state *s, uint8_t **outputBuffer)
{
    uint32_t x = 0;
    uint8_t *ptr = *outputBuffer;
    for (int i = 0; i < 4; i++)
    {
        x |= (uint32_t)ptr[i] << (i * 8);
    }
    *outputBuffer = ptr + 4;
    *s = x;
}

static inline uint32_t getCFforDecodingSymbol(state *s, uint32_t scaleBits)
{
    // by doing bitwise or with scalebits-1 we get relevant bits for determining the frequency
    return *s & ((1u << scaleBits) - 1);
}

static state normaliseDecoder(state *s, uint8_t **outputBuffer)
{
    state x = *s;
    if (x < lowerBound)
    {
        uint8_t *ptr = *outputBuffer;
        do
            x = (x << 8) | *ptr++;
        while (x < lowerBound);
        *outputBuffer = ptr;
    }

    return x; // return updated value of state
}

static void decoder(state *s, uint8_t **outputBuffer, uint32_t start, uint32_t frequency, uint32_t scaleBits)
{
    uint32_t mask = (1u << scaleBits) - 1;
    uint32_t x = *s; // copy state

    x = frequency * (x >> scaleBits) + (x & mask) - start; // decode x, no need to use complex maths like for encoding
    *s = normaliseDecoder(&x, outputBuffer);               // first we calculate the new stat
}

typedef struct
{
    uint32_t upperBound;
    uint32_t frequencyInverse;
    uint32_t bias; // Bias
    uint16_t frequencyCompliment;
    uint16_t reciprocalShift; // Reciprocal shift
} encoderSymbol;

typedef struct
{
    uint16_t start;     // Start of range.
    uint16_t frequency; // Symbol frequency.
} decoderSymbol;

// Initializes an encoder symbol to start "start" and frequency "frequency"
static inline void encodingSymbolInitialise(encoderSymbol *symb, uint32_t start, uint32_t frequency, uint32_t scaleBits)
{
    assert(scaleBits <= 16);
    assert(start <= (1u << scaleBits));
    if (frequency > ((1u << scaleBits) - start))
        frequency = ((1u << scaleBits) - start);

    symb->upperBound = ((lowerBound >> scaleBits) << 8) * frequency;
    symb->frequencyCompliment = ((1 << scaleBits) - frequency);
    if (frequency < 2)
    {
        // if frequency = 1 frequency  inverse = frequency so
        symb->frequencyInverse = ~0u;
        symb->reciprocalShift = 0;
        symb->bias = start + (1 << scaleBits) - 1;
    }
    else
    {
        //
        uint32_t shift = 0;
        while (frequency > (1u << shift))
            shift++;

        symb->frequencyInverse = (uint32_t)(((1ull << (shift + 31)) + frequency - 1) / frequency);
        symb->reciprocalShift = shift - 1;

        symb->bias = start;
    }
}
// Initialize a decoder symbol to start "start" and frequency "frequency"
static inline void decodingSymbolInitialise(decoderSymbol *s, uint32_t start, uint32_t frequency)
{
    if (start >= (1 << 16))
        start = (1 << 16) - 1;
    if (frequency > ((1 << 16) - start))
        frequency = ((1u << 16) - start);
    s->start = start;
    s->frequency = frequency;
}

static inline void getSymbolFromEncoder(state *s, uint8_t **outputBuffer, encoderSymbol const *sym)
{
    if (sym->upperBound == 0)
        return; // can't encode symbol with frequency=0
    uint32_t x = *s;
    normaliseEncoder(&x, outputBuffer, sym->upperBound);

    uint32_t q = (uint32_t)(((uint64_t)x * sym->frequencyInverse) >> 32) >> sym->reciprocalShift;
    *s = x + sym->bias + q * sym->frequencyCompliment;
}

// this function is used if symbol table is already present and we don't need to calculate the symbol on the fly
static inline void decoderWithSymbolTable(state *s, uint8_t **outputBuffer, decoderSymbol const *sym, uint32_t scaleBits)
{
    decoder(s, outputBuffer, sym->start, sym->frequency, scaleBits);
}

// this function is used if the symbol table is not availiable and we have to calculate the symbol
static inline void decoderWithoutSymbolTable(state *s, uint32_t start, uint32_t frequency, uint32_t scaleBits)
{
    uint32_t mask = (1u << scaleBits) - 1;
    uint32_t x = *s;
    *s = frequency * (x >> scaleBits) + (x & mask) - start;
}

static inline void RansDecAdvanceSymbolStep(state *r, decoderSymbol const *sym, uint32_t scaleBits)
{
    decoderWithoutSymbolTable(r, sym->start, sym->frequency, scaleBits);
}

#endif