#include <SDL2/SDL.h>

const Uint32 ANIMATION_FRAME_TIME = 1000/25*3;
const int WINDOW_WIDTH = 640;
const int WINDOW_HEIGHT = 480;
const int FRAME_INIT_X = 0;
const int FRAME_INIT_Y = 0;
const int ANIMATION_FRAME_WIDTH = 60;
const int ANIMATION_FRAME_HEIGHT = 60;
int ENDTURN = 0;


/** Names of sprites
* Names for array indexes to simulate associative array
*/
enum Actors
{
  BALL = 0,
  PLAYER1 = 1,
  PLAYER2 = 2,
  NET = 3
};

/** Sprite
* Holds all informations about sprite.
*/
struct Sprite
{
  SDL_Rect srcrect; /**< current animation frame from sprite sheet */
  SDL_Rect dstrect; /**< where to render sprite */
  SDL_Texture *texture;
  SDL_Point d; /**< change in position to apply */
  double angle; /**< rotation to apply, used only by BALL */
  Uint32 last_anim_frame_change; /**< time of last change of animation frame*/
};
typedef struct Sprite Sprite;

Sprite load_sprite(SDL_Renderer *renderer, const char *filename)
{
  Sprite sprite;
  SDL_Surface *surface = SDL_LoadBMP(filename);
  sprite.texture = SDL_CreateTextureFromSurface(renderer, surface);
  SDL_FreeSurface(surface);
  SDL_SetTextureBlendMode(sprite.texture, SDL_BLENDMODE_BLEND);
  sprite.last_anim_frame_change = ANIMATION_FRAME_TIME;
  sprite.srcrect.x = FRAME_INIT_X;
  sprite.srcrect.y = FRAME_INIT_Y;
  sprite.srcrect.w = ANIMATION_FRAME_WIDTH;
  sprite.srcrect.h = ANIMATION_FRAME_HEIGHT;
  sprite.dstrect.w = ANIMATION_FRAME_WIDTH;
  sprite.dstrect.h = ANIMATION_FRAME_HEIGHT;
  sprite.d.x = 0;
  sprite.d.y = 0;
  sprite.angle = 0;
  return sprite;
}

void load_sprites(SDL_Renderer *renderer, Sprite *sprite)
{
  sprite[PLAYER1] = load_sprite(renderer, "player01.bmp");
  sprite[PLAYER2] = load_sprite(renderer, "player02.bmp");
  sprite[BALL] = load_sprite(renderer, "ball.bmp");
  sprite[NET] = load_sprite(renderer, "net.bmp");


  sprite[PLAYER1].dstrect.y = WINDOW_HEIGHT - ANIMATION_FRAME_HEIGHT;
  sprite[PLAYER2].dstrect.y = WINDOW_HEIGHT - ANIMATION_FRAME_HEIGHT;

  sprite[NET].srcrect.x = 0;
  sprite[NET].srcrect.y = 0;
  sprite[NET].srcrect.w = 10;
  sprite[NET].srcrect.h = 200;
  sprite[NET].dstrect.x = WINDOW_WIDTH / 2 - sprite[NET].srcrect.w / 2;
  sprite[NET].dstrect.y = WINDOW_HEIGHT - sprite[NET].srcrect.h;
  sprite[NET].dstrect.w = sprite[NET].srcrect.w;
  sprite[NET].dstrect.h = sprite[NET].srcrect.h;

}

void place_sprites_on_start(Sprite *sprite, int delivery)
{
  sprite[PLAYER1].dstrect.x = 20;
  sprite[PLAYER2].dstrect.x = 520;

  if (delivery == PLAYER2)
  {
    sprite[BALL].dstrect.x = 20;
    sprite[BALL].dstrect.y = 50;
  }
  else if (delivery == PLAYER1)
  {
    sprite[BALL].dstrect.x = 520;
    sprite[BALL].dstrect.y = 50;
  }
  else
  {
    SDL_Quit();
    exit(EXIT_FAILURE);
  }
  sprite[BALL].d.x = 0;
  sprite[BALL].d.y = 0;
}

