#include <algorithm>
#include <array>
#include <cmath>
#include <cctype>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

constexpr int COLS = 31;
constexpr int ROWS = 21;
constexpr int BASE_X = COLS / 2;
constexpr int BASE_Y = ROWS - 2;
constexpr int PLAYER_START_X = BASE_X - 2;
constexpr int PLAYER_START_Y = BASE_Y;

const std::array<std::string, ROWS> LEVEL = {
    "###############################",
    "#.............#...............#",
    "#..BB..S......#......S..BB....#",
    "#..BB.........#.........BB....#",
    "#......BBBBBBBBBBBBB..........#",
    "#..S.....................S....#",
    "#......#.............#........#",
    "#..BB..#..BBBBBBBBB..#..BB....#",
    "#......#.............#........#",
    "#............SS...............#",
    "#..............SS.............#",
    "#......#.............#........#",
    "#..BB..#..BBBBBBBBB..#..BB....#",
    "#......#.............#........#",
    "#..S.....................S....#",
    "#.........BBBBBBBBBBBBB.......#",
    "#....BB.........#.........BB..#",
    "#....BB..S......#......S..BB..#",
    "#.............BHAB............#",
    "#..............BBB............#",
    "###############################",
};

static_assert(LEVEL.size() == ROWS, "level row count must match ROWS");

enum class Dir { Up, Down, Left, Right };
enum class Tile { Empty, Brick, Steel, Hedge, Base };
enum class State { Menu, Playing, WaveClear, GameOver };

struct Pos {
    int x = 0;
    int y = 0;
};

bool operator==(Pos a, Pos b) { return a.x == b.x && a.y == b.y; }

Pos step(Pos p, Dir dir) {
    switch (dir) {
        case Dir::Up:    return {p.x, p.y - 1};
        case Dir::Down:  return {p.x, p.y + 1};
        case Dir::Left:  return {p.x - 1, p.y};
        case Dir::Right: return {p.x + 1, p.y};
    }
    return p;
}

std::string colorize(const std::string& text, const std::string& code) {
    return "\033[" + code + "m" + text + "\033[0m";
}

std::string directionalIcon(Dir dir, bool player) {
    switch (dir) {
        case Dir::Up:    return colorize(player ? "▲ " : "△ ", player ? "1;32" : "1;31");
        case Dir::Down:  return colorize(player ? "▼ " : "▽ ", player ? "1;32" : "1;31");
        case Dir::Left:  return colorize(player ? "◀ " : "◁ ", player ? "1;32" : "1;31");
        case Dir::Right: return colorize(player ? "▶ " : "▷ ", player ? "1;32" : "1;31");
    }
    return colorize(player ? "▲ " : "△ ", player ? "1;32" : "1;31");
}

std::string targetIcon() { return colorize("◎ ", "1;36"); }
std::string bulletIcon(bool fromPlayer) { return colorize("• ", fromPlayer ? "1;33" : "1;35"); }

Dir opposite(Dir dir) {
    switch (dir) {
        case Dir::Up:    return Dir::Down;
        case Dir::Down:  return Dir::Up;
        case Dir::Left:  return Dir::Right;
        case Dir::Right: return Dir::Left;
    }
    return Dir::Down;
}

struct Board {
    std::array<std::array<Tile, COLS>, ROWS> tiles{};

    Board() { reset(); }

    void reset() {
        for (int y = 0; y < ROWS; ++y) {
            if (static_cast<int>(LEVEL[y].size()) != COLS) {
                throw std::runtime_error("every level row must be exactly COLS characters wide");
            }
            for (int x = 0; x < COLS; ++x) {
                switch (LEVEL[y][x]) {
                    case '#':
                    case 'S': tiles[y][x] = Tile::Steel; break;
                    case 'B': tiles[y][x] = Tile::Brick; break;
                    case 'H': tiles[y][x] = Tile::Hedge; break;
                    case 'A': tiles[y][x] = Tile::Base; break;
                    default:  tiles[y][x] = Tile::Empty; break;
                }
            }
        }
    }

    bool inside(Pos p) const { return p.x >= 0 && p.x < COLS && p.y >= 0 && p.y < ROWS; }

    bool blocksMovement(Pos p) const {
        if (!inside(p)) return true;
        const Tile tile = tiles[p.y][p.x];
        return tile == Tile::Brick || tile == Tile::Steel || tile == Tile::Base;
    }

