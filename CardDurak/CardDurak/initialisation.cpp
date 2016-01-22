#include "durak.h"

field_t create_field() {
	field_t fi;
	Uint8 i = 0;
	for (; i < NUM_OF_CARDS_IN_FIELD * 2; i++) {
		fi.cards[i] = UINT8_MAX;
	}
	for (i = 0; i < 13; i++)
		fi.used[i] = 0;
	fi.played_num = 0;
	fi.first_move = 1;
	fi.trump = UINT8_MAX;
	return fi;
}
player_t create_player() {
	player_t pl;
	pl.card_count = 0;
	pl.checked = UINT8_MAX;
	pl.first_card = NULL;
	pl.take = 0;
	return pl;
}
card* create_card_list(Uint8 card_code) {
	card* ptr = (card*)malloc(sizeof(card));
	ptr->card_code = card_code;
	ptr->next = NULL;
	return ptr;
}
player_t* create_enemies(Uint8 count) {
	//player_t* enemies = (player_t*)malloc(sizeof(player_t)*MAX_ENEMIES);
	static player_t enemies[MAX_ENEMIES];
	Uint8 i = 0;
	for (; i < count; i++)
		enemies[i] = create_player();
	return enemies;
}
int* create_sizes(int w, int h) {
	static int sizes[2];
	//int* sizes = (int*)malloc(sizeof(int)*2);
	sizes[0] = w;
	sizes[1] = h;
	return sizes;
}