#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <array>
#include <vector>
#include <cmath>
#include <string>
#include <algorithm>
#include <random>
#include <optional>
#include <filesystem>
#include <iostream>
#include <limits>

// ─── Constants ───────────────────────────────────────────────────────────────
const int TILE    = 20;
const int COLS    = 28;
const int ROWS    = 31;
const int WIN_W   = COLS * TILE;          // 560
const int WIN_H   = ROWS * TILE + 60;     // 680  (60px UI bar at bottom)
const float PI    = 3.14159265f;
const float TILE_CENTER = TILE * 0.5f;
const float ENTITY_RADIUS = 7.f;
const sf::Vector2i GHOST_HOME_TILE{13, 13};

// ─── Map ─────────────────────────────────────────────────────────────────────
// 0=empty  1=wall  2=dot  3=power-pellet  4=ghost-door
const int MAP[ROWS][COLS] = {
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,2,2,2,2,2,2,2,2,2,2,2,2,1,1,2,2,2,2,2,2,2,2,2,2,2,2,1},
    {1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1},
    {1,3,1,1,1,1,2,1,1,1,1,1,2,1,1,2,1,1,1,1,1,2,1,1,1,1,3,1},
    {1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1},
    {1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1},
    {1,2,1,1,1,1,2,1,1,2,1,1,1,1,1,1,1,1,2,1,1,2,1,1,1,1,2,1},
    {1,2,1,1,1,1,2,1,1,2,1,1,1,1,1,1,1,1,2,1,1,2,1,1,1,1,2,1},
    {1,2,2,2,2,2,2,1,1,2,2,2,2,1,1,2,2,2,2,1,1,2,2,2,2,2,2,1},
    {1,1,1,1,1,1,2,1,1,1,1,1,0,1,1,0,1,1,1,1,1,2,1,1,1,1,1,1},
    {1,1,1,1,1,1,2,1,1,1,1,1,0,1,1,0,1,1,1,1,1,2,1,1,1,1,1,1},
    {1,1,1,1,1,1,2,1,1,0,0,0,0,0,0,0,0,0,0,1,1,2,1,1,1,1,1,1},
    {1,1,1,1,1,1,2,1,1,0,1,1,1,4,4,1,1,1,0,1,1,2,1,1,1,1,1,1},
    {1,1,1,1,1,1,2,1,1,0,1,0,0,0,0,0,0,1,0,1,1,2,1,1,1,1,1,1},
    {0,0,0,0,0,0,2,0,0,0,1,0,0,0,0,0,0,1,0,0,0,2,0,0,0,0,0,0},
    {1,1,1,1,1,1,2,1,1,0,1,0,0,0,0,0,0,1,0,1,1,2,1,1,1,1,1,1},
    {1,1,1,1,1,1,2,1,1,0,1,1,1,1,1,1,1,1,0,1,1,2,1,1,1,1,1,1},
    {1,1,1,1,1,1,2,1,1,0,0,0,0,0,0,0,0,0,0,1,1,2,1,1,1,1,1,1},
    {1,1,1,1,1,1,2,1,1,0,1,1,1,1,1,1,1,1,0,1,1,2,1,1,1,1,1,1},
    {1,1,1,1,1,1,2,1,1,0,1,1,1,1,1,1,1,1,0,1,1,2,1,1,1,1,1,1},
    {1,2,2,2,2,2,2,2,2,2,2,2,2,1,1,2,2,2,2,2,2,2,2,2,2,2,2,1},
    {1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1},
    {1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1},
    {1,3,2,2,1,1,2,2,2,2,2,2,2,0,0,2,2,2,2,2,2,2,1,1,2,2,3,1},
    {1,1,1,2,1,1,2,1,1,2,1,1,1,1,1,1,1,1,2,1,1,2,1,1,2,1,1,1},
    {1,1,1,2,1,1,2,1,1,2,1,1,1,1,1,1,1,1,2,1,1,2,1,1,2,1,1,1},
    {1,2,2,2,2,2,2,1,1,2,2,2,2,1,1,2,2,2,2,1,1,2,2,2,2,2,2,1},
    {1,2,1,1,1,1,1,1,1,1,1,1,2,1,1,2,1,1,1,1,1,1,1,1,1,1,2,1},
    {1,2,1,1,1,1,1,1,1,1,1,1,2,1,1,2,1,1,1,1,1,1,1,1,1,1,2,1},
    {1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
};

// ─── Types ────────────────────────────────────────────────────────────────────
enum class Dir { None, Left, Right, Up, Down };

sf::Vector2f dirVec(Dir d) {
    switch (d) {
        case Dir::Left:  return {-1,  0};
        case Dir::Right: return { 1,  0};
        case Dir::Up:    return { 0, -1};
        case Dir::Down:  return { 0,  1};
        default:         return { 0,  0};
    }
}

Dir opposite(Dir d) {
    switch (d) {
        case Dir::Left:  return Dir::Right;
        case Dir::Right: return Dir::Left;
        case Dir::Up:    return Dir::Down;
        case Dir::Down:  return Dir::Up;
        default:         return Dir::None;
    }
}

// ─── Helpers ─────────────────────────────────────────────────────────────────
int wrapCol(int col) {
    if (col < 0) return COLS - 1;
    if (col >= COLS) return 0;
    return col;
}

int tileIndex(float pos) {
    return static_cast<int>(std::floor(pos / TILE));
}

