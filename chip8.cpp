#include "chip8.hpp"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <random>
#include <cstdio>



uint8_t rand_n(uint8_t n){
    std::random_device rd;
    std::mt19937 gen(rd());  //Mersenne Twister engine

    std::uniform_int_distribution<> dis(0,n);

    return dis(gen);
}

Chip8::Chip8(char* romname){

     audio = new Audio();

     if(!audio->is_initialized()){
        std::cerr <<"Failed to initialize audio"<<std::endl;
        SDL_Quit();
     }
    rom_name=romname;
    __d({
    std::cout<<"loading rom:"<<rom_name<<std::endl;
    });
    const uint32_t entry_point=0x200; //all rom data will start here

    const uint8_t font[]={
0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
0x20, 0x60, 0x20, 0x20, 0x70, // 1
0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
0x90, 0x90, 0xF0, 0x10, 0x10, // 4
0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
0xF0, 0x10, 0x20, 0x40, 0x40, // 7
0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
0xF0, 0x90, 0xF0, 0x90, 0x90, // A
0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
0xF0, 0x80, 0x80, 0x80, 0xF0, // C
0xE0, 0x90, 0x90, 0x90, 0xE0, // D
0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};
    state= EmulatorState::RUNNING;

    PC=entry_point;
    memcpy(&ram[0],font, sizeof(font));
    read_rom();
    stack_ptr=&stack[0];
  
 
}

Chip8::~Chip8(){
 

}


char* Chip8::get_name(){
    const char* app_name="SPEmu Chip8 -";
    size_t appNameLength = std::strlen(app_name);
    size_t romNameLength = std::strlen(rom_name);

    char* result = new char[appNameLength+romNameLength+1];
    std::strcpy(result,app_name);
    std::strcat(result,rom_name);
    return result;
}



void Chip8::read_rom(){
    uint32_t entry_point=0x200;
    std::fstream file;
    file.open(rom_name,std::ios::in|std::ios::binary);
    if(!file.is_open()){
        std::cerr<<"unable to open file"<<std::endl;
        return;
    }

    file.seekg(0,std::ios::end);
    std::streamsize size = file.tellg();
    file.seekg(0,std::ios::beg);
    rom_size=size;
    size_t buffer_size = sizeof(ram)-entry_point;

     if(rom_size>buffer_size){
        std::cout<<"can't read the rom it's too big"<<std::endl;
        file.close();
        return;
     }
    
    file.read(reinterpret_cast<char*>(&ram[entry_point]),rom_size);
    file.close();
}



void Chip8::show_rom_data(){
    std::cout<<"Rom size:"<<rom_size<<std::endl;
    for(uint16_t i=0;i<rom_size;i++){
        std::cout<<std::hex<<(int)ram[PC+i]<<std::endl;
    }
}

#define INSTRUCTION_PER_SECOND  500

void Chip8::run(){
  
  for(uint32_t i=0;i<INSTRUCTION_PER_SECOND/60;i++){
  execute_instruction();
  }
   update_timers();
}


