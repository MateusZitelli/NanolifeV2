// Nanolife - Simple artificial life simulator

// Copyright 2011 Mateus Zitelli <zitellimateus@gmail.com>

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
//(at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
// MA 02110-1301, USA.

#include <stdio.h>
#include <stdlib.h>
#include <SDL.h>
#include <time.h>

#define SX 1200
#define SY 1000
#define WIDTH 1200
#define HEIGHT 1000
#define BPP 4
#define DEPTH 32

#define MEM_SIZE 50
#define MAX_AGE 2000.0
#define MAX_LOUPS 1000
#define MAX_GENENARATION_UP_SHOW 500

int last, VAR_TAX = 20;
struct bot *bots;

/*
0x0 - ptr++
0x1 - ptr--
0x2 - memory[b->ptr]++
0x3 - memory[b->ptr]--
0x4 - while(memory[b->ptr]){
0x5 - }
0x6 - memory[b->ptr] = 1 if have a compatible bot in front or 0 if not
0x7 - memory[b->ptr] = dir
0x8 - new_genoma[next_adr] = memory[b->ptr]
0x9 - Rotate clockwise - dir++
0x10 - Rotate anticlockwise - dir--
0x11 - Move foward
0x12 - Move Back
0x13 - Reproduce
0x14 - Atack
0x15 - Divide energy
0x16 - Sex reproduction
0x17 - Create a new especie
*/

/*Actions dict:

*/

struct bot
{
  int p;
  int lp; // Last position
  float energy;
  short gcode[MEM_SIZE];
  short memory[MEM_SIZE];
  short new_gcode[MEM_SIZE];
  long int nl;
  short loops[MAX_LOUPS];
  short loops_ptr[MAX_LOUPS];
  short ptr;
  short pos;
  short dir;
  short r;
  short g;
  short b;
  int age;
  short generation;
  int last_adr;
  struct bot *dad;
};

char *itoa(int value, char *str, int radix)
{
  static char dig[] = "0123456789"
    "abcdefghijklmnopqrstuvwxyz";
  int n = 0, neg = 0;
  int v;
  char *p, *q;
  char c;

  if (radix == 10 && value < 0)
  {
    value = -value;
    neg = 1;
  }
  v = value;
  do
  {
    str[n++] = dig[v % radix];
    v /= radix;
  } while (v);
  if (neg)
    str[n++] = '-';
  str[n] = '\0';

  for (p = str, q = p + (n - 1); p < q; ++p, --q)
    c = *p, *p = *q, *q = c;
  return str;
}

void set_bot(struct bot *b, struct bot *dad, int p, float e, short *g, struct bot **lb, short gen) {
  short i;
  int cr, cg, cb;

  if (dad != NULL) {
    // Perform additional checks if needed, for example:
    // Check if dad is within the bots array bounds if applicable.
    b->dad = dad;
  } else {
    b->dad = NULL; // or handle the case appropriately
  }
  // Reset or initialize all fields to ensure no data carries over from a previous usage of this bot slot
  b->p = p;
  b->lp = 0; // Assuming lp is meant to be reset. If it should inherit from dad or have a specific initial value, adjust this accordingly.
  b->energy = e;
  b->age = MAX_AGE;
  b->generation = gen + 1;
  b->nl = 0;
  memset(b->loops, 0, sizeof(b->loops));
  memset(b->loops_ptr, 0, sizeof(b->loops_ptr));
  b->ptr = 0;
  b->pos = 0;
  b->dir = rand() % 4; // Random direction
  b->last_adr = 0;

  // Clear memory and new_gcode arrays
  memset(b->memory, 0, sizeof(b->memory));
  memset(b->new_gcode, 0, sizeof(b->new_gcode));

  // Copy genetic code from parent or initialization array
  for (i = 0; i < MEM_SIZE; i++) {
    b->gcode[i] = g[i];
  }

  // Calculate color based on genetic code
  cr = ((g[MEM_SIZE - 9] + g[MEM_SIZE - 8] + g[MEM_SIZE - 7]) / 57.0) * 255;
  cg = ((g[MEM_SIZE - 6] + g[MEM_SIZE - 5] + g[MEM_SIZE - 4]) / 57.0) * 255;
  cb = ((g[MEM_SIZE - 3] + g[MEM_SIZE - 2] + g[MEM_SIZE - 1]) / 57.0) * 255;
  b->r = cr > 255 ? 255 : cr;
  b->g = cg > 255 ? 255 : cg;
  b->b = cb > 255 ? 255 : cb;
}

