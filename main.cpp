#include <algorithm>
#include <iostream>
#include <list>
#include <raylib.h>
#include <vector>

struct Player {
  Vector2 pos;
  Rectangle size;
  bool can_shoot;
};

struct Laser {
  float x;
  float height;
  bool active;
};

enum BallSize { SMALL = 10, MEDIUM = 20, LARGE = 40 };

struct Ball {
  BallSize r;
  Vector2 pos;
  float vv; // vertical velocity
  float vh; // horizontal velocity
  bool active;
};

struct ShowPoints {
  int points_gained;
  int duration;
  Vector2 position;
};

#define G .4
#define ELASTICITY 2.f

#define PLAYER_SPEED 5
#define START_BALL_NUM 2
#define LASER_SPEED 8

#define SHOW_POINTS_DURATION 40
#define SHOW_POINTS_FLOAT_SPEED 2

static const int SCREEN_WIDTH = 800;
static const int SCREEN_HEIGHT = 480;

static Player player = Player{};
static std::list<Ball> balls;
static Laser laser = Laser{.x = 0, .height = 0, .active = false};

static std::vector<ShowPoints> show_points;

static int score = 0;
static bool game_over = false;
static bool has_won = false;

float RandomNZFloat(float min, float max) {
  float n = 0.f;
  while (n == 0.f) {
    n = min + (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) *
                  (max - min);
  }
  return n;
}

#define RANDOM_BALL_VH() RandomNZFloat(-5.f, 5.f)

void InitGame(void) {

  game_over = false;
  has_won = false;
  score = 0;

  player = Player{
      .pos = {.x = SCREEN_WIDTH / 2.f - player.size.width / 2,
              .y = SCREEN_HEIGHT},
      .size = {.width = 12, .height = 30},
      .can_shoot = true,
  };

  laser = Laser{
      .x = 0,
      .height = 0,
      .active = false,
  };

  show_points.clear();
  balls.clear();

  for (int i = 0; i < START_BALL_NUM; i++) {
    balls.push_back(
        Ball{.r = BallSize::LARGE,
             .pos = Vector2{.x = RandomNZFloat(0 + BallSize::LARGE,
                                               SCREEN_WIDTH - BallSize::LARGE),
                            .y = RandomNZFloat(0 + BallSize::LARGE,
                                               SCREEN_HEIGHT / 4.f)},
             .vv = 0,
             .vh = RANDOM_BALL_VH(),
             .active = true});
  }
}

void DrawGame(void) {
  BeginDrawing();
  ClearBackground(RAYWHITE);

  DrawText(TextFormat("SCORE: %d", score), 10, 10, 20, GRAY);

  if (game_over) {
    if (has_won) {
      DrawText("YOU WIN", SCREEN_WIDTH / 2 - 7 * 12, SCREEN_HEIGHT / 2, 24,
               GREEN);
    } else {
      DrawText("GAME OVER", SCREEN_WIDTH / 2 - 9 * 12, SCREEN_HEIGHT / 2, 24,
               RED);
    }

  } else {
    for (auto &ball : balls) {
      if (ball.active) {
        DrawCircleV(ball.pos, static_cast<float>(ball.r), DARKGRAY);
      }
    }

    DrawTriangle({.x = player.pos.x + player.size.width,
                  .y = player.pos.y - player.size.height},
                 player.pos,
                 {.x = player.pos.x + player.size.width * 2, .y = player.pos.y},
                 RED);

    if (laser.active) {
      DrawLineV({.x = laser.x, .y = SCREEN_HEIGHT},
                {.x = laser.x, .y = SCREEN_HEIGHT - laser.height}, RED);
    }

    for (auto &show_point : show_points) {
      if (show_point.duration > 0) {
        DrawText(TextFormat("+%d", show_point.points_gained),
                 show_point.position.x, show_point.position.y, 20, DARKGREEN);
      }
    }
  }

  EndDrawing();
}

void ResetLaser(void) {
  laser.active = false;
  laser.height = 0;
  player.can_shoot = true;
}

