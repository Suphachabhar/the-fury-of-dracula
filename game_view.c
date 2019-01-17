////////////////////////////////////////////////////////////////////////
// COMP2521 19t0 ... the Fury of Dracula
// game_view.c: GameView ADT implementation
//
// 2014-07-01	v1.0	Team Dracula <cs2521@cse.unsw.edu.au>
// 2017-12-01	v1.1	Team Dracula <cs2521@cse.unsw.edu.au>
// 2018-12-31	v2.0	Team Dracula <cs2521@cse.unsw.edu.au>

#include <assert.h>
#include <err.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <sysexits.h>
#include <string.h>

#include "game.h"
#include "game_view.h"
#include "map.h"
#include "DDList.h"
#include "Queue.h"

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

typedef struct game_view
{
	/// @todo REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	Map world_map;
	player_message *messages;
	size_t num_of_turns;
	int health[NUM_PLAYERS];
	int scores;
	bool is_vampire_alive;
	DLList player_path[NUM_PLAYERS];
	DLList dracula_precise_path;
	Trail dracula_trail[TRAIL_SIZE];
} game_view;

//helper function
static location_t abbrevToLocation(char *abbrev);
static void game_score_decrese(game_view *gv, int lost_score);
static bool is_Hunter_Dead(game_view *gv, enum player player);
static void hunter_decrease_health(game_view *gv, enum player player, int lost_health);
static void hunter_increase_health(game_view *gv, enum player player, int added_health);
static location_t *location_hunter_can_move(
	game_view *gv, size_t *n_locations,
	location_t from, enum player player, round_t round,
	bool road, bool rail, bool sea);
static void hunter_action(game_view *gv, enum player player);
static bool is_Dracula_Dead(game_view *gv);
static void dracula_decrease_health(game_view *gv, int lost_health);
static void dracula_increase_health(game_view *gv, int added_health);
static location_t *location_dracula_can_move(
	game_view *gv, size_t *n_locations,
	location_t from, bool road, bool sea);
static void dracula_move(game_view *gv);
static void dracula_action(game_view *gv);