    bool blocksBullet(Pos p) const { return blocksMovement(p); }

    bool damage(Pos p, int& score) {
        if (!inside(p)) return false;
        if (tiles[p.y][p.x] == Tile::Brick) {
            tiles[p.y][p.x] = Tile::Empty;
            score += 5;
            return true;
        }
        if (tiles[p.y][p.x] == Tile::Base) {
            tiles[p.y][p.x] = Tile::Empty;
            return true;
        }
        return tiles[p.y][p.x] == Tile::Steel;
    }

    std::string icon(Pos p) const {
        switch (tiles[p.y][p.x]) {
            case Tile::Empty: return "  ";
            case Tile::Brick: return colorize("▓▓", "38;5;130");
            case Tile::Steel: return colorize("██", "1;37");
            case Tile::Hedge: return colorize("♣ ", "1;32");
            case Tile::Base:  return colorize("⌂ ", "1;33");
        }
        return "  ";
    }
};

struct Bullet {
    Pos pos;
    Dir dir = Dir::Up;
    bool fromPlayer = false;
    bool alive = true;
};

struct Tank {
    Pos pos;
    Dir dir = Dir::Up;
    int reload = 0;
    bool alive = true;
};

struct Enemy : Tank {
    int think = 0;
};

struct Game {
    Board board;
    Tank player;
    std::vector<Enemy> enemies;
    std::vector<Bullet> bullets;
    State state = State::Menu;
    int lives = 3;
    int score = 0;
    int wave = 1;
    std::mt19937 rng{1337};

    void start(bool fullReset) {
        board.reset();
        bullets.clear();
        if (fullReset) {
            lives = 3;
            score = 0;
            wave = 1;
        }
        spawnPlayer();
        spawnWave();
        state = State::Playing;
    }

    void spawnPlayer() {
        player = {};
        player.pos = {PLAYER_START_X, PLAYER_START_Y};
        player.dir = Dir::Up;
        player.alive = true;
    }

    void spawnWave() {
        enemies.clear();
        const std::array<Pos, 7> starts = {{{2, 2}, {COLS / 2, 1}, {COLS - 3, 2}, {4, 5}, {COLS - 5, 5}, {8, 9}, {COLS - 9, 9}}};
        const int count = std::min<int>(3 + wave, starts.size());
        for (int i = 0; i < count; ++i) {
            Enemy enemy;
            enemy.pos = starts[i];
            enemy.dir = Dir::Down;
            enemy.reload = 1 + i % 2;
            enemy.think = i % 3;
            enemies.push_back(enemy);
        }
    }
};

bool occupiedByEnemy(const Game& game, Pos pos) {
    return std::any_of(game.enemies.begin(), game.enemies.end(), [pos](const Enemy& enemy) {
        return enemy.alive && enemy.pos == pos;
    });
}

bool canMoveTo(const Game& game, Pos pos, bool isPlayer) {
    if (game.board.blocksMovement(pos)) return false;
    if (isPlayer) return !occupiedByEnemy(game, pos);
    return !(game.player.alive && game.player.pos == pos) && !occupiedByEnemy(game, pos);
}

void fire(Game& game, const Tank& tank, bool fromPlayer) {
    if (!tank.alive || tank.reload > 0) return;
    Pos muzzle = step(tank.pos, tank.dir);
    if (game.board.blocksMovement(muzzle)) return;
    game.bullets.push_back({muzzle, tank.dir, fromPlayer, true});
}

bool lineOfSight(const Board& board, Pos from, Pos to) {
    if (from.x != to.x && from.y != to.y) return false;
    Dir dir = Dir::Right;
    if (from.x > to.x) dir = Dir::Left;
    else if (from.x < to.x) dir = Dir::Right;
    else if (from.y > to.y) dir = Dir::Up;
    else dir = Dir::Down;
    for (Pos p = step(from, dir); !(p == to); p = step(p, dir)) {
        if (board.blocksBullet(p)) return false;
    }
    return true;
}

Dir directionToward(Pos from, Pos to) {
    const int dx = to.x - from.x;
    const int dy = to.y - from.y;
    if (std::abs(dx) > std::abs(dy)) return dx > 0 ? Dir::Right : Dir::Left;
    return dy > 0 ? Dir::Down : Dir::Up;
}