SDL_bool process_events(unsigned int *is_npc)
{
  SDL_Event event;
  while(SDL_PollEvent(&event))
  {

    switch(event.type)
    {

      case SDL_WINDOWEVENT_CLOSE:
        return SDL_TRUE;

      case SDL_QUIT:
        return SDL_TRUE;

      case SDL_KEYDOWN:
        if (event.key.keysym.sym == SDLK_F1)
        {
          *is_npc = *is_npc ^ 1; /* toggle first bit in numer for player 1, used in control_oponent */
        }
        if (event.key.keysym.sym == SDLK_F2)
        {
          *is_npc = *is_npc ^ 2; /* toggle second bit in numer for player 2, used in control_oponent */
        }
        return SDL_FALSE;

      default:
        return SDL_FALSE;

    }
  }
  return SDL_FALSE;
}

void control_player(Sprite *sprites)
{
  const int step = 5;
  const int jump = 16;
  const Uint8 *keyboard_state = SDL_GetKeyboardState(NULL);

  /* PLAYER 1 */
  if (keyboard_state[SDL_SCANCODE_LEFT])
  {
    sprites[PLAYER1].d.x = -step;
  }
  else if (keyboard_state[SDL_SCANCODE_RIGHT])
  {
    sprites[PLAYER1].d.x = step;
  }
  else
  {
    sprites[PLAYER1].d.x = 0;
  }

  if (keyboard_state[SDL_SCANCODE_UP])
  {
    if (sprites[PLAYER1].dstrect.y == WINDOW_HEIGHT - ANIMATION_FRAME_HEIGHT) /* IS standing on the ground */
    {
      sprites[PLAYER1].d.y-=jump;
    }
  }

  /* PLAYER 2 */
  if (keyboard_state[SDL_SCANCODE_A])
  {
    sprites[PLAYER2].d.x = -step;
  }
  else if (keyboard_state[SDL_SCANCODE_D])
  {
    sprites[PLAYER2].d.x = step;
  }
  else
  {
    sprites[PLAYER2].d.x = 0;
  }

  if (keyboard_state[SDL_SCANCODE_W])
  {
    if (sprites[PLAYER2].dstrect.y == WINDOW_HEIGHT - ANIMATION_FRAME_HEIGHT) /* IS standing on the ground */
    {
      sprites[PLAYER2].d.y -= jump;
    }
  }

}

void control_oponent(Sprite *sprites, unsigned int is_npc)
{
  if (ENDTURN) {return;}
  const int step = 5;
  const int jump = 16;
  const int max_jump = 150;
  int x_diff;
  float x_ratio;
  int npc;

  if (sprites[BALL].dstrect.x > sprites[NET].dstrect.x && (is_npc == 2 || is_npc == 3)) /* binary 10 OR 11 */
  {
    npc = PLAYER2;
  }
  else if (sprites[BALL].dstrect.x < sprites[NET].dstrect.x && (is_npc == 1 || is_npc == 3)) /* binary 01 or 11 */
  {
    npc = PLAYER1;
  }
  else
  {
  return;
  }
  x_diff = sprites[BALL].dstrect.x - sprites[npc].dstrect.x;
  x_ratio = (float) x_diff / 55; /**< 1.0..-1.0 player left positive, player right negative*/

  if (x_ratio < -1) /* go forward or backward if not uder ball */
  {
    sprites[npc].d.x = -step;
  }
  else if (x_ratio > 1)
  {
    sprites[npc].d.x = step;
  }

  else /* if under ball */
  {
    sprites[npc].d.x = 0;

    if (x_ratio == 0) /* if just in the center of ball, step sideways to avoid 90 degree ball movement */
    {
      if(SDL_GetTicks() % 2 == 0)
      {
        sprites[npc].d.x = step;
      }
      else
      {
        sprites[npc].d.x = -step;
      }
    }

    if (sprites[BALL].dstrect.y >  max_jump && sprites[npc].dstrect.y == WINDOW_HEIGHT - ANIMATION_FRAME_HEIGHT) /* if ball close enough, jump */
    {
      sprites[npc].d.y -= jump;
    }
  }
}


