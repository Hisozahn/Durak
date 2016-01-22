#include "durak.h"

void logSDLError(const char*const msg) {
	//printf("%s SDL_Error: %s\n", msg, SDL_GetError());
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