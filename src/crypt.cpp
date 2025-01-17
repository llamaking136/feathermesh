#include <aes.hpp>
#include <crypt.h>
#include <config.h>
#include <channels.h>
#include <llog.h>
#include <util.h>

uint8_t aes_crypt_ctr_buffer[MAX_PAYLOAD_SIZE];

AESResult aes_crypt_ctr_xcrypt(uint8_t* data, uint8_t data_length, uint32_t packet_id, uint32_t node_id, uint8_t* key_ptr, uint8_t key_length)
{
    AESResult result = { nullptr, 0 };

    AES_ctx ctx;
    uint8_t iv[16];

    // LLOG_DEBUG("here0");

    // memory shenanagens
    memset(iv, 0, 16);
    memcpy(iv, &packet_id, sizeof(uint32_t));
    memcpy(iv + 8, &node_id, sizeof(uint32_t));

    // printnhex(iv, 16, 0);

    // LLOG_DEBUG("here1");

    // AES256
    if (key_length == AES256_len)
    {
        ctx.key_length = AES256_len;
        ctx.key_exp_size = AES256_key_exp_size;
        ctx.key_32bit = AES256_32key;
        ctx.key_rounds = AES256_rounds;
    }
    // AES192
    else if (key_length == AES192_len)
    {
        ctx.key_length = AES192_len;
        ctx.key_exp_size = AES192_key_exp_size;
        ctx.key_32bit = AES192_32key;
        ctx.key_rounds = AES192_rounds;
    }
    // AES128
    else if (key_length == AES128_len)
    {
        ctx.key_length = AES128_len;
        ctx.key_exp_size = AES128_key_exp_size;
        ctx.key_32bit = AES128_32key;
        ctx.key_rounds = AES128_rounds;
    }
    else
    {
        LLOG_CRITICAL("Could not determine which AES size to use given a keylength of %u!", key_length);
        return result;
    }
    ctx.RoundKey = (uint8_t*)malloc(ctx.key_exp_size);
    // LLOG_DEBUG("length: %u  exp_size: %u  32bit: %u  rounds: %u", ctx.key_length, ctx.key_exp_size, ctx.key_32bit, ctx.key_rounds);

    AES_init_ctx_iv(&ctx, key_ptr, iv);

    if (data_length > MAX_PAYLOAD_SIZE)
    {
        LLOG_ERROR("Trying to xcrypt data with length longer than maximum of %d bytes given %d bytes.", MAX_PAYLOAD_SIZE, data_length);
        free(ctx.RoundKey);
        return result;
    }

    uint16_t new_data_length = data_length; // (key_length - (data_length % key_length)) + data_length;

    // if (new_data_length > MAX_PAYLOAD_SIZE)
    // {
    //     LLOG_ERROR("%d bytes segmented to key length of %d is longer than maximum of %d bytes (originally %d bytes).", new_data_length, key_length, MAX_PAYLOAD_SIZE, data_length);
    //     return result;
    // }

    // LLOG_DEBUG("here2");

    // more memory shenanagens
    memset(aes_crypt_ctr_buffer, 0, MAX_PAYLOAD_SIZE);
    memcpy(aes_crypt_ctr_buffer, data, data_length);

    AES_CTR_xcrypt_buffer(&ctx, aes_crypt_ctr_buffer, new_data_length);

    result.data = aes_crypt_ctr_buffer;
    result.length = new_data_length;

    // LLOG_DEBUG("here3");

    free(ctx.RoundKey);

    return result;
}