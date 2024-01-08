#include "../common/common.h"
#include <assert.h>
#include <stdbool.h>
#include <time.h>
/* 宏 */
#define RED FB_COLOR(255, 0, 0)
#define ORANGE FB_COLOR(255, 165, 0)
#define YELLOW FB_COLOR(255, 255, 0)
#define GREEN FB_COLOR(0, 255, 0)
#define CYAN FB_COLOR(0, 127, 255)
#define BLUE FB_COLOR(0, 0, 255)
#define PURPLE FB_COLOR(139, 0, 255)
#define WHITE FB_COLOR(255, 255, 255)
#define BLACK FB_COLOR(0, 0, 0)
#define BACKGROUND_COLOR FB_COLOR(28, 61, 89)

#define IMAGE_W(image) (image->pixel_w)
#define IMAGE_H(image) (image->pixel_h)

#define IN_START_MENU(x, y) (x >= start_border_x && x <= start_border_x + start_border_w && y >= start_border_y && y <= start_border_y + start_border_h)
#define IN_EXIT_MENU(x, y) (x >= exit_border_x && x <= exit_border_x + exit_border_w && y >= exit_border_y && y <= exit_border_y + exit_border_h)
#define ON_MOVE(x, y) (x >= 0 && x <= 200 && y >= SCREEN_HEIGHT - 120 && y <= SCREEN_HEIGHT)
#define SHOOT(x, y) (x >= shoot_x && x <= SCREEN_WIDTH && y >= shoot_y && y <= SCREEN_HEIGHT)
#define RERUN(x, y) (x >= death_x - 85 && x <= death_x + 295 && y >= death_y + 220 && y <= death_y + 320)
#define ROCK_OUT_OF_UI(x, y, image) (x <= background_x || x >= (SCREEN_WIDTH - background_x - IMAGE_W(image)) || y >= SCREEN_HEIGHT - 40)
#define RECT_COLLISION(x, y, w, h, x1, y1) (x1 >= x && x1 <= (x + w) && y1 >= y && y1 <= (y + h))

#define BULLET_SATRT_X (player_x + 50)
#define BULLET_START_y (player_y - 60)

#define player_collision_x (player_x + IMAGE_W(player) / 2)
#define player_collision_y (player_y + IMAGE_H(player) / 2)
#define player_collision_left_x (player_x + 10)
#define player_collision_left_y (player_y + IMAGE_H(player) / 2)
#define player_collision_right_x (player_x + IMAGE_W(player) - 10)
#define player_collision_right_y (player_y + IMAGE_H(player) / 2)
#define player_collision_top_x (player_x + IMAGE_W(player) / 2)
#define player_collision_top_y (player_y)

#define max_bullets_num 3
#define max_rock_num 7
#define max_expl_num 9
#define microseconds 100000

#define death_x (SCREEN_WIDTH / 2 - IMAGE_W(death) / 2)
#define death_y (SCREEN_HEIGHT / 2 - IMAGE_H(death) / 2)

#define player_init_health 5
#define player_init_x SCREEN_WIDTH / 2 - 50
#define player_init_y SCREEN_HEIGHT - 80
/* 枚举变量 */

enum status
{
    INITIAL,
    RUNNING,
    PENDING,
    EXIT
};

/* 全局变量 */
static int touch_fd;
enum status current_status = INITIAL;
fb_image *background = NULL;
fb_image *player = NULL;
fb_image *left_arrow = NULL;
fb_image *right_arrow = NULL;
fb_image *shooting = NULL;
fb_image *bullet = NULL;
fb_image *rocks[max_rock_num];
fb_image *expls[max_expl_num];
fb_image *death = NULL;

/* background */
const int background_x = 115;
const int background_y = 0;
/* start border */
const int start_border_x = SCREEN_WIDTH / 2 - 125;
const int start_border_y = SCREEN_HEIGHT / 2 - 80;
const int start_border_w = 250;
const int start_border_h = 70;
/* start text */
const int start_text_x = SCREEN_WIDTH / 2 - 120;
const int start_text_y = SCREEN_HEIGHT / 2 - 20;
const int start_text_size = 60;
/* exit border */
const int exit_border_x = 5;
const int exit_border_y = SCREEN_HEIGHT / 2 - 50;
const int exit_border_w = 100;
const int exit_border_h = 80;
/* exit text  */
const int exit_text_x = 5;
const int exit_text_y = SCREEN_WIDTH / 2;
const int exit_text_size = 50;
/* player */
int player_x = player_init_x;
int player_y = player_init_y;
const int player_speed = 5;
const int player_max_x = SCREEN_WIDTH - 210;
int player_health = player_init_health;
/* left arrow */
const int left_arrow_x = 5;
const int left_arrow_y = SCREEN_HEIGHT - 100;
/* right arrow */
const int right_arrow_x = 60;
const int right_arrow_y = SCREEN_HEIGHT - 100;
/* shooting */
const int shoot_x = SCREEN_WIDTH - 110;
const int shoot_y = SCREEN_HEIGHT - 120;
/* bullets */
const int bullets_speed = 30;
int bullets_num = 0;
int bullets_x[max_bullets_num];
int bullets_y[max_bullets_num];
bool bullets_used[max_bullets_num];
/* player_info */
int score = 0;
const int health_x = background_x;
const int health_y = 30;
const int left_bullets_num_x = background_x;
const int left_bullets_num_y = 70;
const int score_x = background_x;
const int score_y = 110;
const int text_size = 30;
/* rock */
int rocks_num = 0;
int rocks_x[max_rock_num];
int rocks_y[max_rock_num];
int rocks_dx[max_rock_num];
int rocks_dy[max_rock_num];
int rocks_type[max_rock_num];
bool rocks_used[max_rock_num];
/*  */
int prev_touch_x;

