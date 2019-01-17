////////////////////////////////////////////////////////////////////////
// COMP2521 19t0 ... the Fury of Dracula
// hunter_view.c: the HunterView ADT implementation
//
// 2014-07-01	v1.0	Team Dracula <cs2521@cse.unsw.edu.au>
// 2017-12-01	v1.1	Team Dracula <cs2521@cse.unsw.edu.au>
// 2018-12-31	v2.0	Team Dracula <cs2521@cse.unsw.edu.au>

#include <assert.h>
#include <err.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sysexits.h>
#include <string.h>

#include "game.h"
#include "game_view.h"
#include "hunter_view.h"
#include "DDList.h"

// #include "map.h" ... if you decide to use the Map ADT
enum
{
	MAX_TRAPS_NUMBER = 6
};

typedef struct hunter_view
{
	/// @todo REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	GameView gv;
	char *past_plays;
	player_message *messages;
	DLList player_path[NUM_PLAYERS];
	int num_of_turns;
	int traps[MAX_TRAPS_NUMBER];
	int is_vampire_alive;
	location_t vampire_location;
} hunter_view;

//helper function
static void trap_update(hunter_view *hv);
static location_t abbrevToLocation(char *abbrev);
static void hunter_update_location(hunter_view *hv, enum player player, location_t new_location);
static void apply_hunter_rule(hunter_view *hv, enum player player, char *token, location_t this_location);
static void dracula_update_location(hunter_view *hv, location_t new_location);
static void apply_dracula_rule(hunter_view *hv, char *token, location_t this_location);

hunter_view *hv_new(char *past_plays, player_message messages[])
{
	/// @todo REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	hunter_view *new = malloc(sizeof *new);
	if (new == NULL)
		err(EX_OSERR, "couldn't allocate HunterView");
	(*new) = (hunter_view){
		.gv = gv_new(past_plays, messages),
		.num_of_turns = 0,
		.past_plays = strdup(past_plays),
		.messages = NULL,
		.is_vampire_alive = 0};
	//initialize player life and path
	for (int i = 0; i < NUM_PLAYERS; i++)
	{
		new->player_path[i] = newDLList();
	}
	//initialize trap
	for (int i = 0; i < MAX_TRAPS_NUMBER; i++)
		new->traps[i] = UNKNOWN_LOCATION;
	//check past_plays
	if (strcmp(past_plays, "") == 0)
		return new;

	char *token, *s;
	s = strdup(past_plays);
	while ((token = strsep(&s, " ")) != NULL)
	{
		// apply rule for each turn
		// get location (pos1+2)
		char abbrev[3];
		abbrev[0] = token[1];
		abbrev[1] = token[2];
		abbrev[2] = '\0';
		location_t this_location = abbrevToLocation(abbrev);
		//get player for each turn
		enum player this_player = new->num_of_turns % NUM_PLAYERS;
		if (this_player == PLAYER_DRACULA)
		{
			//rule of dracula
			dracula_update_location(new, this_location);
			apply_dracula_rule(new, token, this_location);
		}
		else
		{
			hunter_update_location(new, this_player, this_location);
			apply_hunter_rule(new, this_player, token, this_location);
		}
		new->num_of_turns++;
	}
	free(s);
	new->messages = malloc(new->num_of_turns * sizeof(player_message));
	for (size_t i = 0; i < new->num_of_turns; i++)
	{
		strcpy(new->messages[i], messages[i]);
	}
	return new;
}

void hv_drop(hunter_view *hv)
{
	/// @todo REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	gv_drop(hv->gv);
	free(hv->past_plays);
	if (hv->messages != NULL)
		free(hv->messages);
	for (int i = 0; i < NUM_PLAYERS; i++)
	{
		freeDLList(hv->player_path[i]);
	}
	free(hv);
}

round_t hv_get_round(hunter_view *hv)
{
	/// @todo REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	return gv_get_round(hv->gv);
	;
}

enum player hv_get_player(hunter_view *hv)
{
	/// @todo REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	return gv_get_player(hv->gv);
}

int hv_get_score(hunter_view *hv)
{
	/// @todo REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	return gv_get_score(hv->gv);
}

int hv_get_health(hunter_view *hv, enum player player)
{
	/// @todo REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	return gv_get_health(hv->gv, player);
}

location_t hv_get_location(hunter_view *hv, enum player player)
{
	/// @todo REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	return gv_get_location(hv->gv, player);
}

void hv_get_trail(
	hunter_view *hv, enum player player,
	location_t trail[TRAIL_SIZE])
{
	/// @todo REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	gv_get_history(hv->gv, player, trail);
}

