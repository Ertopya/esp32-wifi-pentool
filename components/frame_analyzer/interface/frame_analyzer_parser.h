/**
 * @file frame_analyzer_parser.h
 * @author risinek (risinek@gmail.com)
 * @date 2021-04-05
 * @copyright Copyright (c) 2021
 * 
 * @brief Provides interface for parsing functionality
 */
#ifndef FRAME_ANALYZER_PARSER_H
#define FRAME_ANALYZER_PARSER_H

#include <stdint.h>
#include "esp_wifi_types.h"

#include "frame_analyzer_types.h"

/**
 * @brief Determines whether BSSID inside of the given frame matches given BSSID.
 * 
 * @param frame 
 * @param bssid 
 * @return bool 
 */
bool is_frame_bssid_matching(wifi_promiscuous_pkt_t *frame, uint8_t *bssid);

/**
 * @brief Parses EAPoL packet from given frame.
 * 
 * @param frame 
 * @return eapol_packet_t* if parsing successful 
 * @return \c NULL if no EAPoL packet was found
 * @return \c NULL if frame is protected
 */
eapol_packet_t *parse_eapol_packet(data_frame_t *frame);

/**
 * @brief Parses EAPoL-Key packet from EAPoL packet
 * 
 * @note result does not include EAPoL header
 * @param eapol_packet 
 * @return eapol_key_packet_t* if parsing successful
 * @return \c NULL if no EAPoL-Key packet found
 */
eapol_key_packet_t *parse_eapol_key_packet(eapol_packet_t *eapol_packet);

/**
 * @brief Verify if the packet is a PROBE frame
 * 
 * @param frame 
 * @return bool
 */
bool is_probe_frame(data_frame_t * frame);

#endif