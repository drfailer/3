#ifndef TOOLS_MEMORY_POOL
#define TOOLS_MEMORY_POOL
#include <cassert>
#include <cstring>
#include <cstdint>
#include <cstdlib>

#define DEFAULT_ALIGN 2*sizeof(void*)
#define DEFAULT_ALLOCATOR Allocator{   \
        .alloc = heap_allocator_alloc, \
        .free = heap_allocator_free,   \
        .data = nullptr,               \
    }

/******************************************************************************/
/*                                 allocator                                  */
/******************************************************************************/

struct Allocator {
    void *(*alloc)(void*, size_t, size_t);
    void (*free)(void*, void*);
    void *data;
};

inline void *alloc(Allocator &allocator, size_t size, size_t align = DEFAULT_ALIGN) {
    return allocator.alloc(allocator.data, size, align);
}

template <typename T>
inline T *alloc(Allocator &allocator, size_t size = 1, size_t align = DEFAULT_ALIGN) {
    return (T*)allocator.alloc(allocator.data, sizeof(T) * size, align);
}

inline void free(Allocator &allocator, void *ptr) {
    allocator.free(allocator.data, ptr);
}

/******************************************************************************/
/*                               heap allocator                               */
/******************************************************************************/

inline void *heap_allocator_alloc(void*, size_t size, size_t) {
    return malloc(size);
}

inline void heap_allocator_free(void*, void *ptr) {
    free(ptr);
}

/******************************************************************************/
/*                                   arena                                    */
/******************************************************************************/

struct ArenaRegion {
    size_t pos = 0;
    size_t size = 0;
    char *mem = nullptr;
    ArenaRegion *next = nullptr;
    ArenaRegion *prev = nullptr;
};

// TODO: I may not need all of this
ArenaRegion *arena_region_create(size_t size);
void  arena_region_destroy(ArenaRegion *region);
void *arena_region_alloc(ArenaRegion *region, size_t size, size_t align);

// TODO: improve this implementation
struct Arena {
    ArenaRegion *head = nullptr;
    ArenaRegion *tail = nullptr;
    size_t default_region_size = 0;
};

#define ARENA_DEFAULT_REGION_SIZE (1024*1024)
Arena arena_create(size_t default_region_size = ARENA_DEFAULT_REGION_SIZE);
void arena_destroy(Arena *arena);
void *arena_alloc(Arena *arena, size_t size, size_t align);

inline void  arena_allocator_free(void*, void*) {} // TODO: we may want to be able to free the last element
inline void *arena_allocator_alloc(void *arena, size_t size, size_t align) {
    return arena_alloc((Arena*)arena, size, align);
}
inline Allocator arena_allocator(Arena *arena) {
    return Allocator{
        .alloc = arena_allocator_alloc,
        .free = arena_allocator_free,
        .data = arena,
    };
}

/******************************************************************************/
/*                                    pool                                    */
/******************************************************************************/

/*
 * Helper function used to allocate elements of specific types.
 */
template <typename T>
T *arena_alloc(Arena *arena, size_t size = 1, size_t align = DEFAULT_ALIGN) {
    return (T*)arena_alloc(arena, size, align);
}

template <typename T>
struct MemPoolAst {
    T data = {};
    MemPoolAst<T> *prev = nullptr;
    MemPoolAst<T> *next = nullptr;
};

template <typename T>
struct MemPool {
    MemPoolAst<T> *free_list_head = nullptr;
    MemPoolAst<T> *used_list_head = nullptr;
};

template <typename T, typename ...Args>
void mem_pool_init(MemPool<T> *pool, size_t default_capacity = 0) {
    pool->free_list_head = nullptr;
    pool->used_list_head = nullptr;
    for (size_t i = 0; i < default_capacity; ++i) {
        auto ast = new MemPoolAst<T>();
        ast->next = pool->free_list_head;
        pool->free_list_head = ast;
    }
}

template <typename T>
void mem_pool_destroy(MemPool<T> *pool) {
    auto delete_list = [](MemPoolAst<T> *head) {
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
T *mem_pool_alloc(MemPool<T> *pool, T const &value = {}) {
    MemPoolAst<T> *ast = nullptr;

    if (pool->free_list_head == nullptr) {
        pool->free_list_head = new MemPoolAst<T>();
    }
    ast = pool->free_list_head;
    pool->free_list_head = ast->next;
    ast->next = pool->used_list_head;
    if (ast->next != nullptr) {
        ast->next->prev = ast;
    }
    pool->used_list_head = ast;
    ast->data = value;
    return (T*)ast;
}

template <typename T>
void mem_pool_release(MemPool<T> *pool, T *data) {
    auto ast = (MemPoolAst<T>*)data;
    if (ast->prev != nullptr) {
        ast->prev->next = ast->next;
    } else {
        assert(pool->used_list_head = ast);
        pool->used_list_head = ast->next;
    }
    if (ast->next != nullptr) {
        ast->next->prev = ast->prev;
    }
    ast->prev = nullptr;
    ast->next = pool->free_list_head;
    pool->free_list_head = ast;
}

#endif