/* 函数声明 */
void init();
void start_task_loop();

// void erase_png_or_font(int x, int y, fb_image *image, int color);

inline void draw_background();
void draw_start_menu();
void draw_exit();
void draw_game_ui();
void draw_final_menu();

void draw_player();
void shoot_bullet();
void move_bullet();
void draw_bullet();
void draw_player_info();
void draw_die();
void draw_pending();
void add_rock();
void move_rock();
void draw_rock();
void update_ui(int fd);
void destroy_rock(int i);
void destroy_bullet(int i);

void collision_detection();

void touch_event_cb(int fd);
void handle_initial_stage();
void handle_running_stage();
void handle_pending_stage();
void handle_exit_stage();

/* main */
int main(int argc, char *argv[])
{
    init();
    draw_start_menu();
    start_task_loop();
    return 0;
}

/* 函数体 */
void init()
{
    fb_init("/dev/fb0");
    font_init("./font.ttc");

    background = fb_read_png_image("./img/background.png");
    player = fb_read_png_image("./img/player.png");
    left_arrow = fb_read_png_image("./img/left_arrow.png");
    right_arrow = fb_read_png_image("./img/right_arrow.png");
    shooting = fb_read_png_image("./img/shoot.png");
    bullet = fb_read_png_image("./img/bullet.png");
    death = fb_read_png_image("./img/death.png");
    for (int i = 0; i < max_rock_num; i++)
    {
        char path[20];
        sprintf(path, "./img/rock%d.png", i);
        rocks[i] = fb_read_png_image(path);
    }
    for (int i = 0; i < max_expl_num; i++)
    {
        char path[20];
        sprintf(path, "./img/expl%d.png", i);
        expls[i] = fb_read_png_image(path);
    }

    touch_fd = touch_init("/dev/input/event2");

    memset(bullets_used, 0, max_bullets_num);
    memset(rocks_used, 0, max_rock_num);

    srand((unsigned)time(NULL));
}

void start_task_loop()
{
    task_add_file(touch_fd, touch_event_cb);
    task_loop();
}

inline void draw_background()
{
    fb_draw_image(background_x, background_y, background, 0);
}

void draw_start_menu()
{
    fb_draw_rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, WHITE);
    draw_background();

    fb_draw_border(start_border_x, start_border_y, start_border_w, start_border_h, RED);
    fb_draw_text(start_text_x, start_text_y, "开始游戏", start_text_size, RED);
    fb_update();

    draw_exit();
}

void draw_exit()
{
    fb_draw_border(exit_border_x, exit_border_y, exit_border_w, exit_border_h, RED);
    fb_draw_text(5, SCREEN_HEIGHT / 2, "EXIT", 50, RED);
    fb_update();
}

void draw_game_ui()
{
    draw_background();
    fb_draw_image(player_x, player_y, player, 0);
    fb_draw_image(left_arrow_x, left_arrow_y, left_arrow, 0);
    fb_draw_image(right_arrow_x, right_arrow_y, right_arrow, 0);
    fb_draw_image(shoot_x, shoot_y, shooting, 0);
    fb_draw_border(0, SCREEN_HEIGHT - 120, 60, 120, BLACK);
    fb_draw_border(60, SCREEN_HEIGHT - 120, 60, 120, BLACK);
    fb_update();
}

void draw_final_menu()
{
    fb_draw_rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, WHITE);
    fb_draw_text(start_text_x - 100, start_text_y, "感谢您的游玩!", start_text_size, RED);
    fb_draw_text(start_text_x - 300, start_text_y + 60, "您给的100分是我继续读书的动力!", start_text_size, RED);
    fb_update();
}

void draw_player()
{
    fb_draw_image(player_x, player_y, player, 0);
}