void Chip8::execute_instruction(){
   instruction.opcode = (ram[PC]<<8)|(ram[PC+1]);
   uint16_t inst=instruction.opcode;
   instruction.NNN = inst&0x0FFF;
   instruction.NN =  inst&0x0FF;
   instruction.N =  inst&0x0F;
   instruction.X = (inst>>8)&0x0F;
   instruction.Y = (inst>>4)&0x00F;
 

 __d({
      printf("Address: %04x, Opcode:%04x,Desc:",PC,instruction.opcode);
   });
   PC+=2;
  
    switch (instruction.opcode>>12&0x0F)
    {
    case 0x00:
        if(instruction.NN==0xe0){
           __d({
             std::cout<<"Clear the screen"<<std::endl;
           });

            memset(display,0,sizeof(display));

        }else if(instruction.NN==0xee){
           
            uint16_t address=*--stack_ptr;
            PC=address;
            __d({
            std::cout<<"Return from subroutine to address:"<<std::hex<<PC<<std::endl;
            });
        } else{
            //Unimplemented /inavalid opcode..
        }
        break;
    case 0x02:
      __d({
        std::cout<<"Push return address " <<PC<<" to the stack and jump to the "<<std::hex<<instruction.NNN<<std::endl;
         });
        *stack_ptr++ = PC;
        
        PC=instruction.NNN;
        break;
    
    case 0x0A:
        __d({
        std::cout<<"set index register to I address:"<<std::hex<<(int)instruction.NNN<<std::endl;
        });
        I=instruction.NNN;
        break;

    case 0x06:
        __d({
        std::cout<<"Sets V"<<std::hex<<(int)instruction.X<<" to:"<<std::hex<<(int)instruction.NN<<std::endl;
        });
        V[instruction.X]=instruction.NN;
        break;

    case 0x07:
       __d({
        std::cout<<"Set (Vx+=NN) V"<<instruction.X<<"+="<<std::hex<<(int)instruction.NN<<std::endl;
         });

        V[instruction.X]+=instruction.NN;
        break;

    case 0x0D:{
        //DXYN: Draw N height sprite at coords X,Y : Read from memory location I:
        //Screen pixels are XOR'd with spirte bits.
        // VF (Carry flag) is set if any screen pixels are set off: This is useful 
        // for collision detection  or other reasons

        __d({
            printf("\n");
        });
         uint8_t X = V[instruction.X]&63;
         uint8_t Y = V[instruction.Y]&31;
         const uint8_t org_X = X;
        V[0xF] =0; // Init carry flag to 0

        for(uint8_t i = 0; i < instruction.N; i++) {
    const uint8_t sprite_data = ram[I + i];
    X = org_X;
    
    for(int8_t j = 7; j >= 0; j--) {
        // Calculate screen pixel position
        uint16_t pos = X + Y * 64;
        
        if(sprite_data & (1 << j)) {
            // Collision detection
            if(display[pos]) {
                V[0xF] = 1; // Set carry flag if pixel was already on
            }
            display[pos] ^= 1; // XOR pixel with sprite bit
        }

        X++;
        if(X >= 64) break; // Ensure X does not exceed screen width
    }
    
    Y++;
    if(Y >= 32) break; // Ensure Y does not exceed screen height
}

         break;
    }

    case 0x01:
        //0x1NNN: jump to address NNN
         __d({
            printf("\n");
        });
        PC=instruction.NNN;
        break;

    case 0x03:
        //0x3xNN check if VX ==NN if so skip the next instruction
         __d({
            printf("\n");
        });
        if(V[instruction.X]==instruction.NN){
            PC+=2;
        }

        break;

    case 0x04:
        //0x4xNN check if VX !=NN if so skip the next instruction
         __d({
            printf("\n");
        });

        if(V[instruction.X]!=instruction.NN){
            PC+=2;
        }
        break;

    case 0x05:
         __d({
            printf("\n");
        });
        if(instruction.N !=0) break;
        //0x4xNN check if VX ==VY if so skip the next instruction
        if(V[instruction.X]==V[instruction.Y]){
            PC+=2;
        }
        break;

    case 0x08:{
         __d({
            printf("\n");
        });
        switch (instruction.N)
        {
        case 0:
             V[instruction.X] =V[instruction.Y];
            break;
        case 1:
            V[instruction.X] |=V[instruction.Y];
            break;
        
        case 2:
            V[instruction.X] &=V[instruction.Y];
            break;

        case 3:
            V[instruction.X] ^=V[instruction.Y];
            break;
        case 4:
            if((uint16_t)(V[instruction.X]+V[instruction.Y])>255){
                V[0xF]=1;
            }else{
                V[0xF]=0;
            }
            V[instruction.X] +=V[instruction.Y];
            break;
        
        case 5:
            V[0xF]=V[instruction.Y]<=V[instruction.X];
            
            V[instruction.X] -= V[instruction.Y];
            break;

        case 6:
            V[0xF]= V[instruction.X]&1;
            V[instruction.X]>>=1;
            break;

        case 7:
            V[0xF]=V[instruction.X]<=V[instruction.Y];
            V[instruction.X] =V[instruction.Y]-V[instruction.X];
            break;

        case 0xE:
            V[0xF]= (V[instruction.X]>>7)&1;
            V[instruction.X] <<=1;
            break;
        
        default:
            // Wrong or unimplemented.
            break;
        }

       
        break;
    }

     case 0x9:
       __d({
        printf("\n");
        });
        if(V[instruction.X]!=V[instruction.Y]){
            PC+=2;
        }
        break;

    case 0xb:
        //BNNN jump to address xxx+ register v0
         __d({
            printf("\n");
        });
        PC=instruction.NNN+V[0];
        break;
    case 0xc:
        //Vx = random number less than or equal to nn
         __d({
            printf("\n");
        });
        V[instruction.X] = rand_n(instruction.NN);
        break;

    case 0x0E:
         __d({
            printf("\n");
        });
        if(instruction.NN==0x9E){
            //0x0E9E :Skip next instruction if ke in VX is pressed
            if(keypad[V[instruction.X]]){
                PC+=2;
            }
        }else if(instruction.NN==0xA1){
            //0x0EA1: Skip next instruction if key in VX is not pressed
            if(!keypad[V[instruction.X]]){
                PC+=2;
            }

        }
        break;
 
    case 0xF:{
         __d({
            printf("\n");
        });
        switch (instruction.NN)
        {


        
        case 0x0A:{
            //0xFX0A A key press is awaited an then stored in VX(blocking operation, all instruction halted uintil next key event)
            bool key_pressed=false;
            for(uint8_t key=0;key<sizeof(keypad);key++){
                if(keypad[key]){
                    V[instruction.X]=key;
                }
            }
            if(key_pressed==false){
                std::cout<<"NO input yet"<<std::endl;
                PC-=2;
            }
             break;
        }

    
        case 0x1E:
            //Adds VX to I. VF is not affected
            I+=V[instruction.X];
            break;

        case 0x07:
            V[instruction.X] = delay_timer;
            break;

        case 0x15:
            delay_timer = V[instruction.X];
            break;

        case 0x18:
            sound_timer = V[instruction.X];
            break;

        case 0x29:
            I=V[instruction.X] * 5;
            break;

        case 0x33:{
            uint8_t bcd = V[instruction.X];
            ram[I+2] = bcd%10;
            bcd /=10;
            ram[I+1] = bcd%10;
            bcd/=10;
            ram[I] = bcd;
            break;
        }

        case 0x55: {
            // 0xFX55: Store V0 to VX in memory starting at address I.
            // CHIP-8 increments I by the number of registers stored.
            for (uint8_t reg = 0; reg <= instruction.X; reg++) {
                ram[I + reg] = V[reg];
            }
            //I += (instruction.X + 1); // Increment I if using CHIP-8 standard.
            break;
        }

        case 0x65: {
            // 0xFX65: Read V0 to VX from memory starting at address I.
            for (uint8_t reg = 0; reg <= instruction.X; reg++) {
                V[reg] = ram[I + reg];
            }
            //I += (instruction.X + 1); // Increment I if using CHIP-8 standard.
            break;
        }
        
        default:
            break;
        }

        break;
    }

    default:
       std::cout<<"unknown opcode:"<<std::hex<<(int)instruction.opcode<<std::endl;
        break;
    }
   }


void Chip8::update_timers(){
    if(delay_timer>0) delay_timer--;
    if(sound_timer>0){
         audio->play_tit_sound();
        sound_timer--;
        }
}

void Chip8::reset(){
    memset(display,0,sizeof(display));
    PC=0x200;
}
