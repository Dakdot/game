// We pledge our Honor that we have abided by the Stevens Honor System.

// - Thiago Andrade:
// Created card system (vectors that hold the decks, the defines,
// everything under CARD HELPER FUNCTIONS comment (line 67), main(), and
// setup())
// - Vincent Olivieri:
// Created game logic (everything under MAIN GAME LOGIC comment (line 227))

#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <stdexcept>
#include <string>
#include <sys/resource.h>
#include <thread>
#include <vector>

// So that I can type "s" after a number for seconds
using namespace std::chrono_literals;

// Defines card colors hexadecminally
// Each card is represented by an 8 bit number as follows:
// 0000          0000
// Card color    Card number/value/special/type
#define RED 0x00
#define YELLOW 0x10
#define GREEN 0x20
#define BLUE 0x30

#define ZERO 0x00
#define ONE 0x01
#define TWO 0x02
#define THREE 0x03
#define FOUR 0x04
#define FIVE 0x05
#define SIX 0x06
#define SEVEN 0x07
#define EIGHT 0x08
#define NINE 0x09

#define SKIP 0x0a
#define DRAW_TWO 0x0b
#define REVERSE 0x0c

#define WILD_CARD 0xdd
#define WILD_DRAW_FOUR 0xee

// Control how many times the `shuffle()` function randomly swaps cards
#define SHUFFLE_RANDOM_PASSES 1000

// Global variables for the decks
std::vector<unsigned char> deck;
std::vector<unsigned char> discard;
std::vector<unsigned char> player_hand;
std::vector<unsigned char> cpu_hand;

// Keep track of whether the player and CPU said UNO!
bool player_said_uno = false;
bool cpu_said_uno = false;

// Keep track of if the card changed between turns.
// If it didn't, this will prevent the card from being processed twice
// if player or CPU didn't place a card (i.e. they drew a card)
bool card_changed = true;

// CARD HELPER FUNCTIONS

// Shuffle cards by randomly selecting two cards and swapping their position
// in the vector.
void shuffle(std::vector<unsigned char> &deck) {
  srand(time(0));
  size_t deck_size = deck.size();

  for (int i = 0; i < SHUFFLE_RANDOM_PASSES; i++) {
    unsigned int idx_first = rand() % deck_size;
    unsigned int idx_second = rand() % deck_size;

    unsigned short tmp = deck[idx_first];
    deck[idx_first] = deck[idx_second];
    deck[idx_second] = tmp;
  }
}

// Move card from the top of one deck to the top of another
void move_card(std::vector<unsigned char> &from,
               std::vector<unsigned char> &to) {
  // If the from deck is empty, replace the deck with the discard pile
  if (from == deck && from.empty()) {
    std::cout << "The deck was empty! It has been replenished with the discard "
                 "pile and shuffled."
              << std::endl;
    unsigned char top = discard.back();
    discard.pop_back();
    for (auto e : discard)
      deck.push_back(e);
    shuffle(deck);
    discard.clear();
    discard.push_back(top);
  } else if (from.empty()) {
    throw std::runtime_error("the from deck is empty. cannot move card!");
  }
  unsigned char c = from.back();
  from.pop_back();
  to.push_back(c);
}

// Move a specific card from a deck to another deck
// If there are multiple of a card in the `from` deck, this function will
// only move the first instance of the card
void play_card(std::vector<unsigned char> &from, std::vector<unsigned char> &to,
               unsigned char c) {
  for (int i = 0; i < from.size(); i++) {
    if (from[i] == c) {
      from.erase(from.begin() + i);
      to.push_back(c);
      return;
    }
  }
}

// Evaluate whether the given card is playable with the given top card
bool is_playable(unsigned char card, unsigned char top) {
  if ((card & 0x0f) == (top & 0x0f) || (card & 0xf0) == (top & 0xf0))
    return true;
  if (card == WILD_CARD)
    return true;
  if (card == WILD_DRAW_FOUR)
    return true;
  return false;
}

// Populate a deck with a full deck of cards
// Useful during initialization
void create_deck(std::vector<unsigned char> &deck) {
  for (int c = 0x0000; c < 0x0040; c += 0x0010) {
    deck.push_back(c | ZERO);

    for (int j = 0; j < 2; j++) {
      for (int i = 1; i < 10; i++) {
        deck.push_back(c | i);
      }

      deck.push_back(c | SKIP);
      deck.push_back(c | DRAW_TWO);
      deck.push_back(c | REVERSE);
    }
  }

  for (int i = 0; i < 4; i++) {
    deck.push_back(WILD_CARD);
    deck.push_back(WILD_DRAW_FOUR);
  }
}

// Print a deck in its hexademical representation of the cards
// Used for debugging
void print_deck(std::vector<unsigned char> &deck) {
  unsigned int idx = 0;
  for (unsigned char c : deck) {
    printf("0x%02x ", c);
    idx++;
    if (idx >= 12) {
      printf("\n");
      idx = 0;
    }
  }
  printf("\n%lu elements\n", deck.size());
}

