#include <transmit_queue.h>

TransmitQueue::TransmitQueue()
{
    this->queue_ptr = (transmit_queue_record*)malloc(sizeof(transmit_queue_record) * TRANSMIT_QUEUE_LEN);
    memset(this->queue_ptr, 0, TRANSMIT_QUEUE_LEN);
    this->queue_length = 0;
}

bool TransmitQueue::move_indexes(uint8_t dest, uint8_t src)
{
    if (dest >= TRANSMIT_QUEUE_LEN ||
        src >= TRANSMIT_QUEUE_LEN)
        return false;
    
    memmove(&this->queue_ptr[dest], &this->queue_ptr[src], sizeof(transmit_queue_record));
    memset(&this->queue_ptr[src], 0, sizeof(transmit_queue_record));

    return true;
}

bool TransmitQueue::push(transmit_queue_record* ptr)
{
    if (this->queue_length >= TRANSMIT_QUEUE_LEN)
        return false;
    
    for (int32_t i = TRANSMIT_QUEUE_LEN - 1; i > -1; i--)
    {
        this->move_indexes(i + 1, i);
    }

    memcpy(&this->queue_ptr[0], ptr, sizeof(transmit_queue_record));
    this->queue_length++;

    return true;
}

bool TransmitQueue::pop(transmit_queue_record* result)
{
    if (this->queue_length <= 0)
        return false;
    
    memcpy(result, &this->queue_ptr[this->queue_length - 1], sizeof(transmit_queue_record));
    memset(&this->queue_ptr[this->queue_length - 1], 0, sizeof(transmit_queue_record));
    this->queue_length--;

    // printnhex((uint8_t*)this->queue_ptr, sizeof(transmit_queue_record) * TRANSMIT_QUEUE_LEN, 0);
    
    return true;
}