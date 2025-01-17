#include <pb_helper.h>
#include <llog.h>

bool pb_helper_decode(uint8_t* buffer, uint16_t buffer_length, const pb_msgdesc_t* fields, void* dest_struct)
{
    pb_istream_t _istream = pb_istream_from_buffer(buffer, buffer_length);
    bool status = pb_decode(&_istream, fields, dest_struct);

    if (status == false)
    {
        LLOG_ERROR("Protobuf decode failed: '%s'", PB_GET_ERROR(&_istream));
    }

    return status;
}

bool pb_helper_encode(uint8_t* buffer, uint16_t buffer_length, const pb_msgdesc_t* fields, void* src_struct, uint16_t* bytes_written)
{
    pb_ostream_t _ostream = pb_ostream_from_buffer(buffer, buffer_length);
    bool status = pb_encode(&_ostream, fields, src_struct);

    *bytes_written = _ostream.bytes_written;
    if (status == false)
    {
        LLOG_ERROR("Protobuf encode failed: '%s'", PB_GET_ERROR(&_ostream));
        *bytes_written = 0;
    }

    return status;
}

bool pb_helper_encode_submessage(uint8_t* buffer, uint16_t buffer_length, const pb_msgdesc_t* fields, void* src_struct, uint16_t* bytes_written)
{
    pb_ostream_t _ostream = pb_ostream_from_buffer(buffer, buffer_length);
    bool status = pb_encode_submessage(&_ostream, fields, src_struct);

    *bytes_written = _ostream.bytes_written;
    if (status == false)
    {
        LLOG_ERROR("Protobuf submessage encode failed: '%s'", PB_GET_ERROR(&_ostream));
        *bytes_written = 0;
    }

    return status;
}