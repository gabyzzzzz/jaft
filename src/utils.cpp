#include "libraries.h"
#include "../lib/jaft.h"

template<typename T> void jaft::free_vector(T*& ptr) {
    free(ptr);
    ptr = NULL;
}

template<typename T> void jaft::free_matrix(T**& ptr, int lines) {
    for (int i = 0; i < lines; i++) free_vector(ptr[i]);
    free(ptr);
    ptr = NULL;
}

template<typename T> void jaft::free_tensor(T***& ptr, int depth, int lines) {
    for (int i = 0; i < depth; i++) free_matrix(ptr[i], lines);
    free(ptr);
    ptr = NULL;
}


void* jaft::allocate_vector(int length, size_t type_size) {
    void* ptr = malloc(length * type_size);
    if (ptr == NULL) throw std::bad_alloc();
    return ptr;
}

void** jaft::allocate_matrix(int lines, int rows, size_t type_size) {
    void** ptr = (void**) malloc(lines * sizeof(void*));
    if (!ptr) throw std::bad_alloc();
    for (int i = 0; i < lines; i++) {
        try {
            ptr[i] = allocate_vector(rows, type_size);
        }
        catch(const std::exception& e){
            for (int j = 0; j < i; j++)  free_vector(ptr[j]);
            free(ptr);
            throw;
        }
    }
    return ptr;
}

void*** jaft::allocate_tensor(int depth, int lines, int rows, size_t type_size) {
    void*** ptr = (void***) malloc(depth * sizeof(void**));
    if (!ptr) throw std::bad_alloc();
    for (int i = 0; i < depth; i++) {
        try {
            ptr[i] = allocate_matrix(lines, rows, type_size);
        }
        catch(const std::exception& e) {
            for (int j = 0; j < i; j++) free_matrix(ptr[j], lines);
            free(ptr);
            throw;
        }
    }
    return ptr;
}