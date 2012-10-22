#ifndef MC_MESSAGE_INTERNAL_H_
#define MC_MESSAGE_INTERNAL_H_

/*
 * Internal functions exposed for test.
 */
void encode_uint16(uint16_t input, uint8_t* output);
void decode_uint16(const uint8_t* input, uint16_t* output);

void encode_uint32(uint32_t input, uint8_t* output);
void decode_uint32(const uint8_t* input, uint32_t* output);

int main_test(void);

#endif /* MC_MESSAGE_INTERNAL_H_ */
