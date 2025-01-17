#pragma once

#include <pb.h>
#include <pb_encode.h>
#include <pb_decode.h>

bool pb_helper_decode(uint8_t*, uint16_t, const pb_msgdesc_t*, void*);
bool pb_helper_encode(uint8_t*, uint16_t, const pb_msgdesc_t*, void*, uint16_t*);
bool pb_helper_encode_submessage(uint8_t*, uint16_t, const pb_msgdesc_t*, void*, uint16_t*);