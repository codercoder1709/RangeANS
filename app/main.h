#pragma once
void interleavedRANS(uint8_t *decodingBytes, size_t inputSize, uint8_t *outputBuffer, size_t maxOutputSize, uint8_t *inputArray, encoderSymbol encodingSymbols[256], uint8_t *&rans_begin, uint8_t cummulativeFreq2Symbol[16384], const uint32_t prob_bits, decoderSymbol decodingSymbols[256]);

void rans(uint8_t *decodingBytes, size_t inputSize, uint8_t *outputBuffer, size_t maxOutputSize, uint8_t *inputArray, encoderSymbol encodingSymbols[256], uint8_t *&rans_begin, uint8_t cummulativeFreq2Symbol[16384], const uint32_t prob_bits, decoderSymbol decodingSymbols[256]);

void interleavedRANSEncode(uint8_t *outputBuffer, size_t maxOutputSize, size_t inputSize, uint8_t *inputArray, encoderSymbol encodingSymbols[256], uint8_t *&rans_begin);

void interLeavedRANSDecoder(uint8_t *rans_begin, size_t inputSize, uint8_t cummulativeFreq2Symbol[16384], const uint32_t prob_bits, uint8_t *decodingBytes, decoderSymbol decodingSymbols[256]);

void checkInterleavedRANSResults(uint8_t *inputArray, uint8_t *decodingBytes, size_t inputSize);

void ransEncode(uint8_t *outputBuffer, size_t maxOutputSize, size_t inputSize, uint8_t *inputArray, encoderSymbol encodingSymbols[256], uint8_t *&rans_begin);

void ransDecode(uint8_t *rans_begin, size_t inputSize, uint8_t cummulativeFreq2Symbol[16384], const uint32_t prob_bits, uint8_t *decodingBytes, decoderSymbol decodingSymbols[256]);

void checkResults(uint8_t *inputArray, uint8_t *decodingBytes, size_t inputSize);