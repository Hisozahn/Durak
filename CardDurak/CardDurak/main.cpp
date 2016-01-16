#include <SDL.h>
#include <stdio.h>

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
#define TIMER_DELAY ((33 / 10) * 1000 / 200)
const char*const BACKGROUND_IMAGE = "Images/background.bmp";
const char*const CARDSET_IMAGE = "Images/CardSet.bmp";
const char*const CARDBACK_1 = "Images/card_back_1.bmp";
const char*const CARDBACK_2 = "Images/card_back_2.bmp";
const char*const BUTTONS = "Images/buttons.bmp";
const char*const START_PIC = "Images/Start_pic.bmp";
const int CARD_WIDTH = 91;
const int CARD_HEIGHT = 137;
const int CARD_DX = 7;
const int CARD_DY = 6;
const int BUTTON_WIDTH = 90;
const int BUTTON_HEIGHT = 40;
const Uint8 DEFAULT_CARD_NUM = 6;
const Uint8 NUM_OF_CARDS_IN_FIELD = 6;
const int VERTICAL_OFFSET = 30;
const int HORIZONTAL_OFFSET = 30;
const int DECK_PLACE = CARD_WIDTH + 102;

typedef struct card{
	struct card* next;
	Uint8 card_code;
} card;
typedef struct{
	Uint8 cards[NUM_OF_CARDS_IN_FIELD * 2];
	Uint8 used[13];
	Uint8 played_num;
	Uint8 first_move;
	Uint8 trump;
} field_t;
typedef struct{
	card* first_card;
	Uint8 card_count;
	Uint8 checked;
	Uint8 in_game;
	Uint8 take;
} player_t;

int FilterEvents(void* userdata, const SDL_Event *event) {
	int width = ((int*)userdata)[0];
	int height = ((int*)userdata)[1];
	int x = event->button.x;
	int y = event->button.y;
	int xt = HORIZONTAL_OFFSET * 2 + CARD_HEIGHT;
	int yt = VERTICAL_OFFSET * 2 + CARD_HEIGHT;
	if (event->type == SDL_KEYDOWN && event->key.keysym.scancode != SDL_SCANCODE_SPACE)
		return 0;
	else if ((event->type == SDL_MOUSEBUTTONDOWN || event->type == SDL_MOUSEBUTTONUP)
		&& event->button.button != SDL_BUTTON_RIGHT && event->button.button != SDL_BUTTON_LEFT)
		return 0;
	else if (event->type == SDL_MOUSEBUTTONDOWN) {
		if (y < height - VERTICAL_OFFSET - CARD_HEIGHT)
			return 0;
		if (y > height - VERTICAL_OFFSET)
			return 0;
		if (x < HORIZONTAL_OFFSET)
			return 0;
		if (x > width - HORIZONTAL_OFFSET - DECK_PLACE &&
			x < width - DECK_PLACE / 2 - BUTTON_WIDTH / 2)
			return 0;
		if (x > width - DECK_PLACE / 2 + BUTTON_WIDTH / 2)
			return 0;
		return 1;
	}
	else if (event->type == SDL_WINDOWEVENT && event->window.event != SDL_WINDOWEVENT_SIZE_CHANGED)
		return 0;
	return 1;
}
void logSDLError(const char*const msg) {
	printf("%s SDL_Error: %s\n", msg, SDL_GetError());
}
void cleanup(const char* const format, ...) {
	va_list valist;
	size_t i, num = SDL_strlen(format);
	va_start(valist, format);
	for (i = 0; i < num; i++) {
		if (format[i] == 'm') {
			SDL_RemoveTimer(va_arg(valist, SDL_TimerID));
			SDL_Delay(TIMER_REMOVING_TIME);
		}
		if (format[i] == 'w')
			SDL_DestroyWindow(va_arg(valist, SDL_Window*));
		if (format[i] == 'r')
			SDL_DestroyRenderer(va_arg(valist, SDL_Renderer*));
		if (format[i] == 't')
			SDL_DestroyTexture(va_arg(valist, SDL_Texture*));
		if (format[i] == 's')
			SDL_FreeSurface(va_arg(valist, SDL_Surface*));
	}
	va_end(valist);
}
Uint8 get_field_beat(field_t* const field) {
	Uint8 check = 0, i;
	if (field->cards[0] == UINT8_MAX)
		return 1;
	for (i = 0; i < NUM_OF_CARDS_IN_FIELD; i++) {
		if (field->cards[i] != UINT8_MAX && field->cards[i + NUM_OF_CARDS_IN_FIELD] == UINT8_MAX)
			check = 1;
	}
	return check;
}
void button_click(Uint8 move, Uint8 *step, field_t* field, Uint8 *finish_move, Uint8 *player_ready, Uint8 *player_take, Uint8 pl) {
	Uint8 check = get_field_beat(field);
	if (!move && *step == FORWARD_STEP && !*finish_move) {
		if (!check)
			*player_ready = *finish_move = 1;
		else
			*step = FIGHT_STEP;
	}
	else if (*finish_move)
		*player_ready = 1;
	else if (pl == 3) {
		if (!check)
			*finish_move = *player_ready = 1;
		else
			*finish_move = *player_take = *player_ready = 1;
	}
}

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
	static player_t enemies[MAX_ENEMIES];
	Uint8 i = 0;
	for (; i < count; i++)
		enemies[i] = create_player();
	return enemies;
}
int* create_sizes(int w, int h) {
	static int sizes[2];
	sizes[0] = w;
	sizes[1] = h;
	return sizes;
}

