
/*
#include "mc_message.h"
#include "mc_message_internal.h"

#include <stdio.h>
#define OUT printf

static int check_failed = 0;

#define CHECK(cond) do { \
    if (!(cond)) { \
        OUT("%d: check failed " #cond "\n", __LINE__); \
        ++check_failed; \
    } \
} while (0)

int str_equal(const char* s1, const char* s2) {
    for (;*s1 == *s2; ++s1, ++s2) {
        if (*s1 == 0) {
            return 1;
        }
    }
    return 0;
}

int str_equal_n(const char* s1, const char* s2, int n) {
    for (;n && *s1 == *s2; ++s1, ++s2, --n) {
        if (*s1 == 0) {
            return 1;
        }
    }
    return 0;
}

void test_uint16() {
    const int buf_len = sizeof(uint16_t) + 1;
    uint8_t buf[buf_len];
    uint16_t out_int;
    buf[buf_len - 1] = 0xFF; // Overflow guard.

    encode_uint16(123, buf);
    decode_uint16(buf, &out_int);
    CHECK(out_int == 123);

    encode_uint16(0xFEBC, buf);
    decode_uint16(buf, &out_int);
    CHECK(out_int == 0xFEBC);

    // Make sure there were no overflows.
    CHECK(buf[buf_len - 1] == 0xFF);
}

void test_uint32() {
    const int buf_len = sizeof(uint32_t) + 1;
    uint8_t buf[buf_len];
    uint32_t out_int;
    buf[buf_len - 1] = 0xFF; // Overflow guard.

    encode_uint32(123, buf);
    decode_uint32(buf, &out_int);
    CHECK(out_int == 123);

    encode_uint32(0xFEBCDEAB, buf);
    decode_uint32(buf, &out_int);
    CHECK(out_int == 0xFEBCDEAB);

    // Make sure there were no overflows.
    CHECK(buf[buf_len - 1] == 0xFF);
}

*/

	/*
void test_state() {
	struct state input;
	struct position p;

    struct state input = {
        .owner_state = BIKE_OWNED,
        .lock_state = BIKE_UNLOCKED,
        .repair_state = BIKE_WORKING,
        .timestamp = 0xABCDEF01,
        .bike_id = "012345678901234",
        .account = "987654321",
        .pin = "1234",
        .battery_level = 245,
        .positions_count = 1,
    };
    struct position p = {
        .compressed_lat = "123",
        .compressed_lng = "1234",
        .timestamp = 12,
    };
    input.positions[0] = p;
	
    struct state output;
    const int buf_len = MAX_STATE_SIZE + 1;
    uint8_t buf[buf_len];
    buf[buf_len - 1] = 0xFF; // Overflow guard.

    // Buffer too small.
    CHECK(encode_state(&input, buf, MAX_STATE_SIZE - 1) == ERR_FULL);
    CHECK(decode_state(buf, 1, &output) == ERR_TOO_SHORT);

    // Incorrect input format (TODO).
    //buf[0] = buf[1] = buf[2] = buf[3] = 0;
    //CHECK(decode_state(buf, buf_len, &output) == ERR_PROTO);

    CHECK(encode_state(&input, buf, buf_len) > 0);
    CHECK(decode_state(buf, buf_len, &output) > 0);

    CHECK(output.repair_state == input.repair_state);
    CHECK(output.owner_state == input.owner_state);
    CHECK(output.lock_state == input.lock_state);
    CHECK(output.timestamp == input.timestamp);
    CHECK(str_equal(output.bike_id, input.bike_id));
    CHECK(str_equal(output.account, input.account));
    CHECK(str_equal(output.pin, input.pin));

    CHECK(output.battery_level == input.battery_level);
    CHECK(output.positions_count == input.positions_count);
    CHECK(str_equal_n(output.positions[0].compressed_lat,
                      input.positions[0].compressed_lat,
                      4));
    CHECK(str_equal_n(output.positions[0].compressed_lng,
                      input.positions[0].compressed_lng,
                      5));
    CHECK(output.positions[0].timestamp == input.positions[0].timestamp);
    // Make sure there were no overflows.
    CHECK(buf[buf_len - 1] == 0xFF);
};

void test_state_update() {
    struct state_update input = {
        .repair_state = BIKE_BROKEN,
        .owner_state = BIKE_HOLD,
        .timestamp = 0xABCDEF01,
        .account = "987654321",
        .pin = "1234",
        .pin_timeout = 0xFFFF,
        .hold_timeout = 26542,
    };
    struct state_update output;
    const int buf_len = STATE_UPDATE_SIZE + 1;
    uint8_t buf[buf_len];
    buf[buf_len - 1] = 0xFF; // Overflow guard.

    // Buffer too small.
    CHECK(encode_state_update(&input, buf, STATE_UPDATE_SIZE - 1) == ERR_FULL);
    CHECK(decode_state_update(buf, STATE_UPDATE_SIZE - 1, &output)
          == ERR_TOO_SHORT);

    // Incorrect input format.
    buf[0] = buf[1] = buf[2] = buf[3] = 0;
    CHECK(decode_state_update(buf, buf_len, &output) == ERR_PROTO);

    CHECK(encode_state_update(&input, buf, buf_len) == STATE_UPDATE_SIZE);
    CHECK(decode_state_update(buf, buf_len, &output) == STATE_UPDATE_SIZE);

    CHECK(output.repair_state == input.repair_state);
    CHECK(output.owner_state == input.owner_state);
    CHECK(output.timestamp == input.timestamp);

    CHECK(str_equal(output.pin, input.pin));
    CHECK(str_equal(output.account, input.account));

    CHECK(output.pin_timeout == input.pin_timeout);
    CHECK(output.hold_timeout == input.hold_timeout);

    // Make sure there were no overflows.
    CHECK(buf[buf_len - 1] == 0xFF);
};


void test_position() {
    struct position p;
    //double lattitude, longitude;
    compress_position("1234.5678S", "54321.0123E", &p);
    // TODO: finish this test.
    //decompress_position(&p, &lattitude, &longitude);
}

int main_test(void) {
    test_uint16();
    test_uint32();
    test_state();
    test_state_update();
   // test_position();
    if (check_failed == 0) {
        OUT("OK\n");
    }
    return check_failed;
}
*/
