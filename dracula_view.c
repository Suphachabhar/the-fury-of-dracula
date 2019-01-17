////////////////////////////////////////////////////////////////////////
// COMP2521 19t0 ... the Fury of Dracula
// dracula_view.c: the DraculaView ADT implementation
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
#include <stdio.h>

#include "dracula_view.h"
#include "game.h"
#include "game_view.h"
#include "DDList.h"
#include "map.h"
// #include "map.h" ... if you decide to use the Map ADT

enum encounter
{
	NO_ENCOUNTER,
	TRAP_ENCOUNTER,
	VAMPIRE_ENCOUNTER
};

typedef struct Trail
{
	location_t location;
	enum encounter encounter;
} Trail;

typedef struct dracula_view
{
	/// @todo REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	GameView gv;
	Map world_map;
	player_message *messages;
	size_t num_of_turns;
	int health[NUM_PLAYERS];
	DLList player_path[NUM_PLAYERS];
	DLList dracula_precise_path;
	bool is_vampire_alive;
	Trail dracula_trail[TRAIL_SIZE];
} dracula_view;

//helper function
static location_t abbrevToLocation(char *abbrev);
static bool is_Hunter_Dead(dracula_view *dv, enum player player);
static void hunter_action(dracula_view *dv, enum player player);
static void hunter_decrease_health(dracula_view *dv, enum player player, int lost_health);
static void hunter_increase_health(dracula_view *dv, enum player player, int added_health);
static void dracula_move(dracula_view *dv);
static void dracula_action(dracula_view *dv);
static void dracula_decrease_health(dracula_view *dv, int lost_health);
static void dracula_increase_health(dracula_view *dv, int added_health);

dracula_view *dv_new(char *past_plays, player_message messages[])
{
	/// @todo REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	dracula_view *new = malloc(sizeof *new);
	if (new == NULL)
		err(EX_OSERR, "couldn't allocate DraculaView");
	(*new) = (dracula_view){
		.gv = gv_new(past_plays, messages),
		.world_map = map_new(),
		.dracula_precise_path = newDLList(),
		.num_of_turns = 0,
		.messages = NULL,
		.is_vampire_alive = false};
	//initialize player path
	for (int i = 0; i < NUM_PLAYERS; i++)
	{
		new->player_path[i] = newDLList();
		if (i == PLAYER_DRACULA)
			new->health[i] = GAME_START_BLOOD_POINTS;
		else
			new->health[i] = GAME_START_HUNTER_LIFE_POINTS;
	}

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
		strncpy(abbrev, token + 1, 2);
		location_t this_location = abbrevToLocation(abbrev);

		//get player for each turn
		enum player this_player = new->num_of_turns % NUM_PLAYERS;

		// 1st move to new location
		DLListInsert(new->player_path[this_player], this_location);

		if (this_player == PLAYER_DRACULA)
		{
			//dracula turn
			dracula_move(new);
			dracula_action(new);
		}
		else
		{
			//hunter turn
			hunter_action(new, this_player);
		}
		new->num_of_turns++;
	}
	free(s);
	new->messages = malloc((new->num_of_turns / NUM_PLAYERS) * sizeof(player_message));
	size_t j = 0;
	for (size_t i = PLAYER_DRACULA; i < new->num_of_turns; i += NUM_PLAYERS)
	{
		strcpy(new->messages[j], messages[i]);
		j++;
	}
	return new;
}

void dv_drop(dracula_view *dv)
{
	/// @todo REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	gv_drop(dv->gv);
	if (dv->messages != NULL)
		free(dv->messages);
	for (int i = 0; i < NUM_PLAYERS; i++)
	{
		freeDLList(dv->player_path[i]);
	}
	freeDLList(dv->dracula_precise_path);
	free(dv);
}

round_t dv_get_round(dracula_view *dv)
{
	/// @todo REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	return gv_get_round(dv->gv);
}

int dv_get_score(dracula_view *dv)
{
	/// @todo REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	return gv_get_score(dv->gv);
}

int dv_get_health(dracula_view *dv, enum player player)
{
	/// @todo REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	return gv_get_health(dv->gv, player);
}

location_t dv_get_location(dracula_view *dv, enum player player)
{
	/// @todo REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	if (player == PLAYER_DRACULA)
	{
		if (dv->dracula_precise_path->nitems == 0)
			return UNKNOWN_LOCATION;
		else
			return dv->dracula_precise_path->last->location;
	}
	else
		return gv_get_location(dv->gv, player);
}

