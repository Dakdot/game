#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
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

std::vector<unsigned char> deck;
std::vector<unsigned char> discard;
std::vector<unsigned char> player_hand;
std::vector<unsigned char> cpu_hand;

void move_card(std::vector<unsigned char> &from,
               std::vector<unsigned char> &to) {
  if (from.empty())
    throw std::runtime_error("the from deck is empty. cannot move card!");
  unsigned char c = from.back();
  from.pop_back();
  to.push_back(c);
}

void deal_cards(std::vector<unsigned char> &from,
                std::vector<unsigned char> &to) {
  for (int i = 0; i < 7; i++) {
    move_card(from, to);
  }
}

void shuffle(std::vector<unsigned char> &deck) {
  srand(time(0));
  size_t deck_size = deck.size();

  for (int i = 0; i < 1000; i++) {
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

void print_card(unsigned char c) {
  unsigned char number_nib = c & 0x0f;
  unsigned char color_nib = c & 0xf0;

  if (c == WILD_CARD) {
    printf("\x1b[;95mWILD CARD");
  } else if (c == WILD_DRAW_FOUR) {
    return;
    printf("\x1b[;95mWILD DRAW FOUR");
    return;
  }

  if (color_nib == RED)
    printf("\x1b[;31mRED");
  else if (color_nib == YELLOW)
    printf("\x1b[;33mYELLOW");
  else if (color_nib == GREEN)
    printf("\x1b[;32mGREEN");
  else if (color_nib == BLUE)
    printf("\x1b[;34mBLUE");

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

void flip_first_card() {
  std::cout << "The first card is flipped..." << std::endl;
  move_card(deck, discard);
  std::this_thread::sleep_for(2s);
  std::cout << "It's a ";
  print_card(discard.back());
  std::cout << "!";
}

int main() {
  std::cout << "Welcome to UNO!" << std::endl;
  std::cout << "You and your opponent are each dealt seven cards..."
            << std::endl;

  create_deck(deck);
  shuffle(deck);
  deal_cards(deck, player_hand);
  deal_cards(deck, cpu_hand);

  std::this_thread::sleep_for(3s);

  std::cout << "Your hand:" << std::endl;
  print_hand(player_hand);

  std::this_thread::sleep_for(3s);

  flip_first_card();

  return 0;
}