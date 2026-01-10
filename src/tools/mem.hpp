#ifndef TOOLS_MEMORY_POOL
#define TOOLS_MEMORY_POOL
#include <cassert>
#include <cstring>
#include <cstdint>

namespace mem {

// TODO: put implementation in a c file

struct ArenaRegion {
    size_t pos = 0;
    size_t mem_len;
    char *mem = nullptr;
    ArenaRegion *next = nullptr;
    ArenaRegion *prev = nullptr;

    ArenaRegion(size_t mem_len, ArenaRegion *prev = nullptr)
        : mem_len(mem_len), mem(new char[mem_len]), prev(prev) {}
    ~ArenaRegion() { delete[] mem; }

    bool is_power_of_two(uintptr_t x) {
        return (x & (x-1)) == 0;
    }

    uintptr_t align_forward(uintptr_t ptr, size_t align) {
        assert(is_power_of_two(align));
        uintptr_t a = (uintptr_t)align;
        uintptr_t mod = ptr & (a - 1);
        if (mod != 0) {
            ptr += a - mod;
        }
        return ptr;
    }

    void *alloc(size_t size, size_t align) {
        uintptr_t addr = align_forward((uintptr_t)&this->mem[this->pos], align);
        size_t pos = (size_t)(addr - (uintptr_t)this->mem);

        if (pos + size <= this->mem_len) {
            void *ptr = &this->mem[pos];
            this->pos = pos + size;
            memset(ptr, 0, size);
            return ptr;
        }
        return nullptr;
    }
};

struct Arena {
    ArenaRegion *head = nullptr;
    ArenaRegion *tail = nullptr;
    size_t region_size = 0;

    Arena(size_t region_size) : head(new ArenaRegion(region_size)), region_size(region_size) {
        this->tail = this->head;
    }

    ~Arena() {
        auto cur = this->head;
        while (cur != nullptr) {
            auto next = cur->next;
            delete cur;
            cur = next;
        }
    }

    template <typename T>
    T *alloc(size_t size = 1, size_t align = 2*sizeof(void*)) {
        assert(size <= this->region_size);

        auto region = this->head;
        while (region != nullptr) {
            void *ptr = region->alloc(size * sizeof(T), align);
            if (ptr != nullptr) {
                return (T*)ptr;
            }
            region = region->next;
        }

        region = new ArenaRegion(this->region_size);
        this->tail->next = region;
        region->prev = this->tail;
        this->tail = region;
        return (T*)region->alloc(size * sizeof(T), align);
    }
};

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
