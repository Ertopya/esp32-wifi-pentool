/**
 * @file frame_analyzer.c
 * @author risinek (risinek@gmail.com)
 * @date 2021-04-05
 * @copyright Copyright (c) 2021
 * 
 * @brief Implements frame analysis
 */
#include "frame_analyzer.h"

#include <stdint.h>
#include <string.h>

#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG
#include "esp_log.h"
#include "esp_err.h"
#include "esp_event.h"

#include "wifi_controller.h"
#include "frame_analyzer_parser.h"

static const char *TAG = "frame_analyzer";
static uint8_t target_bssid[6];
static search_type_t search_type = -1;
static const uint8_t nbr_desired_mgmt_frames = 5;
static uint8_t mgmt_frame_ctr = 0;


/**
 * @brief Analyzes data frames from sniffer.
 *  
 * @param args 
 * @param event_base 
 * @param event_id 
 * @param event_data 
 */
static void data_frame_handler(void *args, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    wifi_promiscuous_pkt_t *frame = (wifi_promiscuous_pkt_t *) event_data;
    ESP_LOGV(TAG, "Got data frame");

    if(!is_frame_bssid_matching(frame, target_bssid)){
        ESP_LOGV(TAG, "Not matching BSSIDs.");
        return;
    }

    // Check if frame is an eapol packet
    eapol_packet_t *eapol_packet = parse_eapol_packet((data_frame_t *) frame->payload);
    if(eapol_packet == NULL){
        ESP_LOGV(TAG, "Not an EAPOL packet.");
        return;
    }

    // Check if the eapol frame is type key
    eapol_key_packet_t *eapol_key_packet = parse_eapol_key_packet(eapol_packet);
    if(eapol_key_packet == NULL){
        ESP_LOGV(TAG, "Not an EAPOL-Key packet");
        return;
    }

    // Now we have an eapol key packet, the one we look for in handshake 
    if(search_type == SEARCH_HANDSHAKE){
        // TODO handle timeouts properly by e.g. for cycle
        ESP_ERROR_CHECK_WITHOUT_ABORT(esp_event_post(FRAME_ANALYZER_EVENTS, DATA_FRAME_EVENT_EAPOLKEY_FRAME, frame, sizeof(wifi_promiscuous_pkt_t) + frame->rx_ctrl.sig_len, portMAX_DELAY));
        return;
    }
}



static void mgmt_frame_handler(void *args, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    wifi_promiscuous_pkt_t *frame = (wifi_promiscuous_pkt_t *) event_data;

    if(!is_frame_bssid_matching(frame, target_bssid)){
        ESP_LOGV(TAG, "Not matching BSSIDs.");
        return;
    }

    if (is_probe_frame((data_frame_t *) frame->payload) && (mgmt_frame_ctr < nbr_desired_mgmt_frames)){
        if (mgmt_frame_ctr < nbr_desired_mgmt_frames){
            mgmt_frame_ctr++;       // Increment only what we need to avoid type overflow
        }
        ESP_LOGI(TAG,"Got MGMT frame");
        ESP_ERROR_CHECK_WITHOUT_ABORT(esp_event_post(FRAME_ANALYZER_EVENTS, DATA_FRAME_EVENT_PROBE_FRAME, frame, sizeof(wifi_promiscuous_pkt_t) + frame->rx_ctrl.sig_len, portMAX_DELAY));  
        return;
    }
}

void frame_analyzer_capture_start(search_type_t search_type_arg, const uint8_t *bssid){
    ESP_LOGI(TAG, "Frame analysis started...");
    search_type = search_type_arg;
    mgmt_frame_ctr = 0;
    memcpy(&target_bssid, bssid, 6);
    ESP_ERROR_CHECK(esp_event_handler_register(SNIFFER_EVENTS, SNIFFER_EVENT_CAPTURED_DATA, &data_frame_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(SNIFFER_EVENTS, SNIFFER_EVENT_CAPTURED_MGMT, &mgmt_frame_handler, NULL));
}

void frame_analyzer_capture_stop(){
    ESP_ERROR_CHECK(esp_event_handler_unregister(ESP_EVENT_ANY_BASE, ESP_EVENT_ANY_ID, &data_frame_handler));
    ESP_ERROR_CHECK(esp_event_handler_unregister(ESP_EVENT_ANY_BASE, ESP_EVENT_ANY_ID, &mgmt_frame_handler));
}
