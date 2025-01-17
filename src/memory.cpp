#include <memory.h>
#include <stdlib.h>

#ifdef __arm__
// should use uinstd.h to define sbrk but Due causes a conflict
// extern "C" char* sbrk(int incr);
#include <unistd.h>
#else  // __ARM__
extern char *__brkval;
#endif  // __arm__

int MemoryManager::get_free_memory() {
    char top = 0;
    int free_mem = 0;
#ifdef __arm__
    free_mem = &top - reinterpret_cast<char*>(sbrk(0));
#elif defined(CORE_TEENSY) || (ARDUINO > 103 && ARDUINO != 151)
    free_mem = &top - __brkval;
#else  // __arm__
    free_mem = __brkval ? &top - __brkval : &top - __malloc_heap_start;
#endif  // __arm__

    return free_mem;
}

MemoryManager memory_manager;