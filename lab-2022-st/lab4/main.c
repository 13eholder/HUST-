#include <stdio.h>
#include "../common/common.h"
#include <stdbool.h>

#define COLOR_BACKGROUND	FB_COLOR(0xff,0xff,0xff)

#define RED	FB_COLOR(255,0,0)
#define ORANGE	FB_COLOR(255,165,0)
#define YELLOW	FB_COLOR(255,255,0)
#define GREEN	FB_COLOR(0,255,0)
#define CYAN	FB_COLOR(0,127,255)
#define BLUE	FB_COLOR(0,0,255)
#define PURPLE	FB_COLOR(139,0,255)
#define WHITE   FB_COLOR(255,255,255)
#define BLACK   FB_COLOR(0,0,0)

#define MAX_FINGER 10
#define MAX_PIXELS 1200

#define UNUSED -1

#define SIZE(a) (sizeof(a)/sizeof(a[0]))

typedef struct Point{
    int x,y;
};

int colors[9]={RED,ORANGE,YELLOW,GREEN,CYAN,BLUE,PURPLE,WHITE,BLACK};
struct Point points[MAX_FINGER][MAX_PIXELS];
int count[MAX_FINGER];

void draw_rect(int x, int y, int w, int h, int color){
    if(x < 0) { w += x; x = 0;}
	if(x+w > SCREEN_WIDTH) { w = SCREEN_WIDTH-x;}
	if(y < 0) { h += y; y = 0;}
	if(y+h >SCREEN_HEIGHT) { h = SCREEN_HEIGHT-y;}
	if(w<=0 || h<=0) return;

	int *buf = _begin_draw(x,y,w,h);
    for(int i=x;i<x+w;i++){
        for(int j=y;j<y+h;j++){
            FB_DRAW_PIEXL(buf,i,j,color);
        }
    }
	return;
}

void draw_line(struct Point start, struct Point end,int color)
{
    int x1=start.x,y1=start.y,x2=end.x,y2=end.y;
    int dx= (x1>x2)?-1:1;
    int dy= (y1>y2)?-1:1;
    if(x2==x1){
        for(int y=y1;y!=(y2+dy);y+=dy)
            draw_rect(x1,y,20,20,color);
    }else{
        double gradient= 1.0* (y2-y1)/ (x2-x1);
        if(abs(gradient)<=0.5){
            for(int x=x1;x!=(x2+dx);x+=dx){
                int y= (x-x1)*(y2-y1)/(x2-x1)+y1;
                draw_rect(x,y,20,20,color);
            }
        }else{
            for(int y=y1;y!=(y2+dy);y+=dy){
                int x= (y-y1)*(x2-x1)/(y2-y1)+x1;
                draw_rect(x,y,20,20,color);
            }
        }
    }    
}
void draw_clear(){
    fb_draw_border(0,0,160,100,BLACK);
    fb_draw_text(20,60,"清",40,CYAN);
    fb_draw_text(80,60,"除",40,CYAN);
}

void clear(){
    fb_draw_rect(0,0,SCREEN_WIDTH,SCREEN_HEIGHT,WHITE);
    draw_clear();
}

static int touch_fd;
static void touch_event_cb(int fd)
{
	int type,x,y,finger;
	type = touch_read(fd, &x,&y,&finger);

	switch(type){
	case TOUCH_PRESS:
		printf("TOUCH_PRESS：x=%d,y=%d,finger=%d\n",x,y,finger);
        if(0<=x&&x<=160&&0<=y&&y<=100){
            clear();
        }
        count[finger]=0;
        points[finger][0].x=x;
        points[finger][0].y=y;
		break;
	case TOUCH_MOVE:
		printf("TOUCH_MOVE：x=%d,y=%d,finger=%d\n",x,y,finger);
        ++count[finger];
        points[finger][count[finger]].x=x;
        points[finger][count[finger]].y=y;
        draw_line(points[finger][count[finger]-1],points[finger][count[finger]],colors[finger]);
		break;
	case TOUCH_RELEASE:
		printf("TOUCH_RELEASE：x=%d,y=%d,finger=%d\n",x,y,finger);
		break;
	case TOUCH_ERROR:
		printf("close touch fd\n");
		close(fd);
		task_delete_file(fd);
		break;
	default:
		return;
	}
	fb_update();
	return;
}

int main(int argc, char *argv[])
{
	fb_init("/dev/fb0");
    font_init("./font.ttc");
	fb_draw_rect(0,0,SCREEN_WIDTH,SCREEN_HEIGHT,COLOR_BACKGROUND);
    draw_clear();
	fb_update();

	//打开多点触摸设备文件, 返回文件fd
	touch_fd = touch_init("/dev/input/event2");
	//添加任务, 当touch_fd文件可读时, 会自动调用touch_event_cb函数
	task_add_file(touch_fd, touch_event_cb);
	
	task_loop(); //进入任务循环
	return 0;
}
