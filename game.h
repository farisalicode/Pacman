#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<math.h>
#include<string.h>
#include<GL/glut.h>
#include<pthread.h>
#include<semaphore.h>



// Window dimensions.
const int window_width = 1080;
const int window_height = 680;
// Board dimensions.
const int board_width = 1070;
const int board_height = 660;

int game_over = 0;
unsigned int score = 0;
unsigned int target = 384;
unsigned int lives = 4;

struct Pacman
{
    // Pacman characteristics.
    float x_coordinate;
    float y_coordinate;
    float radius;
    float mouth_angle;
    float x_velocity;
    float y_velocity;
};

struct Pellet
{
    // Pellet characteristics.
    float x_coordinate;
    float y_coordinate;
    int eaten;
};

struct Power_Pellet
{
    float x_coordinate;
    float y_coordinate;
    int eaten;
    float side;
};

struct Ghost
{
    // Ghost characteristics.
    float x_coordinate;
    float y_coordinate;
    float radius;
    // RGB colour scheme.
    float red;
    float green;
    float blue;

    //bools 

    int inhouse;
    int vulnerable;
    int boasted;
};

const float velocity = 3.5f;
const int number_pellets = 600;
float pellet_radius = 1.5f;
const int number_power_pellets = 4;
const int number_ghosts = 4;

float ghost_velocity[4] = {1.0,1.0,1.0,1.0};
float velocity_boast = 1.0f;

const int number_rows = 20;
const int number_columns = 30;
struct Pellet* pellets;
struct Pellet pellet2D[20][30];
struct Power_Pellet power_pellets[4] = {
                                        {33.0f,86.0f,0,10.0f}
                                        ,{33.0f,626.0f,0,10.0f}
                                        ,{1035.0f,86.0f,0,10.0f}
                                        ,{1035.0f,626.0f,0,10.0f}
                                       };
struct Pacman pacman = {300.0f,300.0f,8.5f,45.0f,+velocity,0.0f};
struct Ghost ghosts[4] = {
                          {520.0f,375.0f,8.5f,255.0f,0.0f,0.0f,1,0,0}
                         ,{550.0f,375.0f,8.5f,255.0f,191.25f,201.45f,1,0,0}
                         ,{520.0f,395.0f,8.5f,0.0f,255.0f,255.0f,1,0,0}
                         ,{550.0f,395.0f,8.5f,255.0f,127.5f,0.0f,1,0,0}
                         };
int draw_mouth = 1;
int mouth_frame_count = 0;
int leavePermitAndKey[4] = {-1,-1,-1,-1}; //each for each ghost
int enemyframecount = 0;
int power_up_frame_count = 0;
int power_up = 0;
int ghost_readers_count = 0;

sem_t start_game;
sem_t end_game;
sem_t power_up_signal_one;
sem_t power_up_signal_two;
sem_t pacman_lock;
sem_t permit;
sem_t key;
sem_t boast_signal;
sem_t allowChecking;
int nowdo = 1;

pthread_mutex_t ghosts_locks[4];
pthread_mutex_t ghost_reader_lock;
pthread_mutex_t score_lock;
pthread_t pacman_thread_id = -1;
pthread_t power_pellet_thread_id = -1;
pthread_t mario_1 = -1;
pthread_t mario_2 = -1;
pthread_t mario_3 = -1;
pthread_t mario_4 = -1;
pthread_t boast_1 = -1;
pthread_t boast_2 = -1;
pthread_t boast_3 = -1;
pthread_t boast_4 = -1;
int enemythreadno1 = 0;
int enemythreadno2 = 1;
int enemythreadno3 = 2;
int enemythreadno4 = 3;

void initOpenGL(int width,int height) 
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0,width,height,0);
    glMatrixMode(GL_MODELVIEW);
    return;
}

void keyboard(int key, int xx, int yy) 
{
    switch (key) 
    {
        case GLUT_KEY_RIGHT:
        pacman.x_velocity = +velocity;
        pacman.y_velocity = 0.0f;
        break;

        case GLUT_KEY_LEFT:
        pacman.x_velocity = -velocity;
        pacman.y_velocity = 0.0f;
        break;

        case GLUT_KEY_UP:
        pacman.x_velocity = 0.0f;
        pacman.y_velocity = -velocity;
        break;

        case GLUT_KEY_DOWN:
        pacman.x_velocity = 0.0f;
        pacman.y_velocity = +velocity;
        break;
        
        case GLUT_KEY_F1:
        exit(0);
        break;
    }
    return;
}

void initialize_pellets()
{
    pellets = (struct Pellet*)malloc((sizeof(struct Pellet) * number_pellets));
    int row = 0;
    int column = 0;
    // Calculate the spacing between pellets.
    float x_pellet_spacing = 35.75f;
    float y_pellet_spacing = 30.0f;
    float x_start = 20.0f;
    float y_start = 76.0f;
    float x = x_start;
    float y = y_start;
    for (int i = 0; i < number_pellets; ++i)
    {
        pellets[i].x_coordinate = x;
        pellets[i].y_coordinate = y;
        pellet2D[row][column].x_coordinate = x;
        pellet2D[row][column].y_coordinate = y;
        ++column;
        pellets[i].eaten = 0;
        x += x_pellet_spacing;
        if ((i + 1) % number_columns == 0)
        {
            x = x_start;
            y += y_pellet_spacing;
            ++row;
            column = 0;
        }
    }
    pellets[313].eaten = 1;
    pellets[314].eaten = 1;
    pellets[315].eaten = 1;
    pellets[316].eaten = 1;
    pellets[343].eaten = 1;
    pellets[344].eaten = 1;
    pellets[345].eaten = 1;
    pellets[346].eaten = 1;
}

#include "drawing.h"

