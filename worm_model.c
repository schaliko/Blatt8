// A simple variant of the game Snake
//
// Used for teaching in classes
//
// Author:
// Franz Regensburger
// Ingolstadt University of Applied Sciences
// (C) 2011
//
// The worm model

#include <curses.h>
#include "worm.h"
#include "board_model.h"
#include "worm_model.h"

enum ColorPairs theworm_wcolor; 

// Initialize the worm
enum ResCodes initializeWorm(struct worm* aworm, int len_max, int len_cur,
                                    struct pos headpos, enum WormHeading dir, enum ColorPairs color) {
    int i;
    // Initialize last usable index to len_max -1
    aworm->maxindex = len_max - 1 ;
    // Current last usable index in array. May grow upto maxindex
    aworm->cur_lastindex = len_cur - 1;
	// Initialize headindex
    aworm->headindex = 0; // Index pointing to head position is set to 0
    // Mark all elements as unused in the array of positions.
    // This allows for the effect that the worm appears element by element at
    // the start of each level    
    for (i = 0; i <= aworm->maxindex; i++) {
        aworm->wormpos[i].y = UNUSED_POS_ELEM;
        aworm->wormpos[i].x = UNUSED_POS_ELEM;
    }

    // Initialize position of worms head
    aworm->wormpos[aworm->headindex] = headpos;
    // Initialize the heading of the worm
    setWormHeading(aworm, dir);
    // Initialze color of the worm
    aworm->wcolor = color;
    return RES_OK;
}

// Show the worms's elements on the display
// Simple version
//void showWorm(struct board* aboard, struct worm* aworm) {
    // Due to our encoding we just need to show the head element
    // All other elements are already displayed
//    placeItem(
//            aboard,
//            aworm->wormpos[aworm->headindex].y,
//            aworm->wormpos[aworm->headindex].x,
//            BC_USED_BY_WORM,SYMBOL_WORM_INNER_ELEMENT,aworm->wcolor);
//}

void showWorm(struct board* aboard, struct worm* aworm) {
    int i = aworm->headindex;
    
    // Display each element of the worm using the appropriate symbol
    do {
        // Determine the symbol based on the index
        char symbol;
        if (i == aworm->headindex) {
            symbol = SYMBOL_WORM_HEAD_ELEMENT; // Head element
        } 
            //else if (aworm->wormpos[i].x == UNUSED_POS_ELEM) {
            // Unused position, should not happen, but handle it gracefully
            //continue;
//        }
         else if (i == (aworm->headindex + 1) % (aworm->cur_lastindex + 1)) {
            symbol = SYMBOL_WORM_TAIL_ELEMENT; // Tail element
        } else {
            symbol = SYMBOL_WORM_INNER_ELEMENT; // Inner element
        }

        // Display the symbol for the current element
        placeItem(
            aboard,
            aworm->wormpos[i].y,
            aworm->wormpos[i].x,
            BC_USED_BY_WORM, symbol, aworm->wcolor);

        // Move to the previous index in the ring buffer
        i = (i + 1) % (aworm->cur_lastindex + 1);
        //i = (i - 1 + (aworm->cur_lastindex + 1)) % (aworm->cur_lastindex + 1);
    } 
    //while (i != (aworm->headindex - 1 + (aworm->cur_lastindex + 1)) % (aworm->cur_lastindex + 1));
    while ( i != aworm->headindex && aworm->wormpos[i].x != UNUSED_POS_ELEM);
}


void cleanWormTail(struct board* aboard, struct worm* aworm) {
    int tailindex;
    // Compute tailindex
    tailindex = (aworm->headindex + 1) % (aworm->cur_lastindex + 1);
    // Check the array of worm elements.
    // Is the array element at tailindex already in use?
    // Checking either array theworm_wormpos_y
    // or theworm_wormpos_x is enough.
    if (aworm->wormpos[tailindex].y != UNUSED_POS_ELEM) {
        // YES: place a SYMBOL_FREE_CELL at the tail's position
        placeItem(aboard, aworm->wormpos[tailindex].y, aworm->wormpos[tailindex].x,
	    BC_FREE_CELL, SYMBOL_FREE_CELL, COLP_FREE_CELL);
    }
}