float tileCenter(int index) {
    return index * TILE + TILE_CENTER;
}

bool isWall(int col, int row, bool ghostMode = false) {
    if (row < 0 || row >= ROWS) return true;
    col = wrapCol(col);
    int t = MAP[row][col];
    if (t == 1) return true;
    if (!ghostMode && t == 4) return true; // ghost door blocks pacman
    return false;
}

bool canOccupy(float px, float py, bool ghostMode = false, float radius = ENTITY_RADIUS) {
    auto blocked = [&](float x, float y) {
        int c = wrapCol(tileIndex(x));
        int r = tileIndex(y);
        return isWall(c, r, ghostMode);
    };

    return !blocked(px - radius, py - radius)
        && !blocked(px + radius, py - radius)
        && !blocked(px - radius, py + radius)
        && !blocked(px + radius, py + radius);
}

bool canAdvance(float px, float py, Dir d, float distance, bool ghostMode = false,
                float radius = ENTITY_RADIUS) {
    auto dv = dirVec(d);
    return canOccupy(px + dv.x * distance, py + dv.y * distance, ghostMode, radius);
}

bool canMoveDir(float px, float py, Dir d, bool ghostMode = false,
                float radius = ENTITY_RADIUS) {
    return canAdvance(px, py, d, TILE_CENTER + 1.f, ghostMode, radius);
}

float moveEntity(float& px, float& py, Dir d, float distance, bool ghostMode = false,
                 float radius = ENTITY_RADIUS) {
    if (d == Dir::None || distance <= 0.f) return 0.f;

    auto dv = dirVec(d);
    float moved = 0.f;
    float remaining = distance;
    while (remaining > 0.f) {
        float step = std::min(1.f, remaining);
        if (!canAdvance(px, py, d, step, ghostMode, radius)) break;
        px += dv.x * step;
        py += dv.y * step;
        moved += step;
        remaining -= step;
    }
    return moved;
}

// Snap to grid centre if within threshold
float snapAxis(float v) {
    return std::round((v - TILE_CENTER) / TILE) * TILE + TILE_CENTER;
}

bool nearCentre(float v) {
    return std::abs(v - snapAxis(v)) < 3.0f;
}

bool approxEq(float a, float b, float eps = 0.01f) {
    return std::abs(a - b) <= eps;
}

// ─── Drawing helpers ──────────────────────────────────────────────────────────

// Draw pac-man as a pie shape
void drawPacman(sf::RenderTarget& rt, float x, float y, float angle, float mouthAngle) {
    const int POINTS = 30;
    sf::VertexArray va(sf::PrimitiveType::TriangleFan, POINTS + 2);
    va[0].position  = {x, y};
    va[0].color     = sf::Color::Yellow;
    float startA = angle + mouthAngle;
    float endA   = angle + (2.f * PI - mouthAngle);
    float r = 9.f;
    for (int i = 0; i <= POINTS; ++i) {
        float a = startA + (endA - startA) * ((float)i / POINTS);
        va[i + 1].position = {x + r * std::cos(a), y + r * std::sin(a)};
        va[i + 1].color    = sf::Color::Yellow;
    }
    rt.draw(va);
}

// Draw ghost body
void drawGhost(sf::RenderTarget& rt, float x, float y, sf::Color col, bool frightened, float time) {
    sf::Color body = frightened ? sf::Color(0, 0, 200) : col;
    if (frightened && std::fmod(time, 0.5f) < 0.25f && time < 2.f)
        body = sf::Color::White; // flash warning

    float r = 9.f;
    // Upper dome
    const int DOME = 16;
    sf::VertexArray dome(sf::PrimitiveType::TriangleFan, DOME + 2);
    dome[0].position = {x, y};
    dome[0].color = body;
    for (int i = 0; i <= DOME; ++i) {
        float a = PI + PI * ((float)i / DOME);
        dome[i + 1].position = {x + r * std::cos(a), y + r * std::sin(a)};
        dome[i + 1].color = body;
    }
    rt.draw(dome);

    // Rectangle body
    sf::RectangleShape rect({r * 2.f, r});
    rect.setOrigin({r, 0.f});
    rect.setPosition({x, y});
    rect.setFillColor(body);
    rt.draw(rect);

    // Wavy bottom
    float bx = x - r;
    float by = y + r;
    const int WAVES = 3;
    float ww = (r * 2.f) / WAVES;
    for (int i = 0; i < WAVES; ++i) {
        sf::CircleShape bump(ww * 0.5f);
        bump.setFillColor(sf::Color(0, 0, 0));
        bump.setPosition({bx + i * ww, by - ww * 0.5f});
        rt.draw(bump);
    }

    if (!frightened) {
        // Eyes
        sf::CircleShape eyeWhite(3.f);
        eyeWhite.setFillColor(sf::Color::White);
        eyeWhite.setOrigin({3.f, 3.f});
        eyeWhite.setPosition({x - 3.5f, y - 3.f});
        rt.draw(eyeWhite);
        eyeWhite.setPosition({x + 3.5f, y - 3.f});
        rt.draw(eyeWhite);

        sf::CircleShape pupil(1.5f);
        pupil.setFillColor(sf::Color(0, 0, 200));
        pupil.setOrigin({1.5f, 1.5f});
        pupil.setPosition({x - 3.f, y - 3.f});
        rt.draw(pupil);
        pupil.setPosition({x + 4.f, y - 3.f});
        rt.draw(pupil);
    } else {
        // Scared face
        sf::RectangleShape mouth({r * 1.2f, 2.f});
        mouth.setOrigin({r * 0.6f, 1.f});
        mouth.setPosition({x, y + 2.f});
        mouth.setFillColor(sf::Color::White);
        rt.draw(mouth);
    }
}

