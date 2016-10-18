#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdlib.h>

#include "cpu_speed.h"
#include "graphics.h"
#include "lcd.h"
#include "sprite.h"


// Initial Vars

unsigned char PlayerLives = 9;
unsigned char PlayerScore = 0;
unsigned char SnakeSpeed = 30;

typedef int bool;
#define true 1
#define false 0

bool ShowWalls = false;

Sprite snake_body;
unsigned char snake_body_bmp[3] = {0b11111111, 0b11111111, 0b11111111};

Sprite snake_food;
unsigned char snake_food_bmp[3] = {0b11111111, 0b11111111, 0b11111111};


unsigned char SNAKE_BODY_LENGTH = 1;
unsigned char SNAKE_DIRECTION = 0;

// General struct to position snake, snake body & food
struct SPRITE {
	unsigned char x;
	unsigned char y;
};

struct walls {
	unsigned char x1;
	unsigned char y1;
	unsigned char x2;
	unsigned char y2;
};

#define MAX_SNAKE_SIZE 30

struct SPRITE snake[MAX_SNAKE_SIZE]; 
struct SPRITE food;
struct walls wall[3];

void initial_screen() {
	
	clear_screen();
	
	draw_string(LCD_X/2-40, LCD_Y/3, "Austin Wilshire");
	draw_string(LCD_X/2-20, LCD_Y/1.5, "n9337407");

	show_screen();
	
}

void initial_setup() {
	// Set the CPU speed to 8MHz (you must also be compiling at 8MHz)
        set_clock_speed(CPU_8MHz);

        // Get our data directions set up as we please (everything is an output unless specified...)
        DDRB = 0b01111100;
        DDRF = 0b10011111;
        DDRD = 0b11111100;

        // Initialise the LCD screen
        lcd_init(LCD_DEFAULT_CONTRAST);
	// Backlight
	DDRC |= 1 << PIN7;
	PORTC = 1 << PIN7;
	//sei();
}


void draw_snake_body(unsigned char x, unsigned char y) {
	init_sprite(&snake_body, x, y, 3, 3, snake_body_bmp);
	draw_sprite(&snake_body);
}

void draw_snake_head(unsigned char x, unsigned char y) {
	// draws head of the snake
	// Keeping track of the snake makes it easy to keep track of where it is going
	draw_snake_body(x, y);
	set_pixel(x + 1, y + 1, 0);
}

void draw_snake() {
	draw_snake_head(snake[0].x, snake[0].y);

	unsigned char i;
	for (i = 0; i < SNAKE_BODY_LENGTH; i++) {
		draw_snake_body(snake[i].x, snake[i].y);
	}

	show_screen();
}

void reset_snake() {
	//clear_screen();
	PlayerLives -= 1;
	snake[0].x = 20;
	snake[0].y = 15;
	SNAKE_BODY_LENGTH = 1;
	
	//_delay_ms(200);
	SNAKE_DIRECTION = 0;
	//show_screen();
}

void player_controls() {

        // The right button (SW3)
        if((PINF >> 5) & 0b1 ){
                ShowWalls = true;
        // The left button (SW2)
        } else if ((PINF >> 6) & 0b1) {
                ShowWalls = false;
        // The up button
        } else if ((PIND >> 1) & 0b1){
		if (SNAKE_DIRECTION == 2 ){
			reset_snake();
		} else {
			SNAKE_DIRECTION = 1;
		}
		
        // The Down button
        } else if ((PINB >> 7) & 0b1) {
		if (SNAKE_DIRECTION == 1 ){
                        reset_snake();
                } else {
			SNAKE_DIRECTION = 2;
		}
	// Right
        } else if (PIND & 1) {
		if (SNAKE_DIRECTION == 4 ){
                        reset_snake();
                } else {
			SNAKE_DIRECTION = 3;
		}
	// Left
	} else if ((PINB >> 1) & 1){
		if (SNAKE_DIRECTION == 3 ){
                        reset_snake();
                } else {
			SNAKE_DIRECTION = 4;
		}
	}       
       
}

void snake_movement() {
	// 1 = up
	// 2 = down
	// 3 = right
	// 4 = left
	if (SNAKE_DIRECTION != 0) {
		if (SNAKE_DIRECTION == 1){
			//_delay_ms(30);
			snake[0].y -= 1;
		} else if (SNAKE_DIRECTION == 2) {
			//_delay_ms(30);
			snake[0].y += 1;
		} else if (SNAKE_DIRECTION == 3) {
			//_delay_ms(30);
			snake[0].x += 1; 
		} else if (SNAKE_DIRECTION == 4) {
			//_delay_ms(30);
			snake[0].x -= 1;
		} else {
			snake[0].x = snake[0].x;
			snake[0].y = snake[0].y;
		}
	}
}

