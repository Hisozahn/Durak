#include "durak.h"

void renderTexture_full(SDL_Texture *const tex, SDL_Renderer *const rend, int x, int y) {
	SDL_Rect pos;
	pos.x = x;
	pos.y = y;
	SDL_QueryTexture(tex, NULL, NULL, &pos.w, &pos.h);
	if (SDL_RenderCopy(rend, tex, NULL, &pos)) {
		//logSDLError("ren_part failed!");
	}
}
void renderMark(SDL_Renderer *const rend, int width, int height, Uint8 mode, Uint8 move) {
	SDL_Rect rect;
	if (!move) {
		rect.x = (width - DECK_PLACE) / 2 - RECTANGLE_W / 2;
		rect.y = height - RECTANGLE_H - 2;
	}
	else if (move == 1 && mode != 1) {
		rect.x = 5;
		rect.y = height / 2 - RECTANGLE_H / 2;
	}
	else if (move == 2 || (move == 1 && mode == 1)) {
		rect.x = (width - DECK_PLACE) / 2 - RECTANGLE_W / 2;
		rect.y = 2;
	}
	else if (move == 3) {
		rect.x = width - DECK_PLACE;
		rect.y = height / 2 - RECTANGLE_H / 2;
	}
	rect.h = RECTANGLE_H;
	rect.w = RECTANGLE_W;
	SDL_RenderDrawRect(rend, &rect);
}
void renderTexture_part_card(SDL_Texture *const tex, SDL_Renderer *const rend, int x, int y, int xt, int yt) {
	SDL_Rect new_rect, old_rect;
	old_rect.x = xt;
	old_rect.y = yt;
	old_rect.h = CARD_HEIGHT;
	old_rect.w = CARD_WIDTH;
	new_rect.x = x;
	new_rect.y = y;
	new_rect.w = CARD_WIDTH;
	new_rect.h = CARD_HEIGHT;
	if (SDL_RenderCopy(rend, tex, &old_rect, &new_rect)) {
		//logSDLError("ren_part failed!");
	}
}
void renderTexture_tile(SDL_Texture *const tex, SDL_Renderer *const rend, int width, int height) {
	SDL_Rect pos;
	int i, j, xt, yt;
	SDL_QueryTexture(tex, NULL, NULL, &pos.w, &pos.h);
	xt = width / pos.w + 1;
	yt = height / pos.h + 1;
	for (i = 0; i < xt; i++) {
		int x = pos.w * i;
		for (j = 0; j < yt; j++) {
			int y = pos.h * j;
			renderTexture_full(tex, rend, x, y);
		}
	}
}
void render_card(SDL_Texture *const  cardset, SDL_Renderer *const rend, int x, int y, Uint8 card_nom) {
	int horizontal, vertical, xt, yt;
	vertical = card_nom / 13;
	horizontal = card_nom - 13 * vertical;
	xt = horizontal * CARD_WIDTH + horizontal * CARD_DX;
	yt = vertical * CARD_HEIGHT + vertical * CARD_DY;
	renderTexture_part_card(cardset, rend, x, y, xt, yt);
}
void render_player(SDL_Texture *const cardset, SDL_Renderer *const rend, player_t player, int width, int heigth) {
	Uint8 count = player.card_count;
	int y = heigth - VERTICAL_OFFSET - CARD_HEIGHT;
	int x = HORIZONTAL_OFFSET, i = 0;
	if (count == 0)
		return;
	if (count == 1 && player.checked != 0) {
		render_card(cardset, rend, (width - DECK_PLACE) / 2 - CARD_WIDTH / 2, y, card_get(0, player.first_card, player.card_count));
		return;
	}
	for (; i < count; i++) {
		if (player.checked == UINT8_MAX || i != player.checked)
			render_card(cardset, rend, x + i * (width - HORIZONTAL_OFFSET * 2 -
				CARD_WIDTH - DECK_PLACE) / (count - 1), y, card_get(i, player.first_card, player.card_count));
	}
}
void render_line(SDL_Texture *const card_back1, SDL_Texture *const card_back2, SDL_Renderer *const rend, int width, int heigth, Uint8 count, Uint8 part) {
	int y, x;
	int i = 0;
	x = HORIZONTAL_OFFSET;
	y = VERTICAL_OFFSET * 2 + CARD_HEIGHT;
	if (part == 1)
		y = VERTICAL_OFFSET;
	if (part == 2)
		x = width - HORIZONTAL_OFFSET - CARD_HEIGHT - DECK_PLACE;
	if (count == 1 && part == 1) {
		renderTexture_full(card_back1, rend, (width - DECK_PLACE) / 2 - CARD_WIDTH / 2, y);
		return;
	}
	else if (count == 1) {
		renderTexture_full(card_back2, rend, x, heigth / 2 - CARD_WIDTH / 2);
		return;
	}
	i = 0;
	if (part == 1) {
		for (; i < count; i++) {
			renderTexture_full(card_back1, rend, x + i * (width - HORIZONTAL_OFFSET * 2 -
				CARD_WIDTH - DECK_PLACE) / (count - 1), y);
		}
	}
	else {
		for (; i < count; i++) {
			renderTexture_full(card_back2, rend, x, y + i * (heigth - VERTICAL_OFFSET * 4 -
				CARD_HEIGHT * 2 - CARD_WIDTH) / (count - 1));
		}
	}
}
void render_enemies(SDL_Texture *const card_back1, SDL_Texture *const card_back2, SDL_Renderer *const rend, player_t* enemies, int width, int heigth, Uint8 mode) {
	if (mode == 1)
		return render_line(card_back1, card_back2, rend, width, heigth, enemies[0].card_count, 1);
	if (mode > 1) {
		render_line(card_back1, card_back2, rend, width, heigth, enemies[0].card_count, 0);
		render_line(card_back1, card_back2, rend, width, heigth, enemies[1].card_count, 1);
	}
	if (mode == 3)
		render_line(card_back1, card_back2, rend, width, heigth, enemies[2].card_count, 2);
}
void render_deck(SDL_Texture *const cardset, SDL_Texture *const card_back2, SDL_Renderer *const rend, player_t deck, int width, int heigth) {
	int i = 0;
	int x = width - HORIZONTAL_OFFSET - CARD_WIDTH;
	int y = heigth / 2 - CARD_HEIGHT / 2;
	if (deck.card_count == 0)
		return;
	render_card(cardset, rend, x, y, card_get(0, deck.first_card, deck.card_count));
	x = x + CARD_WIDTH / 2 - CARD_HEIGHT / 2;
	y += CARD_HEIGHT / 2;
	for (; i < deck.card_count - 1; i++) {
		renderTexture_full(card_back2, rend, x - 2 * i, y + 2 * i);
	}
}
void render_field(SDL_Texture *const cardset, SDL_Renderer *const rend, field_t field, int width, int height) {
	int i = 0;
	int x = HORIZONTAL_OFFSET * 2 + CARD_HEIGHT;
	int y = VERTICAL_OFFSET * 2 + CARD_HEIGHT;
	width -= (x * 2 + CARD_WIDTH + DECK_PLACE);
	height -= (y * 2 + CARD_HEIGHT);
	for (; i < 6; i++) {
		render_card(cardset, rend, x + width / 2 * (i % 3), y + height * (i / 3), field.cards[i]);
		render_card(cardset, rend, x + width / 2 * (i % 3) + 15, y + height * (i / 3) + 15, field.cards[i + 6]);
	}

}
void render_button(SDL_Texture *const buttons, SDL_Renderer *const rend, int width, int height, Uint8 button_nom) {
	SDL_Rect new_rect, old_rect;
	old_rect.x = BUTTON_WIDTH * button_nom;
	old_rect.y = 0;
	old_rect.h = BUTTON_HEIGHT;
	old_rect.w = BUTTON_WIDTH;
	new_rect.x = width - DECK_PLACE / 2 - BUTTON_WIDTH / 2;
	new_rect.y = height - BUTTON_HEIGHT / 2 - VERTICAL_OFFSET - CARD_HEIGHT / 2;
	new_rect.w = BUTTON_WIDTH;
	new_rect.h = BUTTON_HEIGHT;
	if (SDL_RenderCopy(rend, buttons, &old_rect, &new_rect)) {
		//logSDLError("ren_part failed!");
	}
}