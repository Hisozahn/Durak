#include "durak.h"
#include <stdio.h>

static int FilterEvents(void* userdata, const SDL_Event *event) {
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
player_t create_player();
field_t create_field();
static SDL_Texture* loadTexture(const char* file, SDL_Renderer *const renderer){
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
	SDL_Event sdlevent;
	sdlevent.type = SDL_USEREVENT;
	SDL_EventState(SDL_SYSWMEVENT, SDL_IGNORE);
	SDL_EventState(SDL_KEYUP, SDL_IGNORE);
	SDL_EventState(SDL_TEXTEDITING, SDL_IGNORE);
	SDL_EventState(SDL_TEXTINPUT, SDL_IGNORE);
	SDL_EventState(SDL_KEYMAPCHANGED, SDL_IGNORE);
	SDL_EventState(SDL_MOUSEWHEEL, SDL_IGNORE);
	sizes[0] = SCREEN_WIDTH;
	sizes[1] = SCREEN_HEIGHT;
	SDL_SetEventFilter((SDL_EventFilter)FilterEvents, sizes);
	setup(&me, enemies, &deck, mode, &move);
	field.trump = card_get(0, deck.first_card, deck.card_count);
	field.trump = field.trump / 13;
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
		SDL_WaitEvent(&e);
		SDL_PollEvent(&e);
		printf("%d\n", e.type);
		if (e.type == SDL_QUIT){
			quit = 1;
		}
		else if (e.type == SDL_MOUSEBUTTONDOWN && me.checked == UINT8_MAX) {
			SDL_PushEvent(&sdlevent);
			if (!move || pl == 3) {
				movement = 1;
				button = e.button.button;
				me.checked = get_my_card_by_coordinates(e.button.x, e.button.y, sizes[0], sizes[1], me.card_count);
			}
			if (!click && me.checked == UINT8_MAX && e.button.y < sizes[1] - HORIZONTAL_OFFSET - CARD_HEIGHT / 2 + BUTTON_HEIGHT / 2
				&& e.button.y > sizes[1] - HORIZONTAL_OFFSET - CARD_HEIGHT / 2 - BUTTON_HEIGHT / 2
				&& e.button.x > sizes[0] - DECK_PLACE) {
				button_click(move, &step, &field, &finish_move, &player_ready, &me.take, pl);
				click = 1;
			}
		}
		else if (e.type == SDL_MOUSEBUTTONUP && button == e.button.button) {
			movement = click = 0;
			SDL_PushEvent(&sdlevent);
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
			SDL_PushEvent(&sdlevent);
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
			SDL_PushEvent(&sdlevent);
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
			renderMark(renderer, sizes[0], sizes[1], mode, move);
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
			if (players_next_move(&me, enemies, pl, &move, mode, &check_finish))
				quit = 2;
			SDL_PushEvent(&sdlevent);
			continue;
		}
		if (step == FIGHT_STEP && pl != 3 && !finish_move) {
			enemies[pl].take = enemy_fight(&me, enemies, &field, move, mode, pl);
			if (enemies[pl].take)
				finish_move = 1;
			step = FORWARD_STEP;
			SDL_PushEvent(&sdlevent);
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
			SDL_PushEvent(&sdlevent);
		}
	}
	cleanup("tttttrw", background, cardset, card_back1, card_back2, 
		buttons, renderer, window);
	for (i = 0; i < 3; i++) {
		if (enemies[i].card_count)
			card_list_free(&enemies[i].first_card);
	}
	if (me.card_count)
		card_list_free(&me.first_card);
	SDL_Quit();
	if (quit == 2)
		return 0;
	else
		return 1;
}
int main(int argc, char* args[])
{
	SDL_Window* window = NULL;
	SDL_Renderer *renderer = NULL;
	SDL_Event e;
	Uint8 mode = 1;
	SDL_Texture *start = NULL;
	int quit = 0, exit = 0, width = SCREEN_WIDTH, heigth = SCREEN_HEIGHT;
	while (!exit) {
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
		SDL_EventState(SDL_MOUSEBUTTONDOWN, SDL_IGNORE);
		SDL_EventState(SDL_MOUSEBUTTONDOWN, SDL_ENABLE);
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
		exit = side(mode);
		quit = 0;
	}
	return 0;
}