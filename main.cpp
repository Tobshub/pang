#include <algorithm>
#include <raylib.h>
#include <vector>

void InitGame(void);
void UpdateGame(void);
void DrawGame(void);

struct Player {
  Vector2 pos;
  Rectangle size;
  bool can_shoot;
};

#define G 1

#define START_BALL_NUM 2

enum BallSize { SMALL = 10, MEDIUM = 20, LARGE = 40 };

struct Ball {
  BallSize r;
  Vector2 pos;
  int angle;
  int v;
};

#define LAZER_SPEED 1

struct Lazer {
  float x;
  float height;
  bool active;
};

static const int SCREEN_WIDTH = 800;
static const int SCREEN_HEIGHT = 680;

static Player player = Player{};

static std::vector<Ball> balls;

static Lazer lazer = Lazer{.x = 0, .height = 0, .active = false};

static bool game_over = false;

int main(void) {

  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Pang");

  InitGame();

  while (!WindowShouldClose()) {
    UpdateGame();
    DrawGame();
  }

  CloseWindow();
  return 0;
}

float RandomFloat(float min, float max) {
  return min + ((float)rand() / (float)RAND_MAX) * (max - min);
}

int RandomInt(int min, int max) { return min + (rand() % (max - min + 1)); }

void InitGame(void) {
  SetTargetFPS(60);

  game_over = false;

  player.size = {.width = 12, .height = 30};
  player.pos = {.x = SCREEN_WIDTH / 2.f - player.size.width / 2,
                .y = SCREEN_HEIGHT};

  balls.clear();

  for (int i = 0; i < START_BALL_NUM; i++) {
    balls.push_back(
        Ball{.r = BallSize::LARGE,
             .pos = Vector2{.x = RandomFloat(0 + BallSize::LARGE,
                                             SCREEN_WIDTH - BallSize::LARGE),
                            .y = SCREEN_HEIGHT / 4.f},
             .angle = RandomInt(0, 180),
             .v = 0});
  }
}

void DrawGame(void) {
  BeginDrawing();
  ClearBackground(RAYWHITE);

  if (game_over) {
    DrawText("GAME OVER", 300, 300, 24, RED);
  } else {
    for (auto &ball : balls) {
      DrawCircleV(ball.pos, ball.r, GRAY);
    }

    DrawTriangle({.x = player.pos.x + player.size.width,
                  .y = player.pos.y - player.size.height},
                 player.pos,
                 {.x = player.pos.x + player.size.width * 2, .y = player.pos.y},
                 RED);

    if (lazer.active) {
      DrawLineV({.x = lazer.x, .y = SCREEN_HEIGHT},
                {.x = lazer.x, .y = SCREEN_HEIGHT - lazer.height}, RED);
    }
  }

  EndDrawing();
}

void UpdateGame(void) {
  if (game_over) {
    if (IsKeyPressed(KEY_ENTER)) {
      InitGame();
    }
  } else {
    for (auto &ball : balls) {
      // falling
      if (ball.pos.y + (float)ball.r >= SCREEN_HEIGHT && ball.v > 0) {
        ball.v *= -1;
        ball.v > 0 ? ball.v -= G * 2
                   : ball.v += G * 2; // account for elasticity
      }
      if (ball.pos.x - (float)ball.r <= 0 ||
          ball.pos.y + (float)ball.r >= SCREEN_HEIGHT) {
        ball.angle *= -1;
      }
      ball.pos.y =
          std::min<float>(ball.pos.y + ball.v, (float)SCREEN_HEIGHT - ball.r);
      ball.v += G;
    }
  }
}
