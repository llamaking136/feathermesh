#pragma once

#include <stdint.h>
#include <unistd.h>

class MemoryManager
{
private:

public:
     MemoryManager() {}
    ~MemoryManager() {}
    
    int get_free_memory();
};

extern MemoryManager memory_manager;