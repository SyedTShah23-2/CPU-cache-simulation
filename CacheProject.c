#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define _CRT_SECURE_NO_WARNING
#define MAX_LINE_LENGTH 100


// Cache block structure
typedef struct {
    bool valid;
    bool dirty;
    unsigned long tag;
    unsigned long last_used; // For LRU replacement
    char* data;
} CacheBlock;


// Cache structure
typedef struct {
    int size;
    int block_size;
    int associativity;
    int num_sets;
    int num_blocks_per_set;
    bool is_unified;
    bool is_write_back;
    bool is_write_allocate;
    CacheBlock** blocks;
    int hits;
    int misses;
} Cache;

void initCacheBlocks(const int block_size, const Cache* cache) {
    for (int i = 0; i < cache->num_sets; i++) {
        cache->blocks[i] = (CacheBlock*)malloc(cache->num_blocks_per_set * sizeof(CacheBlock));
        for (int j = 0; j < cache->num_blocks_per_set; j++) {
            cache->blocks[i][j].valid = false;
            cache->blocks[i][j].dirty = false;
            cache->blocks[i][j].tag = 0;
            cache->blocks[i][j].last_used = 0;
            cache->blocks[i][j].data = (char*)malloc(block_size * sizeof(char));
        }
    }
}

// Function to initialize cache
Cache* initCache(const int size, const int block_size, const int associativity,
                 const bool is_unified, const bool is_write_back, const bool is_write_allocate) {
    Cache* cache = malloc(sizeof(Cache));

    //Here, just assign all missing parameters needed and counters for hit and miss
    //Write your code here.

    cache->size = size;
    cache->block_size = block_size;
    cache->associativity = associativity;
    //*
    cache->is_unified = is_unified;
    cache->is_write_back = is_write_back;
    cache->is_write_allocate = is_write_allocate;
    cache->hits = 0;
    cache->misses = 0;
    cache->num_blocks_per_set = associativity == 0 ? 1 : associativity;
    cache->num_sets = size / (block_size * cache->num_blocks_per_set);
    cache->blocks = (CacheBlock**)malloc(cache->num_sets * sizeof(CacheBlock*));
    initCacheBlocks(block_size, cache);
    return cache;
}


// Function to free cache memory
void freeCache(Cache* cache) {
    for (int i = 0; i < cache->num_sets; i++) {
        for (int j = 0; j < cache->num_blocks_per_set; j++) {
            free(cache->blocks[i][j].data);
        }
        //Write the missing code to free cache
        //Write code here

        free(cache->blocks[i]);

    }
    free(cache->blocks);
    free(cache);
}


// Function to find least recently used block index
int findLRUBlock(const CacheBlock* blocks, const int num_blocks_per_set) {
    int lru_index = 0;
    unsigned long min_last_used = blocks[0].last_used;
    for (int i = 1; i < num_blocks_per_set; i++) {
        if (blocks[i].last_used < min_last_used) {
            min_last_used = blocks[i].last_used;
            lru_index = i;
        }
    }
    return lru_index;
}


// Function to simulate cache access
void accessCache(Cache* cache, const unsigned long address, const int operation) {
    const unsigned long tag = address / cache->block_size;
    const int set_index = tag % cache->num_sets;
    int block_index = -1;


    // Search for the block in the set
    for (int i = 0; i < cache->num_blocks_per_set; i++) {
        if (cache->blocks[set_index][i].valid && cache->blocks[set_index][i].tag == tag) {
            block_index = i;
            break;
        }
    }


    if (block_index != -1) {
        // Cache hit
        cache->hits++;
        cache->blocks[set_index][block_index].last_used++; // Update LRU
        if (operation == 1 && cache->is_write_back) {
            cache->blocks[set_index][block_index].dirty = true;
        }
    } else {
        // Cache miss
        cache->misses++;
        // Find LRU block index
        const int lru_index = findLRUBlock(cache->blocks[set_index], cache->num_blocks_per_set);
        // Replace the least recently used block with new block
        cache->blocks[set_index][lru_index].valid = true;
        cache->blocks[set_index][lru_index].tag = tag;
        cache->blocks[set_index][lru_index].last_used = 0; // Reset LRU
        cache->blocks[set_index][lru_index].dirty = operation == 1 && cache->is_write_back;
    }
}


// Run cache simulation with given parameters and return hit rate
float runCacheSimulation(int cache_size, int block_size, int associativity) {
    // Initialize cache
    Cache* cache = initCache(cache_size, block_size, associativity, true, true, true);


    // Read operations and addresses from trace file
    FILE* file = fopen("trace.txt", "r");
    if (file == NULL) {
        printf("Error opening file.\n");
        return -1;
    }
    char line[MAX_LINE_LENGTH];
    unsigned long address;
    int operation;
    while (fgets(line, MAX_LINE_LENGTH, file) != NULL) {
        sscanf(line, "%d %lx", &operation, &address);
        accessCache(cache, address, operation);
    }
    fclose(file);


    float hit_rate = (float)cache->hits / (cache->hits + cache->misses);
    freeCache(cache);
    return hit_rate;
}


int main() {
    // Define parameters for experiments
    int cache_sizes[] = { 8192, 16384, 32768, 65536 }; // Cache sizes in bytes
    int block_sizes[] = { 64, 128, 256, 512 }; // Block sizes in bytes
    int associativities[] = { 1, 2, 4, 8 }; // Associativity levels


    // Experiment 1: Block Size vs. Hit Rate
    printf("Experiment 1: Block Size vs. Hit Rate\n");
    printf("Block Size (bytes)\tHit Rate\n");
    for (int i = 0; i < sizeof(block_sizes) / sizeof(int); i++) {
        float hit_rate = runCacheSimulation(cache_sizes[0], block_sizes[i], associativities[0]);
        printf("%d\t\t\t%.2f%%\n", block_sizes[i], hit_rate * 100);
    }


    // Experiment 2: Associativity vs. Hit Rate
    printf("\nExperiment 2: Associativity vs. Hit Rate\n");
    printf("Associativity\t\tHit Rate\n");
    for (int i = 0; i < sizeof(associativities) / sizeof(int); i++) {
        float hit_rate = runCacheSimulation(cache_sizes[0], block_sizes[0], associativities[i]);
        printf("%d\t\t\t%.2f%%\n", associativities[i], hit_rate * 100);
    }


    return 0;
}