// ─── Ghost ────────────────────────────────────────────────────────────────────
enum class GhostMode { Chase, Scatter, Frightened, Dead };

struct Ghost {
    float x, y;
    Dir   dir = Dir::Left;
    sf::Color color;
    GhostMode mode  = GhostMode::Scatter;
    float frighTimer = 0.f;
    float scatterTimer = 0.f;
    float deadTimer  = 0.f;
    sf::Vector2i scatterTarget;
    int id; // 0=Blinky 1=Pinky 2=Inky 3=Clyde

    float speed() const {
        if (mode == GhostMode::Dead)       return 200.f;
        if (mode == GhostMode::Frightened) return 80.f;
        return 110.f;
    }

    sf::Vector2i tilePos() const {
        return {wrapCol(tileIndex(x)), tileIndex(y)};
    }

    void respawn() {
        static constexpr float startX[] = {13.5f, 13.5f, 11.5f, 15.5f};
        static constexpr float startY[] = {11.5f, 14.5f, 14.5f, 14.5f};
        x = startX[id] * TILE;
        y = startY[id] * TILE;
        dir = Dir::Left;
        mode = GhostMode::Scatter;
        frighTimer = 0.f;
        scatterTimer = 6.f;
        deadTimer = 0.f;
    }
};

// ─── PacMan ───────────────────────────────────────────────────────────────────
struct Pacman {
    float x, y;
    Dir   dir     = Dir::None;
    Dir   nextDir = Dir::None;
    float mouthT  = 0.f; // 0..1
    bool  mouthOpen = true;
    float speed   = 140.f;

    sf::Vector2i tilePos() const {
        return {wrapCol(tileIndex(x)), tileIndex(y)};
    }
};

// ─── Game State ───────────────────────────────────────────────────────────────
enum class GameState { Menu, Playing, Dead, LevelClear, GameOver };

struct Game {
    int dots[ROWS][COLS];
    int score = 0;
    int lives = 3;
    int level = 1;
    GameState state = GameState::Menu;
    float stateTimer = 0.f;
    int totalDots = 0;
    int eatenDots = 0;
    float frightTime = 0.f; // global frightened time remaining
    int ghostEatCombo = 0;  // combo multiplier for eating ghosts

    Pacman pac;
    Ghost  ghosts[4];

    void initDots() {
        eatenDots = 0; totalDots = 0;
        for (int r = 0; r < ROWS; ++r)
            for (int c = 0; c < COLS; ++c) {
                dots[r][c] = MAP[r][c];
                if (dots[r][c] == 2 || dots[r][c] == 3) ++totalDots;
            }
    }

    void reset(bool fullReset = false) {
        if (fullReset) { score = 0; lives = 3; level = 1; }
        initDots();
        frightTime = 0.f;
        ghostEatCombo = 0;

        // Pac-Man start position
        pac.x = 13.5f * TILE;
        pac.y = 23.5f * TILE;
        pac.dir = Dir::None; pac.nextDir = Dir::None;
        pac.mouthT = 0.f;

        // Ghost starting positions and colors
        float gx[] = {13.5f, 13.5f, 11.5f, 15.5f};
        float gy[] = {11.5f, 14.5f, 14.5f, 14.5f};
        sf::Color gc[] = {sf::Color::Red, sf::Color(255,180,255), sf::Color(0,255,255), sf::Color(255,165,0)};
        sf::Vector2i scatter[] = {{25,0},{2,0},{27,29},{0,29}};
        for (int i = 0; i < 4; ++i) {
            ghosts[i].x  = gx[i] * TILE;
            ghosts[i].y  = gy[i] * TILE;
            ghosts[i].color = gc[i];
            ghosts[i].scatterTarget = scatter[i];
            ghosts[i].id  = i;
            ghosts[i].dir = Dir::Left;
            ghosts[i].mode = GhostMode::Scatter;
            ghosts[i].frighTimer = 0.f;
            ghosts[i].scatterTimer = 6.f;
            ghosts[i].deadTimer = 0.f;
        }
    }
};

// ─── Ghost AI ─────────────────────────────────────────────────────────────────
int tileDistSq(sf::Vector2i a, sf::Vector2i b) {
    int dx = a.x - b.x, dy = a.y - b.y;
    return dx*dx + dy*dy;
}

Dir bestDirToward(float gx, float gy, Dir curDir, sf::Vector2i target, bool ghostMode = true,
                  bool allowReverse = false) {
    Dir dirs[] = {Dir::Up, Dir::Down, Dir::Left, Dir::Right};
    Dir best = Dir::None;
    int bestDist = std::numeric_limits<int>::max();

    int tc = wrapCol(tileIndex(gx));
    int tr = tileIndex(gy);

    for (Dir d : dirs) {
        if (!allowReverse && d == opposite(curDir)) continue;
        auto dv = dirVec(d);
        int nc = wrapCol(tc + static_cast<int>(dv.x));
        int nr = tr + static_cast<int>(dv.y);
        if (isWall(nc, nr, ghostMode)) continue;
        int dist = tileDistSq({nc, nr}, target);
        if (dist < bestDist) { bestDist = dist; best = d; }
    }
    return best == Dir::None ? opposite(curDir) : best;
}