void updateEnemy(Game& game, Enemy& enemy) {
    enemy.reload = std::max(0, enemy.reload - 1);
    --enemy.think;

    if (lineOfSight(game.board, enemy.pos, game.player.pos)) {
        enemy.dir = directionToward(enemy.pos, game.player.pos);
        if (enemy.reload == 0) {
            fire(game, enemy, false);
            enemy.reload = 5;
        }
        return;
    }

    if (enemy.think <= 0) {
        std::array<Dir, 5> choices = {Dir::Up, Dir::Down, Dir::Left, Dir::Right, directionToward(enemy.pos, {BASE_X, BASE_Y})};
        std::uniform_int_distribution<int> pick(0, static_cast<int>(choices.size()) - 1);
        enemy.dir = choices[pick(game.rng)];
        enemy.think = 2;
    }

    Pos next = step(enemy.pos, enemy.dir);
    if (canMoveTo(game, next, false)) enemy.pos = next;
    else enemy.dir = opposite(enemy.dir);
}

void updateBullets(Game& game) {
    for (auto& bullet : game.bullets) {
        if (!bullet.alive) continue;
        bullet.pos = step(bullet.pos, bullet.dir);
        if (!game.board.inside(bullet.pos)) {
            bullet.alive = false;
            continue;
        }
        if (game.board.blocksBullet(bullet.pos)) {
            if (bullet.pos.x == BASE_X && bullet.pos.y == BASE_Y) game.state = State::GameOver;
            game.board.damage(bullet.pos, game.score);
            bullet.alive = false;
            continue;
        }
        if (bullet.fromPlayer) {
            for (auto& enemy : game.enemies) {
                if (enemy.alive && enemy.pos == bullet.pos) {
                    enemy.alive = false;
                    bullet.alive = false;
                    game.score += 100;
                    break;
                }
            }
        } else if (game.player.alive && game.player.pos == bullet.pos) {
            bullet.alive = false;
            --game.lives;
            if (game.lives <= 0) game.state = State::GameOver;
            else game.spawnPlayer();
        }
    }

    for (std::size_t i = 0; i < game.bullets.size(); ++i) {
        if (!game.bullets[i].alive) continue;
        for (std::size_t j = i + 1; j < game.bullets.size(); ++j) {
            if (game.bullets[j].alive && game.bullets[i].pos == game.bullets[j].pos) {
                game.bullets[i].alive = false;
                game.bullets[j].alive = false;
            }
        }
    }

    game.bullets.erase(std::remove_if(game.bullets.begin(), game.bullets.end(), [](const Bullet& bullet) {
        return !bullet.alive;
    }), game.bullets.end());
}

void updateGame(Game& game, char command) {
    if (game.state == State::Menu || game.state == State::GameOver) {
        if (command == '\n' || command == 'p') game.start(true);
        return;
    }
    if (game.state == State::WaveClear) {
        ++game.wave;
        game.spawnPlayer();
        game.spawnWave();
        game.state = State::Playing;
        return;
    }

    game.player.reload = std::max(0, game.player.reload - 1);
    Dir requested = game.player.dir;
    bool moveRequested = true;
    if (command == 'w') requested = Dir::Up;
    else if (command == 's') requested = Dir::Down;
    else if (command == 'a') requested = Dir::Left;
    else if (command == 'd') requested = Dir::Right;
    else moveRequested = false;

    if (moveRequested) {
        game.player.dir = requested;
        Pos next = step(game.player.pos, game.player.dir);
        if (canMoveTo(game, next, true)) game.player.pos = next;
    } else if (command == 'f' || command == ' ') {
        fire(game, game.player, true);
        game.player.reload = 1;
    }

    for (auto& enemy : game.enemies) if (enemy.alive) updateEnemy(game, enemy);
    updateBullets(game);
    game.enemies.erase(std::remove_if(game.enemies.begin(), game.enemies.end(), [](const Enemy& enemy) {
        return !enemy.alive;
    }), game.enemies.end());

    if (game.enemies.empty() && game.state == State::Playing) {
        game.score += 250 * game.wave;
        game.state = State::WaveClear;
    }
}

