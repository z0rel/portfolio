#include "stdlib.h"


bool game_quited = false;

int get_current_time();

void init_game();
void close_game();
void draw_game();
void act_game(float time);


void win32_UpdateJoystick();


void game_thread(void *data)
{
  init_game();
  int ticks1 = get_current_time();

  while (!game_quited)
  {
      draw_game();

      win32_UpdateJoystick();

      int ticks2 = get_current_time();

      act_game((ticks2 - ticks1) / 1000.0f);
      ticks1 = ticks2;
  };

  close_game();
  exit(0);
}