game_view *gv_new(char *past_plays, player_message messages[])
{
	/// @todo REPLACE THIS WITH YOUR OWN IMPLEMENTATION

	//allocate memory
	puts("5555");
	game_view *new = malloc(sizeof *new);
	if (new == NULL)
		err(EX_OSERR, "couldn't allocate GameView");
	(*new) = (game_view){
		.world_map = map_new(),
		.messages = NULL,
		.num_of_turns = 0,
		.scores = GAME_START_SCORE,
		.is_vampire_alive = false,
		.dracula_precise_path = newDLList(),
	};

	//initialize player life and path
	for (int i = 0; i < NUM_PLAYERS; i++)
	{
		new->player_path[i] = newDLList();
		if (i == PLAYER_DRACULA)
			new->health[i] = GAME_START_BLOOD_POINTS;
		else
			new->health[i] = GAME_START_HUNTER_LIFE_POINTS;
	}
	//initialize dracula trail
	for (int i = 0; i < TRAIL_SIZE; i++)
	{
		new->dracula_trail[i].location = UNKNOWN_LOCATION;
		new->dracula_trail[i].encounter = NO_ENCOUNTER;
	}

	//check past_plays
	if (strcmp(past_plays, "") == 0)
		return new;

	//analyze each turn
	char *token, *s;
	s = strdup(past_plays);
	while ((token = strsep(&s, " ")) != NULL)
	{
		// get abbrev location from token
		char abbrev[3];
		strncpy(abbrev, token + 1, 2);
		location_t this_location = abbrevToLocation(abbrev);

		//get player for each turn
		enum player this_player = gv_get_player(new);

		// 0th revive dead hunter
		if (this_player != PLAYER_DRACULA)
			if (new->health[this_player] == 0 && new->player_path[this_player]->last->location == ST_JOSEPH_AND_ST_MARYS)
				new->health[this_player] = GAME_START_HUNTER_LIFE_POINTS;

		// 1st move to new location
		DLListInsert(new->player_path[this_player], this_location);
		// 2nd player action
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

	//copy messages
	new->messages = malloc(new->num_of_turns * sizeof(player_message));
	for (size_t i = 0; i < new->num_of_turns; i++)
	{
		strcpy(new->messages[i], messages[i]);
	}
	return new;
}

void gv_drop(game_view *gv)
{
	// @todo REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	map_drop(gv->world_map);
	if (gv->messages != NULL)
		free(gv->messages);
	for (int i = 0; i < NUM_PLAYERS; i++)
	{
		freeDLList(gv->player_path[i]);
	}
	freeDLList(gv->dracula_precise_path);
	free(gv);
}

round_t gv_get_round(game_view *gv)
{
	/// @todo REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	return gv->num_of_turns / NUM_PLAYERS;
}

enum player gv_get_player(game_view *gv)
{
	/// @todo REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	return gv->num_of_turns % NUM_PLAYERS;
}

int gv_get_score(game_view *gv)
{
	/// @todo REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	return gv->scores;
}

int gv_get_health(game_view *gv, enum player player)
{
	/// @todo REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	return gv->health[player];
}

location_t gv_get_location(game_view *gv, enum player player)
{
	/// @todo REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	if (gv->player_path[player]->nitems == 0)
		return UNKNOWN_LOCATION;
	else{
		return gv->player_path[player]->last->location;
	}
		
}

void gv_get_history(
	game_view *gv, enum player player,
	location_t trail[TRAIL_SIZE])
{
	/// @todo REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	DLListNode *curr = gv->player_path[player]->last;
	for (size_t i = 0; i < TRAIL_SIZE; i++)
	{
		if (i < gv->player_path[player]->nitems)
		{
			trail[i] = curr->location;
			curr = curr->prev;
		}
		else
			trail[i] = UNKNOWN_LOCATION;
	}
}

location_t *gv_get_connections(
	game_view *gv, size_t *n_locations,
	location_t from, enum player player, round_t round,
	bool road, bool rail, bool sea)
{
	/// @todo REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	*n_locations = 0;
	if (!valid_location_p(from))
		return NULL;
	if (player == PLAYER_DRACULA)
		return location_dracula_can_move(gv, n_locations, from, road, sea);

	else
		return location_hunter_can_move(gv, n_locations, from, player, round, road, rail, sea);
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

static void game_score_decrese(game_view *gv, int lost_score)
{
	gv->scores -= lost_score;
	if (gv->scores < 0)
	{
		gv->scores = 0;
		//game end
	}
}

static bool is_Hunter_Dead(game_view *gv, enum player player)
{
	if (gv->health[player] == 0)
	{
		//instance teleport to hosipital
		gv->player_path[player]->last->location = ST_JOSEPH_AND_ST_MARYS;
		return true;
	}
	else
		return false;
}

static void hunter_decrease_health(game_view *gv, enum player player, int lost_health)
{
	if (gv->health[player] - lost_health < 0)
	{
		gv->health[player] = 0;
		game_score_decrese(gv, SCORE_LOSS_HUNTER_HOSPITAL);
	}
	else
		gv->health[player] -= lost_health;
}

static void hunter_increase_health(game_view *gv, enum player player, int added_health)
{
	gv->health[player] += added_health;
	if (gv->health[player] > GAME_START_HUNTER_LIFE_POINTS)
		gv->health[player] = GAME_START_HUNTER_LIFE_POINTS;
}

static location_t *location_hunter_can_move(
	game_view *gv, size_t *n_locations,
	location_t from, enum player player, round_t round,
	bool road, bool rail, bool sea)
{
	DLList connection_list = newDLList();
	//stay at same city
	DLListInsert(connection_list, from);

	int visited[NUM_MAP_LOCATIONS];
	for (int i = 0; i < NUM_MAP_LOCATIONS; i++)
		visited[i] = -1;
	visited[from] = 1;

	//get trail move
	int sum = (round + (int)player) % 4;

	for (map_adj *curr = gv->world_map->connections[from]; curr != NULL; curr = curr->next)
	{
		if ((road && curr->type == ROAD && visited[curr->v] == -1) || (sea && curr->type == BOAT && visited[curr->v] == -1))
		{
			visited[curr->v] = 1;
			DLListInsert_nodup(connection_list, curr->v);
		}
		// travel by rail rule
		if (rail && curr->type == RAIL && sum > 0)
		{
			Queue q = newQueue();
			int rail_visited[NUM_MAP_LOCATIONS];
			for (int i = 0; i < NUM_MAP_LOCATIONS; i++)
				rail_visited[i] = -1;
			rail_visited[curr->v] = 1;
			DLListInsert_nodup(connection_list, curr->v);
			QueueJoin(q, curr->v);

			while (!QueueIsEmpty(q))
			{
				location_t next = QueueLeave(q);
				if (rail_visited[next] + 1 <= sum)
				{
					for (map_adj *curr2 = gv->world_map->connections[next]; curr2 != NULL; curr2 = curr2->next)
					{
						if (curr->type == RAIL && rail_visited[curr->v] == -1)
						{
							DLListInsert_nodup(connection_list, curr2->v);
							rail_visited[curr->v] = rail_visited[next] + 1;
							QueueJoin(q, curr2->v);
						}
					}
				}
			}
			dropQueue(q);
		}
	}

	//get all location
	*n_locations = connection_list->nitems;
	location_t *result = malloc(*n_locations * sizeof(location_t));
	DLListNode *curr = connection_list->first;
	for (size_t i = 0; i < *n_locations; i++)
	{
		result[i] = curr->location;
		curr = curr->next;
	}
	return result;
}

static void hunter_action(game_view *gv, enum player player)
{
	// arrive at drucula trail
	/* for(size_t i =0;i<TRAIL_SIZE;i++){
		if(this_location == gv->dracula_trail[i])
		// all Hunters can see which positions in the trail have MOVEs that resulted in Dracula being in that city (they cannot see the actual moves)
	}
	*/
	location_t this_location = gv->player_path[player]->last->location;
	//encounter traps
	for (size_t i = 0; i < get_traps(gv->world_map, this_location); i++)
	{
		hunter_decrease_health(gv, player, LIFE_LOSS_TRAP_ENCOUNTER);
		remove_trap(gv->world_map, this_location);
		if (is_Hunter_Dead(gv, player))
			return;
	}

	//encounter vampire
	if (gv->is_vampire_alive && this_location == get_vampire_location(gv->world_map))
	{
		gv->is_vampire_alive = false;
		set_vampire_location(gv->world_map, UNKNOWN_LOCATION);
	}

	//encounter dracula
	if (this_location == gv->dracula_trail[0].location)
	{
		hunter_decrease_health(gv, player, LIFE_LOSS_DRACULA_ENCOUNTER);
		dracula_decrease_health(gv, LIFE_LOSS_HUNTER_ENCOUNTER);

		is_Dracula_Dead(gv);
		is_Hunter_Dead(gv, player);
	}

	//At the end of action phase. If the Hunter is in the same city or sea they were in last turn, they are treated as resting.
	if (gv->player_path[player]->nitems > 1 && gv->player_path[player]->last->prev->location == gv->player_path[player]->last->location)
		hunter_increase_health(gv, player, LIFE_GAIN_REST);
}

static bool is_Dracula_Dead(game_view *gv)
{
	return gv->health[PLAYER_DRACULA] == 0;
}

static void dracula_decrease_health(game_view *gv, int lost_health)
{
	if (gv->health[PLAYER_DRACULA] - lost_health < 0)
	{
		gv->health[PLAYER_DRACULA] = 0;
		//game end
	}
	else
		gv->health[PLAYER_DRACULA] -= lost_health;
}

static void dracula_increase_health(game_view *gv, int added_health)
{
	gv->health[PLAYER_DRACULA] += added_health;
	if (gv->health[PLAYER_DRACULA] > GAME_START_BLOOD_POINTS)
		gv->health[PLAYER_DRACULA] = GAME_START_BLOOD_POINTS;
}

static location_t *location_dracula_can_move(
	game_view *gv, size_t *n_locations,
	location_t from, bool road, bool sea)
{
	DLList connection_list = newDLList();

	int visited[NUM_MAP_LOCATIONS];

	for (int i = 0; i < NUM_MAP_LOCATIONS; i++)
		visited[i] = -1;
	visited[from] = 1;
	// dracula can't go to ST_JOSEPH_AND_ST_MARYS
	visited[ST_JOSEPH_AND_ST_MARYS] = 1;
	//dramula move

	bool can_hide = true;
	bool can_doubleback = true;
	// can't hide in sea
	if ((valid_location_p(from) && location_get_type(from) == SEA) || from == SEA_UNKNOWN)
		can_hide = false;
	// can't go a location in his most recent 5 moves
	location_t trail[TRAIL_SIZE];
	gv_get_history(gv, PLAYER_DRACULA, trail);
	for (int i = 0; i < TRAIL_SIZE - 1; i++)
	{
		// it cannot be a location in his most recent 5 moves
		if (gv->dracula_trail[i].location != UNKNOWN_LOCATION)
			visited[gv->dracula_trail[i].location] = 1;
		// he cannot make a HIDE move if he already has a HIDE move in his trail
		if (trail[i] == HIDE)
			can_hide = false;
		// he cannot make a DOUBLE_BACK move if he already has a DOUBLE_BACK move in his trail
		if (trail[i] >= DOUBLE_BACK_1 && trail[i] <= DOUBLE_BACK_5)
			can_doubleback = false;
	}
	if (can_hide)
		DLListInsert(connection_list, HIDE);
	if (can_doubleback)
		for (int i = DOUBLE_BACK_1; i <= DOUBLE_BACK_5; i++)
			DLListInsert(connection_list, i);
	/* 
		-- not finished -- 
		if he makes a DOUBLE_BACK move, the Hunters know...
		 - that his most recent move is a DOUBLE_BACK
		 - the position in the trail of the location he has doubled back to
		*/

	for (map_adj *curr = gv->world_map->connections[from]; curr != NULL; curr = curr->next)
	{
		if ((road && curr->type == ROAD && visited[curr->v] == -1) || (sea && curr->type == BOAT && visited[curr->v] == -1))
		{
			visited[curr->v] = 1;
			DLListInsert(connection_list, curr->v);
		}
	}

	*n_locations = connection_list->nitems;
	if (*n_locations == 0)
	{
		DLListInsert(connection_list, TELEPORT);
		(*n_locations)++;
	}

	location_t *result = malloc(*n_locations * sizeof(location_t));
	DLListNode *curr = connection_list->first;
	for (size_t i = 0; i < *n_locations; i++)
	{
		result[i] = curr->location;
		curr = curr->next;
	}
	return result;
}

static void dracula_move(game_view *gv)
{
	//location_t this_location = gv->player_path[PLAYER_DRACULA]->last->location;

	// update dracula trail
	Trail left_trail = gv->dracula_trail[TRAIL_SIZE - 1];
	for (int i = TRAIL_SIZE - 1; i > 0; i--)
	{
		gv->dracula_trail[i] = gv->dracula_trail[i - 1];
	}

	if (left_trail.location != UNKNOWN_LOCATION)
	{
		// if it was a Trap it just vanishes without a trace (yay!)
		if (left_trail.encounter == TRAP_ENCOUNTER)
			remove_trap(gv->world_map, left_trail.location);
		// if it was a Vampire it has matured (eek!)
		if (left_trail.encounter == VAMPIRE_ENCOUNTER && gv->is_vampire_alive)
		{
			gv->is_vampire_alive = false;
			game_score_decrese(gv, SCORE_LOSS_VAMPIRE_MATURES);
		}
	}
}

static void dracula_action(game_view *gv)
{
	/*
		-- Not finished --
		The Hunters can see Dracula’s location whenever he ends his turn in his castle, 
		or in a city currently occupied by a Hunter
		this holds even if his most recent move was a HIDE
	*/

	location_t this_location = gv->player_path[PLAYER_DRACULA]->last->location;
	location_t precise_location = this_location;
	if (this_location >= DOUBLE_BACK_1 && this_location <= DOUBLE_BACK_5)
	{
		int double_back = this_location - DOUBLE_BACK_1 + 1;
		precise_location = gv->dracula_trail[double_back].location;
	}
	else if (this_location == HIDE)
	{
		precise_location = gv->dracula_trail[0].location;
	}
	else if (this_location == TELEPORT)
	{
		precise_location = CASTLE_DRACULA;
	}
	gv->dracula_trail[0].location = precise_location;
	gv->dracula_trail[0].encounter = NO_ENCOUNTER;
	DLListInsert(gv->dracula_precise_path, precise_location);

	if (precise_location == SEA_UNKNOWN || (valid_location_p(precise_location) && location_get_type(precise_location) == SEA))
		dracula_decrease_health(gv, LIFE_LOSS_SEA);
	/*
	-- Not finished --
	Hunters know whenever Dracula is at sea.
	*/

	// place encounter
	if ((valid_location_p(precise_location) && location_get_type(precise_location) == LAND))
	{
		size_t num_of_trap = get_traps(gv->world_map, precise_location);
		if (gv_get_round(gv) % 13 == 0 && num_of_trap < 3)
		{
			gv->is_vampire_alive = true;
			set_vampire_location(gv->world_map, precise_location);
			gv->dracula_trail[0].encounter = VAMPIRE_ENCOUNTER;
		}
		else
		{
			// Dracula cannot place an Encounter in a city if there are already 3 Encounters in that city.
			if (num_of_trap + (get_vampire_location(gv->world_map) == precise_location) < 3)
			{
				add_trap(gv->world_map, precise_location);
				gv->dracula_trail[0].encounter = TRAP_ENCOUNTER;
			}
		}
	}

	if (precise_location == CASTLE_DRACULA && gv->health[PLAYER_DRACULA] > 0)
		dracula_increase_health(gv, LIFE_GAIN_CASTLE_DRACULA);

	game_score_decrese(gv, SCORE_LOSS_DRACULA_TURN);
}

/*
  -- NOT finished --
  Special Rules
 */
