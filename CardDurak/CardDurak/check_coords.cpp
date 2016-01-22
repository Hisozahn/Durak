#include "durak.h"

Uint8 get_my_card_by_coordinates(int x, int y, int width, int height, Uint8 count) {
	/*if (y < height - VERTICAL_OFFSET - CARD_HEIGHT)
	return UINT8_MAX;
	if (y > height - VERTICAL_OFFSET)
	return UINT8_MAX;
	if (x < HORIZONTAL_OFFSET)
	return UINT8_MAX;*/
	if (x > width - HORIZONTAL_OFFSET - DECK_PLACE)
		return UINT8_MAX;
	if (x > width - HORIZONTAL_OFFSET - CARD_WIDTH - DECK_PLACE)
		return count - 1;
	double dd;
	Uint8 res;
	width -= (HORIZONTAL_OFFSET * 2 + CARD_WIDTH + DECK_PLACE);
	x -= HORIZONTAL_OFFSET;
	dd = (double)width / (double)(count - 1);
	res = x / dd;
	if (dd > (double)CARD_WIDTH && res * dd + CARD_WIDTH < x || (count == 1
		&& (x < width / 2 || x > width / 2 + CARD_WIDTH)))
		return UINT8_MAX;
	return res;

}
Uint8 get_field_card_by_coordinates(int x, int y, int width, int height) {
	int xt = HORIZONTAL_OFFSET * 2 + CARD_HEIGHT;
	int yt = VERTICAL_OFFSET * 2 + CARD_HEIGHT, i = 0;
	width -= (xt * 2 + CARD_WIDTH + DECK_PLACE);
	height -= (yt * 2 + CARD_HEIGHT);
	for (; i < 6; i++) {
		if (x > xt + i % 3 * width / 2 && x < xt + i % 3 * width / 2 + CARD_WIDTH &&
			y > yt + height * (i / 3) && y < yt + CARD_HEIGHT + height * (i / 3))
			return i;
	}
	return UINT8_MAX;
}
Uint8 player_lay_card(int x, int y, int width, int height, player_t* player, field_t* field, Uint8 fighter, player_t* const enemies) {
	int xt = HORIZONTAL_OFFSET * 2 + CARD_HEIGHT, i = 0;
	int yt = VERTICAL_OFFSET * 2 + CARD_HEIGHT;
	Uint8 check = player_get_checked(player);
	width -= (xt * 2 + CARD_WIDTH + DECK_PLACE);
	height -= (yt * 2 + CARD_HEIGHT);
	if (y > yt && y < yt + height + CARD_HEIGHT && x > xt && x < xt + width + CARD_HEIGHT) {
		if (field->first_move == 1 || field->used[check % 13] == 1) {
			for (; i < NUM_OF_CARDS_IN_FIELD; i++) {
				if (fighter != 3 && !(enemies[fighter]).card_count)
					return 0;
				if (field->cards[i] == UINT8_MAX) {
					field->cards[i] = check;
					player_card_pop(player, player->checked);
					field->used[check % 13] = 1;
					field->first_move = 0;
					return 1;
				}
			}
		}
	}
	return 0;
}
Uint8 player_figth_card(int x, int y, int width, int height, player_t* player, field_t* field) {
	Uint8 card_num = get_field_card_by_coordinates(x, y, width, height);
	Uint8 check = player_get_checked(player);
	Uint8 card = field->cards[card_num];
	if (card_num == UINT8_MAX || field->cards[card_num + NUM_OF_CARDS_IN_FIELD] != UINT8_MAX)
		return 0;
	if (card != UINT8_MAX
		&& !(get_suit(check) != field->trump && get_suit(card) == field->trump)
		&& ((get_card_price(check) > get_card_price(card) && get_suit(check) == get_suit(card))
			|| (get_suit(check) == field->trump && get_suit(card) != field->trump))) {
		field->cards[card_num + NUM_OF_CARDS_IN_FIELD] = check;
		player_card_pop(player, player->checked);
		field->used[check % 13] = 1;
		return 1;
	}
	return 0;
}