void maze_collisions()
{
    float x = pacman.x_coordinate;
    float y = pacman.y_coordinate;
    float x_v = pacman.x_velocity;
    float y_v = pacman.y_velocity;
    float v = velocity;
    float r = pacman.radius;
    // Ghost house.
    if((y + r) >= 350.0f && (y - r) <= 362.0f && y_v == +v && (x >= 478.0f && x <= 598.0f))
    {
        pacman.y_coordinate -= pacman.y_velocity;
    }
    if((y - r) <= 422.0f && (y + r) >= 410.0f && y_v == -v && (x >= 478.00f && x <= 598.0f))
    {
        pacman.y_coordinate -= pacman.y_velocity;
    }
    if((x + r) >= 480.0f && (x - r) <= 488.0f && x_v == +v && (y >= 346.0f && y <= 422.0f))
    {
        pacman.x_coordinate -= pacman.x_velocity;
    }
    if((x - r) <= 596.0f && (x + r) >= 588.0f && x_v == -v && (y >= 346.0f && y <= 422.0f))
    {
        pacman.x_coordinate -= pacman.x_velocity;
    }
    // Segment 1.
    if((x - r) <= 308.0f && (x - r) >= 278.0f && x_v == -v && (y >= 129.0f && y <= 145.0f))
    {
        pacman.x_coordinate -= pacman.x_velocity;
    }
    if((y + r) >= 129.0f && (y - r) <= 139.0f && y_v == +v && (x >= 50.00f && x <= 310.0f))
    {
        pacman.y_coordinate -= pacman.y_velocity;
    }
    if((y + r) >= 129.0f && (y - r) <= 139.0f && y_v == -v && (x >= 50.00f && x <= 310.0f))
    {
        pacman.y_coordinate -= pacman.y_velocity;
    }
    if((y - r) <= 202.0f && (y - r) >= 172.0f && y_v == -v && (x >= 47.75f && x <= 63.75f))
    {
        pacman.y_coordinate -= pacman.y_velocity;
    }
    if((x + r) >= 50.75f && (x - r) <= 58.75f && x_v == +v && (y >= 131.0f && y <= 204.0f))
    {
        pacman.x_coordinate -= pacman.x_velocity;
    }
    if((x + r) >= 50.75f && (x - r) <= 58.75f && x_v == -v && (y >= 131.0f && y <= 204.0f))
    {
        pacman.x_coordinate -= pacman.x_velocity;
    }
    // Segment 2.
    if((x + r) >= 768.0f && (x + r) <= 798.0f && x_v == +v && (y >= 129.0f && y <= 145.0f))
    {
        pacman.x_coordinate -= pacman.x_velocity;
    }
    if((y + r) >= 129.0f && (y - r) <= 139.0f && y_v == +v && (x >= 766.75f && x <= 1027.0f))
    {
        pacman.y_coordinate -= pacman.y_velocity;
    }
    if((y + r) >= 129.0f && (y - r) <= 139.0f && y_v == -v && (x >= 766.75f && x <= 1027.0f))
    {
        pacman.y_coordinate -= pacman.y_velocity;
    }
    if((y - r) <= 202.0f  && (y - r) >= 172.0f && y_v == -v && (x >= 1016.0f && x <= 1028.0f))
    {
        pacman.y_coordinate -= pacman.y_velocity;
    }
    if((x + r) >= 1018.75f && (x - r) <= 1026.75f && x_v == +v && (y >= 131.0f && y <= 204.0f))
    {
        pacman.x_coordinate -= pacman.x_velocity;
    }
    if((x + r) >= 1018.75f && (x - r) <= 1026.75f && x_v == -v && (y >= 131.0f && y <= 204.0f))
    {
        pacman.x_coordinate -= pacman.x_velocity;
    }
    // Segment 3.
    if((x + r) >= 119.0f && (x + r) <= 149.0f && x_v == +v && (y >= 189.0f && y <= 207.0f))
    {
        pacman.x_coordinate -= pacman.x_velocity;
    }
    if((y + r) >= 194.0f && (y - r) <= 202.0f && y_v == +v && (x >= 118.0f && x <= 420.0f))
    {
        pacman.y_coordinate -= pacman.y_velocity;
    }
    if((y + r) >= 194.0f && (y - r) <= 202.0f && y_v == -v && (x >= 118.0f && x <= 420.0f))
    {
        pacman.y_coordinate -= pacman.y_velocity;
    }
    if((x + r) >= 409.0f && (x - r) <= 417.0f && x_v == +v && (y >= 58.0f && y <= 208.0f))
    {
        pacman.x_coordinate -= pacman.x_velocity;
    }
    if((x + r) >= 409.0f && (x - r) <= 417.0f && x_v == -v && (y >= 58.0f && y <= 208.0f))
    {
        pacman.x_coordinate -= pacman.x_velocity;
    }
    // Segment 4.
    if((x - r) <= 957.0f && (x - r) >= 927.0f && x_v == -v && (y >= 189.0f && y <= 207.0f))
    {
        pacman.x_coordinate -= pacman.x_velocity;
    }
    if((y + r) >= 194.0f && (y - r) <= 202.0f && y_v == +v && (x >= 655.0f && x <= 958.0f))
    {
        pacman.y_coordinate -= pacman.y_velocity;
    }
    if((y + r) >= 194.0f && (y - r) <= 202.0f && y_v == -v && (x >= 655.0f && x <= 958.0f))
    {
        pacman.y_coordinate -= pacman.y_velocity;
    }
    if((x + r) >= 659.0f && (x - r) <= 667.0f && x_v == +v && (y >= 58.0f && y <= 208.0f))
    {
        pacman.x_coordinate -= pacman.x_velocity;
    }
    if((x + r) >= 659.0f && (x - r) <= 667.0f && x_v == -v && (y >= 58.0f && y <= 208.0f))
    {
        pacman.x_coordinate -= pacman.x_velocity;
    }
    // Segment 5.
    if((x + r) >= 85.0f && (x + r) <= 115.0f && x_v == +v && (y >= 249.0f && y <= 265.0f))
    {
        pacman.x_coordinate -= pacman.x_velocity;
    }
    if((y + r) >= 253.0f && (y - r) <= 261.0f && y_v == +v && (x >= 83.0f && x <= 491.0f))
    {
        pacman.y_coordinate -= pacman.y_velocity;
    }
    if((y + r) >= 253.0f && (y - r) <= 261.0f && y_v == -v && (x >= 83.0f && x <= 491.0f))
    {
        pacman.y_coordinate -= pacman.y_velocity;
    }
    if((x + r) >= 480.0f && (x - r) <= 488.0f && x_v == +v && (y >= 58.0f && y <= 267.0f))
    {
        pacman.x_coordinate -= pacman.x_velocity;
    }
    if((x + r) >= 480.0f && (x - r) <= 488.0f && x_v == -v && (y >= 58.0f && y <= 267.0f))
    {
        pacman.x_coordinate -= pacman.x_velocity;
    }
    // Segment 6.
    if((x - r) <= 990.0f && (x - r) >= 960.0f && x_v == -v && (y >= 249.0f && y <= 265.0f))
    {
        pacman.x_coordinate -= pacman.x_velocity;
    }
    if((y + r) >= 253.0f && (y - r) <= 261.0f && y_v == +v && (x >= 584.0f && x <= 992.0f))
    {
        pacman.y_coordinate -= pacman.y_velocity;
    }
    if((y + r) >= 253.0f && (y - r) <= 261.0f && y_v == -v && (x >= 584.0f && x <= 992.0f))
    {
        pacman.y_coordinate -= pacman.y_velocity;
    }
    if((x + r) >= 588.0f && (x - r) <= 596.0f && x_v == +v && (y >= 58.0f && y <= 267.0f))
    {
        pacman.x_coordinate -= pacman.x_velocity;
    }
    if((x + r) >= 588.0f && (x - r) <= 596.0f && x_v == -v && (y >= 58.0f && y <= 267.0f))
    {
        pacman.x_coordinate -= pacman.x_velocity;
    }
    // Segment 7.
    if((x - r) <= 384.0f && (x - r) >= 354.0f && x_v == -v && (y >= 365.0f && y <= 383.0f))
    {
        pacman.x_coordinate -= pacman.x_velocity;
    }
    if((x - r) <= 384.0f && (x - r) >= 354.0f && x_v == -v && (y >= 546.0f && y <= 564.0f))
    {
        pacman.x_coordinate -= pacman.x_velocity;
    }
    if((y + r) >= 370.0f && (y - r) <= 378.0f && y_v == +v && (x >= 85.0f && x <= 385.0f))
    {
        pacman.y_coordinate -= pacman.y_velocity;
    }
    if((y + r) >= 370.0f && (y - r) <= 378.0f && y_v == -v && (x >= 85.0f && x <= 385.0f))
    {
        pacman.y_coordinate -= pacman.y_velocity;
    }
    if((x + r) >= 86.0f && (x - r) <= 94.0f && x_v == +v && (y >= 366.0f && y <= 563.0f))
    {
        pacman.x_coordinate -= pacman.x_velocity;
    }
    if((x + r) >= 86.0f && (x - r) <= 94.0f && x_v == -v && (y >= 366.0f && y <= 563.0f))
    {
        pacman.x_coordinate -= pacman.x_velocity;
    }
    if((y + r) >= 551.0f && (y - r) <= 559.0f && y_v == +v && (x >= 85.0f && x <= 385.0f))
    {
        pacman.y_coordinate -= pacman.y_velocity;
    }
    if((y + r) >= 551.0f && (y - r) <= 559.0f && y_v == -v && (x >= 85.0f && x <= 385.0f))
    {
        pacman.y_coordinate -= pacman.y_velocity;
    }
    // Segment 8.
    if((x + r) >= 691.0f && (x + r) <= 721.0f && x_v == +v && (y >= 365.0f && y <= 383.0f))
    {
        pacman.x_coordinate -= pacman.x_velocity;
    }
    if((x + r) >= 691.0f && (x + r) <= 721.0f && x_v == +v && (y >= 546.0f && y <= 564.0f))
    {
        pacman.x_coordinate -= pacman.x_velocity;
    }
    if((y + r) >= 370.0f && (y - r) <= 378.0f && y_v == +v && (x >= 690.0f && x <= 990.0f))
    {
        pacman.y_coordinate -= pacman.y_velocity;
    }
    if((y + r) >= 370.0f && (y - r) <= 378.0f && y_v == -v && (x >= 690.0f && x <= 990.0f))
    {
        pacman.y_coordinate -= pacman.y_velocity;
    }
    if((x + r) >= 981.0f && (x - r) <= 989.0f && x_v == +v && (y >= 366.0f && y <= 563.0f))
    {
        pacman.x_coordinate -= pacman.x_velocity;
    }
    if((x + r) >= 981.0f && (x - r) <= 989.0f && x_v == -v && (y >= 366.0f && y <= 563.0f))
    {
        pacman.x_coordinate -= pacman.x_velocity;
    }
    if((y + r) >= 551.0f && (y - r) <= 559.0f && y_v == +v && (x >= 690.0f && x <= 990.0f))
    {
        pacman.y_coordinate -= pacman.y_velocity;
    }
    if((y + r) >= 551.0f && (y - r) <= 559.0f && y_v == -v && (x >= 690.0f && x <= 990.0f))
    {
        pacman.y_coordinate -= pacman.y_velocity;
    }
    // Segment 9.
    if((x + r) >= 228.0f && (x + r) <= 258.0f && x_v == +v && (y >= 456.0f && y <= 474.0f))
    {
        pacman.x_coordinate -= pacman.x_velocity;
    }
    if((x + r) >= 228.0f && (x + r) <= 258.0f && x_v == +v && (y >= 608.0f && y <= 626.0f))
    {
        pacman.x_coordinate -= pacman.x_velocity;
    }
    if((y + r) >= 461.5f && (y - r) <= 469.5f && y_v == +v && (x >= 227.0f && x <= 455.0f))
    {
        pacman.y_coordinate -= pacman.y_velocity;
    }
    if((y + r) >= 461.5f && (y - r) <= 469.5f && y_v == -v && (x >= 227.0f && x <= 455.0f))
    {
        pacman.y_coordinate -= pacman.y_velocity;
    }
    if((x + r) >= 446.0f && (x - r) <= 454.0f && x_v == +v && (y >= 457.0f && y <= 625.0f))
    {
        pacman.x_coordinate -= pacman.x_velocity;
    }
    if((x + r) >= 446.0f && (x - r) <= 454.0f && x_v == -v && (y >= 457.0f && y <= 625.0f))
    {
        pacman.x_coordinate -= pacman.x_velocity;
    }
    if((y + r) >= 612.0f && (y - r) <= 622.0f && y_v == +v && (x >= 227.0f && x <= 455.0f))
    {
        pacman.y_coordinate -= pacman.y_velocity;
    }
    if((y + r) >= 612.0f && (y - r) <= 622.0f && y_v == -v && (x >= 227.0f && x <= 455.0f))
    {
        pacman.y_coordinate -= pacman.y_velocity;
    }
    // Segment 10.
    if((x - r) <= 920.0f && (x - r) >= 890.0f && x_v == -v && (y >= 456.0f && y <= 474.0f))
    {
        pacman.x_coordinate -= pacman.x_velocity;
    }
    if((x - r) <= 920.0f && (x - r) >= 890.0f && x_v == -v && (y >= 608.0f && y <= 626.0f))
    {
        pacman.x_coordinate -= pacman.x_velocity;
    }
    if((y + r) >= 461.5f && (y - r) <= 469.5f && y_v == +v && (x >= 623.0f && x <= 921.0f))
    {
        pacman.y_coordinate -= pacman.y_velocity;
    }
    if((y + r) >= 461.5f && (y - r) <= 469.5f && y_v == -v && (x >= 623.0f && x <= 921.0f))
    {
        pacman.y_coordinate -= pacman.y_velocity;
    }
    if((x + r) >= 624.0f && (x - r) <= 632.0f && x_v == +v && (y >= 457.0f && y <= 625.0f))
    {
        pacman.x_coordinate -= pacman.x_velocity;
    }
    if((x + r) >= 624.0f && (x - r) <= 632.0f && x_v == -v && (y >= 457.0f && y <= 625.0f))
    {
        pacman.x_coordinate -= pacman.x_velocity;
    }
    if((y + r) >= 612.0f && (y - r) <= 622.0f && y_v == +v && (x >= 623.0f && x <= 921.0f))
    {
        pacman.y_coordinate -= pacman.y_velocity;
    }
    if((y + r) >= 612.0f && (y - r) <= 622.0f && y_v == -v && (x >= 623.0f && x <= 921.0f))
    {
        pacman.y_coordinate -= pacman.y_velocity;
    }
    // Segment 11.
    if((x - r) <= 455.0f && (x - r) >= 425.0f && x_v == -v && (y >= 306.0f && y <= 326.0f))
    {
        pacman.x_coordinate -= pacman.x_velocity;
    }
    if((y + r) >= 312.5f && (y - r) <= 320.5f && y_v == +v && (x >= 11.25f && x <= 456.0f))
    {
        pacman.y_coordinate -= pacman.y_velocity;
    }
    if((y + r) >= 312.5f && (y - r) <= 320.5f && y_v == -v && (x >= 11.25f && x <= 456.0f))
    {
        pacman.y_coordinate -= pacman.y_velocity;
    }
    // Segment 12.
    if((x + r) >= 622.0f && (x + r) <= 652.0f && x_v == +v && (y >= 306.0f && y <= 326.0f))
    {
        pacman.x_coordinate -= pacman.x_velocity;
    }
    if((y + r) >= 312.5f && (y - r) <= 320.5f && y_v == +v && (x >= 621.5f && x <= board_width))
    {
        pacman.y_coordinate -= pacman.y_velocity;
    }
    if((y + r) >= 312.5f && (y - r) <= 320.5f && y_v == -v && (x >= 621.5f && x <= board_width))
    {
        pacman.y_coordinate -= pacman.y_velocity;
    }
    // Segment 13.
    if((y + r) >= 458.5f && (y - r) <= 488.5f && y_v == +v && (x >= 512.0f && x <= 528.0f))
    {
        pacman.y_coordinate -= pacman.y_velocity;
    }
    if((x + r) >= 516.0f && (x - r) <= 524.0f && x_v == +v && (y >= 458.0f && y <= board_height))
    {
        pacman.x_coordinate -= pacman.x_velocity;
    }
    if((x + r) >= 516.0f && (x - r) <= 524.0f && x_v == -v && (y >= 458.0f && y <= board_height))
    {
        pacman.x_coordinate -= pacman.x_velocity;
    }
    // Segment 14.
    if((y + r) >= 458.5f && (y - r) <= 488.5f && y_v == +v && (x >= 551.0f && x <= 565.0f))
    {
        pacman.y_coordinate -= pacman.y_velocity;
    }
    if((x + r) >= 553.0f && (x - r) <= 561.0f && x_v == +v && (y >= 458.0f && y <= board_height))
    {
        pacman.x_coordinate -= pacman.x_velocity;
    }
    if((x + r) >= 553.0f && (x - r) <= 561.0f && x_v == -v && (y >= 458.0f && y <= board_height))
    {
        pacman.x_coordinate -= pacman.x_velocity;
    }
    return;
}

