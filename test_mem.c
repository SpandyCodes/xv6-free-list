#include "types.h"
#include "stat.h"
#include "user.h"
#include <stddef.h>

#define CHUNK_SIZE 4096 // Size of each memory chunk to allocate

int main() {
    char* chunks[100]; // Array to hold pointers to allocated memory chunks
    int i = 0;
    
    // Allocate memory chunks until the array becomes full
    for (; i < 100; i++) {
        chunks[i] = malloc(CHUNK_SIZE);
        if (chunks[i] == NULL) {
            printf(1, "Memory allocation failed at chunk %d\n", i);
            break;
        }
        printf(1, "Allocated chunk %d at address %p\n", i, chunks[i]);
    }
    
    // If the array became full, print a message
    if (i == 100) {
        printf(1, "Array is full. Falling back to linked list...\n");
    }
    
    // Continue allocating memory chunks to test linked list functionality
    for (; i < 200; i++) {
        chunks[i] = malloc(CHUNK_SIZE);
        if (chunks[i] == NULL) {
            printf(1, "Memory allocation failed at chunk %d\n", i);
            break;
        }
        printf(1, "Allocated chunk %d at address %p\n", i, chunks[i]);
    }
    
    // Free all allocated memory chunks
    for (int j = 0; j < i; j++) {
        free(chunks[j]);
        printf(1, "Freed chunk %d at address %p\n", j, chunks[j]);
    }
    
    exit();
}
