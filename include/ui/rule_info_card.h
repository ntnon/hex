// #ifndef RULE_INFO_CARD_H
// #define RULE_INFO_CARD_H

// #include <stdint.h>
// #include <stddef.h>
// #include <stdbool.h>
// #include "../third_party/clay.h"
// #include "../game/rule_system.h"
// #include "../tile/tile.h"

// /* ============================================================================
//  * Rule Info Card System
//  *
//  * Standalone system for visually representing any rule with:
//  * - Icons/symbols for rule types
//  * - Human-readable descriptions
//  * - Visual styling based on rule properties
//  * - Flexible layout options
//  * ============================================================================ */

// /* Rule display configuration */
// typedef struct {
//     bool show_icons;               // Show rule type icons
//     bool show_descriptions;        // Show detailed text descriptions
//     bool show_conditions;          // Show when rule applies
//     bool show_effects;             // Show what rule does
//     bool compact_layout;           // Use smaller, condensed layout
//     float card_width;              // Fixed width (0 for auto-fit)
//     float max_height;              // Maximum card height
// } rule_card_config_t;

// /* Visual representation of a rule */
// typedef struct {
//     const char* type_icon;         // Main rule type icon (Unicode)
//     const char* scope_icon;        // Scope indicator icon
//     const char* target_icon;       // Target indicator icon
//     const char* name;              // Short rule name
//     const char* description;       // Full description
//     Clay_Color primary_color;      // Main card color
//     Clay_Color accent_color;       // Border/highlight color
//     uint32_t display_priority;     // Priority for sorting (higher = more important)
// } rule_display_info_t;

// /* Default configurations */
// extern const rule_card_config_t RULE_CARD_CONFIG_FULL;
// extern const rule_card_config_t RULE_CARD_CONFIG_COMPACT;
// extern const rule_card_config_t RULE_CARD_CONFIG_MINIMAL;

// /* ============================================================================
//  * Core Rule Display Functions
//  * ============================================================================ */

// /**
//  * @brief Generate display information for a rule
//  * @param rule The rule to analyze
//  * @return Complete display information structure
//  */
// rule_display_info_t rule_get_display_info(const rule_t *rule);

// /**
//  * @brief Render a single rule as an info card
//  * @param rule Rule to display
//  * @param config Display configuration
//  * @param element_id Clay element ID for the card
//  */
// void ui_render_rule_info_card(const rule_t *rule,
//                               const rule_card_config_t *config,
//                               Clay_ElementId element_id);

// /**
//  * @brief Render multiple rules in a list
//  * @param rules Array of rule pointers
//  * @param rule_count Number of rules
//  * @param config Display configuration
//  * @param container_id Clay element ID for the container
//  */
// void ui_render_rule_info_list(const rule_t **rules,
//                               uint32_t rule_count,
//                               const rule_card_config_t *config,
//                               Clay_ElementId container_id);

// /* ============================================================================
//  * Rule Analysis Functions
//  * ============================================================================ */

// /**
//  * @brief Get human-readable rule type name
//  * @param rule Rule to analyze
//  * @return Static string with rule type name
//  */
// const char* rule_get_type_name(const rule_t *rule);

// /**
//  * @brief Get scope description
//  * @param scope Rule scope
//  * @return Static string describing the scope
//  */
// const char* rule_get_scope_name(rule_scope_t scope);

// /**
//  * @brief Get target description
//  * @param target Rule target
//  * @return Static string describing the target
//  */
// const char* rule_get_target_name(rule_target_t target);

// /**
//  * @brief Get condition description
//  * @param condition_type Rule condition type
//  * @return Static string describing the condition
//  */
// const char* rule_get_condition_name(rule_condition_type_t condition_type);

// /**
//  * @brief Get effect description
//  * @param effect_type Rule effect type
//  * @return Static string describing the effect
//  */
// const char* rule_get_effect_name(rule_effect_type_t effect_type);

// /* ============================================================================
//  * Icon/Symbol Functions
//  * ============================================================================ */

// /**
//  * @brief Get Unicode icon for rule type
//  * @param rule Rule to get icon for
//  * @return Unicode symbol string
//  */
// const char* rule_get_type_icon(const rule_t *rule);