void* pacman_behaviour()
{
    while(1)
    {
        sem_wait(&pacman_lock);
        // Update pacman's position.
        if((pacman.x_coordinate - pacman.radius) <= 11.75f && pacman.x_velocity == -velocity)
        {
            pacman.x_coordinate -= pacman.x_velocity;
        }
        else if((pacman.x_coordinate + pacman.radius) >= ((float)board_width - 1.75f) && pacman.x_velocity == +velocity)
        {
            pacman.x_coordinate -= pacman.x_velocity;
        }
        if((pacman.y_coordinate - pacman.radius) <= 62.0f && pacman.y_velocity == -velocity)
        {
            pacman.y_coordinate -= pacman.y_velocity;
        }
        else if((pacman.y_coordinate + pacman.radius) >= ((float)board_height - 3.0f) && pacman.y_velocity == +velocity)
        {
            pacman.y_coordinate -= pacman.y_velocity;
        }
        maze_collisions(); // you placed it after sem_post while we are changing coordinates in maz_collision(writing)
        sem_post(&pacman_lock);
        for(int i = 0;i < number_pellets;++i)
        {
            pthread_mutex_lock(&ghost_reader_lock);  //just reading
          ghost_readers_count++;
          if(ghost_readers_count == 1)
            sem_wait(&pacman_lock);

          pthread_mutex_unlock(&ghost_reader_lock);

            if(pellets[i].eaten == 0)
            {
                if(((pacman.x_coordinate - pacman.radius) < pellets[i].x_coordinate) && ((pacman.x_coordinate + pacman.radius) > pellets[i].x_coordinate))
                {
                    if(((pacman.y_coordinate - pacman.radius) < pellets[i].y_coordinate) && ((pacman.y_coordinate + pacman.radius) > pellets[i].y_coordinate))
                    {
                        pellets[i].eaten = 1;
                        pthread_mutex_lock(&score_lock);
                        ++score;
                        pthread_mutex_unlock(&score_lock);
                    }
                }
            }


            pthread_mutex_lock(&ghost_reader_lock);
            ghost_readers_count--;
            if(ghost_readers_count == 0)
               sem_post(&pacman_lock);
            
            pthread_mutex_unlock(&ghost_reader_lock);



        }
    }
    return NULL;
}

