#include "game/resources.h"
void resources_init(resources_t *res) {
    for (int i = 0; i < TILE_TYPE_COUNT; i++) {
        res[i] = 0;
    }
}

int resources_sum(const resources_t *res) {
    int sum = 0;
    for (int i = 0; i < TILE_TYPE_COUNT; i++) {
        sum += res[i];
    }
    return sum;
}

void resources_add_single(resources_t *res, tile_type_t tile_type, int amount) {
    res[tile_type] += amount;
}

void resources_add(resources_t *res, resources_t *add) {
    for (int i = 0; i < TILE_TYPE_COUNT; i++) {
        resources_add_single(res, i, add[i]);
    }
}
