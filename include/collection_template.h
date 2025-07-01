#ifndef COLLECTION_TEMPLATE_H
#define COLLECTION_TEMPLATE_H

#include <stdlib.h>
#include <stdio.h>

// Macro to declare a type-safe collection for any type.
// Place this in your .h file.
#define DECLARE_COLLECTION(TypeName) \
    typedef struct { \
        TypeName* data; \
        int count; \
        int capacity; \
    } TypeName##_collection_t; \
    \
    TypeName##_collection_t TypeName##_collection_create(int capacity); \
    void TypeName##_collection_free(TypeName##_collection_t* collection); \
    void TypeName##_collection_push(TypeName##_collection_t* collection, TypeName item); \
    void TypeName##_collection_clear(TypeName##_collection_t* collection);

// Macro to define the implementation of the collection functions.
// Place this in your .c file.
#define DEFINE_COLLECTION(TypeName) \
    TypeName##_collection_t TypeName##_collection_create(int capacity) { \
        TypeName##_collection_t c = {0}; \
        if (capacity <= 0) capacity = 16; \
        c.data = malloc(capacity * sizeof(TypeName)); \
        if (!c.data) { \
            fprintf(stderr, "Failed to create collection for " #TypeName "\n"); \
            return c; \
        } \
        c.capacity = capacity; \
        c.count = 0; \
        return c; \
    } \
    void TypeName##_collection_free(TypeName##_collection_t* collection) { \
        if (collection && collection->data) { \
            free(collection->data); \
            collection->data = NULL; \
            collection->count = 0; \
            collection->capacity = 0; \
        } \
    } \
    void TypeName##_collection_push(TypeName##_collection_t* collection, TypeName item) { \
        if (!collection) return; \
        if (collection->count >= collection->capacity) { \
            int new_capacity = collection->capacity * 2; \
            TypeName* new_data = realloc(collection->data, new_capacity * sizeof(TypeName)); \
            if (!new_data) { \
                fprintf(stderr, "Failed to resize collection for " #TypeName "\n"); \
                return; \
            } \
            collection->data = new_data; \
            collection->capacity = new_capacity; \
        } \
        collection->data[collection->count++] = item; \
    } \
    void TypeName##_collection_clear(TypeName##_collection_t* collection) { \
        if (collection) { \
            collection->count = 0; \
        } \
    }

#endif // COLLECTION_TEMPLATE_H