int  forbiddenIndexes(int row,int col) 
{ 
    //checking if x and y come as those

  if(row < 0 || col < 0 || row >= 20 || col >= 30)
     return 0;

///horizontal ones

  if(row == 2) {
    if((col >= 1 && col <= 8) || (col >= 21 && col <= 28)) { //indexes
        return 0;
    }
  }else if(row == 4) {
    if((col >= 3 && col <= 11) || (col >= 18 && col <= 26)) { //indexes
        return 0;
    }
  }else if(row == 6) {
    if((col >= 2 && col <= 13) || (col >= 16 && col <= 27)) { //indexes
        return 0;
    }
  }else if(row == 8) {
    if((col >= 0 && col <= 12) || (col >= 17 && col <= 29)) { //indexes
        return 0;
    }
  }else if(row == 10) {
    if((col >= 2 && col <= 10) || (col >= 19 && col <= 27 || (col>=13 && col <=16))) { //indexes
        return 0;
    }
  } else if(row == 11) { //ghost house
    if((col >= 13 && col <= 16)|| (col>=13 && col <=16)) { //indexes
        return 0;
    }
  } else if(row == 13) {
    if((col >= 6 && col <= 12) || (col >= 17 && col <= 25)) { //indexes
        return 0;
    }
  }   else if(row == 16) {
    if((col >= 2 && col <= 10) || (col >= 19 && col <= 27)) { //indexes
        return 0;
    }
  }else if(row == 18) {
    if((col >= 6 && col <= 12) || (col >= 17 && col <= 25)) { //indexes
        return 0;
    }
  }   

/////////////////////////////////vertical ones

if(col == 1) {
   if(row >= 2 && row <= 4) {
      return 0;
   }
}else if(col == 2) {
   if(row >= 10 && row <= 16) {
      return 0;
   }
}else if(col == 11) {
   if(row >= 0 && row <= 4) {
      return 0;
   }
}else if(col == 12) {
   if(row >= 13 && row <= 18) {
      return 0;
   }
}else if(col == 13) {
   if((row >= 0 && row <= 6) || (row >= 10 && row <= 11)) {
      return 0;
   }
}else if(col == 14 || col == 15) {
   if(row >= 13 && row <= 19) {
      return 0;
   }
}else if(col == 16) {  //ghost house, leaving one corner to move freely
   if((row >= 0 && row <= 6) || (row >= 10 && row <= 11)) {
      return 0;
   }
}else if(col == 17) {
   if(row >= 13 && row <= 18) {
      return 0;
   }
}else if(col == 18) {
   if(row >= 0 && row <= 4) {
      return 0;
   }
}else if(col == 27) {
   if(row >= 10 && row <= 16) {
      return 0;
   }
}else if(col == 28) {
   if(row >= 2 && row <= 4) {
      return 0;
   }
}


   return 1; //allowed
}

