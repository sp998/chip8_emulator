#include <SDL2/SDL.h>
#include <iostream>
#include <vector>
#include "chip8.hpp"

#define GRID_WIDTH 64
#define GRID_HEIGHT 32

#define SHOW_GRIDLINES 0

typedef struct Color{
    uint8_t red;
    uint8_t green;
    uint8_t blue;
    struct Color* next;
    struct Color* prev;
}Color;


std::vector<SDL_Rect> grids;


Color red = {.red=255};
Color green={.green=255};
Color blue ={.blue=255};
Color yellow = {.red=255,.green=100};
Color black={};
Color display_color=green;

void setColor(SDL_Renderer* renderer,Color* color);
void __loop(SDL_Renderer* renderer,Chip8* chip8);

int main(int argc,char* argv[]){
   
   if(argc<2){
    std::cout<<"usage:chip8_emu <rom file>";
   }

    Chip8* chip8 = new Chip8(argv[1]);

    
    if(SDL_Init(SDL_INIT_VIDEO)<0){
        SDL_Log("SDL could not initialize! SDL_Error:%s\n",SDL_GetError());
        return 1;
    }

   

    Color* current_color =&black;

    SDL_Window *window = SDL_CreateWindow(chip8->get_name(),SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,64*10,32*10,SDL_WINDOW_SHOWN);

    if (!window){
        SDL_Log("Window could not be created !SDL_Error:%s\n",SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window,-1,SDL_RENDERER_ACCELERATED);

    if(!renderer){
        SDL_Log("Renderer could not be created! SDL_Error :%s\n",SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    bool quit = false;
    SDL_Event e;

    while(!quit){

        while(SDL_PollEvent(&e)!=0){
            if(e.type==SDL_QUIT){
                quit=true;
            }

            if(e.type==SDL_KEYDOWN){
                switch (e.key.keysym.scancode)
                {
                case SDL_SCANCODE_SPACE :
                    if(chip8->state==EmulatorState::RUNNING){
                    chip8->state=EmulatorState::PAUSED;
                    }else{
                        chip8->state = EmulatorState::RUNNING;
                    }
                    break;
                case SDL_SCANCODE_1: chip8->keypad[0x1]=true; break;
                case SDL_SCANCODE_2: chip8->keypad[0x2]=true; break;
                case SDL_SCANCODE_3: chip8->keypad[0x3]=true; break;
                case SDL_SCANCODE_4: chip8->keypad[0xC]=true; break;

                case SDL_SCANCODE_Q: chip8->keypad[0x4]=true; break;
                case SDL_SCANCODE_W: chip8->keypad[0x5]=true; break;
                case SDL_SCANCODE_E: chip8->keypad[0x6]=true; break;
                case SDL_SCANCODE_R: chip8->keypad[0xD]=true; break;
                
                case SDL_SCANCODE_A: chip8->keypad[0x7]=true; break;
                case SDL_SCANCODE_S: chip8->keypad[0x8]=true; break;
                case SDL_SCANCODE_D: chip8->keypad[0x9]=true; break;
                case SDL_SCANCODE_F: chip8->keypad[0xE]=true; break;
                
                case SDL_SCANCODE_Z: chip8->keypad[0xA]=true; break;
                case SDL_SCANCODE_X: chip8->keypad[0x0]=true; break;
                case SDL_SCANCODE_C: chip8->keypad[0xB]=true; break;
                case SDL_SCANCODE_V: chip8->keypad[0xF]=true; break;
                
                default:
                    break;
                }
            }

            if(e.type==SDL_KEYUP){
                switch (e.key.keysym.scancode)
                {
               
                case SDL_SCANCODE_1: chip8->keypad[0x1]=false; break;
                case SDL_SCANCODE_2: chip8->keypad[0x2]=false; break;
                case SDL_SCANCODE_3: chip8->keypad[0x3]=false; break;
                case SDL_SCANCODE_4: chip8->keypad[0xC]=false; break;

                case SDL_SCANCODE_Q: chip8->keypad[0x4]=false; break;
                case SDL_SCANCODE_W: chip8->keypad[0x5]=false; break;
                case SDL_SCANCODE_E: chip8->keypad[0x6]=false; break;
                case SDL_SCANCODE_R: chip8->keypad[0xD]=false; break;
                
                case SDL_SCANCODE_A: chip8->keypad[0x7]=false; break;
                case SDL_SCANCODE_S: chip8->keypad[0x8]=false; break;
                case SDL_SCANCODE_D: chip8->keypad[0x9]=false; break;
                case SDL_SCANCODE_F: chip8->keypad[0xE]=false; break;
                
                case SDL_SCANCODE_Z: chip8->keypad[0xA]=false; break;
                case SDL_SCANCODE_X: chip8->keypad[0x0]=false; break;
                case SDL_SCANCODE_C: chip8->keypad[0xB]=false; break;
                case SDL_SCANCODE_V: chip8->keypad[0xF]=false; break;
                
                default:
                    break;
                }
            }




        }

        setColor(renderer,current_color);
        SDL_RenderClear(renderer);
         

        __loop(renderer,chip8);
        
        SDL_RenderPresent(renderer);
       
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    delete chip8;
    SDL_Quit();
    return 0;
}

void setColor(SDL_Renderer* renderer,Color* color){
    SDL_SetRenderDrawColor(renderer,color->red,color->green,color->blue,255);
}

void loop(Chip8* chip8){
uint64_t start_time= SDL_GetPerformanceCounter();
if(chip8->state==EmulatorState::RUNNING){
chip8->run();
}

uint64_t end_time = SDL_GetPerformanceCounter();

const double time_elapsed = (double)((end_time-start_time)*1000)/SDL_GetPerformanceFrequency();
SDL_Delay(16.67f>time_elapsed?16.67-time_elapsed:0);

}

void __loop(SDL_Renderer* renderer,Chip8* chip8){
    SDL_Rect rect ={.x=0,.y=0,10,10};
    for(int j=0;j<32*10;j+=10){
        rect.y=j;
        for(int i=0;i<10*64;i+=10){
            rect.x=i;

            if(!SHOW_GRIDLINES){
                setColor(renderer,&black);
            }
            SDL_RenderDrawRect(renderer,&rect);
            grids.push_back(rect);
        }
    }

    for(uint16_t i;i<GRID_HEIGHT*GRID_WIDTH;i++){
        bool value= chip8->display[i];
          if(value){
        setColor(renderer,&display_color);
        SDL_RenderFillRect(renderer,&grids[i]);
         if(SHOW_GRIDLINES){
            setColor(renderer,&black);
            SDL_RenderDrawRect(renderer,&grids[i]);
        }
    }else{
        setColor(renderer,&black);
        SDL_RenderFillRect(renderer,&grids[i]);
        if(SHOW_GRIDLINES){
            setColor(renderer,&black);
            SDL_RenderDrawRect(renderer,&grids[i]);
        }
    }
    }
    loop(chip8);
   
    grids.clear();
}