void apply_gravity(Sprite *sprites)
{
  for (int i = BALL; i < PLAYER2+1; i++)
  {
    if (sprites[i].dstrect.y < WINDOW_HEIGHT - sprites[i].dstrect.h)
    {
      sprites[i].d.y += 1;
    }
  }
}

int bounce_ball(Sprite *ball)
{
  if (ball->dstrect.y >= WINDOW_HEIGHT - ball->dstrect.h) /* hit GROUND*/
  {
    ball->d.y = ball->d.y * -0.8;
    ball->d.x *= 0.8;
    if (ball->dstrect.x > WINDOW_WIDTH / 2)
      {return PLAYER1;}
    else
      {return PLAYER2;}
  }

  if (ball->dstrect.y <= 0) /*hit CEILING */
  {
    ball->d.y = ball->d.y * -0.8;
  }

  if (ball->dstrect.x >= WINDOW_WIDTH - ball->dstrect.w) /* hit RIGHT wall */
  {
    ball->d.x = ball->d.x * -0.8;
  }

  if  (ball->dstrect.x <= 0) /* hit LEFT wall */
  {
    ball->d.x = ball->d.x * -0.8;
  }
  return 0;
}

void hit_ball(Sprite *sprites)
{
  if (ENDTURN) {return;}
  int x_diff = 0;
  float x_ratio = 0;
  for (int i = PLAYER1; i < NET; i++)
  {
    if (SDL_HasIntersection(&sprites[i].dstrect, &sprites[BALL].dstrect))
    {
      sprites[BALL].dstrect.y = sprites[i].dstrect.y - sprites[BALL].dstrect.h;
      x_diff = sprites[BALL].dstrect.x - sprites[i].dstrect.x;
      x_ratio = (float) x_diff / 55; /**< 1.0..-1.0 player left positive, player right negative*/
      if (sprites[BALL].d.y >= 0)
      {
        sprites[BALL].d.y = sprites[BALL].d.y * -0.8;
        sprites[BALL].d.x = sprites[BALL].d.x * -0.8;
      }
      else
      {sprites[BALL].d.y += sprites[i].d.y;
       sprites[BALL].d.x = abs(sprites[BALL].d.y) * x_ratio;}


    }
  }


}

void hit_net(Sprite *sprites)
{
if (SDL_HasIntersection(&sprites[BALL].dstrect, &sprites[NET].dstrect))
  {
    if (sprites[BALL].dstrect.x < sprites[NET].dstrect.x)
    {
      sprites[BALL].dstrect.x = sprites[NET].dstrect.x - sprites[BALL].dstrect.w;
    }
    else
    {
      sprites[BALL].dstrect.x = sprites[NET].dstrect.x + sprites[NET].dstrect.w;
    }
    sprites[BALL].d.x *= -0.8;
  }
}

void animate_players(Sprite *sprites)
{
  for (int i = PLAYER1; i < NET; i++)
  {
    SDL_bool moves_verticaly = (sprites[i].d.y < 0);
    if (moves_verticaly)
    {
      sprites[i].srcrect.y = 60; /* animation row */
    }
    else
    {
      sprites[i].srcrect.y = 0;
    }


    /* animation col */
    SDL_bool moves_horizontaly = (sprites[i].d.x != 0);
    if (SDL_TICKS_PASSED(SDL_GetTicks(), sprites[i].last_anim_frame_change) && moves_horizontaly)
    {
      if (sprites[i].srcrect.x < ANIMATION_FRAME_WIDTH) {sprites[i].srcrect.x+=ANIMATION_FRAME_WIDTH;}
      else {sprites[i].srcrect.x = FRAME_INIT_X;}
      sprites[i].last_anim_frame_change = SDL_GetTicks() + ANIMATION_FRAME_TIME;
    }
    else
    {
      if (SDL_TICKS_PASSED(SDL_GetTicks(), sprites[i].last_anim_frame_change))
      {
        sprites[i].srcrect.x = FRAME_INIT_X;
      }
    }
  }
}

