#include "durak.h"

player_t create_player();
field_t create_field();
Uint8 get_ingame_player(player_t*, player_t*, Uint8, Uint8);
void players_picking(player_t* player, player_t* enemies, player_t* deck, Uint8 move, Uint8 mode) {
	Uint8 pl = get_ingame_player(player, enemies, move, mode);

	while (player->card_count < DEFAULT_CARD_NUM && !move && deck->card_count) {
		player_card_push(player, card_get(deck->card_count - 1, deck->first_card, deck->card_count));
		player_card_pop(deck, deck->card_count - 1);
	}
	while (move != 0 && enemies[move - 1].card_count < DEFAULT_CARD_NUM && deck->card_count) {
		player_card_push(&enemies[move - 1], card_get(deck->card_count - 1, deck->first_card, deck->card_count));
		player_card_pop(deck, deck->card_count - 1);
	}
	while (enemies[pl].card_count < DEFAULT_CARD_NUM && deck->card_count) {
		player_card_push(&enemies[pl], card_get(deck->card_count - 1, deck->first_card, deck->card_count));
		player_card_pop(deck, deck->card_count - 1);
	}
	while (player->card_count < DEFAULT_CARD_NUM && pl == 3 && deck->card_count) {
		player_card_push(player, card_get(deck->card_count - 1, deck->first_card, deck->card_count));
		player_card_pop(deck, deck->card_count - 1);
	}
}
void players_zeroing(Uint8 *player_ready, Uint8 *finish_move, Uint8 *step, field_t *field, player_t *enemies, player_t *me) {
	int i = 0, j = 0;
	*player_ready = *finish_move = 0;
	*step = FORWARD_STEP;
	field->first_move = 1;
	for (i = 0; i < 3; i++) {
		if (enemies[i].take) {
			for (j = 0; j < 12; j++) {
				if (field->cards[j] != UINT8_MAX)
					player_card_push(&enemies[i], field->cards[j]);
			}
		}
	}
	if (me->take) {
		for (i = 0; i < 12; i++) {
			if (field->cards[i] != UINT8_MAX)
				player_card_push(me, field->cards[i]);
		}
	}
	for (i = 0; i < 12; i++)
		field->cards[i] = UINT8_MAX;
	for (i = 0; i < 13; i++)
		field->used[i] = 0;
}
Uint8 players_next_move(player_t* const player, player_t* const enemies, Uint8 pl, Uint8 *move, Uint8 mode, Uint8 *check_finish) {
	Uint8 contin = 0, i;
	if (!player->card_count)
		player->in_game = 0;
	else
		contin++;
	for (i = 0; i < 3; i++) {
		if (!enemies[i].card_count)
			enemies[i].in_game = 0;
		else
			contin++;
	}
	if (pl == 3 && !player->take) {
		if (player->in_game)
			*move = 0;
		else
			*move = get_ingame_player(player, enemies, 0, mode) + 1;
	}
	else if (pl == 3 && player->take)
		*move = get_ingame_player(player, enemies, 0, mode) + 1;
	else if (!enemies[pl].take)
		*move = get_ingame_player(player, enemies, pl, mode) + 1;
	else if (enemies[pl].take)
		*move = get_ingame_player(player, enemies, pl + 1, mode) + 1;
	player->take = enemies[pl].take = *check_finish = 0;
	if (contin < 2)
		return 1;
	*move %= 4;
	return 0;
}
void setup(player_t* player, player_t* enemies, player_t* deck, Uint8 mode, Uint8* move) {
	int i;
	Uint8 rnd;
	player_t start_deck = create_player();
	*move = 0;
	for (i = 0; i < 52; i++) {
		player_card_push(&start_deck, i);
	}
	player->in_game = 1;
	for (i = 0; i < mode; i++) {
		enemies[i].in_game = 1;
	}
	for (i = mode; i < 3; i++) {
		enemies[i].in_game = 0;
	}
	for (i = 51; i >= 0; i--) {
		srand(SDL_GetTicks());
		if (i == 0)
			rnd = 0;
		else {
			rnd = rand() % i;
		}
		if (player->card_count < DEFAULT_CARD_NUM)
			player_card_push(player, card_get(rnd, start_deck.first_card, start_deck.card_count));
		else if (enemies[0].card_count < DEFAULT_CARD_NUM)
			player_card_push(&(enemies[0]), card_get(rnd, start_deck.first_card, start_deck.card_count));
		else if (enemies[1].card_count < DEFAULT_CARD_NUM && mode > 1)
			player_card_push(&(enemies[1]), card_get(rnd, start_deck.first_card, start_deck.card_count));
		else if (enemies[2].card_count < DEFAULT_CARD_NUM && mode > 2)
			player_card_push(&(enemies[2]), card_get(rnd, start_deck.first_card, start_deck.card_count));
		else
			player_card_push(deck, card_get(rnd, start_deck.first_card, start_deck.card_count));
		player_card_pop(&start_deck, rnd);
	}
}