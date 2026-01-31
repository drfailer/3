#ifndef TOOLS_MEMORY_POOL
#define TOOLS_MEMORY_POOL
#include <cassert>
#include <cstring>
#include <cstdint>

#define ARENA_DEFAULT_ALIGH 2*sizeof(void*)

namespace mem {

// TODO: put implementation in a c file

struct ArenaRegion {
    size_t pos = 0;
    size_t size = 0;
    char *mem = nullptr;
    ArenaRegion *next = nullptr;
    ArenaRegion *prev = nullptr;
};

// TODO: I may not need all of this
ArenaRegion *arena_region_create(size_t size, ArenaRegion *prev = nullptr);
void arena_region_destroy(ArenaRegion *region);
void arena_region_alloc(ArenaRegion *region, size_t size, size_t align);

// TODO: improve this implementation
struct Arena {
    ArenaRegion *head = nullptr;
    ArenaRegion *tail = nullptr;
    size_t default_region_size = 0;
};

Arena arena_create(size_t default_region_size);
void arena_destory(Arena *arena);
void *arena_alloc(Arena *arena, size_t size, size_t align);

/*
 * Helper function used to allocate elements of specific types.
 */
template <typename T>
T *arena_alloc(Arena *arena, size_t size = 1, size_t align = ARENA_DEFAULT_ALIGH) {
    return (T*)arena_alloc(arena, size, align);
}

template <typename T>
struct MemPoolNode {
    T data = {};
    MemPoolNode<T> *prev = nullptr;
    MemPoolNode<T> *next = nullptr;
};

template <typename T>
struct MemPool {
    MemPoolNode<T> *free_list_head = nullptr;
    MemPoolNode<T> *used_list_head = nullptr;
};

template <typename T, typename ...Args>
void mem_pool_init(MemPool<T> *pool, size_t default_capacity = 0) {
    pool->free_list_head = nullptr;
    pool->used_list_head = nullptr;
    for (size_t i = 0; i < default_capacity; ++i) {
        auto node = new MemPoolNode<T>();
        node->next = pool->free_list_head;
        pool->free_list_head = node;
    }
}

template <typename T>
void mem_pool_destroy(MemPool<T> *pool) {
    auto delete_list = [](MemPoolNode<T> *head) {
        while (head != nullptr) {
            auto next = head->next;
            delete head;
            head = next;
        }
    };
    delete_list(pool->free_list_head);
    delete_list(pool->used_list_head);
}

template <typename T>
T *mem_pool_alloc(MemPool<T> *pool) {
    MemPoolNode<T> *node = nullptr;

    if (pool->free_list_head == nullptr) {
        pool->free_list_head = new MemPoolNode<T>();
    }
    node = pool->free_list_head;
    pool->free_list_head = node->next;
    node->next = pool->used_list_head;
    if (node->next != nullptr) {
        node->next->prev = node;
    }
    pool->used_list_head = node;
    return (T*)node;
}

template <typename T>
void mem_pool_release(MemPool<T> *pool, T *data) {
    auto node = (MemPoolNode<T>*)data;
    if (node->prev != nullptr) {
        node->prev->next = node->next;
    } else {
        assert(pool->used_list_head = node);
        pool->used_list_head = node->next;
    }
    if (node->next != nullptr) {
        node->next->prev = node->prev;
    }
    node->prev = nullptr;
    node->next = pool->free_list_head;
    pool->free_list_head = node;
}

} // end namespace mem

#endif