void shoot_bullet()
{
    for (int i = 0; i < max_bullets_num; i++)
    {
        if (!bullets_used[i])
        {
            ++bullets_num;
            bullets_used[i] = true;
            bullets_x[i] = BULLET_SATRT_X;
            bullets_y[i] = BULLET_START_y;
            printf("shooting!\n");
            break;
        }
    }
}

void move_bullet()
{
    for (int i = 0; i < max_bullets_num; i++)
    {
        if (bullets_used[i])
        {
            bullets_y[i] -= bullets_speed;
            if (bullets_y[i] <= -40)
            {
                bullets_used[i] = false;
                --bullets_num;
            }
        }
    }
}

void draw_bullet()
{
    for (int i = 0; i < max_bullets_num; i++)
    {
        if (bullets_used[i])
        {
            fb_draw_image(bullets_x[i], bullets_y[i], bullet, 0);
        }
    }
}

void draw_player_info()
{
    static char health_text[30], left_bullets_text[30], score_text[30];
    sprintf(health_text, "health: %d", player_health);
    sprintf(left_bullets_text, "bullets: %d", max_bullets_num - bullets_num);
    sprintf(score_text, "score: %d", score);
    fb_draw_text(health_x, health_y, health_text, text_size, PURPLE);
    fb_draw_text(left_bullets_num_x, left_bullets_num_y, left_bullets_text, text_size, PURPLE);
    fb_draw_text(score_x, score_y, score_text, text_size, PURPLE);
}

void draw_die()
{
    for (int i = 0; i < max_expl_num; i++)
    {
        draw_background();
        fb_draw_image(player_x, player_y - 30, expls[i], 0);
        fb_update();
        usleep(microseconds);
    }
}

void draw_pending()
{
    fb_draw_image(death_x, death_y, death, 0);
    char score_text[40];
    sprintf(score_text, "最终得分:%d", score);
    fb_draw_text(death_x - 10, death_y - 130, score_text, 40, RED);
    fb_draw_text(death_x + 45, death_y - 20, "死!", 100, RED);
    fb_draw_text(death_x - 80, death_y + 300, "重新开始游戏", 60, RED);
    fb_draw_border(death_x - 85, death_y + 220, 380, 100, RED);
    fb_update();
}

void add_rock()
{
    if (rocks_num < max_rock_num)
    {
        if (rand() % 2 == 0)
            return;
        for (int i = 0; i < max_rock_num; i++)
        {
            if (!rocks_used[i])
            {
                ++rocks_num;
                rocks_used[i] = true;
                rocks_x[i] = rand() % 600 + 115;
                rocks_y[i] = -20;
                rocks_dx[i] = rand() % 40 - 20;
                rocks_dy[i] = rand() % 20 + 10;
                rocks_type[i] = rand() % 7;
                // printf("rocks #%d,x=%d,y=%d,dx=%d,dy=%d,type=%d",i,rocks_x[i],rocks_y[i],rocks_dx[i],rocks_dy[i],rocks_type[i]);
                break;
            }
        }
    }
}

void move_rock()
{
    for (int i = 0; i < max_rock_num; i++)
    {
        if (rocks_used[i])
        {
            rocks_x[i] += rocks_dx[i];
            rocks_y[i] += rocks_dy[i];
            if (ROCK_OUT_OF_UI(rocks_x[i], rocks_y[i], rocks[i]))
            {
                --rocks_num;
                rocks_used[i] = false;
            }
        }
    }
}

void draw_rock()
{
    for (int i = 0; i < max_rock_num; i++)
    {
        if (rocks_used[i])
        {
            fb_draw_image(rocks_x[i], rocks_y[i], rocks[i], 0);
        }
    }
}

void update_ui(int fd)
{
    move_bullet();
    add_rock();
    move_rock();
    draw_background();
    draw_player();
    draw_bullet();
    draw_rock();
    draw_player_info();
    collision_detection();
    fb_update();
}
void destroy_rock(int i)
{
    --rocks_num;
    rocks_used[i] = false;
}
void destroy_bullet(int i)
{
    --bullets_num;
    bullets_used[i] = false;
}

void collision_detection()
{
    for (int i = 0; i < max_rock_num; i++)
    {
        if (!rocks_used[i])
            continue;
        int rx = rocks_x[i];
        int ry = rocks_y[i];
        int rw = IMAGE_W(rocks[i]);
        int rh = IMAGE_H(rocks[i]);
        for (int j = 0; j < max_bullets_num; j++)
        {
            if (!bullets_used[j])
                continue;
            int bx = bullets_x[j];
            int by = bullets_y[j];
            if (RECT_COLLISION(rx, ry, rw, rh, bx, by) || RECT_COLLISION(rx, ry, rw, rh, bx + IMAGE_W(bullet), by + IMAGE_H(bullet)))
            {
                destroy_rock(i);
                destroy_bullet(j);
                score += rocks_type[i] * 5 + 5;
            }
        }
        if (!rocks_used[i])
            continue;
        if (RECT_COLLISION(rx, ry, rw, rh, player_collision_x, player_collision_y) || RECT_COLLISION(rx, ry, rw, rh, player_collision_left_x, player_collision_left_y) || RECT_COLLISION(rx, ry, rw, rh, player_collision_right_x, player_collision_right_y) || RECT_COLLISION(rx, ry, rw, rh, player_collision_top_x, player_collision_top_y))
        {
            player_health -= 1;
            destroy_rock(i);
            if (player_health < 0)
            {
                current_status = PENDING;
                task_delete_timer(50);
                draw_die();
                draw_pending();
            }
        }
    }
}

