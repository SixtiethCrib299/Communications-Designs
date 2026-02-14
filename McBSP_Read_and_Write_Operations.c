/*************************************************************/
/* Program: dskstart32.c                                     */
/*                                                           */
/* This program can be used as a starting point for writing  */
/* DSK6713 applications. It contains the code necessary to   */
/* initialize the DSK board, TMS320C6713, and AIC_23 codec.  */
/* When CCS is commanded to run your program, it first uses  */
/* the file DSK6713.ccxml in your Project Explorer window to */
/* define a memory map, set some CPLD registers, and         */
/* initialize the EMIF for the memory on the C6713DSK.       */
/*                                                           */
/* The program dskstart32.c continues the initialization. It */
/* uses functions from a UMD modification of TI's TMS320C6713*/
/* DSK Board Support Library (BSL). You can find detailed    */
/* documentation for the BSL by using Windows Explorer to    */
/* navigate to C:\c6713dsk\docs\hlp\c6713dsk.hlp under the   */
/* heading "Software." The modified library, its header      */
/* files, and sources are in the directories:                */
/*    C:\c6713dsk\dsk6713bsl32\lib                           */
/*    C:\c6713dsk\dsk6713bsl32\include                       */
/*    C:\c6713dsk\dsk6713bsl32\sources\dsk6713bsl.zip.       */
/*                                                           */
/* The program first initializes the board support library   */
/* by calling DSK6713_init(). This initializes the chip's    */
/* PLL, configures the EMIF based on the DSK version, and    */
/* sets the CPLD registers to a default state. The source    */
/* code is in the BSL file dsk6713.c.                        */
/*                                                           */
/* Next dskstart32.c initializes the interrupt controller    */
/* registers and installs the default interrupt service      */
/* routines by calling the function intr_reset() in the UMD  */
/* added file DSKintr.c. It clears GIE and PGIE, disables all*/
/* interrupts except RESET in IER, clears the flags in the   */
/* IFR for the the maskable interrupts INT4 - INT15, resets  */
/* the interrupt multiplexers, initializes the interrupt     */
/* service table pointer (ISTP), and sets up the Interrupt   */
/* Service Routine Jump Table. The object modules DSKintr.obj*/
/* and intvecs.obj were added to BSL library dsk6713bsl.lib, */
/* so do not include DSKintr.c and intvecs.asm in your       */
/* project. Some functions included in DSKintr.c are:        */
/*    intr_reset()         Reset interrupt regs to defaults  */
/*    intr_init()          Initialize Interrupt Service      */
/*                                  Table Pointer            */
/*    ints_isn()           Assign ISN to CPU interrupt       */
/*    intr_get_cpu_intr()  Return CPU int. assigned to ISN   */
/*    intr_map()           Place ISN in int. mux. register   */
/*    intr_hook()          Hook ISR to interrupt             */
/* A set of macro functions for setting and clearing bits    */
/* in the IER and IFR are available.  See DSKintr.c and      */
/* DSKintr.h for complete documentation. Some functions from */
/* the CSL interrupt API can also be used. The "interrupt"   */
/* keyword causes the C compiler to generate entrance and    */
/* exit code to an interrupt service routine which saves and */
/* restores registers used by the ISR.  DSP/BIOS handles     */
/* interrupts differently and does not allow use of the      */
/* interrupt key-word.                                       */
/*                                                           */
/* The codec is then started by calling the function         */
/* DSK6713_AIC23_openCodec(). This function configures       */
/* serial port McBSP0 to act as a unidirectional control     */
/* channel in the SPI mode transmitting 16-bit words. It     */
/* and then configures the AIC23 stereo codec to operate in  */
/* the DSP mode with 16-bit data words. The default sampling */
/* rate is 48 kHz. Next McBSP1 is configured to send data    */
/* samples to the codec or receive data samples from the     */
/* codec in the DSP format using 32-bit words. The first word*/
/* transmitted by the AIC23 is the left channel sample.      */
/* The right channel sample is transmitted immediately after */
/* the left sample. The AIC23 generates a single frame sync  */
/* at the beginning of the left channel sample. Therefore, a */
/* 32-bit word received by McBSP1 contains the left sample   */
/* in the upper 16 bits and the right sample in the lower 16 */
/* bits. The 16-bit samples are in 2's complement format.    */
/* Words transmitted from McBSP1 to AIC23 must have the same */
/* format. The codec and McBSP1 are configured so that the   */
/* codec generates the frame syncs and shift clocks.         */
/*                                                           */
/* Function DSK6713_AIC23_openCodec() in dsk6713_opencodec.c */
/* is a modification of the same function in the BSL module  */
/* DSK6713_AIC23_openCodec.c. It configures McBSP1 to trans- */
/* mit and receive 32-bit words rather than 16-bit words by  */
/* changing the RWDLEN1 value to 32BIT, XWDLEN1 to 32BIT,    */
/* RFRLEN1 to OF(0), and XFRLEN1 to OF(0) in structure       */
/* mcbspCfgData in dsk6713_opencodec.c. This causes McBSP1   */
/* to use a single phase frame consisting of one 32-bit word */
/* per frame. Words are sent to the codec by using the BSL   */
/* function DSK6713_AIC23_write(). This function polls the   */
/* XRDY flag and returns immediately without sending the     */
/* sample if it is false and also returns the value 0. It    */
/* sends the sample if XRDY is 1 (TRUE) and returns the value*/
/* 1.  Words are read from the codec by using the function   */
/* DSK6713_AIC23_read(). It polls the RRDY flag and returns  */
/* immediately if it is FALSE without reading a word and also*/
/* returns the value FALSE. If RRDY is TRUE, it reads a word */
/* and returns the value TRUE.                               */
/*                                                           */
/* The codec can be set to the sampling rates shown in the   */
/* table below by using the function                         */
/*    DSK6713_AIC23_setFreq(handle, freq ID)                 */
/*                                                           */
/*      freq ID                Value  Frequency              */
/*   DSK6713_AIC23_FREQ_8KHZ,  0x06,    8000 Hz              */
/*   DSK6713_AIC23_FREQ_16KHZ, 0x2c,   16000 Hz              */
/*   DSK6713_AIC23_FREQ_24KHZ, 0x20,   24000 Hz              */
/*   DSK6713_AIC23_FREQ_32KHZ, 0x0c,   32000 Hz              */
/*   DSK6713_AIC23_FREQ_44KHZ, 0x11,   44100 Hz              */
/*   DSK6713_AIC23_FREQ_48KHZ, 0x00,   48000 Hz              */
/*   DSK6713_AIC23_FREQ_96KHZ, 0x0e,   96000 Hz              */
/*                                                           */
/*                                                           */
/* Finally, dskstart.c enters an infinite loop that reads a  */
/* pair of samples from the codec A/D and loops it back out  */
/* to the codec D/A. This loop should be replaced by the C   */
/* code to achieve the goals of your task.                   */
/*************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <dsk6713.h>
#include <dsk6713_aic23.h>
#include <DSKintr.h>

#include <math.h> 

/* Codec configuration settings */
/* See dsk6713_aic23.h and the TLV320AIC23 Stereo Audio CODEC Data Manual */
/* for a detailed description of the bits in each of the 10 AIC23 control */
/* registers in the following configuration structure.                    */

