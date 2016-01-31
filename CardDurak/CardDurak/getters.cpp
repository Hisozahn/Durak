#include "durak.h"

Uint8 get_card_price(Uint8 card_code) {
	return card_code % 13;
}
Uint8 get_suit(Uint8 card_code) {
	return card_code / 13;
}
Uint8 get_ingame_player(player_t* const player, player_t* const enemies, Uint8 move, Uint8 mode) {
	Uint8 i;
	for (i = move; i < mode; i++) {
		if (enemies[i].in_game) {
			return i;
		}
			
	}
	if (player->in_game) {
		return 3;
	}
	for (i = 0; i < mode; i++) {
		if (enemies[i].in_game) {
			return i;
		}
	}
	return UINT8_MAX;
}
Uint8 get_card_count(card  * const ptr) {
	Uint8 count = 0L;
	card* new_ptr = (card*)ptr;
	while (new_ptr != NULL) {
		count++;
		new_ptr = new_ptr->next;
	}
	return count;
}
Uint8 get_button_num(Uint8 move, Uint8 step, Uint8 finish_move, field_t* const field, Uint8 check, Uint8 pl) {
	if (!move && step == FORWARD_STEP && !finish_move) {
		if (field->cards[0] == UINT8_MAX)
			return 3;
		else if (!check)
			return 0;
		else
			return 1;
	}
	else if (finish_move)
		return 0;
	else if (pl == 3) {
		if (!check)
			return 0;
		else
			return 2;
	}
	return 0;
}