// Print the English color of a card (with ANSI escape codes)
void print_color(unsigned char c) {
  unsigned char color_nib = c & 0xf0;

  if (color_nib == RED)
    printf("\x1b[;31mRED");
  else if (color_nib == YELLOW)
    printf("\x1b[;33mYELLOW");
  else if (color_nib == GREEN)
    printf("\x1b[;32mGREEN");
  else if (color_nib == BLUE)
    printf("\x1b[;34mBLUE");

  if ((c & 0x0f) != 0x0f)
    printf("\x1b[;0m");
}

// Print the English color and value/type of a card (with ANSI escape code)
void print_card(unsigned char c) {
  unsigned char number_nib = c & 0x0f;

  if (c == WILD_CARD) {
    printf("\x1b[;95mWILD CARD\x1b[;0m");
    return;
  } else if (c == WILD_DRAW_FOUR) {
    printf("\x1b[;95mWILD DRAW FOUR\x1b[;0m");
    return;
  }

  print_color(c | 0x0f);

  if (number_nib <= 9) {
    printf(" %d", number_nib);
  } else if (number_nib == SKIP) {
    printf(" SKIP");
  } else if (number_nib == DRAW_TWO) {
    printf(" DRAW TWO");
  } else if (number_nib == REVERSE) {
    printf(" REVERSE");
  }
  printf("\x1b[;0m");
}

// Print the English of a whole deck
void print_hand(std::vector<unsigned char> &deck) {
  for (int i = 0; i < deck.size(); i++) {
    print_card(deck[i]);
    if (i % 12 == 11) {
      printf("\n");
    } else if (i < deck.size() - 1) {
      printf("\x1b[;0m, ");
    }
  }
  printf("\n");
}

// MAIN GAME LOGIC

// Prompt the user for a color
unsigned char prompt_color(const std::string &prompt) {
  std::cout << prompt << std::endl;
  std::cout << "\x1b[;31mr\x1b[;33my\x1b[;32mg\x1b[;34mb\x1b[;0m > ";
  while (true) {
    std::string line;
    std::getline(std::cin, line);

    if (line == "r") {
      return RED;
    } else if (line == "y") {
      return YELLOW;
    } else if (line == "g") {
      return GREEN;
    } else if (line == "b") {
      return BLUE;
    }
  }
}

char turn = 0; // 0 = player, 1 = cpu

// Deal seven cards to each player
// Used during initialization
void deal_cards(std::vector<unsigned char> &from,
                std::vector<unsigned char> &to) {
  for (int i = 0; i < 7; i++) {
    move_card(from, to);
  }
}

// Flip over the first card from the deck
// Used during initialization
// (Includes dramatic pauses)
void flip_first_card() {
  std::cout << "The first card is flipped..." << std::endl;
  move_card(deck, discard);
  std::this_thread::sleep_for(2s);
  std::cout << "It's a ";
  print_card(discard.back());
  std::cout << "!" << std::endl;
  std::this_thread::sleep_for(2s);
}

// Process the last card that was placed on the deck and perform
// any necessary logic
void process_card(unsigned char &c) {
  srand(time(0));

  // For all the players that no longer have 1 card in their hand, reset
  // their "said uno" status
  if (player_hand.size() > 1)
    player_said_uno = false;
  if (cpu_hand.size() > 1)
    cpu_said_uno = false;

  // Check if a player won
  if (turn == 0 && player_hand.empty()) {
    if (player_said_uno) {
      std::cout << "You won!!" << std::endl;
      exit(0);
    } else {
      std::cout << "You forgot to say UNO! You drew two cards!" << std::endl;
      move_card(deck, player_hand);
      move_card(deck, player_hand);
    }
  } else if (turn == 1 && cpu_hand.empty()) {
    if (cpu_said_uno) {
      std::cout << "CPU wins! Better luck next time!" << std::endl;
      exit(0);
    } else {
      std::cout << "CPU forgot to say UNO! It drew two cards." << std::endl;
      move_card(deck, cpu_hand);
      move_card(deck, cpu_hand);
    }
  }

  // If the card was already processed, don't process it again!
  if (!card_changed) {
    if (turn == 0)
      turn = 1;
    else
      turn = 0;
    return;
  }

  card_changed = false;

  if (c == WILD_CARD || c == WILD_DRAW_FOUR) {
    unsigned char col;
    if (turn == 0) {
      col = prompt_color("You get to choose the color!");
    } else {
      col = rand() & 0x30;
    }
    c = (c & 0x0f) | col;
    std::cout << "The color in play is now ";
    print_color(col);
    std::cout << "!" << std::endl;

    if ((c & 0x0f) == (WILD_CARD & 0x0f)) {
      if (turn == 0)
        turn = 1;
      else
        turn = 0;
    }

    if ((c & 0x0f) == (WILD_DRAW_FOUR & 0x0f)) {
      if (turn == 0) {
        std::cout << "CPU drew four cards! Its turn was skipped." << std::endl;
        for (int i = 0; i < 4; i++) {
          move_card(deck, cpu_hand);
        }
      } else {
        std::cout << "You drew four cards! Your turn has been skipped."
                  << std::endl;
        for (int i = 0; i < 4; i++) {
          move_card(deck, player_hand);
        }
      }
    }
    std::this_thread::sleep_for(3s);
  } else if ((c & 0x0f) == SKIP) {
    if (turn == 0) {
      std::cout << "CPU was skipped!" << std::endl;
    } else {
      std::cout << "You were skipped!" << std::endl;
    }
    std::this_thread::sleep_for(2s);
  } else if ((c & 0x0f) == REVERSE) {
    if (turn == 0) {
      std::cout << "The turn order was reversed... it's your turn again!"
                << std::endl;
    } else {
      std::cout << "The turn order was reversed... the CPU plays again!"
                << std::endl;
    }
    std::this_thread::sleep_for(2s);
  } else if ((c & 0x0f) == DRAW_TWO) {
    if (turn == 0) {
      std::cout << "CPU drew two cards! Its turn was skipped." << std::endl;
      move_card(deck, cpu_hand);
      move_card(deck, cpu_hand);
    } else {
      std::cout << "You drew two cards! Your turn has been skipped."
                << std::endl;
      move_card(deck, player_hand);
      move_card(deck, player_hand);
    }
    std::this_thread::sleep_for(3s);
  } else {
    if (turn == 0)
      turn = 1;
    else
      turn = 0;
  }
}