float distance(float px,float py,float x1,float y1) {
   return sqrt(pow(px-x1,2)+pow(py-y1,2));
}

void selectNext (int startRow,int startCol,int* currentRow,int* currentCol,int i) {   
    //conditions and flags will be here now
    
    float leastDistance =-1,maxdistance = -1, row = -1,col = -1 ;

     float px = pacman.x_coordinate,py = pacman.y_coordinate,currentDistance = -1;
     
     
     if(ghosts[i].vulnerable == 0){

     if(forbiddenIndexes(startRow-1,startCol)){
     leastDistance = distance(px,py,pellet2D[startRow-1][startCol].x_coordinate,pellet2D[startRow-1][startCol].y_coordinate); //upper row
     row =startRow-1;
     col = startCol;}

     if(forbiddenIndexes(startRow+1,startCol)){currentDistance = distance(px,py,pellet2D[startRow+1][startCol].x_coordinate,pellet2D[startRow+1][startCol].y_coordinate);
     if(leastDistance > currentDistance || leastDistance == -1) { //lower row
       leastDistance = currentDistance;
        row = startRow+1;
     col = startCol;
    }}

    if(forbiddenIndexes(startRow,startCol+1)){currentDistance = distance(px,py,pellet2D[startRow][startCol+1].x_coordinate,pellet2D[startRow][startCol+1].y_coordinate);
    if(leastDistance > currentDistance || leastDistance == -1) { //lower row
          leastDistance = currentDistance;
           row = startRow;
           col = startCol+1;
    }}

    if(forbiddenIndexes(startRow,startCol-1)){currentDistance = distance(px,py,pellet2D[startRow][startCol-1].x_coordinate,pellet2D[startRow][startCol-1].y_coordinate);
    if(leastDistance > currentDistance || leastDistance == -1) { //lower row
           leastDistance = currentDistance;
            row = startRow;
            col = startCol-1;
    }   }

    }
    else {
      
      
        
     if(forbiddenIndexes(startRow-1,startCol)){maxdistance = distance(px,py,pellet2D[startRow-1][startCol].x_coordinate,pellet2D[startRow-1][startCol].y_coordinate); //upper row
     row =startRow-1;
     col = startCol;}

     if(forbiddenIndexes(startRow+1,startCol)){currentDistance = distance(px,py,pellet2D[startRow+1][startCol].x_coordinate,pellet2D[startRow+1][startCol].y_coordinate);
     if(maxdistance < currentDistance || maxdistance == -1) { //lower row
       maxdistance = currentDistance;
        row = startRow+1;
     col = startCol;
    }}

    if(forbiddenIndexes(startRow,startCol+1)){currentDistance = distance(px,py,pellet2D[startRow][startCol+1].x_coordinate,pellet2D[startRow][startCol+1].y_coordinate);
    if(maxdistance < currentDistance || maxdistance == -1) { //lower row
          maxdistance = currentDistance;
           row = startRow;
           col = startCol+1;
    }}

    if(forbiddenIndexes(startRow,startCol-1)){currentDistance = distance(px,py,pellet2D[startRow][startCol-1].x_coordinate,pellet2D[startRow][startCol-1].y_coordinate);
    if(maxdistance < currentDistance || maxdistance == -1) { //lower row
           maxdistance = currentDistance;
            row = startRow;
            col = startCol-1;
    }   }

    }

    if(!(col == -1 || row == -1)){
    *currentCol = col;
    *currentRow = row;
    }

}

