#ifndef AIRGLOVE_CLIENT_H
#define AIRGLOVE_CLIENT_H

#include <stdint.h>

#if defined(_WIN32)
#if defined(AGC_BUILD)
#define AGC_API __declspec(dllexport)
#else
#define AGC_API __declspec(dllimport)
#endif
#else
#define AGC_API __attribute__((visibility("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define AGC_ABI_VERSION 1

#define AGC_LEFT 0
#define AGC_RIGHT 1

#define AGC_JOINT_COUNT 26
#define AGC_JOINT_FLOATS 8
#define AGC_HAND_FLOATS (AGC_JOINT_COUNT * AGC_JOINT_FLOATS)
#define AGC_FINGER_COUNT 5

#define AGC_OK 0
#define AGC_NONE 0
#define AGC_FRESH 1
#define AGC_ERROR (-1)

typedef struct agc_client agc_client;

AGC_API int agc_abi_version(void);
AGC_API const char* agc_version(void);
AGC_API const char* agc_last_error(void);

AGC_API agc_client* agc_create(const char* listen_address, int listen_port, const char* spine_address, int spine_port);
AGC_API void agc_destroy(agc_client* client);

AGC_API int agc_get_hand(agc_client* client, int side, float* joints, int capacity, uint8_t* joint_valid,
                         int32_t* seq, int64_t* sender_time_us, double* age_s);
AGC_API int agc_get_wrist(agc_client* client, int side, float* orientation_xyzw, int32_t* seq, int64_t* sender_time_us,
                          double* age_s);

AGC_API int agc_get_device_status(agc_client* client, int* left_connected, int* right_connected, double* age_s);
AGC_API int agc_get_spine_alive(agc_client* client, double* age_s);

AGC_API int agc_set_haptics(agc_client* client, int side, const int* strengths, int count);
AGC_API int agc_request_haptics(agc_client* client, int side);
AGC_API int agc_get_haptics(agc_client* client, int side, int* strengths, int capacity, double* age_s);
AGC_API int agc_get_haptics_result(agc_client* client, int side, int* success, double* age_s);

AGC_API int agc_poll_alarm(agc_client* client, char* code, int code_capacity, int* side);

AGC_API void agc_set_stale_threshold(agc_client* client, double seconds);
AGC_API void agc_get_stats(agc_client* client, uint64_t* datagrams, uint64_t* malformed, uint64_t* dropped_seq);

#ifdef __cplusplus
}
#endif

#endif