void touch_event_cb(int fd)
{
    switch (current_status)
    {
    case INITIAL:
        handle_initial_stage();
        break;
    case RUNNING:
        handle_running_stage();
        break;
    case PENDING:
        handle_pending_stage();
        break;
    case EXIT:
        handle_exit_stage();
        break;
    default:
        break;
    }
}

void handle_initial_stage()
{
    int type, x, y, finger;
    type = touch_read(touch_fd, &x, &y, &finger);
    switch (type)
    {
    case TOUCH_PRESS:
        // printf("TOUCH_PRESS: x=%d,y=%d,finger=%d\n", x, y, finger);
        if (IN_START_MENU(x, y))
        {
            current_status = RUNNING;
            draw_game_ui();
            task_add_timer(50, update_ui);
        }
        if (IN_EXIT_MENU(x, y))
        {
            current_status = EXIT;
        }
        break;
    case TOUCH_MOVE:
    case TOUCH_RELEASE:
        // printf("TOUCH_RELEASE: x=%d,y=%d,finger=%d\n", x, y, finger);
        break;
    case TOUCH_ERROR:
        printf("close touch fd\n");
        close(touch_fd);
        task_delete_file(touch_fd);
        break;
    default:
        return;
    }
    return;
}

void handle_running_stage()
{
    int type, x, y, finger;
    type = touch_read(touch_fd, &x, &y, &finger);
    switch (type)
    {
    case TOUCH_PRESS:
    case TOUCH_MOVE:
        // printf("TOUCH_PRESS: x=%d,y=%d,finger=%d\n", x, y, finger);
        if (IN_EXIT_MENU(x, y))
        {
            current_status = EXIT;
        }
        if (ON_MOVE(x, y))
        {
            if (x >= prev_touch_x)
                player_x += player_speed;
            if (x < prev_touch_x)
                player_x -= player_speed;
            if (player_x < background_x)
                player_x = background_x;
            if (player_x > player_max_x)
                player_x = player_max_x;
        }
        if (SHOOT(x, y))
        {
            shoot_bullet();
        }
        prev_touch_x = x;
        break;
    case TOUCH_RELEASE:
        // printf("TOUCH_MOVE: x=%d,y=%d,finger=%d\n", x, y, finger);
        break;
    case TOUCH_ERROR:
        printf("close touch fd\n");
        close(touch_fd);
        task_delete_file(touch_fd);
        break;
    default:
        return;
    }
    return;
}

void handle_pending_stage()
{
    int type, x, y, finger;
    type = touch_read(touch_fd, &x, &y, &finger);
    switch (type)
    {
    case TOUCH_PRESS:
    case TOUCH_MOVE:
    case TOUCH_RELEASE:
        // printf("TOUCH_RELEASE: x=%d,y=%d,finger=%d\n", x, y, finger);
        if (IN_EXIT_MENU(x, y))
        {
            current_status = EXIT;
        }
        if (RERUN(x, y))
        {
            current_status = RUNNING;
            player_health = player_init_health;
            player_x = player_init_x;
            player_y = player_init_y;
            bullets_num = 0;
            rocks_num = 0;
            score = 0;
            memset(bullets_used, 0, max_bullets_num);
            memset(rocks_used, 0, max_rock_num);
            draw_game_ui();
            task_add_timer(50, update_ui);
        }
        break;
    case TOUCH_ERROR:
        printf("close touch fd\n");
        close(touch_fd);
        task_delete_file(touch_fd);
        break;
    default:
        return;
    }
    return;
}

void handle_exit_stage()
{
    draw_final_menu();

    close(touch_fd);
    task_delete_file(touch_fd);

    fb_free_image(background);
    fb_free_image(player);
    fb_free_image(left_arrow);
    fb_free_image(right_arrow);
    fb_free_image(shooting);
    fb_free_image(bullet);
    fb_free_image(death);

    for (int i = 0; i < max_rock_num; i++)
        fb_free_image(rocks[i]);
    for (int i = 0; i < max_expl_num; i++)
        fb_free_image(expls[i]);

    printf("程序正常退出!\n");
    exit(EXIT_SUCCESS);
}
