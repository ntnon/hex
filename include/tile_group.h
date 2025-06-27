   #include "tile.h"

   #define TILE_GROUP_ARRAY_INITIAL_CAPACITY 8

   // Structure definition
   typedef struct {
       int id;
       tile **data;
       int count;
   } tile_group;

   typedef struct {
       int count;
       tile_group **data;
       int capacity;
   } tile_group_array;

   // Initialization and destruction
   void tile_group_init(tile_group* group, int id, tile** tiles, int tileCount);
   void tile_group_free(tile_group* group);

   tile_group_array tile_group_array_create();
   void tile_group_array_free(tile_group_array* array);

   // Utility functions
   void tile_group_print(const tile_group* group);
   void tile_group_validate(const tile_group* group);

   // Modification functions
   void tile_group_add_tile(tile_group* group, tile* tile);
   void tile_group_remove_tile(tile_group* group, tile* tile);

   // Trigger functions
   void tile_group_trigger_all(const tile_group* group);