Dir randomDir(float gx, float gy, Dir curDir, std::mt19937& rng) {
    Dir dirs[] = {Dir::Up, Dir::Down, Dir::Left, Dir::Right};
    std::shuffle(dirs, dirs + 4, rng);
    int tc = wrapCol(tileIndex(gx));
    int tr = tileIndex(gy);
    for (Dir d : dirs) {
        if (d == opposite(curDir)) continue;
        auto dv = dirVec(d);
        int nc = wrapCol(tc + static_cast<int>(dv.x));
        int nr = tr + static_cast<int>(dv.y);
        if (!isWall(nc, nr, true)) return d;
    }
    return opposite(curDir);
}

sf::Vector2i ghostTarget(const Ghost& g, const Pacman& pac, const Ghost* ghosts) {
    auto pt = pac.tilePos();
    auto dv = dirVec(pac.dir);

    switch (g.id) {
        case 0: // Blinky: directly at pac
            return pt;
        case 1: { // Pinky: 4 tiles ahead of pac
            return {pt.x + (int)dv.x * 4, pt.y + (int)dv.y * 4};
        }
        case 2: { // Inky: vector trick with Blinky
            sf::Vector2i ahead = {pt.x + (int)dv.x * 2, pt.y + (int)dv.y * 2};
            sf::Vector2i blinky = ghosts[0].tilePos();
            return {2 * ahead.x - blinky.x, 2 * ahead.y - blinky.y};
        }
        case 3: { // Clyde: chase if far, scatter corner if close
            int dx = g.tilePos().x - pt.x, dy = g.tilePos().y - pt.y;
            if (dx*dx + dy*dy > 64) return pt;
            return g.scatterTarget;
        }
        default: return pt;
    }
}

bool runSelfTests() {
    bool ok = true;
    auto check = [&](bool condition, const std::string& name) {
        if (condition) {
            std::cout << "PASS: " << name << '\n';
        } else {
            std::cerr << "FAIL: " << name << '\n';
            ok = false;
        }
    };

    check(approxEq(snapAxis(23.f * TILE), 23.5f * TILE),
          "snapAxis recenters positions to tile centers");
    check(nearCentre(23.5f * TILE), "nearCentre accepts a centered lane position");
    check(!nearCentre(23.f * TILE), "nearCentre rejects tile-edge positions");
    check(canMoveDir(13.5f * TILE, 23.5f * TILE, Dir::Left),
          "Pac-Man can move immediately from the centered spawn tile");

    float blockedX = 1.5f * TILE;
    float blockedY = 1.5f * TILE;
    float blockedMove = moveEntity(blockedX, blockedY, Dir::Left, 10.f);
    check(blockedMove < 10.f && approxEq(blockedX, 27.f) &&
              canOccupy(blockedX, blockedY) && !canAdvance(blockedX, blockedY, Dir::Left, 1.f),
          "moveEntity stops cleanly at wall boundaries");

    Ghost deadGhost;
    deadGhost.id = 0;
    deadGhost.x = 13.5f * TILE;
    deadGhost.y = 11.5f * TILE;
    deadGhost.dir = Dir::Up;
    deadGhost.mode = GhostMode::Dead;
    check(bestDirToward(deadGhost.x, deadGhost.y, deadGhost.dir, GHOST_HOME_TILE, true, true) == Dir::Down,
          "dead ghosts can reverse toward home");

    Ghost respawnGhost;
    respawnGhost.id = 2;
    respawnGhost.respawn();
    check(approxEq(respawnGhost.x, 11.5f * TILE) && approxEq(respawnGhost.y, 14.5f * TILE),
          "ghost respawn restores centered starting coordinates");

    Ghost homeGhost;
    homeGhost.id = 1;
    homeGhost.mode = GhostMode::Dead;
    homeGhost.x = tileCenter(GHOST_HOME_TILE.x);
    homeGhost.y = tileCenter(GHOST_HOME_TILE.y);
    bool atCentreX = nearCentre(homeGhost.x);
    bool atCentreY = nearCentre(homeGhost.y);
    if (homeGhost.mode == GhostMode::Dead && atCentreX && atCentreY &&
        homeGhost.tilePos() == GHOST_HOME_TILE) {
        homeGhost.x = tileCenter(GHOST_HOME_TILE.x);
        homeGhost.y = tileCenter(GHOST_HOME_TILE.y);
        homeGhost.mode = GhostMode::Scatter;
        homeGhost.scatterTimer = 6.f;
        homeGhost.dir = Dir::Left;
    }
    check(homeGhost.mode == GhostMode::Scatter && homeGhost.dir == Dir::Left,
          "dead ghosts revive once they reach the home tile");

    return ok;
}

