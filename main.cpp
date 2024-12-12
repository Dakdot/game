// TODO:
// - Implement winning / losing
// - Implement say UNO & failed to say UNO
// If we have time:
// - Bot difficulty

#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

using namespace std::chrono_literals;

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

std::vector<unsigned char> deck;
std::vector<unsigned char> discard;
std::vector<unsigned char> player_hand;
std::vector<unsigned char> cpu_hand;

// CARD HELPER FUNCTIONS

void move_card(std::vector<unsigned char> &from,
               std::vector<unsigned char> &to) {
  if (from.empty())
    throw std::runtime_error("the from deck is empty. cannot move card!");
  unsigned char c = from.back();
  from.pop_back();
  to.push_back(c);
}

void play_card(std::vector<unsigned char> &from, std::vector<unsigned char> &to,
               unsigned char c) {
  for (int i = 0; i < from.size(); i++) {
    if (from[i] == c) {
      from.erase(from.begin() + i);
      to.push_back(c);
    }
  }
}

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

void print_card(unsigned char c) {
  unsigned char number_nib = c & 0x0f;

  if (c == WILD_CARD) {
    printf("\x1b[;95mWILD CARD");
    return;
  } else if (c == WILD_DRAW_FOUR) {
    printf("\x1b[;95mWILD DRAW FOUR");
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

unsigned char prompt_color(const std::string &prompt) {
  std::cout << prompt << std::endl;
  std::cout << "\x1b[;31mr\x1b[;33my\x1b[;32mg\x1b[;34mb\x1b[;0m > ";
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

// MAIN GAME LOGIC

char turn = 0;      // 0 = player, 1 = cpu
char direction = 1; // 1 or -1

void deal_cards(std::vector<unsigned char> &from,
                std::vector<unsigned char> &to) {
  for (int i = 0; i < 7; i++) {
    move_card(from, to);
  }
}

void flip_first_card() {
  std::cout << "The first card is flipped..." << std::endl;
  move_card(deck, discard);
  std::this_thread::sleep_for(2s);
  std::cout << "It's a ";
  print_card(discard.back());
  std::cout << "!" << std::endl;
}

void process_card(unsigned char &c) {
  srand(time(0));

  if (c == WILD_CARD || c == WILD_DRAW_FOUR) {
    unsigned char col;
    if (turn == 0) {
      col = prompt_color("You get to choose the color!");
    } else {
      unsigned char col = rand() & 0x03;
    }
    c = (c & 0x0f) | col;
    std::cout << "The color in play is now ";
    print_color(col);
    std::cout << "!" << std::endl;

    if (c == WILD_DRAW_FOUR) {
      if (turn == 0) {
        std::cout << "CPU draws four cards!" << std::endl;
        for (int i = 0; i < 4; i++) {
          move_card(deck, cpu_hand);
        }
      } else {
        std::cout << "You draw four cards!" << std::endl;
        for (int i = 0; i < 4; i++) {
          move_card(deck, player_hand);
        }
      }
    }
  } else if ((c & 0x0f) == SKIP) {
    if (turn == 0) {
      std::cout << "CPU was skipped!" << std::endl;
    } else {
      std::cout << "You were skipped!" << std::endl;
    }
  } else if ((c & 0x0f) == REVERSE) {
    if (turn == 0) {

    } else {
    }
  } else if ((c & 0x0f) == DRAW_TWO) {
    if (turn == 0) {
      std::cout << "CPU drew two cards! Its turn was skipped." << std::endl;
      move_card(deck, cpu_hand);
      move_card(deck, cpu_hand);
    } else {
      std::cout << "You drew two cards!" << std::endl;
      move_card(deck, player_hand);
      move_card(deck, player_hand);
    }
  } else {
    if (turn == 0)
      turn = 1;
    else
      turn = 0;
  }
}

void process_turn() {
  unsigned char top = discard.back();
  srand(time(0));

  if (turn == 0) {
    std::cout << "It is your turn. Your hand is:" << std::endl;
    print_hand(player_hand);

    std::vector<unsigned char> playables;
    for (auto c : player_hand) {
      if ((c & 0x0f) == (top & 0x0f) || (c & 0xf0) == (top & 0xf0)) {
        playables.push_back(c);
      }
    }

    if (playables.empty()) {
      std::cout << "You have no playable cards! Press RETURN to draw one..."
                << std::endl;
      std::string unused;
      std::getline(std::cin, unused);

      move_card(deck, player_hand);
      unsigned char drawn = player_hand.back();
      std::cout << "You drew a ";
      print_card(drawn);
      std::cout << ". ";
      if ((drawn & 0x0f) == (top & 0x0f) || (drawn & 0xf0) == (top & 0xf0)) {
        std::cout << "Press RETURN to play it." << std::endl;
        std::getline(std::cin, unused);
        move_card(player_hand, discard);
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

      int result;
      do {
        std::cout << "> ";
        std::cin >> result;
      } while (result > playables.size() || result < 0);
      play_card(player_hand, discard, playables[result]);
    }
  }

  if (turn == 1) {
    std::vector<unsigned char> playables;
    for (auto c : cpu_hand) {
      if ((c & 0x0f) == (top & 0x0f) || (c & 0xf0) == (top & 0xf0)) {
        playables.push_back(c);
      }
    }

    if (playables.empty()) {
      std::cout << "CPU drew a card!";
      move_card(deck, cpu_hand);

      unsigned char drawn = cpu_hand.back();
      if ((drawn & 0x0f) == (top & 0x0f) || (drawn & 0xf0) == (top & 0xf0)) {
        play_card(cpu_hand, discard, drawn);
        std::cout << std::endl << "CPU placed a ";
        print_card(drawn);
        std::cout << "!" << std::endl;
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
    }
  }
}

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
    process_card(top);
    process_turn();
    std::this_thread::sleep_for(5s);
  }

  return 0;
}