char compatible(struct bot *b1, struct bot *b2)
{
  if (b1 == NULL || b2 == NULL)
    return 0;
  int i, pos;
  for (i = 0; i < 5; i++)
  {
    pos = rand() % 9 + 1;
    if (b1->gcode[MEM_SIZE - pos] != b2->gcode[MEM_SIZE - pos])
      return (0);
  }
  return (1);
}

float compatibility(short *gcode, struct bot *b)
{
  int i;
  float val = 0;
  for (i = 0; i < MEM_SIZE; i++)
  {
    if (gcode[i] == b->gcode[i])
      val += 1;
  }
  return (val / MEM_SIZE);
}

void compute(struct bot *b, struct bot **lb)
{
  --b->energy;
  short ngcode[MEM_SIZE];
  int mean, index = MEM_SIZE / 2, i;
  --b->age;
  if (b->pos - 1 > MEM_SIZE || b->ptr - 1 > MEM_SIZE || b->ptr < 0 || b->nl > MAX_LOUPS)
    return;
  switch (b->gcode[b->pos++])
  {
    case 1:
      b->ptr++;
      break;
    case 2:
      b->ptr--;
      break;
    case 3:
      b->memory[b->ptr]++;
      break;
    case 4:
      b->memory[b->ptr]--;
      break;
    case 5:
      if (b->memory[b->ptr])
        b->loops[b->nl] = b->pos;
      b->loops_ptr[b->nl++] = b->ptr;
      break;
    case 6:
      if (b->nl && b->memory[b->loops_ptr[b->nl - 1]] <= 0)
      {
        b->pos = b->loops[b->nl - 1];
      }
      else if (b->nl)
      {
        --b->nl;
      }
      break;
    case 7:
      switch (b->dir)
      {
        case 0:
          if (b->p + 1 < SX * SY && lb[b->p + 1] != NULL)
          {
            if (compatible(lb[b->p + 1], b))
            {
              b->memory[b->ptr] = 2;
            }
            else
            {
              b->memory[b->ptr] = 1;
            }
          }
          else
          {
            b->memory[b->ptr] = 0;
          }
          break;
        case 1:
          if (b->p - SX >= 0 && lb[b->p - SX] != NULL)
          {
            if (compatible(lb[b->p - SX], b))
            {
              b->memory[b->ptr] = 2;
            }
            else
            {
              b->memory[b->ptr] = 1;
            }
          }
          else
          {
            b->memory[b->ptr] = 0;
          }
          break;
        case 2:
          if (b->p - 1 >= 0 && lb[b->p - 1] != NULL)
          {
            if (compatible(lb[b->p - 1], b))
            {
              b->memory[b->ptr] = 2;
            }
            else
            {
              b->memory[b->ptr] = 1;
            }
          }
          else
          {
            b->memory[b->ptr] = 0;
          }
          break;
        case 3:
          if (b->p + SX < SX * SY && lb[b->p + SX] != NULL)
          {
            if (compatible(lb[b->p + SX], b))
            {
              b->memory[b->ptr] = 2;
            }
            else
            {
              b->memory[b->ptr] = 1;
            }
          }
          else
          {
            b->memory[b->ptr] = 0;
          }
          break;
      }
      break;
    case 8:
      b->memory[b->ptr] = b->dir;
      break;
    case 9:
      if (b->last_adr < MEM_SIZE)
        b->new_gcode[b->last_adr++] = b->memory[b->ptr];
      break;
    case 10:
      switch (b->dir)
      {
        case 0:
          b->dir = 1;
          break;
        case 1:
          b->dir = 2;
          break;
        case 2:
          b->dir = 3;
          break;
        case 3:
          b->dir = 0;
          break;
      }
      break;
    case 11:
      switch (b->dir)
      {
        case 0:
          b->dir = 3;
          break;
        case 1:
          b->dir = 0;
          break;
        case 2:
          b->dir = 1;
          break;
        case 3:
          b->dir = 2;
          break;
      }
      break;
    case 12:
      // b->energy -= 40;
      switch (b->dir)
      {
        case 0:
          if (b->p + 1 < SX * SY && lb[b->p + 1] == NULL)
          {
            lb[b->p + 1] = b;
            b->p = b->p + 1;
          }
          break;
        case 1:
          if (b->p - SX >= 0 && lb[b->p - SX] == NULL)
          {
            lb[b->p - SX] = b;
            b->p = b->p - SX;
          }
          break;
        case 2:
          if (b->p - 1 >= 0 && lb[b->p - 1] == NULL)
          {
            lb[b->p - 1] = b;
            b->p = b->p - 1;
          }
          break;
        case 3:
          if (b->p + SX < SX * SY && lb[b->p + SX] == NULL)
          {
            lb[b->p + SX] = b;
            b->p = b->p + SX;
          }
          break;
      }
      break;
    case 13:
      // b->energy -= 40;
      switch (b->dir)
      {
        case 2:
          if (b->p + 1 < SX * SY && lb[b->p + 1] == NULL)
          {
            lb[b->p + 1] = b;
            b->p = b->p + 1;
          }
          break;
        case 3:
          if (b->p - SX >= 0 && lb[b->p - SX] == NULL)
          {
            lb[b->p - SX] = b;
            b->p = b->p - SX;
          }
          break;
        case 0:
          if (b->p - 1 >= 0 && lb[b->p - 1] == NULL)
          {
            lb[b->p - 1] = b;
            b->p = b->p - 1;
          }
          break;
        case 1:
          if (b->p + SX < SX * SY && lb[b->p + SX] == NULL)
          {
            lb[b->p + SX] = b;
            b->p = b->p + SX;
          }
          break;
      }
      break;
    case 14:
      if(b->energy / 5.0 <= 0) {
        break;
      }
      for (i = 0; i < MEM_SIZE; i++)
      {
        ngcode[i] = b->gcode[i];
      }
      for (i = 0; i < 100; i++)
      {
        if (rand() % 1000 < VAR_TAX)
          ngcode[rand() % MEM_SIZE] = rand() % 20;
        else
          break;
      }
      switch (b->dir)
      {
        case 0:
          if (b->p + 1 < SX * SY && lb[b->p + 1] == NULL)
          {
            lb[b->p + 1] = &bots[last];
            set_bot(&bots[last++], b, b->p + 1, b->energy / 5.0, ngcode, lb, b->generation);
            b->energy -= b->energy / 5.0;
          }
          break;
        case 1:
          if (b->p - SX >= 0 && lb[b->p - SX] == NULL)
          {
            lb[b->p - SX] = &bots[last];
            set_bot(&bots[last++], b, b->p - SX, b->energy / 5.0, ngcode, lb, b->generation);
            b->energy -= b->energy / 5.0;
          }
          break;
        case 2:
          if (b->p - 1 >= 0 && lb[b->p - 1] == NULL)
          {
            lb[b->p - 1] = &bots[last];
            set_bot(&bots[last++], b, b->p - 1, b->energy / 5.0, ngcode, lb, b->generation);
            b->energy -= b->energy / 5.0;
          }
          break;
        case 3:
          if (b->p + SX < SX * SY && lb[b->p + SX] == NULL)
          {
            lb[b->p + SX] = &bots[last];
            set_bot(&bots[last++], b, b->p + SX, b->energy / 5.0, ngcode, lb, b->generation);
            b->energy -= b->energy / 5.0;
          }
          break;
      }
      break;
    case 15:
      switch (b->dir)
      {
        index = rand() % MEM_SIZE;
        case 0:
          if (b->p + 1 < SX * SY && b->p + SX < SX * SY && lb[b->p + 1] != NULL && lb[b->p + SX] == NULL)
          {
            //&& compatible(lb[b->p + 1], b)) {
            for (i = 0; i < index; i++)
            {
              ngcode[i] = b->gcode[i];
            }
            for (i = index; i < MEM_SIZE; i++)
            {
              ngcode[i] = lb[b->p + 1]->gcode[i];
            }
            for (i = 0; i < 100; i++)
            {
              if (rand() % 1000 < VAR_TAX)
                ngcode[rand() % MEM_SIZE] = rand() % 20;
                else
              break;
            }
            // set_bot(&bots[last++], b, b->p + SX, b->energy / 5.0 + lb[b->p + 1]->energy / 5.0, ngcode, lb, b->generation > lb[b->p + 1]->generation ? b->generation : lb[b->p + 1]->generation);
            set_bot(&bots[last++], b, b->p + SX, b->energy / 5.0, ngcode, lb, b->generation > lb[b->p + 1]->generation ? b->generation : lb[b->p + 1]->generation);
            b->energy -= b->energy / 5.0;
            // lb[b->p + 1]->energy -= lb[b->p + 1]->energy / 5.0;
          }
          break;
        case 1:
          if (b->p - SX >= 0 && b->p + 1 < SX * SY && lb[b->p - SX] != NULL && lb[b->p + 1] == NULL)
          {
            //&& compatible(lb[b->p - SX], b)) {
            for (i = 0; i < index; i++)
            {
              ngcode[i] = b->gcode[i];
            }
            for (i = index; i < MEM_SIZE; i++)
            {
              ngcode[i] = lb[b->p - SX]->gcode[i];
            }
            for (i = 0; i < 100; i++)
            {
              if (rand() % 1000 < VAR_TAX)
                ngcode[rand() % MEM_SIZE] = rand() % 20;
                else
              break;
            }
            // set_bot(&bots[last++], b, b->p + 1, b->energy / 5.0 + lb[b->p - SX]->energy / 5.0, ngcode, lb, b->generation > lb[b->p - SX]->generation ? b->generation : lb[b->p - SX]->generation);
            set_bot(&bots[last++], b, b->p + 1, b->energy / 5.0, ngcode, lb, b->generation > lb[b->p - SX]->generation ? b->generation : lb[b->p - SX]->generation);
            b->energy -= b->energy / 5.0;
            // lb[b->p - SX]->energy -= lb[b->p - SX]->energy / 5.0;
          }
          break;
        case 2:
          if (b->p - 1 >= 0 && b->p - SX >= 0 && lb[b->p - 1] != NULL && lb[b->p - SX] == NULL)
          {
            //&& compatible(lb[b->p - 1], b)) {
            for (i = 0; i < index; i++)
            {
              ngcode[i] = b->gcode[i];
            }
            for (i = index; i < MEM_SIZE; i++)
            {
              ngcode[i] = lb[b->p - 1]->gcode[i];
            }
            for (i = 0; i < 100; i++)
            {
              if (rand() % 1000 < VAR_TAX)
                ngcode[rand() % MEM_SIZE] = rand() % 20;
                else
              break;
            }
            // set_bot(&bots[last++], b, b->p - SX, b->energy / 5.0 + lb[b->p - 1]->energy / 5.0, ngcode, lb, b->generation > lb[b->p - 1]->generation ? b->generation : lb[b->p - 1]->generation);
            set_bot(&bots[last++], b, b->p - SX, b->energy / 5.0, ngcode, lb, b->generation > lb[b->p - 1]->generation ? b->generation : lb[b->p - 1]->generation);
            b->energy -= b->energy / 5.0;
            // lb[b->p - 1]->energy -= lb[b->p - 1]->energy / 5.0;
          }
          break;
        case 3:
          if (b->p + SX < SX * SY && b->p - 1 >= 0 && lb[b->p + SX] != NULL && lb[b->p - 1] == NULL)
          {
            //&& compatible(lb[b->p + SX], b)) {
            for (i = 0; i < index; i++)
            {
              ngcode[i] = b->gcode[i];
            }
            for (i = index; i < MEM_SIZE; i++)
            {
              ngcode[i] = lb[b->p + SX]->gcode[i];
            }
            for (i = 0; i < 100; i++)
            {
              if (rand() % 1000 < VAR_TAX)
                ngcode[rand() % MEM_SIZE] = rand() % 20;
                else
              break;
            }
            // set_bot(&bots[last++], b, b->p - 1, b->energy / 5.0 + lb[b->p + SX]->energy / 5.0, ngcode, lb, b->generation > lb[b->p + SX]->generation ? b->generation : lb[b->p + SX]->generation);
            set_bot(&bots[last++], b, b->p - 1, b->energy / 5.0, ngcode, lb, b->generation > lb[b->p + SX]->generation ? b->generation : lb[b->p + SX]->generation);
            b->energy -= b->energy / 5.0;
            // lb[b->p + SX]->energy -= lb[b->p + SX]->energy / 5.0;
          }
          break;
      }
      break;

    case 16:
      switch (b->dir)
      {
        case 0:
          if (b->p + 1 < SX * SY && lb[b->p + 1] != NULL)
          {
            b->energy += lb[b->p + 1]->energy / 10.0;
            lb[b->p + 1]->energy = lb[b->p + 1]->energy / 10.0 * 9;
          }
          break;
        case 1:
          if (b->p - SX >= 0 && lb[b->p - SX] != NULL)
          {
            b->energy += lb[b->p - SX]->energy / 10.0;
            lb[b->p - SX]->energy = lb[b->p - SX]->energy / 10.0 * 9;
          }
          break;
        case 2:
          if (b->p - 1 >= 0 && lb[b->p - 1] != NULL)
          {
            b->energy += lb[b->p - 1]->energy / 10.0;
            lb[b->p - 1]->energy = lb[b->p - 1]->energy / 10.0 * 9;
          }
          break;
        case 3:
          if (b->p + SX < SX * SY && lb[b->p + SX] != NULL)
          {
            b->energy += lb[b->p + SX]->energy / 10.0;
            lb[b->p + SX]->energy = lb[b->p + SX]->energy / 10.0 * 9;
          }
          break;
      }
      break;
    case 17:
      switch (b->dir)
      {
        case 0:
          if (b->p + 1 < SX * SY && lb[b->p + 1] != NULL && compatible(lb[b->p + 1], b))
          {
            mean = (b->energy + lb[b->p + 1]->energy) / 2.0;
            b->energy = mean;
            lb[b->p + 1]->energy = mean;
          }
          break;
        case 1:
          if (b->p - SX >= 0 && lb[b->p - SX] != NULL && compatible(lb[b->p - SX], b))
          {
            mean = (b->energy + lb[b->p - SX]->energy) / 2.0;
            b->energy = mean;
            lb[b->p - SX]->energy = mean;
          }
          break;
        case 2:
          if (b->p - 1 >= 0 && lb[b->p - 1] != NULL && compatible(lb[b->p - 1], b))
          {
            mean = (b->energy + lb[b->p - 1]->energy) / 2.0;
            b->energy = mean;
            lb[b->p - 1]->energy = mean;
          }
          break;
        case 3:
          if (b->p + SX < SX * SY && lb[b->p + SX] != NULL && compatible(lb[b->p + SX], b))
          {
            mean = (b->energy + lb[b->p + SX]->energy) / 2.0;
            b->energy = mean;
            lb[b->p + SX]->energy = mean;
          }
          break;
      }
      break;
    case 18:
      switch (b->dir)
      {
        case 0:
          if (b->p + 1 < SX * SY && lb[b->p + 1] == NULL)
          {
            lb[b->p + 1] = &bots[last];
            set_bot(&bots[last++], b, b->p + 1, b->energy / 5.0, b->new_gcode, lb, b->generation);
            b->energy -= b->energy / 5.0;
          }
          break;
        case 1:
          if (b->p - SX >= 0 && lb[b->p - SX] == NULL)
          {
            lb[b->p - SX] = &bots[last];
            set_bot(&bots[last++], b, b->p - SX, b->energy / 5.0, b->new_gcode, lb, b->generation);
            b->energy -= b->energy / 5.0;
          }
          break;
        case 2:
          if (b->p - 1 >= 0 && lb[b->p - 1] == NULL)
          {
            lb[b->p - 1] = &bots[last];
            set_bot(&bots[last++], b, b->p - 1, b->energy / 5.0, b->new_gcode, lb, b->generation);
            b->energy -= b->energy / 5.0;
          }
          break;
        case 3:
          if (b->p + SX < SX * SY && lb[b->p + SX] == NULL)
          {
            lb[b->p + SX] = &bots[last];
            set_bot(&bots[last++], b, b->p + SX, b->energy / 5.0, b->new_gcode, lb, b->generation);
            b->energy -= b->energy / 5.0;
          }
          break;
      }
      break;
    case 19:
      if (b->last_adr < MEM_SIZE)
        b->new_gcode[b->last_adr++] = b->gcode[b->pos++];
      break;
  }
  // b->memory[b->ptr] = b->memory[b->ptr] % 9;
}

