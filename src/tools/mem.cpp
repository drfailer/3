#include "mem.hpp"
#include <cassert>

static bool is_power_of_two(uintptr_t x) {
    return (x & (x-1)) == 0;
}

static uintptr_t align_forward(uintptr_t ptr, size_t align) {
    assert(is_power_of_two(align));
    uintptr_t a = (uintptr_t)align;
    uintptr_t mod = ptr & (a - 1);
    if (mod != 0) {
        ptr += a - mod;
    }
    return ptr;
}

ArenaRegion *arena_region_create(size_t size, ArenaRegion *prev = nullptr) {
    // TODO: might want to use malloc and allocate the buffer at the same time
    ArenaRegion *region = new ArenaRegion{};
    assert(region != nullptr && "failed to create arena region.");

    region->size = size;
    region->mem = new char[size];
    region->next = nullptr;
    region->prev = prev;
    assert(region->mem != nullptr && "failed to allocate arena region memory.");
    return region;
}

void arena_region_destroy(ArenaRegion *region) {
    delete[] region->mem;
    delete region;
}

void arena_region_alloc(ArenaRegion *region, size_t size, size_t align) {
    uintptr_t addr = align_forward((uintptr_t)&region->mem[region->pos], align);
    size_t pos = (size_t)(addr - (uintptr_t)region->mem);

    if (pos + size <= region->size) {
        void *ptr = &region->mem[pos];
        region->pos = pos + size;
        memset(ptr, 0, size);
        return ptr;
    }
    return nullptr;
}

Arena arena_create(size_t default_region_size) {
    Arena arena;
    arena.default_region_size = default_region_size;
    arena.head = arena_region_create(default_region_size);
    arena.tail = arena.head;
    return arena;
}

void arena_destroy(Arena *arena) {
    ArenaRegion *cur = arena->head;
    while (cur != nullptr) {
        ArenaRegion *next = cur->next;
        arena_region_destroy(cur);
        cur = next;
    }
}

// TODO: the current implemantation loops through all the regions to find the
//       first one that has sufficient space for the allocation. It would be
//       better if the head pointer was moved whenever a region is full.
void *arena_alloc(Arena *arena, size_t size, size_t align = ARENA_DEFAULT_ALIGH) {
    ArenaRegion *region = nullptr;

    // look for a region that can hold this size
    // TODO: move the head when a region is nearly full
    if (size < arena->default_region_size) {
        region = arena->head;
        while (region != nullptr) {
            void *ptr = region->alloc(size, align);
            if (ptr != nullptr) {
                return (T*)ptr;
            }
            region = region->next;
        }
    }
    // allocate a new region
    region = new ArenaRegion(std::max(arena->region_size, size));
    arena->tail->next = region;
    region->prev = arena->tail;
    arena->tail = region;
    return (void*)region->alloc(size * sizeof(T), align);
}