void dv_get_player_move(
	dracula_view *dv, enum player player,
	location_t *start, location_t *end)
{
	// @todo REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	location_t trail[TRAIL_SIZE];
	dv_get_trail(dv, player, trail);
	*end = trail[0];
	*start = trail[1];
	// location_t prev = *end;
	// for (int i = 1; i < TRAIL_SIZE; i++)
	// {
	// 	if (trail[i] == -1)
	// 		*start = prev;
	// 	prev = trail[i];
	// }
	return;
}

void dv_get_locale_info(
	dracula_view *dv, location_t where,
	int *n_traps, int *n_vamps)
{
	/// @todo REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	if (dv->is_vampire_alive && get_vampire_location(dv->world_map) == where)
	{
		*n_vamps = dv->is_vampire_alive;
	}

	else
	{
		*n_vamps = 0;
	}

	*n_traps = get_traps(dv->world_map, where);

	return;
}

void dv_get_trail(
	dracula_view *dv, enum player player,
	location_t trail[TRAIL_SIZE])
{
	/// @todo REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	if (player == PLAYER_DRACULA)
	{
		DLListNode *curr = dv->dracula_precise_path->last;
		for (size_t i = 0; i < TRAIL_SIZE; i++)
		{
			if (i < dv->dracula_precise_path->nitems)
			{
				trail[i] = curr->location;
				curr = curr->prev;
			}
			else
				trail[i] = -1;
		}
	}
	else
		gv_get_history(dv->gv, player, trail);
}

location_t *dv_get_dests(
	dracula_view *dv, size_t *n_locations, bool road, bool sea)
{
	/// @todo REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	*n_locations = 0;
	if (dv->dracula_precise_path->last != NULL)
		return gv_get_connections(dv->gv, n_locations, dv->dracula_precise_path->last->location, PLAYER_DRACULA, 0, road, false, sea);
	else
		return NULL;
}

