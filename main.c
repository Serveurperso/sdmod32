#pragma config UPLLEN   = OFF      // USB PLL Enabled
//#pragma config FPLLMUL  = MUL_20   // PLL Multiplier
#pragma config FPLLMUL  = MUL_24   // PLL Multiplier
#pragma config UPLLIDIV = DIV_2    // USB PLL Input Divider
#pragma config FPLLIDIV = DIV_2    // PLL Input Divider
#pragma config FPLLODIV = DIV_1    // PLL Output Divider
#pragma config FPBDIV   = DIV_1    // Peripheral Clock divisor
#pragma config FWDTEN   = OFF      // Watchdog Timer
#pragma config WDTPS    = PS1      // Watchdog Timer Postscale
#pragma config FCKSM    = CSDCMD   // Clock Switching & Fail Safe Clock Monitor
#pragma config OSCIOFNC = OFF      // CLKO Enable
#pragma config POSCMOD  = HS       // Primary Oscillator
#pragma config IESO     = ON       // Internal/External Switch-over
#pragma config FSOSCEN  = OFF      // Secondary Oscillator Enable (KLO was off)
#pragma config FNOSC    = PRIPLL   // Oscillator Selection
#pragma config CP       = OFF      // Code Protect
#pragma config BWP      = OFF      // Boot Flash Write Protect
#pragma config PWP      = OFF      // Program Flash Write Protect
#pragma config ICESEL   = ICS_PGx2 // ICE/ICD Comm Channel Select
#pragma config DEBUG    = OFF      // Background Debugger Enable

#include <stdbool.h>
#include <stdint.h>
#include <plib.h>

#include "mod32.h"

#include ".\fatfs\ff.h"

FATFS fso;        // The FATFS structure (file system object) holds dynamic work area of individual logical drives
DIR dir;          // The DIR structure is used for the work area to read a directory by f_oepndir, f_readdir function
FILINFO fileInfo; // The FILINFO structure holds a file information returned by f_stat and f_readdir function
FIL file;         // The FIL structure (file object) holds state of an open file

int main() {
 uint16_t i = 0;

 SYSTEMConfigPerformance(80000000L);
 INTEnableSystemMultiVectoredInt();

 // Stereo PWM
 OpenTimer2(T2_ON | T2_PS_1_1, 1 << BITDEPTH);
 OpenOC1(OC_ON | OC_TIMER2_SRC | OC_PWM_FAULT_PIN_DISABLE, 0, 0);
 OpenOC2(OC_ON | OC_TIMER2_SRC | OC_PWM_FAULT_PIN_DISABLE, 0, 0);

 // Sampler
 ConfigIntTimer3(T3_INT_ON | T3_INT_PRIOR_3);
 OpenTimer3(T3_ON | T3_PS_1_1, SYSCLK / SAMPLERATE);

 // Led outputs
 LEDTRIS = 0x0;

 while(disk_initialize(0));
 f_mount(0, &fso);
 f_chdir(PATH);
 f_opendir(&dir, ".");

 loadNextFile();

 for(;;) {
  while((SoundBuffer.writePos + 1 & SOUNDBUFFERSIZE - 1) != SoundBuffer.readPos) {
   if(!i) {

    // You can call
    //loadNextFile();
    //loadPreviousFile();

    // You can change playing speed and frequency
    //Player.amiga = AMIGA;                               // Default frequency
    //Player.samplesPerTick = SAMPLERATE / (2 * 125 / 5); // Default speed (Hz = 2 * BPM / 5)

    player();
    i = Player.samplesPerTick;
   }
   mixer();
   i--;
  }
 }

}

void __ISR(_TIMER_3_VECTOR, ipl3) T3InterruptHandler() {

 if(SoundBuffer.writePos != SoundBuffer.readPos) {
  OC1RS = SoundBuffer.left[SoundBuffer.readPos];
  OC2RS = SoundBuffer.right[SoundBuffer.readPos];
  SoundBuffer.readPos++;
  SoundBuffer.readPos &= SOUNDBUFFERSIZE - 1;
 }

 mT3ClearIntFlag();
}