void* marioenemy(void* a) {

    int* temp = (int* )a;
    int i = *temp ;
    int startRow = -1; //these are already reached and started from
    int startCol = -1; 
    int currentRow = -1;  //these are to be reached
    int currentCol = -1;
    if(i ==0 || i== 2){ //for red ghost
    currentRow = 9;  
    currentCol = 14;
    }
    else {
        currentRow = 9;  
        currentCol = 15;
    }


    while(1) {

        if(ghosts[i].inhouse == 1) {
                    
                        ///////////////////fixing the coordinates
                      if(i == 0) 
                        {
                            pthread_mutex_lock(&ghosts_locks[0]);
                            ghosts[0].x_coordinate = 520.0;
                            ghosts[0].y_coordinate = 375.0f;
                            ghosts[0].inhouse = 1;
                             pthread_mutex_unlock(&ghosts_locks[0]);
                        }
                        else if (i==1) 
                        {
                            pthread_mutex_lock(&ghosts_locks[1]);
                          ghosts[1].x_coordinate = 550.0f;
                          ghosts[1].y_coordinate = 375.0f;
                          ghosts[1].inhouse = 1;
                           pthread_mutex_unlock(&ghosts_locks[1]);
                        }
                        else if(i == 2) 
                        {
                            pthread_mutex_lock(&ghosts_locks[2]);
                           ghosts[2].x_coordinate = 520.0f;
                           ghosts[2].y_coordinate = 395.0f;
                           ghosts[2].inhouse = 1;
                          pthread_mutex_unlock(&ghosts_locks[2]);
                        }
                        else 
                        {
                            pthread_mutex_lock(&ghosts_locks[3]);
                             ghosts[3].x_coordinate = 550.0f;
                             ghosts[3].y_coordinate = 395.0f;
                             ghosts[3].inhouse = 1;
                            pthread_mutex_unlock(&ghosts_locks[3]);
                        }

                        ///////////////////fixing the coordinates 
           sem_wait(&permit);
           sem_wait(&key);
           ghosts[i].inhouse = 0;
           startRow = -1; //these are already reached and started from
           startCol = -1; //starting all over again 
           if(i ==0 || i== 2){ //for red ghost
              currentRow = 9;  
              currentCol = 14;
            }
           else {
                currentRow = 9;  
                currentCol = 15;
           }
           leavePermitAndKey[i] = 800; //gap for other to leave the house
        }
        else if(enemyframecount > 0 && ghosts[i].inhouse == 0) {
          

          if(startRow == -1) { //haven't reached first pallet

                  pthread_mutex_lock(&ghosts_locks[i]);
                  ghosts[i].x_coordinate = pellet2D[currentRow][currentCol].x_coordinate; //aligned
                  ghosts[i].y_coordinate = pellet2D[currentRow][currentCol].y_coordinate;
                  pthread_mutex_unlock(&ghosts_locks[i]);

                  startRow = currentRow;
                  startCol = currentCol;


          }
          else{
            
          pthread_mutex_lock(&ghost_reader_lock);
          ghost_readers_count++;
          if(ghost_readers_count == 1)
            sem_wait(&pacman_lock);

          pthread_mutex_unlock(&ghost_reader_lock);

         if(distance(pacman.x_coordinate,pacman.y_coordinate,ghosts[i].x_coordinate,ghosts[i].y_coordinate) < 27.25f) {
                 ghosts[i].x_coordinate = pacman.x_coordinate;                       //8.5+8.5
                 ghosts[i].y_coordinate = pacman.y_coordinate;
         }
         else
        {
                selectNext(startRow,startCol,&currentRow,&currentCol,i); 
        }


            pthread_mutex_lock(&ghost_reader_lock);
            ghost_readers_count--;
            if(ghost_readers_count == 0)
               sem_post(&pacman_lock);
            
            pthread_mutex_unlock(&ghost_reader_lock);
            
            /////////////////////////////////////
            pthread_mutex_lock(&ghosts_locks[i]);
            ////////////////////////////////////

                   if(startCol > currentCol) {
                         ghosts[i].x_coordinate -= ghost_velocity[i];
                         if(ghosts[i].x_coordinate < pellet2D[currentRow][currentCol].x_coordinate) {
                             ghosts[i].x_coordinate = pellet2D[currentRow][currentCol].x_coordinate;
                           }

                   }
                   else if(startCol < currentCol){
                         ghosts[i].x_coordinate += ghost_velocity[i];
                         if(ghosts[i].x_coordinate > pellet2D[currentRow][currentCol].x_coordinate) {
                             ghosts[i].x_coordinate = pellet2D[currentRow][currentCol].x_coordinate;
                           }
                   }
                   else if(startCol == currentCol) {
                         if(ghosts[i].x_coordinate > pellet2D[startRow][startCol].x_coordinate) {
                           ghosts[i].x_coordinate -= ghost_velocity[i];
                           if(ghosts[i].x_coordinate < pellet2D[currentRow][currentCol].x_coordinate) {
                             ghosts[i].x_coordinate = pellet2D[currentRow][currentCol].x_coordinate;
                           }
                         }
                         else {
                            ghosts[i].x_coordinate += ghost_velocity[i];
                            if(ghosts[i].x_coordinate > pellet2D[currentRow][currentCol].x_coordinate) {
                             ghosts[i].x_coordinate = pellet2D[currentRow][currentCol].x_coordinate;
                           }
                         }
                   }
                  if(startRow > currentRow) {
                          ghosts[i].y_coordinate -= ghost_velocity[i];
                          if(ghosts[i].y_coordinate < pellet2D[currentRow][currentCol].y_coordinate) {
                             ghosts[i].y_coordinate = pellet2D[currentRow][currentCol].y_coordinate;
                           }
                  }
                  else if(startRow < currentRow) {
                          ghosts[i].y_coordinate += ghost_velocity[i];
                          if(ghosts[i].y_coordinate > pellet2D[currentRow][currentCol].y_coordinate) {
                             ghosts[i].y_coordinate = pellet2D[currentRow][currentCol].y_coordinate;
                           }
                  }
                  else if(startRow == currentRow) {
                         if(ghosts[i].y_coordinate > pellet2D[currentRow][currentCol].y_coordinate) {
                                 ghosts[i].y_coordinate -= ghost_velocity[i];
                          if(ghosts[i].y_coordinate < pellet2D[currentRow][currentCol].y_coordinate) {
                             ghosts[i].y_coordinate = pellet2D[currentRow][currentCol].y_coordinate;
                           }
                         }
                         else {
                            ghosts[i].y_coordinate += ghost_velocity[i];
                          if(ghosts[i].y_coordinate > pellet2D[currentRow][currentCol].y_coordinate) {
                             ghosts[i].y_coordinate = pellet2D[currentRow][currentCol].y_coordinate;
                           }
                         }
                  }

            /////////////////////////////////////
            pthread_mutex_unlock(&ghosts_locks[i]);
            ////////////////////////////////////

              //reached 
              if(ghosts[i].x_coordinate == pellet2D[currentRow][currentCol].x_coordinate && ghosts[i].y_coordinate == pellet2D[currentRow][currentCol].y_coordinate) {
                startRow = currentRow;
                startCol = currentCol;
              }

        }
        enemyframecount = 0;
        }
    }
    return NULL;
}