// /**
//  * @brief Get Unicode icon for rule scope
//  * @param scope Rule scope
//  * @return Unicode symbol string
//  */
// const char* rule_get_scope_icon(rule_scope_t scope);

// /**
//  * @brief Get Unicode icon for rule target
//  * @param target Rule target
//  * @return Unicode symbol string
//  */
// const char* rule_get_target_icon(rule_target_t target);

// /**
//  * @brief Get Unicode icon for rule condition
//  * @param condition_type Rule condition type
//  * @return Unicode symbol string
//  */
// const char* rule_get_condition_icon(rule_condition_type_t condition_type);

// /**
//  * @brief Get Unicode icon for rule effect
//  * @param effect_type Rule effect type
//  * @return Unicode symbol string
//  */
// const char* rule_get_effect_icon(rule_effect_type_t effect_type);

// /* ============================================================================
//  * Text Generation Functions
//  * ============================================================================ */

// /**
//  * @brief Generate full rule description
//  * @param rule Rule to describe
//  * @param buffer Output buffer
//  * @param buffer_size Size of output buffer
//  * @return Number of characters written
//  */
// int rule_generate_description(const rule_t *rule, char *buffer, size_t buffer_size);

// /**
//  * @brief Generate condition text
//  * @param rule Rule to describe condition for
//  * @param buffer Output buffer
//  * @param buffer_size Size of output buffer
//  * @return Number of characters written
//  */
// int rule_generate_condition_text(const rule_t *rule, char *buffer, size_t buffer_size);

// /**
//  * @brief Generate effect text with values
//  * @param rule Rule to describe effect for
//  * @param buffer Output buffer
//  * @param buffer_size Size of output buffer
//  * @return Number of characters written
//  */
// int rule_generate_effect_text(const rule_t *rule, char *buffer, size_t buffer_size);

// /**
//  * @brief Generate scope text
//  * @param rule Rule to describe scope for
//  * @param buffer Output buffer
//  * @param buffer_size Size of output buffer
//  * @return Number of characters written
//  */
// int rule_generate_scope_text(const rule_t *rule, char *buffer, size_t buffer_size);

// /* ============================================================================
//  * Color and Styling Functions
//  * ============================================================================ */

// /**
//  * @brief Get color scheme for a rule type
//  * @param rule Rule to get colors for
//  * @param primary Output for primary color
//  * @param accent Output for accent color
//  */
// void rule_get_colors(const rule_t *rule, Clay_Color *primary, Clay_Color *accent);

// /**
//  * @brief Get display priority for sorting rules
//  * @param rule Rule to get priority for
//  * @return Priority value (higher = more important)
//  */
// uint32_t rule_get_display_priority(const rule_t *rule);

// /**
//  * @brief Check if rule should be highlighted
//  * @param rule Rule to check
//  * @return true if rule has significant visual impact
//  */
// bool rule_is_visually_important(const rule_t *rule);

// /* ============================================================================
//  * Configuration Helpers
//  * ============================================================================ */

// /**
//  * @brief Create custom rule card configuration
//  * @param show_icons Include icons in display
//  * @param show_descriptions Include text descriptions
//  * @param compact Use compact layout
//  * @return Configuration structure
//  */
// rule_card_config_t rule_card_config_create(bool show_icons,
//                                            bool show_descriptions,
//                                            bool compact);

// /**
//  * @brief Sort rules by display priority
//  * @param rules Array of rule pointers to sort
//  * @param rule_count Number of rules
//  */
// void rule_sort_by_priority(const rule_t **rules, uint32_t rule_count);

// /* ============================================================================
//  * Utility Functions
//  * ============================================================================ */

// /**
//  * @brief Get tile type name for display
//  * @param tile_type Tile type enum value
//  * @return Human-readable tile type name
//  */
// const char* rule_get_tile_type_name(tile_type_t tile_type);

// /**
//  * @brief Get tile type color
//  * @param tile_type Tile type enum value
//  * @return Color representing the tile type
//  */
// Clay_Color rule_get_tile_type_color(tile_type_t tile_type);

// /**
//  * @brief Check if rule affects specific tile type
//  * @param rule Rule to check
//  * @param tile_type Tile type to check against
//  * @return true if rule can affect this tile type
//  */
// bool rule_affects_tile_type(const rule_t *rule, tile_type_t tile_type);

// #endif // RULE_INFO_CARD_H
