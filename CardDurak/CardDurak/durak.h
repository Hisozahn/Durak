#ifndef _DURAK_H_
#define _DURAK_H_

#include "SDL.h"
#define SCREEN_WIDTH 900
#define SCREEN_HEIGHT 700
#define MAX_ENEMIES 3
#define UINT8_MAX 0xFF
#define HEARTS 0
#define DIAMONDS 1
#define CLUBS 2
#define SPADES 3
#define FORWARD_STEP 0
#define FIGHT_STEP 1
#define TIMER_REMOVING_TIME 20
#define TIMER_DELAY (1000 / 150)
#define RECTANGLE_H 30
#define RECTANGLE_W 30
#define NUM_OF_CARDS_IN_FIELD  6

#define BACKGROUND_IMAGE  "Images/background.bmp"
#define CARDSET_IMAGE  "Images/CardSet.bmp"
#define CARDBACK_1  "Images/card_back_1.bmp"
#define CARDBACK_2  "Images/card_back_2.bmp"
#define BUTTONS  "Images/buttons.bmp"
#define START_PIC  "Images/Start_pic.bmp"
#define CARD_WIDTH  91
#define CARD_HEIGHT  137
#define CARD_DX  7
#define CARD_DY  6
#define BUTTON_WIDTH  90
#define BUTTON_HEIGHT  40
#define DEFAULT_CARD_NUM  6

#define VERTICAL_OFFSET  30
#define HORIZONTAL_OFFSET  30
#define DECK_PLACE  193

typedef struct card {
	struct card* next;
	Uint8 card_code;
} card;
typedef struct {
	Uint8 cards[NUM_OF_CARDS_IN_FIELD * 2];
	Uint8 used[13];
	Uint8 played_num;
	Uint8 first_move;
	Uint8 trump;
} field_t;
typedef struct {
	card* first_card;
	Uint8 card_count;
	Uint8 checked;
	Uint8 in_game;
	Uint8 take;
} player_t;
#endif