// Process a player's turn
// If it's the human player, will prompt for what card they want to play
// If it's the CPU, will perform magical bot calculations to make decisions
// (i.e. random) :)
void process_turn() {
  unsigned char top = discard.back();
  srand(time(0));

  // Player's turn
  if (turn == 0) {
    std::cout << "It is your turn. Your hand is:" << std::endl;
    print_hand(player_hand);
    std::this_thread::sleep_for(1s);

    std::vector<unsigned char> playables;
    for (auto c : player_hand) {
      if (is_playable(c, top)) {
        playables.push_back(c);
      }
    }

    if (playables.empty()) {
      std::cout << "You have no playable cards! Press RETURN to draw one..."
                << std::endl;
      std::cin.clear();
      std::cin.ignore();
      std::cin.get();

      move_card(deck, player_hand);
      unsigned char drawn = player_hand.back();
      std::cout << "You drew a ";
      print_card(drawn);
      std::cout << ". ";
      if (is_playable(drawn, top)) {
        std::cout << "Press RETURN to play it." << std::endl;
        std::cin.clear();
        std::cin.ignore();
        std::cin.get();
        move_card(player_hand, discard);
        card_changed = true;
      } else {
        std::cout << "Your turn is over." << std::endl;
      }

    } else {
      std::cout << "Please type the index of the card you would like to play: "
                << std::endl;
      for (int i = 0; i < playables.size(); i++) {
        std::cout << "[" << i << "] ";
        print_card(playables[i]);
        std::cout << std::endl;
      }

      if (player_hand.size() == 2 && !player_said_uno) {
        std::cout << "[" << playables.size()
                  << "] Select to say UNO! as you play this card" << std::endl;
      }

      int result;
      do {
        std::cout << "> ";
        std::cin >> result;
        if (result == playables.size()) {
          std::cout << "You said UNO! Now play your card:" << std::endl;
          player_said_uno = true;
        }
      } while (result >= playables.size() || result < 0);
      play_card(player_hand, discard, playables[result]);
      card_changed = true;
    }
  }

  // CPU's turn
  if (turn == 1) {
    std::vector<unsigned char> playables;
    for (auto c : cpu_hand) {
      if (is_playable(c, top)) {
        playables.push_back(c);
      }
    }

    if (cpu_hand.size() == 2 && !cpu_said_uno) {
      if (rand() % 10 != 9) {
        std::cout << "CPU said UNO!" << std::endl;
        cpu_said_uno = true;
      }
    }

    if (playables.empty()) {
      std::cout << "CPU drew a card!";
      std::this_thread::sleep_for(2s);
      move_card(deck, cpu_hand);

      unsigned char drawn = cpu_hand.back();
      if (is_playable(drawn, top)) {
        play_card(cpu_hand, discard, drawn);
        std::cout << std::endl << "CPU placed a ";
        print_card(drawn);
        std::cout << "!" << std::endl;
        card_changed = true;
      } else {
        std::cout << " That was the end of its turn." << std::endl;
      }
    } else {
      unsigned char idx = rand() % playables.size();
      unsigned char c = playables[idx];

      play_card(cpu_hand, discard, c);
      std::cout << "CPU played a ";
      print_card(c);
      std::cout << "!" << std::endl;
      card_changed = true;
    }
    std::this_thread::sleep_for(2s);
  }
}

// Perform setup of game
void setup() {
  std::cout << "Welcome to UNO!" << std::endl;
  std::cout << "You and your opponent are each dealt seven cards..."
            << std::endl;

  create_deck(deck);
  shuffle(deck);
  deal_cards(deck, player_hand);
  deal_cards(deck, cpu_hand);

  std::this_thread::sleep_for(3s);

  flip_first_card();
}

int main() {
  setup();

  while (true) {
    unsigned char &top = discard.back();
    std::cout << "CPU HAND:" << std::endl;
    print_hand(cpu_hand);
    process_card(top);
    process_turn();
  }

  return 0;
}