void* boast_getter(void* args)
{
    int* temp = (int* )args;
    int i = *temp ;
    unsigned long int boast_frames = 0;
    while(1)
    {
        if(ghosts[i].inhouse == 0 && ghosts[i].boasted == 0)
        {
            sem_wait(&boast_signal);
            ghost_velocity[i] += velocity_boast;
            ghosts[i].boasted = 1;
            boast_frames = 0;
        }
        if(ghosts[i].boasted == 1)
        {
            ++boast_frames;
        }
        if((ghosts[i].inhouse == 1 && ghosts[i].boasted == 1) || (boast_frames == 400000000 && ghosts[i].boasted == 1))
        {
            ghost_velocity[i] -= velocity_boast;
            ghosts[i].boasted = 0;
            sem_post(&boast_signal);
        }
    }
    return NULL;
}


void* power_pellet_consumption()
{
    while(1)
    {
       
        for(int i = 0;i < number_power_pellets;++i)
        {   sem_wait(&allowChecking);
            if(power_pellets[i].eaten == 0 && power_pellets[i].x_coordinate <= (pacman.x_coordinate + pacman.radius) && power_pellets[i].x_coordinate >= (pacman.x_coordinate - pacman.radius))
            {
                if(power_pellets[i].eaten == 0 && power_pellets[i].y_coordinate <= (pacman.y_coordinate + pacman.radius) && power_pellets[i].y_coordinate >= (pacman.y_coordinate - pacman.radius))
                {
                    power_pellets[i].eaten = 1;
                    power_up = 1;
                    for(int j = 0;j<4;++j)
                    {
                        ghosts[j].vulnerable = 1;
                    }
                    sem_post(&allowChecking);
                    sem_post(&power_up_signal_two);
                    sem_wait(&power_up_signal_one); //this is the problem
                    sem_wait(&power_up_signal_two);
                    power_pellets[i].eaten = 0;
                    break;
                }
                else
                  sem_post(&allowChecking);
            }
            else 
              sem_post(&allowChecking);
        }
        
        
    }
    return NULL;
}