// ─── Sound generation (no external files needed) ─────────────────────────────
// Generates a PCM tone that sweeps from f1 to f2 Hz over `secs` seconds.
sf::SoundBuffer makeTone(float f1, float f2, float secs, float vol = 0.35f) {
    const unsigned SR = 44100;
    auto n = static_cast<std::uint64_t>(SR * secs);
    std::vector<std::int16_t> s(n);
    float phase = 0.f;
    for (std::uint64_t i = 0; i < n; ++i) {
        float frac = static_cast<float>(i) / static_cast<float>(n);
        float freq = f1 + (f2 - f1) * frac;
        float env  = std::max(0.f, 1.f - frac * 1.5f);
        // Square wave for classic 8-bit feel
        float wave = (std::sin(phase) >= 0.f) ? 1.f : -1.f;
        s[i] = static_cast<std::int16_t>(vol * 32767.f * env * wave);
        phase += 2.f * PI * freq / static_cast<float>(SR);
    }
    return sf::SoundBuffer(s.data(), n, 1, SR, {sf::SoundChannel::Mono});
}

// Descending death jingle (chromatic slide with vibrato)
sf::SoundBuffer makeDeathSound() {
    const unsigned SR = 44100;
    float secs = 1.8f;
    auto n = static_cast<std::uint64_t>(SR * secs);
    std::vector<std::int16_t> s(n);
    float phase = 0.f;
    for (std::uint64_t i = 0; i < n; ++i) {
        float frac = static_cast<float>(i) / static_cast<float>(n);
        float freq = 480.f * std::pow(0.04f, frac); // exponential fall
        float vib  = 1.f + 0.04f * std::sin(2.f * PI * 8.f * static_cast<float>(i) / SR);
        float env  = std::max(0.f, 1.f - frac * 0.9f);
        float wave = (std::sin(phase) >= 0.f) ? 1.f : -1.f;
        s[i] = static_cast<std::int16_t>(0.4f * 32767.f * env * wave * vib);
        phase += 2.f * PI * freq * vib / static_cast<float>(SR);
    }
    return sf::SoundBuffer(s.data(), n, 1, SR, {sf::SoundChannel::Mono});
}

struct Sounds {
    sf::SoundBuffer dotBuf[2];
    sf::SoundBuffer pelletBuf;
    sf::SoundBuffer deathBuf;
    sf::SoundBuffer ghostBuf;
    std::optional<sf::Sound> dotSnd[2];
    std::optional<sf::Sound> pelletSnd;
    std::optional<sf::Sound> deathSnd;
    std::optional<sf::Sound> ghostSnd;
    int dotIdx = 0;

    Sounds() {
        dotBuf[0] = makeTone(480.f, 480.f, 0.055f); // high blip
        dotBuf[1] = makeTone(360.f, 360.f, 0.055f); // low blip  (alternating)
        pelletBuf = makeTone(200.f, 1200.f, 0.35f);  // rising sweep
        deathBuf  = makeDeathSound();
        ghostBuf  = makeTone(120.f, 1400.f, 0.22f, 0.5f); // fast rising blip
        dotSnd[0].emplace(dotBuf[0]);
        dotSnd[1].emplace(dotBuf[1]);
        pelletSnd.emplace(pelletBuf);
        deathSnd.emplace(deathBuf);
        ghostSnd.emplace(ghostBuf);
    }

    void playDot() {
        dotSnd[dotIdx]->stop();
        dotSnd[dotIdx]->play();
        dotIdx ^= 1;
    }
    void playPellet() { pelletSnd->stop(); pelletSnd->play(); }
    void playDeath()  { deathSnd->stop();  deathSnd->play();  }
    void playGhost()  { ghostSnd->stop();  ghostSnd->play();  }
};