void body_movement() {
        // Checks body length, and moves each body part 1 place in the array
	for (unsigned char i = SNAKE_BODY_LENGTH - 1; i > 0; i--) {
		snake[i] = snake[i - 1];
	}
}

void snake_self_collision() {
	for (unsigned char i = 2; i < SNAKE_BODY_LENGTH; i++) {
		if (snake[0].x == snake[i].x && snake[0].y == snake[i].y){
			PlayerLives -= 1;
			reset_snake();
		}
	}
}

void draw_food() {	
	init_sprite(&snake_food, food.x, food.y, 3, 3, snake_food_bmp);
	draw_sprite(&snake_food);
}

void new_food_position(){
	int random_x = (rand() % (30 + 1 - 1)) + 5; // 30 = max, 5 = min for range
	int random_y = (rand() % (30 + 1 - 1)) + 10;
	food.x = random_x;
	food.y = random_y;
}

void spawn_food(){
	new_food_position();
	
	for (int i = 0; i > SNAKE_BODY_LENGTH; i++){
        	if (food.x == snake[i].x && food.y == snake[i].y){
			new_food_position();
		}
        }

	for (int x = 0; x > 2; x++){
		if ((food.x >= wall[x].x1 && food.x <= wall[x].x2) && (food.y >= wall[x].y1 && food.y <= wall[x].y2)){
			new_food_position();
		}

	}
}

void check_food(){
	if ((snake[0].x == food.x || snake[0].x == food.x + 1 || snake[0].x == food.x - 1) && (snake[0].y == food.y || snake[0].y == food.y + 1 || snake[0].y == food.y - 1)){
		SNAKE_BODY_LENGTH += 1;
		if (ShowWalls == true) {
			PlayerScore += 2;
		} else {
			PlayerScore += 1;
		}	
		spawn_food();
		for (int i = 0; i > SNAKE_BODY_LENGTH; i++){
			if (food.x == snake[i].x && food.y == snake[i].y){}
		}
		draw_food();
			
	}
}

void show_player_stats() {
	draw_string(40, 0, "L: ");
	draw_char(50, 0, PlayerLives + '0');
        draw_string(5, 0, "S: ");
	draw_char(15, 0, PlayerScore + '0');
}

void draw_walls() {
	wall[0].x1 = 5;
	wall[0].y1 = 15;
	wall[0].x2 = 5;
	wall[0].y2 = 25;

	wall[1].x1 = 13;
	wall[1].y1 = 30;
	wall[1].x2 = 25;
	wall[1].y2 = 30;
	
	wall[2].x1 = 50;
	wall[2].y1 = 15;
	wall[2].x2 = 50;
	wall[2].y2 = 30;

	if (ShowWalls == true) {
		draw_line(wall[0].x1, wall[0].y1, wall[0].x2, wall[0].y2);
		draw_line(wall[1].x1, wall[1].y1, wall[1].x2, wall[1].y2);
		draw_line(wall[2].x1, wall[2].y1, wall[2].x2, wall[2].y2);
		
	}
}

void check_walls() {
	if (ShowWalls == true){

		for (unsigned char i = 0; i < 3; i++){
			if ((snake[0].x >= wall[i].x1 && snake[0].x <= wall[i].x2) && (snake[0].y >= wall[i].y1 && snake[0].y <= wall[i].y2)){
				PlayerLives -= 1;
				reset_snake();
			}
		}
	}
}

void snake_renter(){
	if (snake[0].x <= 1){
		snake[0].x = 80;
	} else if (snake[0].x >= 80){
		snake[0].x = 1;
	} else if (snake[0].y <= 10){
		snake[0].y = 40;
	} else if (snake[0].y >= 40){
		snake[0].y = 10;
	} 
}

void update(){
	clear_screen();
	draw_food();
	show_player_stats();
	draw_snake();
        player_controls();
	snake_movement();
	//snake_renter();
        if (SNAKE_DIRECTION != 0) {
		snake_renter();
		body_movement();
	}
	check_food();
	snake_self_collision();
	draw_walls();
	check_walls();
	show_screen();
}

int main(void) {
	initial_setup();
	initial_screen();
	_delay_ms(2000);
	show_player_stats();
	
	snake[0].x = 20;
	snake[0].y = 15;	
	
	new_food_position();
	
	while (PlayerLives > 0) {
		_delay_ms(30);
		update();
	}
	clear_screen();	
	draw_string(LCD_X/2-25, LCD_Y/3, "GAME OVER");
	show_screen();
	_delay_ms(2000);

}
