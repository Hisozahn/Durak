#include "durak.h"

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
	while (list_link->next != NULL) {
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
		pl->card_count = 1;
		return;
	}
	card_add(card_code, &(pl->first_card));
	pl->card_count = get_card_count(pl->first_card);
}
Uint8 player_get_checked(player_t* const pl) {
	return card_get(pl->checked, pl->first_card, pl->card_count);
}