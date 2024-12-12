#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <vector>

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

int main() {
  create_deck(deck);

  return 0;
}