void ActivateLaser(float x) {
  laser.active = true;
  laser.x = x;
  player.can_shoot = false;
}

void UpdateGame(void) {
  if (game_over) {
    if (IsKeyPressed(KEY_ENTER)) {
      InitGame();
    }
  } else {
    if (IsKeyPressed(KEY_LEFT) || IsKeyDown(KEY_LEFT)) {
      player.pos.x = std::max<float>(player.pos.x - PLAYER_SPEED, 0.f);
    } else if (IsKeyPressed(KEY_RIGHT) || IsKeyDown(KEY_RIGHT)) {
      player.pos.x = std::min<float>(player.pos.x + PLAYER_SPEED,
                                     SCREEN_WIDTH - player.size.width * 2);
    }
    if (IsKeyPressed(KEY_SPACE) && player.can_shoot) {
      ActivateLaser(player.pos.x + player.size.width);
    }

    if (laser.active) {
      if (laser.height >= SCREEN_HEIGHT) {
        ResetLaser();
      } else {
        laser.height += LASER_SPEED;
      }
    }

    has_won = true;
    game_over = true;
    for (auto &ball : balls) {
      if (ball.active) {
        has_won = false;
        game_over = false;

        float r = static_cast<float>(ball.r);

        if (CheckCollisionCircleRec(
                ball.pos, r,
                Rectangle{.x = player.pos.x,
                          .y = player.pos.y - player.size.height,
                          .width = player.size.width * 2,
                          .height = player.size.height})) {
          game_over = true;
          break;
        } else if (CheckCollisionCircleRec(
                       ball.pos, r,
                       Rectangle{.x = laser.x,
                                 .y = SCREEN_HEIGHT - laser.height,
                                 .width = 1,
                                 .height = laser.height})) {
          ResetLaser();
          ball.active = false;
          int points_gained = (static_cast<int>(r) / 10) * 50;
          score += points_gained;
          show_points.push_back(ShowPoints{.points_gained = points_gained,
                                           .duration = SHOW_POINTS_DURATION,
                                           .position = ball.pos});

          // create a smaller ball on each side
          // moving in opposite directions
          if (ball.r != BallSize::SMALL) {
            for (int i : {0, 1}) {
              Ball new_ball =
                  Ball{.r = ball.r == BallSize::LARGE ? BallSize::MEDIUM
                                                      : BallSize::SMALL,
                       .pos = Vector2{.x = ball.pos.x +
                                           static_cast<float>(
                                               (i == 0 ? (-1.f * r) : r)),
                                      .y = ball.pos.y},
                       .vv = 0,
                       .vh = ball.vh *
                             static_cast<float>((i == 0 ? ball.vh > 0 ? -1 : 1
                                                 : ball.vh > 0 ? 1
                                                               : -1)),
                       .active = true};
              balls.push_back(new_ball);
            }
          }
        } else {

          if (ball.pos.y + r >= SCREEN_HEIGHT && ball.vv > 0) {
            ball.vv *= -1;
            ball.vv > 0 ? ball.vv -= G *ELASTICITY
                        : ball.vv += G * ELASTICITY; // account for elasticity
          }

          if ((ball.pos.x - r <= 0 && ball.vh < 0) ||
              (ball.pos.x + r >= SCREEN_WIDTH && ball.vh > 0)) {
            ball.vh *= -1;
          }

          ball.pos = {
              .x = std::max<float>(ball.pos.x + ball.vh, r),
              .y = std::min<float>(ball.pos.y + ball.vv,
                                   static_cast<float>(SCREEN_HEIGHT - r))};

          ball.vv += G;
        }
      }
    }

    for (auto &show_point : show_points) {
      if (show_point.duration > 0) {
        show_point.duration -= 1;
        show_point.position.y -= SHOW_POINTS_FLOAT_SPEED;
      }
    }
  }
}

int main(void) {

  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Pang");

  SetTargetFPS(60);
  InitGame();

  while (!WindowShouldClose()) {
    UpdateGame();
    DrawGame();
  }

  CloseWindow();
  return 0;
}