location_t *dv_get_dests_player(
	dracula_view *dv, size_t *n_locations, enum player player,
	bool road, bool rail, bool sea)
{
	/// @todo REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	*n_locations = 0;
	if (player == PLAYER_DRACULA)
		return dv_get_dests(dv, n_locations, road, sea);
	else
	{
		if (dv->player_path[player]->last == NULL)
			return NULL;
		return gv_get_connections(dv->gv, n_locations, dv->player_path[player]->last->location, player, dv_get_round(dv), road, rail, sea);
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

static bool is_Hunter_Dead(dracula_view *dv, enum player player)
{
	if (dv->health[player] == 0)
	{
		//instance teleport to hosipital
		dv->player_path[player]->last->location = ST_JOSEPH_AND_ST_MARYS;
		return true;
	}
	else
		return false;
}

static void hunter_action(dracula_view *dv, enum player player)
{
	// arrive at drucula trail
	/* for(size_t i =0;i<TRAIL_SIZE;i++){
		if(this_location == dv->dracula_trail[i])
		// all Hunters can see which positions in the trail have MOVEs that resulted in Dracula being in that city (they cannot see the actual moves)
	}
	*/

	location_t this_location = dv->player_path[player]->last->location;
	//encounter traps
	for (size_t i = 0; i < get_traps(dv->world_map, this_location); i++)
	{
		hunter_decrease_health(dv, player, LIFE_LOSS_TRAP_ENCOUNTER);
		remove_trap(dv->world_map, this_location);
		if (is_Hunter_Dead(dv, player))
			return;
	}

	//encounter vampire
	if (dv->is_vampire_alive && this_location == get_vampire_location(dv->world_map))
	{
		dv->is_vampire_alive = false;
		set_vampire_location(dv->world_map, UNKNOWN_LOCATION);
	}

	//encounter dracula
	if (this_location == dv->dracula_trail[0].location)
	{
		hunter_decrease_health(dv, player, LIFE_LOSS_DRACULA_ENCOUNTER);
		dracula_decrease_health(dv, LIFE_LOSS_HUNTER_ENCOUNTER);

		is_Hunter_Dead(dv, player);
	}

	//At the end of action phase. If the Hunter is in the same city or sea they were in last turn, they are treated as resting.
	if (dv->player_path[player]->nitems > 1 && dv->player_path[player]->last->prev->location == dv->player_path[player]->last->location)
		hunter_increase_health(dv, player, LIFE_GAIN_REST);
}

static void hunter_decrease_health(dracula_view *dv, enum player player, int lost_health)
{
	if (dv->health[player] - lost_health < 0)
	{
		dv->health[player] = 0;
	}
	else
		dv->health[player] -= lost_health;
}

static void hunter_increase_health(dracula_view *dv, enum player player, int added_health)
{
	dv->health[player] += added_health;
	if (dv->health[player] > GAME_START_HUNTER_LIFE_POINTS)
		dv->health[player] = GAME_START_HUNTER_LIFE_POINTS;
}

static void dracula_move(dracula_view *dv)
{
	//location_t this_location = dv->player_path[PLAYER_DRACULA]->last->location;

	// update dracula trail
	Trail left_trail = dv->dracula_trail[TRAIL_SIZE - 1];
	for (int i = TRAIL_SIZE - 1; i > 0; i--)
	{
		dv->dracula_trail[i] = dv->dracula_trail[i - 1];
	}

	if (left_trail.location != UNKNOWN_LOCATION)
	{
		// if it was a Trap it just vanishes without a trace (yay!)
		if (left_trail.encounter == TRAP_ENCOUNTER)
			remove_trap(dv->world_map, left_trail.location);
		// if it was a Vampire it has matured (eek!)
		if (left_trail.encounter == VAMPIRE_ENCOUNTER && dv->is_vampire_alive)
			dv->is_vampire_alive = false;
	}
}

static void dracula_action(dracula_view *dv)
{
	/*
		-- Not finished --
		The Hunters can see Dracula’s location whenever he ends his turn in his castle, 
		or in a city currently occupied by a Hunter
		this holds even if his most recent move was a HIDE
	*/

	location_t this_location = dv->player_path[PLAYER_DRACULA]->last->location;
	location_t precise_location = this_location;
	if (this_location >= DOUBLE_BACK_1 && this_location <= DOUBLE_BACK_5)
	{
		int double_back = this_location - DOUBLE_BACK_1 + 1;
		precise_location = dv->dracula_trail[double_back].location;
	}
	else if (this_location == HIDE)
	{
		precise_location = dv->dracula_trail[0].location;
	}
	else if (this_location == TELEPORT)
	{
		precise_location = CASTLE_DRACULA;
	}
	dv->dracula_trail[0].location = precise_location;
	dv->dracula_trail[0].encounter = NO_ENCOUNTER;
	DLListInsert(dv->dracula_precise_path, precise_location);

	if (precise_location == SEA_UNKNOWN || (valid_location_p(precise_location) && location_get_type(precise_location) == SEA))
		dracula_decrease_health(dv, LIFE_LOSS_SEA);
	/*
	-- Not finished --
	Hunters know whenever Dracula is at sea.
	*/

	// place encounter
	if ((valid_location_p(precise_location) && location_get_type(precise_location) == LAND))
	{
		size_t num_of_trap = get_traps(dv->world_map, precise_location);
		if ((dv->num_of_turns / NUM_PLAYERS) % 13 == 0 && num_of_trap < 3)
		{
			dv->is_vampire_alive = true;
			set_vampire_location(dv->world_map, precise_location);
			dv->dracula_trail[0].encounter = VAMPIRE_ENCOUNTER;
		}
		else
		{
			// Dracula cannot place an Encounter in a city if there are already 3 Encounters in that city.
			if (num_of_trap + (get_vampire_location(dv->world_map) == precise_location) < 3)
			{
				add_trap(dv->world_map, precise_location);
				dv->dracula_trail[0].encounter = TRAP_ENCOUNTER;
			}
		}
	}

	if (precise_location == CASTLE_DRACULA && dv->health[PLAYER_DRACULA] > 0)
		dracula_increase_health(dv, LIFE_GAIN_CASTLE_DRACULA);
}

static void dracula_decrease_health(dracula_view *dv, int lost_health)
{
	if (dv->health[PLAYER_DRACULA] - lost_health < 0)
	{
		dv->health[PLAYER_DRACULA] = 0;
		//game end
	}
	else
		dv->health[PLAYER_DRACULA] -= lost_health;
}

static void dracula_increase_health(dracula_view *dv, int added_health)
{
	dv->health[PLAYER_DRACULA] += added_health;
	if (dv->health[PLAYER_DRACULA] > GAME_START_BLOOD_POINTS)
		dv->health[PLAYER_DRACULA] = GAME_START_BLOOD_POINTS;
}