location_t *hv_get_dests(
	hunter_view *hv, size_t *n_locations,
	bool road, bool rail, bool sea)
{
	/// @todo REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	*n_locations = 0;
	enum player current_player = hv_get_player(hv);
	if (hv->player_path[current_player]->last != NULL)
		return gv_get_connections(hv->gv, n_locations, hv->player_path[current_player]->last->location, current_player, hv_get_round(hv), road, rail, sea);
	else
		return NULL;
}

location_t *hv_get_dests_player(
	hunter_view *hv, size_t *n_locations, enum player player,
	bool road, bool rail, bool sea)
{
	/// @todo REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	*n_locations = 0;
	if (player == PLAYER_DRACULA)
	{
		//return gv_get_connections(hv->gv, n_locations, hv->player_path[PLAYER_DRACULA]->last->location, PLAYER_DRACULA, hv_get_round(hv), road, false, sea);
		size_t n_temp, n_valid = 0;
		location_t *locations_temp = gv_get_connections(hv->gv, &n_temp, hv->player_path[PLAYER_DRACULA]->last->location, PLAYER_DRACULA, hv_get_round(hv), road, false, sea);
		for (size_t i = 0; i < n_temp; i++)
			if (valid_location_p(locations_temp[i]))
				n_valid++;
		location_t *result = malloc(n_valid * sizeof(location_t));
		size_t n_count = 0;
		for (size_t i = 0; i < n_temp; i++)
			if (valid_location_p(locations_temp[i]))
			{
				result[n_count] = locations_temp[i];
				n_count++;
			}

		free(locations_temp);
		*n_locations = n_valid;
		return result;
	}
	else
	{
		if (hv->player_path[player]->last != NULL)
			return gv_get_connections(hv->gv, n_locations, hv->player_path[player]->last->location, player, hv_get_round(hv), road, rail, sea);
		else
			return NULL;
	}
}

static location_t abbrevToLocation(char *abbrev)
{
	if (strcmp(abbrev, "C?") == 0)
		return CITY_UNKNOWN;
	else if (strcmp(abbrev, "S?") == 0)
		return SEA_UNKNOWN;
	else if (strcmp(abbrev, "HI") == 0)
		return HIDE;
	else if (abbrev[0] == 'D')
	{
		switch (abbrev[1])
		{
		case '1':
			return DOUBLE_BACK_1;
		case '2':
			return DOUBLE_BACK_2;
		case '3':
			return DOUBLE_BACK_3;
		case '4':
			return DOUBLE_BACK_4;
		case '5':
			return DOUBLE_BACK_5;
		default:
			return location_find_by_abbrev(abbrev);
		}
	}
	else
		return location_find_by_abbrev(abbrev);
}

static void hunter_update_location(hunter_view *hv, enum player player, location_t new_location)
{
	DLListInsert(hv->player_path[player], new_location);
}

static void apply_hunter_rule(hunter_view *hv, enum player player, char *token, location_t this_location)
{
	for (size_t i = 3; i < 7; i++)
	{
		switch (token[i])
		{
		case 'T':
			// Trap encountered
			for (size_t j = 0; j < MAX_TRAPS_NUMBER; j++)
				if (hv->traps[j] == this_location)
					hv->traps[j] = -1;
			break;
		case 'V':
			// Immature Vampire was encountered
			hv->is_vampire_alive = 0;
			break;
		case 'D':
			// Dracula was confronted
			break;
		}
	}
}

static void dracula_update_location(hunter_view *hv, location_t new_location)
{
	DLListInsert(hv->player_path[PLAYER_DRACULA], new_location);
}

static void apply_dracula_rule(hunter_view *hv, char *token, location_t this_location)
{
	for (int i = 3; i <= 4; i++)
	{
		switch (token[i])
		{
		case 'T':
			// Trap was placed
			hv->traps[0] = this_location;
			break;
		case 'V':
			// Immature Vampire was placed
			hv->vampire_location = this_location;
			hv->is_vampire_alive = 1;
			break;
		}
	}
	switch (token[5])
	{
	case 'M':
		// Trap left trail
		break;
	case 'V':
		// Vampire has matured

		hv->is_vampire_alive = 0;
		break;
	}
	trap_update(hv);
}

static void trap_update(hunter_view *hv)
{
	for (int i = MAX_TRAPS_NUMBER - 1; i > 0; i--)
	{
		hv->traps[i] = hv->traps[i - 1];
	}
	hv->traps[0] = UNKNOWN_LOCATION;
}