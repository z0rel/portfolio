#include "stdafx.h"
#include "svga/svga.h"
#include "primitives.h"


#define _USE_MATH_DEFINES
#include <cmath>
#include <math.h>


//This function update full screen from scrptr. The array should be at least sv_height*scrpitch bytes size;
void w32_update_screen(void *scrptr, unsigned scrpitch);


//If this variable sets to true - game will quit

extern bool game_quited;


// these variables provide access to joystick and joystick buttons
// In this version joystick is simulated on Arrows and Z X buttons

// [0]-X axis (-501 - left; 501 - right)
// [1]-Y axis (-501 - left; 501 - right)


enum AxisEnum
{
	HorisontalAxis = 0,
	VerticalAxis = 1
};


extern int gAxis[2]; // 0 - not pressed; 1 - pressed
extern int gButtons[6];


/// sv_width and sv_height variables are width and height of screen
extern unsigned int sv_width;
extern unsigned int sv_height;


// This is default fullscreen shadow buffer. You can use it if you want to.
unsigned *shadow_buf = NULL;

typedef double BoardT_xy;
typedef int BoardT_w;
typedef int BoardT_h;

struct board_t
{
	board_t() : is_visible(true) {}

	BoardT_xy x;
    BoardT_xy y;
    BoardT_w w;
    BoardT_h h;
    int color;
	bool is_visible;
};

struct ball_t
{
	double x;
    double y;
	int r;
    int color;
	double speed;
    double x_vect;
    double y_vect;
    double angle;
};

board_t board;
ball_t ball;

#define BLOCS_X 13
#define BLOCS_Y 10


board_t blocs[BLOCS_X][BLOCS_Y];


bool is_start;


// Эти функции вызываются из другого потока, где кнопка нажата, или отпущена
void win32_key_down(unsigned k)
{
    if (k == VK_F1) {
        game_quited=true;
    }
}



void win32_key_up(unsigned) {}


void init_game()
{
	shadow_buf = new unsigned [sv_width * sv_height];
	board.w = 100;
	board.h = 20;
	board.x = sv_width / 2 - board.w / 2;
	board.y  = sv_height - board.h - 10;
	board.color = rgb(0xFF, 0, 0xFF);
	ball.r = 10;
	ball.x = board.x + board.w / 2;
	ball.y = board.y - board.h / 2;
	ball.color = rgb(0x00, 0, 0xFF);
	ball.speed = 1.0;
	ball.x_vect = 0.0;
	ball.y_vect = -1.0;
	is_start = true;
	ball.angle = M_PI / 2.0;

	for (int j = 0; j < BLOCS_Y; j++)
    {
		for (int i = 0; i < BLOCS_X; i++)
        {
			blocs[i][j].is_visible = true;
			blocs[i][j].color = rgb(0x00, 0xFF, 0);
			blocs[i][j].x = i * 40 + 70;
			blocs[i][j].y = j * 25 + 70;
			blocs[i][j].w = 35;
			blocs[i][j].h = 20;
		}
    }
}


void close_game() {
    if (shadow_buf)
    {
        delete shadow_buf;
    }
	shadow_buf = NULL;
}


// Отрисовка игры на экране
void draw_game() {
	if (!shadow_buf)
    {
        return;
    }

	memset(shadow_buf, 0, sv_width * sv_height * 4);

	FillCircle(ball.color, ball.x, ball.y, ball.r);

	for (int j = 0; j < BLOCS_Y; j++)
    {
		for (int i = 0; i < BLOCS_X; i++)
        {
			if (blocs[i][j].is_visible)
            {
                drawFillRect(blocs[i][j].color, blocs[i][j].x, blocs[i][j].y, blocs[i][j].w, blocs[i][j].h);
            }
		}
    }

    drawFillRect(board.color, board.x, board.y, board.w, board.h);

    w32_update_screen(shadow_buf, sv_width * 4);
}



