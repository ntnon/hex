#include "tile/tile.h"

typedef int resources_t;

resources_t resources[TILE_TYPE_COUNT];

void resources_init(resources_t *res);

int resources_sum(const resources_t *resources);

void resources_add(resources_t *res, resources_t *add);

void resources_add_single(resources_t *res, tile_type_t type, int amount);