std::string render(const Game& game) {
    std::array<std::array<std::string, COLS>, ROWS> frame{};
    for (int y = 0; y < ROWS; ++y) {
        for (int x = 0; x < COLS; ++x) frame[y][x] = game.board.icon({x, y});
    }

    const bool showActors = game.state != State::Menu;
    const Pos target = step(game.player.pos, game.player.dir);
    if (showActors && game.player.alive && game.board.inside(target) && !game.board.blocksMovement(target)) {
        frame[target.y][target.x] = targetIcon();
    }
    for (const auto& bullet : game.bullets) {
        if (showActors && bullet.alive && game.board.inside(bullet.pos)) frame[bullet.pos.y][bullet.pos.x] = bulletIcon(bullet.fromPlayer);
    }
    for (const auto& enemy : game.enemies) {
        if (showActors && enemy.alive) frame[enemy.pos.y][enemy.pos.x] = directionalIcon(enemy.dir, false);
    }
    if (showActors && game.player.alive) frame[game.player.pos.y][game.player.pos.x] = directionalIcon(game.player.dir, true);

    std::string out;
    out += "TANKS  Score: " + std::to_string(game.score) + "  Lives: " + std::to_string(game.lives) + "  Wave: " + std::to_string(game.wave) + "\n";
    out += "Controls: W/A/S/D move and aim, F fire, P start/restart, Q quit\n";
    out += "Legend: " + directionalIcon(Dir::Up, true) + "Player tank  " + directionalIcon(Dir::Down, false) + "Enemy tank  "
         + targetIcon() + "Target  " + bulletIcon(true) + "Shell  "
         + colorize("⌂ ", "1;33") + "Base  " + colorize("▓▓", "38;5;130") + "Brick  "
         + colorize("██", "1;37") + "Steel  " + colorize("♣ ", "1;32") + "Hedge\n";
    for (const auto& row : frame) {
        for (const auto& cell : row) out += cell;
        out += '\n';
    }
    if (game.state == State::Menu) out += "Press P to start. Protect the base icon and destroy every enemy tank.\n";
    if (game.state == State::WaveClear) out += "Wave clear! Press any command to continue.\n";
    if (game.state == State::GameOver) out += "Game over. Press P to restart or Q to quit.\n";
    return out;
}

bool selfTest() {
    bool ok = true;
    auto expect = [&](bool condition, const std::string& message) {
        if (!condition) {
            ok = false;
            std::cerr << "SELF-TEST FAILED: " << message << '\n';
        }
    };

    Game game;
    expect(game.board.blocksMovement({0, 0}), "steel perimeter blocks movement");
    expect(!game.board.blocksMovement({1, 1}), "empty floor is passable");
    int score = 0;
    expect(game.board.damage({3, 2}, score), "brick damage succeeds");
    expect(!game.board.blocksMovement({3, 2}), "destroyed brick becomes passable");
    expect(score == 5, "destroying a brick awards 5 points");
    game.start(true);
    expect(game.enemies.size() == 4, "wave one spawns four enemies");
    expect(lineOfSight(game.board, {1, 1}, {13, 1}), "open row has line of sight");
    expect(!lineOfSight(game.board, {1, 1}, {20, 1}), "steel column blocks line of sight");
    const std::string view = render(game);
    expect(view.find("Legend:") != std::string::npos, "render includes visual icon legend");
    expect(view.find("⌂") != std::string::npos, "render shows base icon");
    expect(view.find("◎") != std::string::npos, "render shows aiming target icon");
    expect(view.find("▲") != std::string::npos, "render shows player tank icon");
    expect(view.find("▽") != std::string::npos, "render shows enemy tank icon");
    game.player.dir = Dir::Right;
    game.player.reload = 0;
    fire(game, game.player, true);
    expect(game.bullets.size() == 1 && game.bullets.front().fromPlayer, "player fires a bullet");
    std::cout << (ok ? "All self-tests passed.\n" : "Self-tests failed.\n");
    return ok;
}

} // namespace

int main(int argc, char** argv) {
    if (argc > 1 && std::string(argv[1]) == "--self-test") return selfTest() ? 0 : 1;

    Game game;
    char command = 'p';
    bool running = true;
    while (running) {
        std::cout << "\033[2J\033[H" << render(game) << "Command> " << std::flush;
        if (!(std::cin >> command)) break;
        command = static_cast<char>(std::tolower(static_cast<unsigned char>(command)));
        if (command == 'q') running = false;
        else updateGame(game, command);
    }
    std::cout << "Thanks for playing TANKS!\n";
    return 0;
}