Uint8 get_card_price(Uint8 card_code) {
	return card_code % 13;
}
Uint8 get_suit(Uint8 card_code) {
	return card_code / 13;
}
Uint8 get_ingame_player(player_t* const player, player_t* const enemies, Uint8 move, Uint8 mode) {
	int i;
	for (i = move; i < mode; i++) {
		if (enemies[i].in_game)
			return i;
	}
	if (player->in_game)
		return 3;
	for (i = 0; i < mode; i++) {
		if (enemies[i].in_game)
			return i;
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

void card_add(Uint8 card_code, card *const* ptr) {
	card* list_link = create_card_list(card_code);
	card* link = *(card**)ptr;
	while (link->next != NULL) {
		link = link->next;
	}
	link->next = list_link;
}
Uint8 card_get(Uint8 index, card* const ptr, Uint8 count) {
	Uint8 i;
	card* new_ptr = (card*)ptr;
	if (count <= index || index < 0) {
		logSDLError("Invalid index in \'card_get\'");
		return UINT8_MAX;
	}
	for (i = 0; i < index; i++) {
		new_ptr = new_ptr->next;
	}
	return new_ptr->card_code;
}
void card_pop(Uint8 index, card** ptr) {
	Uint8 i;
	card* del_ptr = NULL, *new_ptr = NULL;
	if (get_card_count(*ptr) < index + 1 || index < 0) {
		logSDLError("Invalid index in \'card_pop\'");
		return;
	}
	if (index == 0) {
		del_ptr = *ptr;
		*ptr = (*ptr)->next;
	}
	else {
		new_ptr = (*ptr);
		for (i = 0; i < index - 1; i++) {
			new_ptr = new_ptr->next;
		}
		del_ptr = new_ptr->next;
		new_ptr->next = del_ptr->next;
	}
	free(del_ptr);
}
void card_list_free(card *const* ptr) {
	card** link_link = (card**)ptr;
	card* list_link = *(card**)ptr;
	card* free_ptr;
	while (list_link != NULL) {
		free_ptr = list_link;
		list_link = list_link->next;
		free(free_ptr);
	}
	*link_link = NULL;
}
void player_card_pop(player_t* pl, Uint8 index) {
	card_pop(index, &(pl->first_card));
	pl->card_count = get_card_count(pl->first_card);
}
void player_card_push(player_t* pl, Uint8 card_code) {
	if (pl->first_card == NULL) {
		pl->first_card = create_card_list(card_code);
		return;
	}
	card_add(card_code, &(pl->first_card));
	pl->card_count = get_card_count(pl->first_card);
}
Uint8 player_get_checked(player_t* const pl) {
	return card_get(pl->checked, pl->first_card, pl->card_count);
}
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
void players_next_move(player_t* const player, player_t* const enemies, Uint8 pl, Uint8 *move, Uint8 mode, Uint8 *check_finish) {
	if (!player->card_count)
		player->in_game = 0;
	if (!enemies[pl].card_count)
		enemies[pl].in_game = 0;
	if (*move && !enemies[*move - 1].card_count)
		enemies[*move - 1].in_game = 0;
	if (pl == 3 && !player->take)
		*move = 0;
	else if (pl == 3 && player->take)
		*move = get_ingame_player(player, enemies, 0, mode) + 1;
	else if (!enemies[pl].take)
		*move = (get_ingame_player(player, enemies, pl, mode)) + 1 % 4;
	else if (enemies[pl].take)
		*move = ((get_ingame_player(player, enemies, pl + 1, mode)) + 1) % 4;
	player->take = enemies[pl].take = *check_finish = 0;
}
void setup(player_t* player, player_t* enemies, player_t* deck, Uint8 mode) {
	int i;
	unsigned short rnd;
	player_t start_deck = create_player();
	for (i = 0; i < 52; i++) {
		player_card_push(&start_deck, i);
	}
	for (i = 0; i < mode; i++) {
		enemies[i].in_game = 1;
	}
	for (i = mode; i < 3; i++) {
		enemies[i].in_game = 0;
	}
	for (i = 51; i >= 0; i--) {
		if (i == 0)
			rnd = 0;
		else {
			_rdrand16_step(&rnd);
			rnd = rnd % i;
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
Uint8 enemy_fight(player_t* const player, player_t* enemies, field_t* field, Uint8 move, Uint8 mode) {
	Uint8 i, card, card2, fighter = UINT8_MAX;
	fighter = get_ingame_player(player, enemies, move, mode);
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

SDL_Texture* loadTexture(const char* file, SDL_Renderer *const renderer){
	SDL_Surface *loadedImage = NULL;
	SDL_Texture *texture = NULL;
	loadedImage = SDL_LoadBMP(file);
	if (loadedImage != NULL){
		texture = SDL_CreateTextureFromSurface(renderer, loadedImage);
		SDL_FreeSurface(loadedImage);
	}
	else
		logSDLError("Image could not be loaded!");
	return texture;
}
void renderTexture_full(SDL_Texture *const tex, SDL_Renderer *const rend, int x, int y){
	SDL_Rect pos;
	pos.x = x;
	pos.y = y;
	SDL_QueryTexture(tex, NULL, NULL, &pos.w, &pos.h);
	if (SDL_RenderCopy(rend, tex, NULL, &pos)) {
		logSDLError("ren_part failed!");
	}
}
void renderTexture_part_card(SDL_Texture *const tex, SDL_Renderer *const rend, int x, int y, int xt, int yt){
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
		logSDLError("ren_part failed!");
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
		logSDLError("ren_part failed!");
	}
}

static int side(Uint8 mode) {
	SDL_Window* window = NULL;
	SDL_Renderer *renderer = NULL;
	SDL_Texture *background = NULL, *cardset = NULL, *card_back1 = NULL, *card_back2 = NULL, *buttons = NULL;
	int x_ch = 0, y_ch = 0, i;
	int* sizes = create_sizes(SCREEN_WIDTH, SCREEN_HEIGHT);
	player_t me = create_player();
	player_t* enemies = create_enemies(mode);
	player_t deck = create_player();
	field_t field = create_field();
	SDL_Event e;
	Uint8 movement = 0, move = 0, quit = 0, step = 0, finish_move = 0, player_ready = 0
		,  button = 0, check_finish = 0, button_num = 0, click = 0;
	Uint32 time = SDL_GetTicks();
	sizes[0] = SCREEN_WIDTH;
	sizes[1] = SCREEN_HEIGHT;
	SDL_SetEventFilter((SDL_EventFilter)FilterEvents, sizes);
	setup(&me, enemies, &deck, mode);
	field.trump = card_get(0, deck.first_card, deck.card_count) / 13;
	if (SDL_Init(SDL_INIT_EVENTS | SDL_INIT_VIDEO) != 0){
		logSDLError("SDL could not initialize!");
		return 1;
	}
	window = SDL_CreateWindow("Lesson 2", 0,
		30, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
	if (window == NULL){
		SDL_Quit();
		logSDLError("Window could not be created!");
		return 1;
	}
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED
		| SDL_RENDERER_PRESENTVSYNC);
	if (renderer == NULL){
		logSDLError("Render could not be initialize");
		SDL_Quit();
		return 1;
	}
	background = loadTexture(BACKGROUND_IMAGE, renderer);
	cardset = loadTexture(CARDSET_IMAGE, renderer);
	card_back1 = loadTexture(CARDBACK_1, renderer);
	card_back2 = loadTexture(CARDBACK_2, renderer);
	buttons = loadTexture(BUTTONS, renderer);
	if (!background || !cardset || !card_back1 || !card_back2 || !buttons){
		cleanup("tttttrw", background, cardset, card_back1, card_back2, buttons, renderer, window);
		SDL_Quit();
		return 1;
	}
	while (!quit){
		Uint8 pl = get_ingame_player(&me, enemies, move, mode);
		Uint8 check = get_field_beat(&field);
		button_num = get_button_num(move, step, finish_move, &field, check, pl);
		SDL_PollEvent(&e);
		if (e.type == SDL_QUIT){
			quit = 1;
		}
		else if (e.type == SDL_MOUSEBUTTONDOWN && me.checked == UINT8_MAX) {
			if (!move || pl == 3) {
				movement = 1;
				button = e.button.button;
				me.checked = get_my_card_by_coordinates(e.button.x, e.button.y, sizes[0], sizes[1], me.card_count);
			}
			if (!click && me.checked == UINT8_MAX && e.button.y < sizes[1] - HORIZONTAL_OFFSET - CARD_HEIGHT / 2 + BUTTON_HEIGHT / 2
				&& e.button.y > sizes[1] - HORIZONTAL_OFFSET - CARD_HEIGHT / 2 - BUTTON_HEIGHT / 2) {
				button_click(move, &step, &field, &finish_move, &player_ready, &me.take, pl);
				click = 1;
			}
		}
		else if (e.type == SDL_MOUSEBUTTONUP && button == e.button.button) {
			movement = click = 0;
			if (!move) {
				if (me.checked != UINT8_MAX && step == FORWARD_STEP) {
					if (player_lay_card(e.button.x, e.button.y, sizes[0], sizes[1], &me, &field, pl, enemies) && e.button.button == SDL_BUTTON_LEFT)
						step = FIGHT_STEP;
				}
			}
			if (pl == 3) {
				if (me.checked != UINT8_MAX && step == FIGHT_STEP) {
					if (player_figth_card(e.button.x, e.button.y, sizes[0], sizes[1], &me, &field))
						step = FORWARD_STEP;
				}
			}
			me.checked = UINT8_MAX;
		}
		else if (e.type == SDL_WINDOWEVENT)
		{
			if (e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
				if (e.window.data1 < SCREEN_WIDTH)
					sizes[0] = SCREEN_WIDTH;
				else
					sizes[0] = e.window.data1;
				if (e.window.data2 < SCREEN_HEIGHT)
					sizes[1] = SCREEN_HEIGHT;
				else
					sizes[1] = e.window.data2;
			}

		}
		else if (e.type == SDL_KEYDOWN && e.key.keysym.scancode == SDL_SCANCODE_SPACE){
			button_click(move, &step, &field, &finish_move, &player_ready, &me.take, pl);
		}
		else if (e.type == SDL_MOUSEMOTION) {
			x_ch = e.motion.x - CARD_WIDTH / 2;
			y_ch = e.motion.y - CARD_HEIGHT / 2;
		}
		if (SDL_GetTicks() - time > TIMER_DELAY) {
			SDL_RenderClear(renderer);
			renderTexture_tile(background, renderer, sizes[0], sizes[1]);
			render_player(cardset, renderer, me, sizes[0], sizes[1]);
			render_enemies(card_back1, card_back2, renderer, enemies, sizes[0], sizes[1], mode);
			render_deck(cardset, card_back2, renderer, deck, sizes[0], sizes[1]);
			render_field(cardset, renderer, field, sizes[0], sizes[1]);
			render_button(buttons, renderer, sizes[0], sizes[1], button_num);
			if (movement == 1 && me.checked != UINT8_MAX) {
				render_card(cardset, renderer, x_ch, y_ch, player_get_checked(&me));
			}
			SDL_RenderPresent(renderer);
			time = SDL_GetTicks();
		}
		if (player_ready && finish_move) {
			players_zeroing(&player_ready, &finish_move, &step, &field, enemies, &me);
			if (deck.card_count)
				players_picking(&me, enemies, &deck, move, mode);
			players_next_move(&me, enemies, pl, &move, mode, &check_finish);
			continue;
		}
		if (step == FIGHT_STEP && pl != 3 && !finish_move) {
			enemies[pl].take = enemy_fight(&me, enemies, &field, move, mode);
			if (enemies[pl].take)
				finish_move = 1;
			step = FORWARD_STEP;
		}
		if (step == FORWARD_STEP && move && !finish_move) {
			if (enemy_forward(&me, enemies, &field, move, mode)) {
				if (pl != 3 && check_finish == 3) {
					check_finish = 0;
					finish_move = 1;
				}
				else if (pl != 3)
					check_finish++;
			}
			else
				check_finish = 0;
			step = FIGHT_STEP;
		}
	}
	cleanup("tttttrw", background, cardset, card_back1, card_back2, buttons, renderer, window);
	SDL_Quit();
	return 0;
}
int main(int argc, char* args[])
{
	SDL_Window* window = NULL;
	SDL_Renderer *renderer = NULL;
	SDL_Event e;
	Uint8 mode = 1;
	SDL_Texture *start = NULL;
	int quit = 0, width = SCREEN_WIDTH, heigth = SCREEN_HEIGHT;
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0){
		logSDLError("SDL could not initialize!");
		return 1;
	}
	window = SDL_CreateWindow("Lesson 2", 0,
		30, width, heigth, SDL_WINDOW_SHOWN);
	if (window == NULL){
		SDL_Quit();
		logSDLError("Window could not be created!");
		return 1;
	}
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED
		| SDL_RENDERER_PRESENTVSYNC);
	if (renderer == NULL){
		logSDLError("Render could not be initialize");
		SDL_Quit();
		return 1;
	}
	start = loadTexture(START_PIC, renderer);
	if (!start){
		cleanup("trw", start, renderer, window);
		SDL_Quit();
		return 1;
	}
	while (!quit){
		SDL_PollEvent(&e);
		if (e.type == SDL_QUIT){
			quit = 1;
		}
		if (e.type == SDL_MOUSEBUTTONDOWN) {
			if (e.button.x < 300) {
				quit = 2;
				mode = 1;
			}
			else if (e.button.x >= 300 && e.button.x < 600) {
				quit = 2;
				mode = 2;
			}
			else if (e.button.x >= 600) {
				quit = 2;
				mode = 3;
			}
		}
		SDL_RenderClear(renderer);
		renderTexture_full(start, renderer, 0, 0);
		SDL_RenderPresent(renderer);
	}
	cleanup("rw", renderer, window);
	if (quit != 2) {
		SDL_Quit();
		return 0;
	}
	return side(mode);
}