// ─── Main ─────────────────────────────────────────────────────────────────────
int main(int argc, char** argv) {
    if (argc > 1 && std::string(argv[1]) == "--self-test") {
        return runSelfTests() ? 0 : 1;
    }

    sf::RenderWindow window(sf::VideoMode({(unsigned)WIN_W, (unsigned)WIN_H}), "Pac-Man");
    window.setFramerateLimit(60);

    // Load font (try several system paths)
    std::optional<sf::Font> fontOpt;
    for (auto& p : {
        "/System/Library/Fonts/Helvetica.ttc",
        "/Library/Fonts/Arial.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/TTF/DejaVuSans.ttf"
    }) {
        if (std::filesystem::exists(p)) {
            sf::Font f(p);
            fontOpt = std::move(f);
            break;
        }
    }
    // Fallback: construct without file if all fail (text won't render, game still works)
    sf::Font font = fontOpt.has_value() ? std::move(*fontOpt) : sf::Font();

    std::mt19937 rng(42);
    Sounds sfx;

    Game game;
    game.reset(true);
    game.state = GameState::Menu;

    sf::Clock clock;

    // Prepare wall shapes (cache them)
    std::vector<sf::RectangleShape> walls;
    for (int r = 0; r < ROWS; ++r) {
        for (int c = 0; c < COLS; ++c) {
            if (MAP[r][c] == 1) {
                sf::RectangleShape wall({(float)TILE - 1.f, (float)TILE - 1.f});
                wall.setPosition({(float)c * TILE + 0.5f, (float)r * TILE + 0.5f});
                wall.setFillColor(sf::Color(0, 0, 180));
                wall.setOutlineColor(sf::Color(100, 100, 255));
                wall.setOutlineThickness(1.f);
                walls.push_back(wall);
            }
        }
    }

    // Ghost house door shapes
    std::vector<sf::RectangleShape> doors;
    for (int r = 0; r < ROWS; ++r) {
        for (int c = 0; c < COLS; ++c) {
            if (MAP[r][c] == 4) {
                sf::RectangleShape door({(float)TILE - 2.f, 4.f});
                door.setPosition({(float)c * TILE + 1.f, (float)r * TILE + TILE * 0.5f - 2.f});
                door.setFillColor(sf::Color(255, 180, 255));
                doors.push_back(door);
            }
        }
    }

    while (window.isOpen()) {
        float dt = clock.restart().asSeconds();
        if (dt > 0.05f) dt = 0.05f;

        // ── Events ────────────────────────────────────────────────────────────
        while (const auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>())
                window.close();

            if (const auto* kp = event->getIf<sf::Event::KeyPressed>()) {
                using K = sf::Keyboard::Key;
                if (game.state == GameState::Menu || game.state == GameState::GameOver) {
                    if (kp->code == K::Enter || kp->code == K::Space) {
                        game.reset(true);
                        game.state = GameState::Playing;
                    }
                } else if (game.state == GameState::Playing) {
                    switch (kp->code) {
                        case K::Left:  game.pac.nextDir = Dir::Left;  break;
                        case K::Right: game.pac.nextDir = Dir::Right; break;
                        case K::Up:    game.pac.nextDir = Dir::Up;    break;
                        case K::Down:  game.pac.nextDir = Dir::Down;  break;
                        case K::Escape: window.close(); break;
                        default: break;
                    }
                } else if (game.state == GameState::Dead) {
                    // wait for timer
                } else if (game.state == GameState::LevelClear) {
                    // wait for timer
                }
            }
        }
        // Also handle held keys
        if (game.state == GameState::Playing) {
            using K = sf::Keyboard::Key;
            if (sf::Keyboard::isKeyPressed(K::Left))  game.pac.nextDir = Dir::Left;
            if (sf::Keyboard::isKeyPressed(K::Right)) game.pac.nextDir = Dir::Right;
            if (sf::Keyboard::isKeyPressed(K::Up))    game.pac.nextDir = Dir::Up;
            if (sf::Keyboard::isKeyPressed(K::Down))  game.pac.nextDir = Dir::Down;
        }

        // ── Update ────────────────────────────────────────────────────────────
        if (game.state == GameState::Playing) {
            // Frighten timer countdown
            if (game.frightTime > 0.f) {
                game.frightTime -= dt;
                if (game.frightTime <= 0.f) {
                    game.frightTime = 0.f;
                    game.ghostEatCombo = 0;
                    for (auto& g : game.ghosts)
                        if (g.mode == GhostMode::Frightened)
                            g.mode = GhostMode::Chase;
                }
            }

            // ── Pac-Man movement ──────────────────────────────────────────────
            {
                Pacman& p = game.pac;
                float speed = p.speed;

                // Try to turn into nextDir if near tile centre
                if (p.nextDir != Dir::None && p.nextDir != p.dir) {
                    bool alignedH = nearCentre(p.y);
                    bool alignedV = nearCentre(p.x);
                    if ((p.nextDir == Dir::Left || p.nextDir == Dir::Right) && alignedH) {
                        p.y = snapAxis(p.y); // snap y to grid
                        if (canMoveDir(p.x, p.y, p.nextDir)) {
                            p.dir = p.nextDir;
                        }
                    } else if ((p.nextDir == Dir::Up || p.nextDir == Dir::Down) && alignedV) {
                        p.x = snapAxis(p.x); // snap x to grid
                        if (canMoveDir(p.x, p.y, p.nextDir)) {
                            p.dir = p.nextDir;
                        }
                    }
                }

                // Move in current direction
                if (p.dir != Dir::None) {
                    float moved = moveEntity(p.x, p.y, p.dir, speed * dt);
                    if (moved <= 0.f) {
                        p.dir = Dir::None;
                    }
                }

                // Tunnel wrap
                if (p.x < -TILE * 0.5f)   p.x = COLS * TILE + TILE * 0.5f;
                if (p.x > COLS * TILE + TILE * 0.5f) p.x = -TILE * 0.5f;

                // Mouth animation
                if (p.dir != Dir::None) {
                    p.mouthT += dt * 4.f;
                    if (p.mouthT > 1.f) { p.mouthT = 0.f; }
                }

                // Eat dots
                sf::Vector2i pacTile = p.tilePos();
                int tc = pacTile.x;
                int tr = pacTile.y;
                if (tc >= 0 && tc < COLS && tr >= 0 && tr < ROWS) {
                    int& cell = game.dots[tr][tc];
                    if (cell == 2) {
                        cell = 0;
                        game.score += 10;
                        ++game.eatenDots;
                        sfx.playDot();
                    } else if (cell == 3) {
                        cell = 0;
                        game.score += 50;
                        ++game.eatenDots;
                        sfx.playPellet();
                        // Frighten all ghosts
                        game.frightTime = 8.f;
                        game.ghostEatCombo = 0;
                        for (auto& g : game.ghosts) {
                            if (g.mode != GhostMode::Dead) {
                                g.dir = opposite(g.dir);
                                g.mode = GhostMode::Frightened;
                            }
                        }
                    }
                }

                // Check win
                if (game.eatenDots >= game.totalDots) {
                    game.state = GameState::LevelClear;
                    game.stateTimer = 2.5f;
                }
            }

            // ── Ghost movement ────────────────────────────────────────────────
            for (auto& g : game.ghosts) {
                // Update mode timers
                if (g.mode == GhostMode::Scatter) {
                    g.scatterTimer -= dt;
                    if (g.scatterTimer <= 0.f) {
                        g.mode = GhostMode::Chase;
                    }
                }

                float spd = g.speed();
                bool atCentreX = nearCentre(g.x);
                bool atCentreY = nearCentre(g.y);

                if (g.mode == GhostMode::Dead && atCentreX && atCentreY &&
                    g.tilePos() == GHOST_HOME_TILE) {
                    g.x = tileCenter(GHOST_HOME_TILE.x);
                    g.y = tileCenter(GHOST_HOME_TILE.y);
                    g.mode = GhostMode::Scatter;
                    g.scatterTimer = 6.f;
                    g.dir = Dir::Left;
                }

                // Choose new direction at intersections
                if (atCentreX && atCentreY) {
                    g.x = snapAxis(g.x);
                    g.y = snapAxis(g.y);

                    sf::Vector2i target;
                    if (g.mode == GhostMode::Scatter || g.mode == GhostMode::Dead) {
                        target = (g.mode == GhostMode::Dead) ?
                            GHOST_HOME_TILE : g.scatterTarget;
                    } else if (g.mode == GhostMode::Frightened) {
                        g.dir = randomDir(g.x, g.y, g.dir, rng);
                        goto moveGhost;
                    } else {
                        target = ghostTarget(g, game.pac, game.ghosts);
                    }
                    g.dir = bestDirToward(g.x, g.y, g.dir, target, true,
                                          g.mode == GhostMode::Dead);
                }
                moveGhost:
                {
                    moveEntity(g.x, g.y, g.dir, spd * dt, true);
                    // Tunnel wrap
                    if (g.x < -TILE * 0.5f)   g.x = COLS * TILE + TILE * 0.5f;
                    if (g.x > COLS * TILE + TILE * 0.5f) g.x = -TILE * 0.5f;
                }
            }

            // ── Collision: pac vs ghosts ───────────────────────────────────────
            for (auto& g : game.ghosts) {
                if (g.mode == GhostMode::Dead) continue;
                float dx = g.x - game.pac.x, dy = g.y - game.pac.y;
                if (dx*dx + dy*dy < 16.f * 16.f) {
                    if (g.mode == GhostMode::Frightened) {
                        // Eat ghost
                        ++game.ghostEatCombo;
                        int pts = 200 * (1 << (game.ghostEatCombo - 1));
                        game.score += pts;
                        g.mode = GhostMode::Dead;
                        g.deadTimer = 0.f;
                        g.dir = opposite(g.dir);
                        sfx.playGhost();
                    } else {
                        // Pac-Man dies
                        sfx.playDeath();
                        --game.lives;
                        if (game.lives <= 0) {
                            game.state = GameState::GameOver;
                            game.stateTimer = 3.5f;
                        } else {
                            game.state = GameState::Dead;
                            game.stateTimer = 2.f;
                        }
                    }
                }
            }
        } else if (game.state == GameState::Dead) {
            game.stateTimer -= dt;
            if (game.stateTimer <= 0.f) {
                // Respawn (keep dots)
                game.pac.x = 13.5f * TILE;
                game.pac.y = 23.5f * TILE;
                game.pac.dir = Dir::None; game.pac.nextDir = Dir::None;
                game.frightTime = 0.f; game.ghostEatCombo = 0;
                for (auto& g : game.ghosts) g.respawn();
                game.state = GameState::Playing;
            }
        } else if (game.state == GameState::LevelClear) {
            game.stateTimer -= dt;
            if (game.stateTimer <= 0.f) {
                ++game.level;
                game.reset(false);
                game.state = GameState::Playing;
            }
        } else if (game.state == GameState::GameOver) {
            game.stateTimer -= dt;
        }

        // ── Draw ──────────────────────────────────────────────────────────────
        window.clear(sf::Color::Black);

        // Draw walls
        for (auto& w : walls) window.draw(w);
        for (auto& d : doors) window.draw(d);

        // Draw dots and pellets
        for (int r = 0; r < ROWS; ++r) {
            for (int c = 0; c < COLS; ++c) {
                int cell = game.dots[r][c];
                if (cell == 2) {
                    sf::CircleShape dot(2.f);
                    dot.setFillColor(sf::Color(255, 200, 150));
                    dot.setOrigin({2.f, 2.f});
                    dot.setPosition({c * TILE + TILE * 0.5f, r * TILE + TILE * 0.5f});
                    window.draw(dot);
                } else if (cell == 3) {
                    // Pulsing power pellet
                    float pulse = 5.f + 3.f * std::sin(clock.getElapsedTime().asSeconds() * 5.f);
                    sf::CircleShape pel(pulse);
                    pel.setFillColor(sf::Color(255, 200, 150));
                    pel.setOrigin({pulse, pulse});
                    pel.setPosition({c * TILE + TILE * 0.5f, r * TILE + TILE * 0.5f});
                    window.draw(pel);
                }
            }
        }

        // Draw ghosts
        if (game.state != GameState::Menu) {
            for (auto& g : game.ghosts) {
                if (g.mode == GhostMode::Dead) continue;
                bool fright = (g.mode == GhostMode::Frightened);
                drawGhost(window, g.x, g.y, g.color, fright, game.frightTime);
            }
        }

        // Draw Pac-Man
        if (game.state == GameState::Playing || game.state == GameState::Dead) {
            float angle = 0.f;
            switch (game.pac.dir) {
                case Dir::Right: angle = 0.f;       break;
                case Dir::Down:  angle = PI * 0.5f; break;
                case Dir::Left:  angle = PI;        break;
                case Dir::Up:    angle = PI * 1.5f; break;
                default:         angle = 0.f;       break;
            }
            float mAngle = 0.3f * std::abs(std::sin(game.pac.mouthT * PI));
            drawPacman(window, game.pac.x, game.pac.y, angle, mAngle);
        }

        // ── UI Bar ────────────────────────────────────────────────────────────
        int barY = ROWS * TILE;
        sf::RectangleShape bar({(float)WIN_W, 60.f});
        bar.setPosition({0.f, (float)barY});
        bar.setFillColor(sf::Color(20, 20, 20));
        window.draw(bar);

        // Score text
        sf::Text scoreText(font, "SCORE: " + std::to_string(game.score), 20);
        scoreText.setFillColor(sf::Color::White);
        scoreText.setPosition({10.f, (float)barY + 5.f});
        window.draw(scoreText);

        // Level text
        sf::Text levelText(font, "LEVEL: " + std::to_string(game.level), 20);
        levelText.setFillColor(sf::Color::Yellow);
        levelText.setPosition({WIN_W * 0.5f - 40.f, (float)barY + 5.f});
        window.draw(levelText);

        // Lives (draw pac icons)
        for (int i = 0; i < game.lives; ++i) {
            drawPacman(window, WIN_W - 30.f - i * 22.f, barY + 40.f, 0.f, 0.4f);
        }

        sf::Text livesLabel(font, "LIVES:", 16);
        livesLabel.setFillColor(sf::Color(200, 200, 200));
        livesLabel.setPosition({WIN_W - 30.f - (float)(game.lives) * 22.f - 60.f, (float)barY + 33.f});
        window.draw(livesLabel);

        // Overlay messages
        if (game.state == GameState::Menu) {
            sf::RectangleShape overlay({(float)WIN_W, (float)WIN_H});
            overlay.setFillColor(sf::Color(0, 0, 0, 160));
            window.draw(overlay);

            sf::Text title(font, "PAC-MAN", 52);
            title.setFillColor(sf::Color::Yellow);
            auto tb = title.getLocalBounds();
            title.setOrigin({tb.size.x * 0.5f, tb.size.y * 0.5f});
            title.setPosition({WIN_W * 0.5f, WIN_H * 0.35f});
            window.draw(title);

            sf::Text sub(font, "Press ENTER or SPACE to start", 22);
            sub.setFillColor(sf::Color::White);
            auto sb = sub.getLocalBounds();
            sub.setOrigin({sb.size.x * 0.5f, sb.size.y * 0.5f});
            sub.setPosition({WIN_W * 0.5f, WIN_H * 0.55f});
            window.draw(sub);

            sf::Text controls(font, "Arrow keys to move   ESC to quit", 18);
            controls.setFillColor(sf::Color(180, 180, 180));
            auto cb = controls.getLocalBounds();
            controls.setOrigin({cb.size.x * 0.5f, cb.size.y * 0.5f});
            controls.setPosition({WIN_W * 0.5f, WIN_H * 0.65f});
            window.draw(controls);
        }

        if (game.state == GameState::Dead) {
            sf::Text msg(font, "OUCH!", 40);
            msg.setFillColor(sf::Color::Red);
            auto mb = msg.getLocalBounds();
            msg.setOrigin({mb.size.x * 0.5f, mb.size.y * 0.5f});
            msg.setPosition({WIN_W * 0.5f, WIN_H * 0.45f});
            window.draw(msg);
        }

        if (game.state == GameState::LevelClear) {
            sf::Text msg(font, "LEVEL CLEAR!", 40);
            msg.setFillColor(sf::Color::Yellow);
            auto mb = msg.getLocalBounds();
            msg.setOrigin({mb.size.x * 0.5f, mb.size.y * 0.5f});
            msg.setPosition({WIN_W * 0.5f, WIN_H * 0.45f});
            window.draw(msg);
        }

        if (game.state == GameState::GameOver) {
            sf::RectangleShape overlay({(float)WIN_W, (float)WIN_H});
            overlay.setFillColor(sf::Color(0, 0, 0, 160));
            window.draw(overlay);

            sf::Text msg(font, "GAME OVER", 52);
            msg.setFillColor(sf::Color::Red);
            auto mb = msg.getLocalBounds();
            msg.setOrigin({mb.size.x * 0.5f, mb.size.y * 0.5f});
            msg.setPosition({WIN_W * 0.5f, WIN_H * 0.38f});
            window.draw(msg);

            sf::Text sc(font, "Final Score: " + std::to_string(game.score), 28);
            sc.setFillColor(sf::Color::White);
            auto scb = sc.getLocalBounds();
            sc.setOrigin({scb.size.x * 0.5f, scb.size.y * 0.5f});
            sc.setPosition({WIN_W * 0.5f, WIN_H * 0.52f});
            window.draw(sc);

            sf::Text restart(font, "Press ENTER to play again", 22);
            restart.setFillColor(sf::Color(200, 200, 200));
            auto rb = restart.getLocalBounds();
            restart.setOrigin({rb.size.x * 0.5f, rb.size.y * 0.5f});
            restart.setPosition({WIN_W * 0.5f, WIN_H * 0.65f});
            window.draw(restart);
        }

        window.display();
    }

    return 0;
}