void setpixel(SDL_Surface *screen, int x, int y, int r, int g, int b)
{
  if (x < 0 || y < 0 || x >= SX || y >= SY || r < 0 || g < 0 || b < 0)
    return;
  if (r > 255)
    r = 255;
  if (g > 255)
    g = 255;
  if (b > 255)
    b = 255;
  Uint32 *pixmem32;
  Uint32 colour;

  colour = SDL_MapRGB(screen->format, r, g, b);

  pixmem32 = (Uint32 *)screen->pixels + screen->w * y + x;
  *pixmem32 = colour;
}

void reset_bot(struct bot *b) {
  b->p = 0;
  b->lp = 0;
  b->energy = 0.0;
  for (int i = 0; i < MEM_SIZE; i++) {
    b->gcode[i] = 0;
    b->memory[i] = 0;
    b->new_gcode[i] = 0;
  }
  b->nl = 0;
  memset(b->loops, 0, sizeof(b->loops));
  memset(b->loops_ptr, 0, sizeof(b->loops_ptr));
  b->ptr = 0;
  b->pos = 0;
  b->dir = 0;
  b->r = b->g = b->b = 0;
  b->age = 0;
  b->generation = 0;
  b->last_adr = 0;
  b->dad = NULL;
}

int main(int argc, char *argv[])
{
  srand(time(0));
  FILE *file;
  file = fopen("data.txt", "a+");
  SDL_Surface *screen;
  SDL_Event event;
  last = 0;
  float total_energy_sum = 0;
  int keypress = 0, k = 0, i, position, j, food = 40, dr = 0, dg = 0, db = 0, depth = 0;
  long long b = 0, v = 0;
  struct bot *atual_dad;
  if (SDL_Init(SDL_INIT_VIDEO) < 0)
    return 1;

  if (!(screen = SDL_SetVideoMode(WIDTH, HEIGHT, DEPTH, SDL_HWSURFACE)))
  {
    SDL_Quit();
    return 1;
  }
  short selected[MEM_SIZE];
  bots = (struct bot *)malloc(sizeof(struct bot) * SX * SY);
  struct bot **lb = (struct bot **)malloc(sizeof(struct bot *) * SX * SY);
  for (i = 0; i < SX * SY; i++)
  {
    lb[i] = NULL;
  }
  short g[MEM_SIZE]; //{
  // 7, 4, 8, 0, 2, 7, 0, 9, 4, 2, 9, 6, 2, 4, 4, 2, 9, 3, 2, 7, 2, 0, 2, 0, 3, 4, 6, 2, 2, 2, 2, 1, 8, 0, 2, 8, 8, 5, 9, 2, 9, 6, 7, 5, 8, 8, 6, 9, 2, 8
  // 7, 6, 8, 8, 7, 3, 3, 9, 4, 4, 9, 6, 8, 0, 6, 4, 9, 2, 2, 6, 9, 6, 2, 0, 3, 4, 6, 7, 2, 2, 2, 1, 8, 0, 9, 8, 5, 5, 7, 2, 9, 6, 8, 0, 8, 6, 6, 9, 9, 9
  //-1, -1, 3, 7, 11, 15, 19, 23, 27, 31, 35, 39, 43, 11, 51, 55, 5, 63, 67, 71, 75, 79, 83, 87, 91, 95, 13, 9, 107, 111, 115, 3, 123, 127, 131, 135, 139, 143, 147, 13, 155, 8, 163, 167, 171, 175, 179, 183, 187, 191, 195, 199, 203, 207, 211, 215, 219, 223, 227, 231, 235, 239, 243, 247, 251, 255, 259, 263, 267, 271, 275, 279, 283, 287, 291, 295, 299, 303, 307, 311, 315, 13, 323, 327, 331, 335, 339, 343, 347, 351, 355, 359, 363, 367, 6, 375, 9, 383, 387, 391, 6, 3, 7, 14, 13, 9, 13, 6, 3, 0, 10, 0, 4, 12, 13, 12, 12, 2, 8, 12, 14, 15, 1, 6, 2, 0, 1, 12, 12, 4, 16, 1, 7, 11, 6, 11, 4, 10, 0, 15, 10, 11, 15, 14, 14, 2, 9, 0, 4, 9, 13, 1, 15, 14, 7, 0, 5, 16, 12, 8, 12, 16, 0, 10, 2, 6, 5, 14, 0, 5, 12, 10, 7, 1, 8, 4, 3, 8, 5, 15, 0, 9, 16, 7, 14, 15, 16, 10, 5, 2, 5, 0, 2, 1, 11, 12, 15, 7, 9, 15
  //};
  // for (i = 0; i < 0; i++)
  //	set_bot(&bots[last++], NULL, rand() % (SX * SY), 1000, g, lb, 0);
  short get = 0, view = 0;
  float comp;
  while (!keypress)
  {
    ++k;
    for (i = 0; i < last; i++)
    {
      compute(&bots[i], lb);
      switch (view)
      {
        case 0:
          setpixel(screen, bots[i].p % SX, bots[i].p / SX % SY, bots[i].r, bots[i].g, bots[i].b);
          break;
        case 1:
          setpixel(screen, bots[i].p % SX, bots[i].p / SX % SY, bots[i].energy, bots[i].energy / 10.0, bots[i].energy / 100.0);
          break;
        case 2:
          setpixel(screen, bots[i].p % SX, bots[i].p / SX % SY, bots[i].age / MAX_AGE * 255, bots[i].age / MAX_AGE * 255, bots[i].age / MAX_AGE * 255);
          break;
        case 3:
          if (bots[i].generation > 2)
            setpixel(screen, bots[i].p % SX, bots[i].p / SX % SY, bots[i].generation / 100.0, bots[i].generation / 100.0, bots[i].generation / 100.0);
          break;
        case 4:
          if (bots[i].generation > 1)
            setpixel(screen, bots[i].p % SX, bots[i].p / SX % SY, bots[i].r, bots[i].g, bots[i].b);
          break;
        case 5:
          if (selected != NULL)
          {
            comp = compatibility(selected, &bots[i]) * 255;
            setpixel(screen, bots[i].p % SX, bots[i].p / SX % SY, comp, comp, comp);
          }
          break;
      }
    }
    for (i = 0; i < SX * SY; i++)
    {
      lb[i] = NULL;
    }
    total_energy_sum = 0;
    for (i = 0; i < last; ) // Remove the increment from here
    {
      if (bots[i].energy > 0 && bots[i].age > 0)
      {
        // Bot is alive, update its position in the lb array and increment i
        lb[bots[i].p] = &bots[i];
        total_energy_sum += bots[i].energy;
        i++; // Move to the next bot only if the current bot is not removed
      }
      else
    {
        // Bot is dead, clear its position in the lb array
        lb[bots[i].p] = NULL;

        if (i != last - 1) // Check if it's not already the last bot
        {
          // Move the last bot to the current position
          bots[i] = bots[--last];

          // Update the lb array to reflect the moved bot's new position
          lb[bots[i].p] = &bots[i];
        }
        else
      {
          // If it's the last bot, just decrement last
          --last;
          break; // No need to increment i, as we're done with this bot
        }

        // Do not increment i, as we need to check the bot that was moved to position i
      }
    }

    for (i = 0; i < last; i++)
    {
      if (k % 10 == 0)
      {
        if (bots[i].energy > v && bots[i].generation > 20)
        {
          b = i;
          v = bots[i].energy;
        }
      }
      else if (get)
      {
        dr += bots[i].r;
        dg += bots[i].g;
        db += bots[i].b;
        if (bots[i].energy > v && bots[i].generation > 20)
        {
          b = i;
          v = bots[i].energy;
        }
      }
    }
    if (get)
    {
      printf("Mean Color -> (%f, %f, %f)\n", dr / (float)last, dg / (float)last, db / (float)last);
      dr = 0;
      dg = 0;
      db = 0;
      printf("Atual best cell specification:");
      v = 0;
      atual_dad = &bots[b];
      while (atual_dad != NULL && depth < MAX_GENENARATION_UP_SHOW)
      {
        printf("#%i# generations up genoma# ", depth++);
        for (i = 0; i < MEM_SIZE - 1; i++)
        {
          printf("%i, ", atual_dad->gcode[i]);
        }
        printf("%i\n", atual_dad->gcode[MEM_SIZE - 1]);
        atual_dad = atual_dad->dad;
      }
      printf("\n\n");
      printf("##################\n");
      depth = 0;
      get = 0;
    }
    for (j = 0; rand() % 100 < food; j++)
    {
      position = rand() % (SX * SY);
      for (i = 0; i < MEM_SIZE; i++)
      {
        g[i] = rand() % 20;
      }
      if (lb[position] == NULL)
        set_bot(&bots[last++], NULL, position, 100000, g, lb, 0);
    }
    if (k % 10 == 0)
    {
      for (i = 0; i < MEM_SIZE - 1; i++)
      {
        fprintf(file, "%i, ", bots[b].gcode[i]);
      }
      fprintf(file, "%i, %i, %f\n", bots[b].gcode[i], last, total_energy_sum);
    }
    if (view != 6)
    {
      SDL_Flip(screen);
      // sprintf(buf ,"%d", k / 100);
      // SDL_SaveBMP(screen, buf);
      SDL_FillRect(screen, NULL, 0x000000);
    }
    while (SDL_PollEvent(&event))
    {
      switch (event.type)
      {
        case SDL_QUIT:
          keypress = 1;
          break;
        case SDL_KEYDOWN:
          switch (event.key.keysym.sym)
          {
            case SDLK_g:
              get = 1;
              break;
            case SDLK_v:
              view = (view + 1) % 7;
              switch (view)
              {
                case 0:
                  printf("Genetic View\n");
                break;
                case 1:
                  printf("Energy View\n");
                break;
                case 2:
                  printf("Age View\n");
                break;
                case 3:
                  printf("Generation View\n");
                break;
                case 4:
                  printf("Filtered Genetic View\n");
                break;
                case 5:
                  printf("Compatibility View\n");
                break;
                case 6:
                  printf("Don't rendening\n");
                  SDL_FillRect(screen, NULL, 0x000000);
                break;
              }
              break;
            case SDLK_UP:
              ++food;
              printf("More Food -> %i\n", food);
              break;
            case SDLK_DOWN:
              --food;
              printf("Less Food -> %i\n", food);
              break;
          }
          break;
        case SDL_MOUSEBUTTONDOWN:
          if (event.button.button == 1)
          {
            if (lb[event.button.x + SX * event.button.y] != NULL)
            {
              v = 0;
              atual_dad = lb[event.button.x + SX * event.button.y];
              while (atual_dad != NULL && depth < MAX_GENENARATION_UP_SHOW)
              {
                printf("#%i# generations up genoma# ", depth++);
                if (depth == 1)
                {
                  for (i = 0; i < MEM_SIZE - 1; i++)
                  {
                    printf("%X,", atual_dad->gcode[i]);
                    selected[i] = atual_dad->gcode[i];
                  }
                  printf("%X\n", atual_dad->gcode[MEM_SIZE - 1]);
                  selected[MEM_SIZE - 1] = atual_dad->gcode[MEM_SIZE - 1];
                }
                else
                {
                  for (i = 0; i < MEM_SIZE - 1; i++)
                  {
                    printf("%X,", atual_dad->gcode[i]);
                  }
                  printf("%X\n", atual_dad->gcode[MEM_SIZE - 1]);
                }
                atual_dad = atual_dad->dad;
              }
              printf("\n\n");
              printf("##################\n");
              depth = 0;
            }
            break;
          }
          else if (event.button.button == 3)
          {
            if (lb[event.button.x + SX * event.button.y] != NULL)
            {
              v = 0;
              atual_dad = lb[event.button.x + SX * event.button.y];
              while (atual_dad != NULL && depth < 1)
              {
                printf("#%i# generations up genoma# ", depth++);
                for (i = 0; i < MEM_SIZE - 1; i++)
                {
                  printf("%X,", atual_dad->gcode[i]);
                  selected[i] = atual_dad->gcode[i];
                }
                printf("%X\n", atual_dad->gcode[MEM_SIZE - 1]);
                selected[MEM_SIZE - 1] = atual_dad->gcode[MEM_SIZE - 1];
                atual_dad = atual_dad->dad;
              }
              printf("\n");
              printf("##################\n");
              depth = 0;
            }
            break;
        }
      }
    }
  }
  return (0);
}
