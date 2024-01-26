#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define title_art "\
┌───────────────────────────────────────────────────────────┐\n\
│                                                           │\n\
│      \033[35m| _     _            _     _            _         \033[0m   │\n\
│      \033[35m| |__ | | __ _  ___| | __(_) __ _  ___| | __      \033[0m   │\n\
│      \033[35m| '_ \\| |/ _` |/ __| |/ /| |/ _` |/ __| |/ /     \033[0m    │\n\
│      \033[35m| |_) | | (_| | (__|   < | | (_| | (__|   <       \033[0m   │\n\
│      \033[35m|_.__/|_|\\__,_|\\___|_|\\_\\/ |\\__,_|\\___|_|\\_\\    \033[0m     │\n\
│      \033[35m                       |__/                       \033[0m   │\n\
│                 _____                                     │\n\
│                |A .  | _____                              │\n\
│                | /.\\ ||A ^  | _____                       │\n\
│                |(_._)|| / \\ ||A _  | _____                │\n\
│                |  |  || \\ / || ( ) ||A_ _ |               │\n\
│                |____V||  .  ||(_'_)||( v )|               │\n\
│                       |____V||  |  || \\ / |               │\n\
│                              |____V||  .  |               │\n\
│                                     |____V|               │\n\
│                                                           │\n\
└───────────────────────────────────────────────────────────┘\n\n\n\
                  press (1) to play   "


// both score must be under 10
#define score_box "\
┌──────────────────────┐\n\
│                      │\n\
│       GAME SCORE     │\n\
│    Player │ Dealer   │\n\
│      %i    │   %i      │\n\
│           │          │\n\
└───────────┴──────────┘\n"


#define card "\
┌─────────┐\n\
│%c        │\n\
│         │\n\
│         │\n\
│    %s    │\n\
│         │\n\
│         │\n\
│        %c│\n\
└─────────┘\n"

#define hidden_card "\
┌─────────┐\n\
│*********│\n\
│*********│\n\
│*********│\n\
│*********│\n\
│*********│\n\
│*********│\n\
│*********│\n\
└─────────┘\n"

#define CARDS 52

#define x_padding 5
#define y_padding 2

const char faces[] = {'A', '2', '3', '4', '5', '6', '7', '8', '9', 'T', 'J', 'Q', 'K'};
const char* suits[] = {"♠","♦","♥","♣"};

typedef enum {
	WIN,
    LOSS,
    TIE,
} GameResult;

typedef struct {
	GameResult result;
	int player_score;
	int dealer_score;
} Game;

void clear_screen(void) {
	fwrite("\x1b[2J", 1, 4, stdout);
	fwrite("\x1b[H", 1, 3, stdout);
}

void clear_up(void) {
	fwrite("\x1b[[1J", 1, 4, stdout);
}

char* handle_input(char* buffer, size_t buffer_size) {
    if (fgets(buffer, buffer_size, stdin) == NULL) {
        fprintf(stderr, "Error reading input\n");
        exit(EXIT_FAILURE);
    }

    size_t length = strlen(buffer);
    if (length > 0 && buffer[length - 1] == '\n') {
        buffer[length - 1] = '\0';
    }

    return buffer;
}

void move_to_position(int x, int y) {
	printf("\033[%d;%dH", y + 1, x + 1);
}

void print_at_offset(char* str, int x, int y) {
	printf("\033[%d;%dH", y + 1, x + 1);
	char *copy = strdup(str);
	if (copy == NULL) {
		fprintf(stderr, "Error: Memory allocation failed\n");
		exit(EXIT_FAILURE);
	}

	int initial = y + 1;
	for (char *p = strtok(copy, "\n"); p != NULL; p = strtok(NULL, "\n")) {
		printf("\033[%d;%dH%s", initial++, x + 1, p);
	}

	free(copy);
}

void shuffle(int *array, size_t n) {
    if (n > 1) {
        for (size_t i = 0; i < n - 1; i++)  {
          size_t j = i + rand() / (RAND_MAX / (n - i) + 1);
          int t = array[j];
          array[j] = array[i];
          array[i] = t;
        }
    }
}

int calculate_hand_value(int hand[], int cards) {
    int value = 0;
    int aces = 0;

    for (int i = 0; i < cards; ++i) {
		int rank = hand[i] % 13 + 1;
        if (rank == 1) {
            // ace
            aces++;
        } else if (rank > 10) {
            // faces
            value += 10;
        } else {
            value += rank;
        }
    }

    // adjust score for aces
	value += aces;
	for (int i = 0; i < aces; i++) {
		if (value + 10 <= 21) {
			value += 10;
		}
	}

    return value;
}

void print_cards(int cards[], int player_cards, int x, int y) {
	// print player cards
	print_at_offset("Your hand:", x, y);
	for (int i = 0; i < player_cards; i++) {
		int f = cards[i] % 13; // face
		int s = cards[i] / 13; // suit

		char card_buffer[sizeof(score_box) + 2];
		sprintf(card_buffer, card, faces[f], suits[s], faces[f]);
		print_at_offset(card_buffer, 12 * i + x, 1 + y);
	}

	char total_buffer[30];
	sprintf(total_buffer, "(Hand total: \033[36m%i\033[0m)", calculate_hand_value(cards, player_cards));
	print_at_offset(total_buffer,  x_padding, 10 + y);
}

void print_dealer_cards(int dealer[], int dealer_cards, int x, int y) {
	// print dealer cards
	print_at_offset("Dealer's hand:", x,  y);
	// print
	print_at_offset(hidden_card, x, 1 + y);	
	for (int i = 1; i < dealer_cards; i++) {
		int f = dealer[i] % 13; // face
		int s = dealer[i] / 13; // suit

		char card_buffer[sizeof(score_box) + 2];
		sprintf(card_buffer, card, faces[f], suits[s], faces[f]);
		print_at_offset(card_buffer, (12 * i) + x, 1 + y);
	}
}

Game play_game(int deck[], int player_score, int dealer_score) {
	// init hands
	// max number of cards in hand is 11	
	int cards[11];
	int player_cards = 0;

	int dealer[11];
	int dealer_cards = 0;

	cards[player_cards++] = deck[rand() % 52];
	cards[player_cards++] = deck[rand() % 52];

	dealer[dealer_cards++] = deck[rand() % 52];
	dealer[dealer_cards++] = deck[rand() % 52];

	Game game_result;


	// player's turn
	while (1) {
		print_cards(cards, player_cards, x_padding, 2);
		print_dealer_cards(dealer, dealer_cards, x_padding, 15);

		// print score box
		char box_buffer[sizeof(score_box)+5];
		sprintf(box_buffer, score_box, player_score, dealer_score);
		print_at_offset(box_buffer, 70, y_padding + 1);

		// y offset should be last cards offset + 15
		print_at_offset("Enter \033[34m[1]\033[0m to hit or \033[35m[2]\033[0m to stand.\n>", x_padding, 26);
		move_to_position(x_padding + 2, 27);

		// TODO
		int choice;
        scanf("%d", &choice);

		int c;
    	while ((c = getchar()) != '\n' && c != EOF);

        if (choice == 1) {
            cards[player_cards++] = deck[rand() % 52];
            if (calculate_hand_value(cards, player_cards) > 21) {
				game_result.result = LOSS;
				game_result.player_score = calculate_hand_value(cards, player_cards);
				game_result.dealer_score = calculate_hand_value(dealer, dealer_cards);
				return game_result;
            }

			// dealer's turn
			if (calculate_hand_value(dealer, dealer_cards) < 17) {
				dealer[dealer_cards++] = deck[rand() % 52];
			}
        } else if (choice == 2) {
            break;
        } else {
			// TODO: scanf auto converts to int so this never happens (?) <- to investigate
			// WAIT FOR USER INPUT
            printf("Invalid choice. Please enter (1) to hit or (2) to stand.\n");
        }


		clear_screen();
	}

	while (calculate_hand_value(dealer, dealer_cards) < 17) {
        dealer[dealer_cards++] = deck[rand() % 52];
    }

	int player_value = calculate_hand_value(cards, player_cards);
    int dealer_value = calculate_hand_value(dealer, dealer_cards);

	game_result.player_score = player_value;
	game_result.dealer_score = dealer_value;

    if (player_value > 21) {
		game_result.result = LOSS;
		return game_result;
    } else if (dealer_value > 21) {
		game_result.result = WIN;
		return game_result;
    } else if (player_value > dealer_value) {
		game_result.result = WIN;
		return game_result;
    } else if (player_value < dealer_value) {
		game_result.result = LOSS;
		return game_result;
    } else {
		game_result.result = TIE;
		return game_result;
    }
}

int main(void) {
	srand(time(NULL));

	clear_screen();
	// printf("\e[?25l"); // hide cursor
	print_at_offset(title_art,  x_padding, y_padding);
	move_to_position(x_padding + 36, y_padding + 19);

	// wait for [ 1 ] keypress
	while (1) {
		char c = getchar();
		if (c == '1') {
			break;
		}
		while (getchar() != '\n');
	}

	clear_screen();

	int deck[CARDS];
	int player_score = 0;
	int dealer_score = 0;

	// init deck
	for (int i = 0; i < CARDS; i++) 
		deck[i] = i;
	
	while (player_score < 10 && dealer_score < 10) {
		shuffle(deck, CARDS);
		Game result = play_game(deck, player_score, dealer_score);

		clear_screen();
		if (result.result == WIN) {
			player_score++;
			print_at_offset("\n\n\n\033[32;1mYou won!\033[0m\n\n",  x_padding, y_padding);
		} else if (result.result == LOSS) {
			char text_buf[90];
			sprintf(text_buf, "\033[31mYou lost.\033[0m The dealer's hand was \033[32m%i\033[0m and your hand was \033[31m%i\033[0m.\n\n", result.dealer_score, result.player_score);
			print_at_offset(text_buf,  x_padding, y_padding);
			dealer_score++;
		} else {
			print_at_offset("\n\n\nYou drew.\n\n",  x_padding, y_padding);
			dealer_score++;
			player_score++;
		}

		char box_buffer[sizeof(score_box)+5];
		sprintf(box_buffer, score_box, player_score, dealer_score);
		print_at_offset(box_buffer, x_padding, y_padding + 2);

		print_at_offset("\033[37mPress \033[;1m[ ENTER ]\033[0m to continue.\nPress [ Q ] to quit.\033[0m\n> ", x_padding, y_padding + 10);
		move_to_position(x_padding + 2, y_padding + 12);
		char input[10];
		handle_input(input, sizeof(input));

		switch (input[0]) {
			case 'q':
			case 'Q':
				clear_screen();
				exit(EXIT_SUCCESS);
				// printf("\e[?25h");
				break;
			default:
				break;
		}

		clear_screen();
	}	

	// printf("\e[?25h");


	// Final screen

    return 0;
}