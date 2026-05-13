# games

Collection of small games.

## TANKS

This branch contains a dependency-free C++17 terminal version of **TANKS**.
Protect the base icon, destroy enemy tank icons, and clear waves for score bonuses.
The terminal renderer uses colored two-cell icons for tanks, the aiming target, shells, terrain, and the base so the whole arena is visible without extra assets.

### Icon legend

- `▲`, `▼`, `◀`, `▶`: your tank and current facing direction.
- `△`, `▽`, `◁`, `▷`: enemy tanks and their facing direction.
- `◎`: current aiming target tile.
- `•`: shell.
- `⌂`: base to protect.
- `▓▓`, `██`, `♣`: brick, steel, and hedge terrain.

### Controls

- `P` starts or restarts the game.
- `W`, `A`, `S`, `D` move your tank and aim its cannon.
- `F` fires a shell.
- `Q` quits.

### Build, test, and run

```sh
make
make test
make run
```
