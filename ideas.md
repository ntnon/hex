# Rule System Ontology (Refined)

## 1. Scopes (What the effect applies to)
Defines the *target domain* of the rule.

- **Local**
  - `Tile`
  - `Pool`
- **Global**
  - `All tiles`
  - `All pools`
  - `All tiles of <color>`
  - `All pools of <color>`
- **Future / Piece-based**
  - `Add tile to piece`
  - `Remove tile from piece`
  - `Boost single tile in piece`
  - `Boost all tiles in piece`

👉 Programmable idea:
`Scope(EntityType, Filter)`
Example:
`Scope(Tile, {color: "red"})`

---

## 2. Conditions (When the rule applies)
Defines **predicates** evaluated against entities or scopes.

- **Adjacency-based**
  - `Tile adjacency`
  - `Pool adjacency`
- **Intrinsic properties**
  - `Tile color`
  - `Pool color`
  - `Pool size`
  - `Pool center`
- **Spatial metrics**
  - `Pool max distance` (span of pool)
  - `Pool compactness` (shared edges density)

👉 Programmable idea:
`Condition(type, parameters)`
Example:
`Condition(Adjacency, {entity: Tile, color: "blue"})`

---

## 3. Effects (What happens when conditions are satisfied)
Defines **state changes** to entities.

- **Production**
  - `Increase production (flat)`
  - `Increase production (percentage)`
  - `Decrease production (flat)`
  - `Decrease production (percentage)`
- **Products**
  - `Add product`
  - `Remove product`
- **Entity properties**
  - `Change color`
  - `Remove entity`
  - `Move entity`
- **Structural**
  - `Add tile`
  - `Remove tile`
  - `Boost tile`

👉 Programmable idea:
`Effect(type, parameters)`
Example:
`Effect(ChangeColor, {entity: Pool, newColor: "green"})`

---

## 4. Modifiers (How/when the effect is applied)
Defines **temporal and probabilistic gates**.

- **Temporal**
  - `Recurring` (for each n-th loop/cycle)
  - `One-time` (per entity or per cycle)
- **Conditional Gates**
  - `Condition-based` (e.g., “green > 20% of total production”)
  - `Luck-based` (random roll or probability)

👉 Programmable idea:
`Modifier(type, parameters)`
Example:
`Modifier(Recurring, {interval: 3})`

---

## 5. Special Entities / Meta concepts
Not rules themselves, but *game-specific primitives* that rules may reference.

- **Stone tile** → `Tile(type: dead, production: 0)`
- **Paintbrush** → `Tool(entity: reskin, scope: Tile/Pool)`

---

## 6. Full Rule Example
```json
{
  "scope": { "entity": "Pool", "filter": { "color": "blue" } },
  "condition": { "type": "Adjacency", "params": { "entity": "Tile", "color": "red" } },
  "effect": { "type": "IncreaseProduction", "params": { "amount": 2, "mode": "flat" } },
  "modifier": { "type": "Recurring", "params": { "interval": 5 } }
}


determine pool modifier by:
- Identifying which tiles have the most same-colored neighbors
- Multiply the number of same-colored neighbors by the number of tiles in the pool

Create rules that determine tile-modifiers

Create rules that modify pool-modifiers

Local rules

Global rules

Rules for tile-factory?

How to implement these rules? as "relics"
How to give the player "relics"?
how to give the player tiles?
- Select from a random set
- What are tiles?
 - pair of tiles? shapes made out of tiles?
 - tile factory?
 - Modify rules for tile factory?

- Special tiles? Like "tile remover" or tiles WITH rules?


Arbitrary grid cell shape (polygons): triangle, tetragon, pentagon, hexagons, etc.
remove ghost tiles (empty tiles) when cloning from preview to main
