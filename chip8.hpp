#ifndef CHIP8
#define CHIP8

#include <string>
#include <cstdint>
#include <cstdbool>
#include <cstddef>
#include <cstring>
#include "audio.hpp"


#ifdef DEBUG
#define __d(code) do { [&]() code (); } while (0)
#else
#define __d(code) do { } while (0)
#endif


typedef struct {
    uint16_t opcode;
    uint16_t NNN; //12 bit address /constant
    uint8_t NN;  // 8 bit constant
    uint8_t N;  //4 bit constant
    uint8_t X; // 4 bit register identifier
    uint8_t Y;// 4 bit register identifier

} instruction_t;

typedef enum{
    QUIT,
    RUNNING,
    PAUSED,
} EmulatorState;

class Chip8{
    private:
        uint8_t ram[4096]={0};
       
        char* rom_name;  //currently running rom;
        size_t rom_size;
        uint16_t entry_point;
        uint16_t stack[12]={0};
        uint16_t* stack_ptr;
        instruction_t instruction;
        uint8_t V[16];
        uint16_t I;
        uint16_t PC;
        Audio* audio;
      
        void read_rom();
        void show_rom_data();
        void execute_instruction();
        void update_timers();

    public:
        EmulatorState state;
        bool keypad[16];
        bool display[64*32];
        char* get_name();
        uint8_t delay_timer; //Decrement 60hz when >0
        uint8_t sound_timer; // Dcrements 60hz when >0
        void run();
        void reset();
        Chip8(char* romname);
        ~Chip8();
};

#endif