DSK6713_AIC23_Config config = { \
    0x0017,  /* 0 DSK6713_AIC23_LEFTINVOL  Left line input channel volume */ \
    0x0017,  /* 1 DSK6713_AIC23_RIGHTINVOL Right line input channel volume */\
    0x00d8,  /* 2 DSK6713_AIC23_LEFTHPVOL  Left channel headphone volume */  \
    0x00d8,  /* 3 DSK6713_AIC23_RIGHTHPVOL Right channel headphone volume */ \
    0x0011,  /* 4 DSK6713_AIC23_ANAPATH    Analog audio path control */      \
    0x0000,  /* 5 DSK6713_AIC23_DIGPATH    Digital audio path control */     \
    0x0000,  /* 6 DSK6713_AIC23_POWERDOWN  Power down control */             \
    0x0043,  /* 7 DSK6713_AIC23_DIGIF      Digital audio interface format */ \
    0x0081,  /* 8 DSK6713_AIC23_SAMPLERATE Sample rate control (48 kHz) */   \
    0x0001   /* 9 DSK6713_AIC23_DIGACT     Digital interface activation */   \
};


void main(void){

  DSK6713_AIC23_CodecHandle hCodec;
  Uint32 sample_pair = 0;

  /* Initialize the interrupt system */
  intr_reset();

  /* dsk6713_init() must be called before other BSL functions */
  DSK6713_init(); /* In the BSL library */

  /* Start the codec */
  hCodec = DSK6713_AIC23_openCodec(0, &config);

  /* Change the sampling rate to 16 kHz */
  DSK6713_AIC23_setFreq(hCodec, DSK6713_AIC23_FREQ_16KHZ);

  /* Read left and right channel samples from the ADC and loop */
  /* them back out to the DAC.                                 */

  int n = 0;
  //int fo;
  float PI = 3.1415926536;
  float fs = 16000;
  float absolute_phase1=0;
  float fo1 = 1000;
  float fo2 = 2000;
  float delta_theta1 = (2*PI*fo1)/fs;
  float absolute_phase2 = 0;
  float delta_theta2 = (2*PI*fo2)/fs;
  
  double s_1=0;
  double s_2=0;

  for(;;){  // Infinite Loop for Polling
    
    
  //while(!DSK6713_AIC23_read(hCodec, &sample_pair));     /* <-- To read in a signal to the board */

  /* Generating 1kHz sine wave */
	absolute_phase1 += delta_theta1;
	float val1 = 15000 * sin(absolute_phase1);
	Uint16 s_1_16 = (Uint16)val1;
	sample_pair= s_1_16;

  /* Resetting the phase after every period to avoid a constantly growing variable, which is costly in memory */
	if(absolute_phase1 > 2*PI){
		absolute_phase1 -= 2*PI;
	}

  /* Same process as above, but for a 2kHz sine wave */
	absolute_phase2 += delta_theta2;
	float val2 = 15000 * sin(absolute_phase2);
	Uint16 s_2_16 = (Uint16)val2;


	if (absolute_phase2 > 2*PI) {
		absolute_phase2 -= 2*PI;
	}

  /* Both sine waves were type-casted as 16-bit signed integers above. 
   * Now, we bit-shift the first sine wave, and concatenate the second
   * sine wave to create a word that is a 32-bit unsigned integer. 
   * This is what can be output from the board's DAC, and both signals 
   * will be outputted (first in left channel, and second in right channel).
  */
    
  int32_t s_1_32 = (int32_t)s_1_16;
	s_1_32 = (s_1_16 << 16) | 0;

	sample_pair = s_1_32 + s_2_16;   // Combining the two functions into one word

	while(!DSK6713_AIC23_write(hCodec, sample_pair)); // Writing/Outputting the waveforms 

  }
}