void act_game_if_not_is_start()
{
	if (is_start) {
        return;
    }

    double dx;
    double dy;
    dx = ball.speed * ball.x_vect;
    dy = ball.speed * ball.y_vect;
    static const Dot_2d intersect_coord(0, 0);

    for (int j = 0; j < BLOCS_Y; j++)
    {
        for (int i = 0; i < BLOCS_X; i++)
        {
            if (blocs[i][j].is_visible)
            {
                BoardT_xy ball_x_plus_dx =  ball.x + dx;
                BoardT_xy ball_y_plus_dy =  ball.y + dy;
                BoardT_w  _w = blocs[i][j].w;
                BoardT_h  _h = blocs[i][j].h;
                BoardT_xy _x = blocs[i][j].x;
                BoardT_xy _y = blocs[i][j].y;
                Dot_2d dot_2d__x__y(_x, _y);
                Dot_2d dot_2d__x_plus_w__y(_x + _w, _y);
                Dot_2d dot_2d__x_plus_w__y_plus_h(_x + _w, _y + _h);
                Dot_2d dot_2d__x__y_plus_h(_x, _y + _h);

                if (  CircleIntersects(ball_x_plus_dx, ball_y_plus_dy, ball.r, _w, dot_2d__x__y,        dot_2d__x_plus_w__y,        intersect_coord)
                   || CircleIntersects(ball_x_plus_dx, ball_y_plus_dy, ball.r, _w, dot_2d__x__y_plus_h, dot_2d__x_plus_w__y_plus_h, intersect_coord)
                   || CircleIntersects(ball_x_plus_dx, ball_y_plus_dy, ball.r, _h, dot_2d__x__y,        dot_2d__x__y_plus_h,        intersect_coord)
                   || CircleIntersects(ball_x_plus_dx, ball_y_plus_dy, ball.r, _h, dot_2d__x_plus_w__y, dot_2d__x_plus_w__y_plus_h, intersect_coord))
                {
                    blocs[i][j].is_visible = false;
                    ball.x_vect = -ball.x_vect;
                    ball.y_vect = -ball.y_vect;
                    dx = ball.speed * ball.x_vect;
                    dy = ball.speed * ball.y_vect;
                    ball.x += dx;
                    ball.y += dy;
                }
            }
        }
    }

    double ball_x_plus_dx = ball.x + dx;
    double ball_y_plus_dy = ball.y + dy;

    static const Dot_2d dot_2d__0_40(0, 40);
    static const Dot_2d dot_2d__0_0(0, 0);

    if (CircleIntersects(ball_x_plus_dx, ball_y_plus_dy, ball.r, board.w, Dot_2d(board.x, board.y), Dot_2d(board.x + board.w, board.y), intersect_coord))
    {
        ball.angle = M_PI / 3.0 + (double(ball.x - board.x) / double(board.w)) * M_PI / 3.0;
        ball.y_vect = -sin(ball.angle);
        ball.x_vect = -cos(ball.angle);
        dx = ball.speed * ball.x_vect;
        dy = ball.speed * ball.y_vect;
        ball.x += dx;
        ball.y += dy;
    }
    else if (  CircleIntersects(ball_x_plus_dx, ball_y_plus_dy, ball.r, sv_width,  dot_2d__0_40,        Dot_2d(sv_width, 40),        intersect_coord)
            || CircleIntersects(ball_x_plus_dx, ball_y_plus_dy, ball.r, sv_height, dot_2d__0_0,         Dot_2d(0, sv_height),        intersect_coord)
            || CircleIntersects(ball_x_plus_dx, ball_y_plus_dy, ball.r, sv_height, Dot_2d(sv_width, 0), Dot_2d(sv_width, sv_height), intersect_coord))
    {
        ball.x_vect = -ball.x_vect;
        ball.y_vect = -ball.y_vect;
        dx = ball.speed * ball.x_vect;
        dy = ball.speed * ball.y_vect;
        ball.x += dx;
        ball.y += dy;
    }
    else if (CircleIntersects(ball_x_plus_dx, ball_y_plus_dy, ball.r, sv_width,  Dot_2d(0, sv_height), Dot_2d(sv_width, sv_height), intersect_coord))
    {
        init_game();
    }
    else
    {
        ball.x += dx;
        ball.y += dy;
    }
}


// Сцена игры. dt - время, прошедшее после предыдущей сцены.
void act_game(float dt)
{
	if (gButtons[3])
    {
		is_start = false;
	}

    act_game_if_not_is_start();

    if (gAxis[HorisontalAxis] < 0)
    {
        if (board.x > 0)
        {
            board.x--;
            if (is_start) {
                ball.x--;
            }
        }
    }
    else if (gAxis[HorisontalAxis] > 0)
    {
        if (board.x < (sv_width - 100))
        {
            board.x++;
            if (is_start) {
                ball.x++;
            }
        }
    }
}