void moveWorm(struct board* aboard, struct worm* aworm, enum GameStates* agame_state) {
    struct pos headpos;

    // Get the current position of the worm's head element and
    headpos = aworm->wormpos[aworm->headindex];    
    // compute the new head position according to current heading.
    // Do not store the new head position in the array of positions, yet.
	headpos.x += aworm->dx;
    headpos.y += aworm->dy;

    // Check if we would leave the display if we move the worm's head according
    // to worm's last direction.
    // We are not allowed to leave the display's window.
    if (headpos.x < 0) {
        *agame_state = WORM_OUT_OF_BOUNDS;
        //endwin();
    } else if (headpos.x > getLastColOnBoard(aboard) ) { 
        *agame_state = WORM_OUT_OF_BOUNDS;
        //endwin();
    } else if (headpos.y < 0) {  
        *agame_state = WORM_OUT_OF_BOUNDS;
        //endwin();
    } else if (headpos.y > getLastRowOnBoard(aboard) ) {
        *agame_state = WORM_OUT_OF_BOUNDS;
        //endwin();
    } 
    else {
        // Check if the worms head hits any items at the new position on the board.
        // Hitting food is good, hitting barriers or worm elements is bad.
        switch ( getContentAt(aboard,headpos) ) {
            case BC_FOOD_1:
                *agame_state = WORM_GAME_ONGOING;
                // Grow worm according to food item digested
                growWorm(aworm, BONUS_1);
                decrementNumberOfFoodItems(aboard);
                break;
            case BC_FOOD_2:
                *agame_state = WORM_GAME_ONGOING;
                // Add the corresponding action for BC_FOOD_2
                growWorm(aworm, BONUS_2);
                decrementNumberOfFoodItems(aboard);
                break;
            case BC_FOOD_3:
                *agame_state = WORM_GAME_ONGOING;
                // Add the corresponding action for BC_FOOD_2
                growWorm(aworm, BONUS_3);
                decrementNumberOfFoodItems(aboard);
                break;
            case BC_BARRIER:
                // That's bad: stop game
                *agame_state = WORM_CRASH;
                break;
            case BC_USED_BY_WORM:
                // That's bad: stop game
                *agame_state = WORM_CROSSING;
                break;
            default:
                // Without default case we get a warning message.
                {;} // Do nothing. C syntax dictates some statement, here.

        }
    }

    // Check the status of *agame_state
    // Go on if nothing bad happened
    if ( *agame_state == WORM_GAME_ONGOING ) {
        // So all is well: we did not hit anything bad and did not leave the
        // window. --> Update the worm structure.
        // Increment headindex
        // Go round if end of worm is reached (ring buffer)
        aworm->headindex++;
        if (aworm->headindex > aworm->cur_lastindex) {
            aworm->headindex = 0;
        }
        // Store new coordinates of head element in worm structure
        aworm->wormpos[aworm->headindex] = headpos;
    }
}

// A simple collision detection
bool isInUseByWorm(struct worm* aworm, struct pos new_headpos) {
    int i;
    bool collision = false;
    i = aworm->headindex;
    do {// Compare the position of the current worm element with the new_headpos
        if (aworm->wormpos[i].y == new_headpos.y && 
            aworm->wormpos[i].x == new_headpos.x) {
			// found!
			collision = true;
			break;
		}
		i = (i + 1) % (aworm->maxindex + 1);
    } while ( i != aworm->headindex && aworm->wormpos[i].x != UNUSED_POS_ELEM);
    // Return what we found out.
    return collision;
}

// Setters
void setWormHeading(struct worm* aworm, enum WormHeading dir) {
    switch(dir) {
        case WORM_UP :// User wants up
            aworm->dx=0;
            aworm->dy=-1;
            break;
        case WORM_DOWN :// User wants down
            aworm->dx=0;
            aworm->dy=+1;
            break;
        case WORM_LEFT      :// User wants left
            aworm->dx=-1;
            aworm->dy=0;
            break;
        case WORM_RIGHT      :// User wants right
            aworm->dx=+1;
            aworm->dy=0;
            break;
    }
} 

struct pos getWormHeadPos(struct worm* aworm) {
    // Structures are passed by value!
    // -> we return a copy here
    return aworm->wormpos[aworm->headindex];
}

int getWormLength(struct worm* aworm) {
    return aworm->cur_lastindex + 1;
}

void growWorm(struct worm* aworm, enum Boni growth) {
    // Play it safe and inhibit surpassing the bound
    if (aworm->cur_lastindex + growth <= aworm->maxindex) {
        aworm->cur_lastindex += growth;
    } else {aworm->cur_lastindex = aworm->maxindex;
        aworm->cur_lastindex = aworm->maxindex;
    }
}
