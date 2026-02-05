#ifndef TOOLS_ARRAY
#define TOOLS_ARRAY
#include <cassert>
#include <vector>
#include <cstring>

template <typename T>
struct Array {
    T *ptr;
    size_t len;
    size_t cap;
    // TODO: allocator

    T &operator[](size_t i) {
        assert(i < len && "error: index out of bound.");
        return ptr[i];
    }

    T const &operator[](size_t i) const {
        assert(i < len && "error: index out of bound.");
        return ptr[i];
    }

    // TODO: it should be iterable!!!
    using iterator = T*;
    using const_iterator = T const*;
    iterator begin() { return ptr; }
    iterator end() { return ptr + len; }
    const_iterator begin() const { return ptr; }
    const_iterator end() const { return ptr + len; }
};

template <typename T>
Array<T> array_create(size_t default_len = 0, size_t default_cap = 0) {
    Array<T> array = {
        .ptr = nullptr,
        .len = default_len,
        .cap = std::max(default_len, default_cap),
    };

    if (array.cap) {
        array.ptr = new T[array.cap];
        assert(array.ptr != nullptr && "error: failed to allocate array.");
    }
    return array;
}

template <typename T>
void array_destroy(Array<T> *array) {
    delete[] array->ptr;
}

template <typename T>
void array_append(Array<T> *array, T const &elt) {
    if (array->len + 1 >= array->cap) {
        array->cap = std::max(array->cap * 2, (size_t)1);
        T *ptr = new T[array->cap];
        memmove(ptr, array->ptr, array->len * sizeof(T));
        delete[] array->ptr;
        array->ptr = ptr;
    }
    array->ptr[array->len] = elt;
    array->len += 1;
}

template <typename T>
Array<T> array_create_from_std_vector(std::vector<T> const &vector) {
    Array<T> array = array_create<T>(vector.size());
    memcpy(array.ptr, vector.data(), vector.size() * sizeof(T));
    return array;
}

#endif