void game_logic()
{
    if(lives == 0 || score == target)
    {
        game_over = 1;
    }
    if(game_over == 1)
    {
        pthread_cancel(power_pellet_thread_id);
        pthread_cancel(pacman_thread_id);
        pthread_cancel(mario_1);
        pthread_cancel(mario_2);
        pthread_cancel(mario_3);
        pthread_cancel(mario_4);
        pthread_cancel(boast_1);
        pthread_cancel(boast_2);
        pthread_cancel(boast_3);
        pthread_cancel(boast_4);

        glutHideWindow();
        sem_post(&end_game);
        pthread_exit(NULL);
    }

        for(int i = 0;i < number_ghosts;++i)
        {
            
    sem_wait(&pacman_lock);
            if(ghosts[i].x_coordinate <= (pacman.x_coordinate + pacman.radius) && ghosts[i].x_coordinate >= (pacman.x_coordinate - pacman.radius))
            {
                if(ghosts[i].y_coordinate <= (pacman.y_coordinate + pacman.radius) && ghosts[i].y_coordinate >= (pacman.y_coordinate - pacman.radius))
                {
                    // Reset.
                    if(power_up == 0)
                    {
                    pacman.x_coordinate = 300.0f;
                    pacman.y_coordinate = 300.0f;
                    sem_post(&pacman_lock);
                    pthread_mutex_lock(&ghosts_locks[0]);
                    ghosts[0].x_coordinate = 520.0;
                    ghosts[0].y_coordinate = 375.0f;
                    pthread_mutex_unlock(&ghosts_locks[0]);
                    
                    pthread_mutex_lock(&ghosts_locks[1]);
                    ghosts[1].x_coordinate = 550.0f;
                    ghosts[1].y_coordinate = 375.0f;
                    pthread_mutex_unlock(&ghosts_locks[1]);

                    pthread_mutex_lock(&ghosts_locks[2]);
                    ghosts[2].x_coordinate = 520.0f;
                    ghosts[2].y_coordinate = 395.0f;
                    pthread_mutex_unlock(&ghosts_locks[2]);

                    pthread_mutex_lock(&ghosts_locks[3]);
                    ghosts[3].x_coordinate = 550.0f;
                    ghosts[3].y_coordinate = 395.0f;
                    pthread_mutex_unlock(&ghosts_locks[3]);

                    --lives;
                    for(int j = 0;j < 4;j++) 
                    {
                        ghosts[j].inhouse = 1;
                    }
                    break;
                    }
                    else if(power_up == 1) 
                    {sem_post(&pacman_lock);
                        if(i == 0) 
                        {
                            pthread_mutex_lock(&ghosts_locks[0]);
                            ghosts[0].x_coordinate = 520.0;
                            ghosts[0].y_coordinate = 375.0f;
                            ghosts[0].inhouse = 1;
                             pthread_mutex_unlock(&ghosts_locks[0]);
                        }
                        else if (i==1) 
                        {
                            pthread_mutex_lock(&ghosts_locks[1]);
                          ghosts[1].x_coordinate = 550.0f;
                          ghosts[1].y_coordinate = 375.0f;
                          ghosts[1].inhouse = 1;
                           pthread_mutex_unlock(&ghosts_locks[1]);
                        }
                        else if(i == 2) 
                        {
                            pthread_mutex_lock(&ghosts_locks[2]);
                           ghosts[2].x_coordinate = 520.0f;
                           ghosts[2].y_coordinate = 395.0f;
                           ghosts[2].inhouse = 1;
                          pthread_mutex_unlock(&ghosts_locks[2]);
                        }
                        else 
                        {
                            pthread_mutex_lock(&ghosts_locks[3]);
                             ghosts[3].x_coordinate = 550.0f;
                             ghosts[3].y_coordinate = 395.0f;
                             ghosts[3].inhouse = 1;
                            pthread_mutex_unlock(&ghosts_locks[3]);
                        }
                    }
                }
                else 
                  {sem_post(&pacman_lock);}
            }
            else 
            {sem_post(&pacman_lock);}
            
                    
        }
    
    if(power_up == 1)
    {
        ++power_up_frame_count;
        if(power_up_frame_count == 800)
        {
            power_up = 0;
            for(int i=0;i<4;i++)
                ghosts[i].vulnerable = 0;
            power_up_frame_count = 0;
            sem_wait(&power_up_signal_two);
            sem_post(&power_up_signal_one);
            sem_post(&power_up_signal_two);
        }
    }
    return;
}

void entry_point()
{
    game_logic();
    glutPostRedisplay();    
    return;
}

void* game_engine() 
{
    initialize_pellets();
    sem_wait(&start_game);   
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(window_width, window_height);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Pacman");
    initOpenGL(window_width,window_height);
    
    sem_init(&allowChecking,0,1);
    sem_wait(&allowChecking);
    sem_init(&power_up_signal_one,0,1);
    sem_wait(&power_up_signal_one);
    sem_init(&power_up_signal_two,0,1);
    sem_wait(&power_up_signal_two);
    sem_init(&pacman_lock,0,1);
    sem_init(&permit,0,2); //2 permits and 2 keys
    sem_init(&key,0,2);
    sem_init(&boast_signal,0,2);

    for(int i = 0;i < 4;++i)
    {
        pthread_mutex_init(&ghosts_locks[i],NULL);
    }
    pthread_mutex_init(&score_lock,NULL);
    pthread_create(&pacman_thread_id,NULL,pacman_behaviour,NULL);
    pthread_create(&power_pellet_thread_id,NULL,power_pellet_consumption,NULL);

    int* enemynoholder = NULL;
    int* boastnoholder = NULL;
    int numer_1 = 0;
    int numer_2 = 1;
    int numer_3 = 2;
    int numer_4 = 3;

    enemynoholder = & enemythreadno2;
    pthread_create(&mario_1,NULL,marioenemy,(void*)enemynoholder);
    boastnoholder = &numer_1;
    pthread_create(&boast_1,NULL,boast_getter,(void*)boastnoholder);

    enemynoholder = & enemythreadno3;
    pthread_create(&mario_2,NULL,marioenemy,(void*)enemynoholder);
    boastnoholder = &numer_2;
    pthread_create(&boast_2,NULL,boast_getter,(void*)boastnoholder);

    enemynoholder = & enemythreadno4;
    pthread_create(&mario_3,NULL,marioenemy,(void*)enemynoholder);
    boastnoholder = &numer_3;
    pthread_create(&boast_3,NULL,boast_getter,(void*)boastnoholder);

    enemynoholder = & enemythreadno1;
    pthread_create(&mario_4,NULL,marioenemy,(void*)enemynoholder);
    boastnoholder = &numer_4;
    pthread_create(&boast_4,NULL,boast_getter,(void*)boastnoholder);
    
    // Called repeatdly by glutMainLoop().
    glutSpecialFunc(keyboard);
    glutIdleFunc(entry_point);
    glutDisplayFunc(draw_game);
    glutMainLoop();
    
    return NULL;
}

void ui_point()
{
    glutPostRedisplay();    
    return;
}

void leave(int key, int xx, int yy)
{
    switch(key) 
    {
        case GLUT_KEY_F1:
        exit(0);
        break;
    }
    return;
}

void finished()
{
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(600, 400);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Pacman");
    initOpenGL(400,400);
    glutSpecialFunc(leave);
    glutIdleFunc(ui_point);
    glutDisplayFunc(draw_ui);
    glutMainLoop();
    return;
}

void choices(unsigned char key, int xx, int yy) 
{
    switch(key) 
    {
        case 'g':
        glutDestroyWindow(glutGetWindow());
        sem_post(&start_game);
        sem_wait(&end_game);
        finished();
        break;
        
        case 'q':
        exit(0);    
        break;
    }
    return;
}

void* ui()
{
    int argc = 0;
    glutInit(&argc,NULL);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(600, 400);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Pacman");
    initOpenGL(400,400);
    glutKeyboardFunc(choices);
    glutIdleFunc(ui_point);
    glutDisplayFunc(draw_ui);
    glutMainLoop();
    return NULL;
}

