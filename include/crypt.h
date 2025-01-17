#pragma once

#include <aes.hpp>
#include <stdint.h>

struct AESResult
{
    uint8_t* data;
    uint16_t length;
};

AESResult aes_crypt_ctr_xcrypt(uint8_t*, uint8_t, uint32_t, uint32_t, uint8_t*, uint8_t);