void animate_ball(Sprite *ball)
{
  if (ball->d.x != 0)
  {
    if (ball->angle <  360)
      {ball->angle += 6;}
    else
      {ball->angle = 0;}
  }
}

void apply_delta(Sprite *sprites)
{
  for (int i = BALL; i < PLAYER2+1; i++)
  {
    sprites[i].dstrect.x += sprites[i].d.x;
    sprites[i].dstrect.y += sprites[i].d.y;

    if (sprites[i].dstrect.x < 0)
    {
      sprites[i].dstrect.x = 0;
    }

    if (sprites[i].dstrect.x > WINDOW_WIDTH - ANIMATION_FRAME_WIDTH)
    {
      sprites[i].dstrect.x = WINDOW_WIDTH - ANIMATION_FRAME_WIDTH;
    }

    if (sprites[i].dstrect.y >= WINDOW_HEIGHT - ANIMATION_FRAME_HEIGHT) /* IS standing on the ground */
    {
    sprites[i].dstrect.y = WINDOW_HEIGHT - ANIMATION_FRAME_HEIGHT;
    }

    if (sprites[i].dstrect.y < 0) /*hit CEILING */
    {
      sprites[i].dstrect.y = 0;
    }

  }
}

void render(SDL_Renderer *renderer, Sprite *sprites)
{

  const SDL_Point *center = NULL;
  const SDL_RendererFlip flip = SDL_FLIP_NONE;

  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
  SDL_RenderClear(renderer);

  for (int i = BALL; i < NET+1; i++)
  {

  SDL_RenderCopyEx(
    renderer, sprites[i].texture,
    &sprites[i].srcrect,
    &sprites[i].dstrect,
    sprites[i].angle,
    center,
    flip);

  }

  SDL_RenderPresent(renderer);
}


int main(int argc, char **argv)
{
  SDL_Window *window = NULL;
  SDL_Renderer *renderer = NULL;
  Sprite sprites[4];
  unsigned int score[3];
  unsigned int is_npc = 0;

  score[BALL] = 0; /**< last player who scored */
  score[PLAYER1] = 0;
  score[PLAYER2] = 0;


  SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO);
  SDL_LogSetAllPriority(SDL_LOG_PRIORITY_INFO);

  window = SDL_CreateWindow(
    "Arcade Volleybal",
    SDL_WINDOWPOS_CENTERED,
    SDL_WINDOWPOS_CENTERED,
    WINDOW_WIDTH,
    WINDOW_HEIGHT,
    SDL_WINDOW_OPENGL);


  renderer = SDL_CreateRenderer(
    window,
    -1,
   SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
   SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

  load_sprites(renderer, sprites);
  place_sprites_on_start(sprites, PLAYER1);

/* <-- GAME LOOP */

  int point = 0;
  Uint32 timeout = 0;
  while(!process_events(&is_npc))
  {
    control_player(sprites);
    control_oponent(sprites, is_npc);
    point = bounce_ball(&sprites[BALL]);
    if (point && !ENDTURN)
    {
      score[point] += 1;
      score[BALL] = point;
      ENDTURN = 1;
      timeout = SDL_GetTicks() + 5000;
    }
    hit_net(sprites);
    hit_ball(sprites);
    apply_gravity(sprites);
    apply_delta(sprites);
    animate_players(sprites);
    animate_ball(&sprites[BALL]);
    render(renderer, sprites);
    if (ENDTURN && SDL_TICKS_PASSED(SDL_GetTicks(), timeout))
    {
      SDL_Log("Player %i: %i -- Player %i: %i\n", PLAYER1, score[PLAYER1], PLAYER2, score[PLAYER2]);
      place_sprites_on_start(sprites, score[BALL]);
      ENDTURN = 0;
    }
   }
/* GAME LOOP -->*/

  for (int i = BALL; i < NET+1; i++)
  {
    SDL_DestroyTexture(sprites[i].texture);
  }
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
}
