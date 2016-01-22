#include "durak.h"

Uint8 enemy_get_min_by_suit(player_t* const enemy, Uint8 price, Uint8 suit, Uint8 trump) {
	Uint8 i, m = UINT8_MAX, count = enemy->card_count, my_price = UINT8_MAX, card;
	for (i = 0; i < count; i++) {
		card = card_get(i, enemy->first_card, count);
		if (get_suit(card) == suit && get_card_price(card) < my_price
			&& get_card_price(card) >= price) {
			my_price = get_card_price(card);
			m = i;
		}
	}
	if (m == UINT8_MAX) {
		for (i = 0; i < count; i++) {
			card = card_get(i, enemy->first_card, count);
			if (get_suit(card) == trump && suit != trump) {
				my_price = get_card_price(card);
				m = i;
			}
		}
	}
	if (m == UINT8_MAX)
		return UINT8_MAX;
	else
		return m;
}
Uint8 enemy_get_min_by_price(player_t* const enemy, field_t* const field) {
	Uint8 i, m = UINT8_MAX, count = enemy->card_count, my_price = UINT8_MAX, card;
	for (i = 0; i < count; i++) {
		card = card_get(i, enemy->first_card, count);
		if (get_card_price(card) < my_price && (field->used[get_card_price(card)] || field->first_move)
			&& get_suit(card) != field->trump) {
			my_price = get_card_price(card);
			m = i;
		}
	}
	if (m == UINT8_MAX) {
		for (i = 0; i < count; i++) {
			card = card_get(i, enemy->first_card, count);
			if (get_card_price(card) < my_price && (field->used[get_card_price(card)] || field->first_move)) {
				my_price = get_card_price(card);
				m = i;
			}
		}
	}
	if (m == UINT8_MAX)
		return UINT8_MAX;
	else
		return m;
}
Uint8 enemy_fight(player_t* const player, player_t* enemies, field_t* field, Uint8 move, Uint8 mode, Uint8 fighter) {
	Uint8 i, card, card2;
	if (fighter == UINT8_MAX)
		return UINT8_MAX;
	for (i = 0; i < NUM_OF_CARDS_IN_FIELD; i++) {
		if (field->cards[i] == UINT8_MAX || field->cards[i + NUM_OF_CARDS_IN_FIELD] != UINT8_MAX)
			continue;
		card = enemy_get_min_by_suit(&(enemies[fighter]), get_card_price(field->cards[i]) + 1, get_suit(field->cards[i]), field->trump);
		if (card == UINT8_MAX)
			return 1;
		else {
			card2 = card_get(card, (enemies[fighter]).first_card, (enemies[fighter]).card_count);
			player_card_pop(&(enemies[fighter]), card);
			field->cards[i + NUM_OF_CARDS_IN_FIELD] = card2;
			field->used[get_card_price(card2)] = 1;
		}
	}
	return 0;
}
Uint8 enemy_forward(player_t* const player, player_t* enemies, field_t* field, Uint8 move, Uint8 mode) {
	Uint8 i, card, card2, fighter = get_ingame_player(player, enemies, move, mode);
	for (i = 0; i < NUM_OF_CARDS_IN_FIELD; i++) {
		if (field->cards[i] != UINT8_MAX)
			continue;
		card = enemy_get_min_by_price(&(enemies[move - 1]), field);
		if (card == UINT8_MAX)
			return 1;
		if ((fighter != 3 && !(enemies[fighter]).card_count) || (fighter == 3 && player->card_count == 0))
			return 1;
		else {
			card2 = card_get(card, (enemies[move - 1]).first_card, (enemies[move - 1]).card_count);
			player_card_pop(&(enemies[move - 1]), card);
			field->cards[i] = card2;
			field->used[get_card_price(card2)] = 1;
			field->first_move = 0;
			return 0;
		}
	}
	return 0;
}