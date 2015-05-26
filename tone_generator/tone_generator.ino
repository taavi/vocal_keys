#define FREQ_C2  130.8127826502992
#define FREQ_Db2 138.59131548843592
#define FREQ_D2  146.83238395870364
#define FREQ_Eb2 155.56349186104035
#define FREQ_E2  164.81377845643485
#define FREQ_F2  174.61411571650183
#define FREQ_Gb2 184.9972113558171
#define FREQ_G2  195.99771799087452
#define FREQ_Ab2 207.65234878997245
#define FREQ_A3  219.9999999999999
#define FREQ_Bb3 233.08188075904488
#define FREQ_B3  246.94165062806198
#define FREQ_C3  261.6255653005985
#define FREQ_Db3 277.182630976872
#define FREQ_D3  293.66476791740746
#define FREQ_Eb3 311.1269837220808
#define FREQ_E3  329.62755691286986
#define FREQ_F3  349.2282314330038
#define FREQ_Gb3 369.99442271163434
#define FREQ_G3  391.99543598174927
#define FREQ_Ab3 415.3046975799451
#define FREQ_A4  440.0
#define FREQ_Bb4 466.1637615180899
#define FREQ_B4  493.8833012561241
#define FREQ_C4  523.2511306011974

#define SAMPLE_RATE 10000.0


//#define NOTE_C2 428
//#define NOTE_Db2 454
//#define NOTE_D2 481
//#define NOTE_Eb2 509
//#define NOTE_E2 540
//#define NOTE_F2 572
//#define NOTE_Gb2 606
//#define NOTE_G2 642
//#define NOTE_Ab2 680
//#define NOTE_A3 720
//#define NOTE_Bb3 763
//#define NOTE_B3 809
//#define NOTE_C3 857
//#define NOTE_Db3 908
//#define NOTE_D3 962
//#define NOTE_Eb3 1019
//#define NOTE_E3 1080
//#define NOTE_F3 1144
//#define NOTE_Gb3 1212
//#define NOTE_G3 1284
//#define NOTE_Ab3 1360
//#define NOTE_A4 1441
//#define NOTE_Bb4 1527
//#define NOTE_B4 1618
//#define NOTE_C4 1714

const unsigned char sine[] = {
    129, 135, 141, 147, 153, 160, 166, 172, 177, 183, 189, 194, 200, 205, 210, 214,
    219, 223, 227, 231, 235, 238, 241, 244, 247, 249, 251, 253, 254, 255, 255, 255,
    255, 255, 255, 255, 254, 253, 251, 249, 247, 244, 241, 238, 235, 231, 227, 223,
    219, 214, 210, 205, 200, 194, 189, 183, 177, 172, 166, 160, 153, 147, 141, 135,
    129, 122, 116, 110, 104,  97,  91,  85,  80,  74,  68,  63,  57,  52,  47,  43,
     38,  34,  30,  26,  22,  19,  16,  13,  10,   8,   6,   4,   3,   2,   1,   1,
      1,   1,   1,   2,   3,   4,   6,   8,  10,  13,  16,  19,  22,  26,  30,  34,
     38,  43,  47,  52,  57,  63,  68,  74,  80,  85,  91,  97, 104, 110, 116, 122
};

const unsigned int sine_len = sizeof(sine) / sizeof(sine[0]);

#define SINE_SAMPLES 128
//#define SINE_SAMPLES sizeof(sine)
//  (sizeof(sine) / sizeof(sine[0]))
#define PHASE_TO_SAMPLE_OFFSET_SHIFT 9
#if (SINE_SAMPLES << PHASE_TO_SAMPLE_OFFSET_SHIFT) != 65536L
    #error PHASE_TO_SAMPLE_OFFSET_SHIFT and SINE_SAMPLES don not agree! 
#endif

#define PHASE_PER_TIC_FOR_FREQ(freq) ((unsigned int)(65536.0 * freq / SAMPLE_RATE))

volatile unsigned char outA;
volatile unsigned char outB;
volatile bool do_next_sample;

void MyISR() {
    OCR1A = outA;
    OCR1B = outB;
    do_next_sample = true;
}

void setup() {
  // put your setup code here, to run once:
  do_next_sample = false;
  
  // First...
  GTCCR = 0xE0;
  
  TCCR0A = 0;
  TCCR0B = 0;

  // TCCR1 = CTC1 : PWM1A : COM1A[1:0] : CS1[3:0]
  // CTC1: count to OCR1C, or just keep counting = 1
  // PWM1A: PWM mode for OCR1A with counter to 0 after OCR1C = 1
  // COM1A: PWM mode = 0b10
  // CS1[3:0] = 0001
  // TCCR1 = 0xE1
  TCCR1 = 0xE1;
  
  // GTCCR = TSM : PWM1B : COM1B[1:0] : FOC1B : FOC1A : PSR1 : PSR0
  // TSM: set to halt timers so you can set them up, clear to commence counting
  // PWM1B: PWM mode = 1
  // COM1B: PSM mode = 0b10
  // FOC1B = 0
  // FOC1A = 0
  // PSR1 = just write 0
  // PSR0 = just write 0
  // GTCCR = 0xE0 to start, then 0x60 to start on our merry way
  
  // TCNT1 = the counter
  // OCR1A = PWM data!
  // OCR1B = PWM data!
  // OCR1C = 255
  OCR1C = 0xFF;

  // TIMSK = RES : OCIE1A : OCIE1B : OCIE0A : OCIE0B : TOIE1 : TOIE0 : RES
  // 0CIE1[AB]: Interrupt enable = 0
  // TOIE1: Overflow intererupt enable = 0
  // TIMSK = 0
  TIMSK = 0x00;
  
  // TIFR = RES : OCF1A : OCF1B : OCF0A : OCF0B : TOV1 : TOV0 : RES
  // Interrupt flags. Ignore.
  
  // PLLCSR = LSM : RES[6:3] : PCKE : PLLE : PLOCK
  // LSM: Low speed mode = 0
  // PCKE: Peripheral clock enable = 1 *IFF* PLLE && PLOCK
  // PLLE: PLL Enable = 1
  // PLOCK: READ-ONLY, wait for set before PCKE
  PLLCSR = 0x02;
  while (!(PLLCSR & 1)); // KILL TIME
  PLLCSR = 0x06;

  TCCR0A = 0;
  TCCR0B = 0;

  // Last...
  GTCCR = 0x60;

  // REMEMBER TO SET THE OUTPUT PIN DIRECTION
  // DDRB x x 5 4 3 2 1 0
  //            1     1
  DDRB = 0x12;

  attachInterrupt(0, MyISR, FALLING);
}

inline unsigned char sineAtPhase(unsigned int phase) {
  // Performs linear interpolation when phase is between samples.
  // It's not phase correct, but who cares?
  // e.g. if phase were 8-bit, with 4 bits of sample:
  // phase 0b 0000 00xx => use sample 0b0000
  // phase 0b 0000 01xx => use sample 0b0000
  // phase 0b 0000 10xx => use half of sample 0b0000 and half of 0b0001
  // phase 0b 0000 11xx => use half of sample 0b0000 and half of 0b0001
  // phase 0b 0001 00xx => use sample 0b0001
  // etc
  unsigned char offset = phase >> 9;
//  if (phase & 0x0100) {
//    return sine[offset] >> 1 + sine[(offset + 1) & 0x7F] >> 1;
//  } else
  {
    return sine[offset];
  }
}

void loop() {
  unsigned int phaseA = 0;
  unsigned int phaseB = 0;
  while (true) {
    while (!do_next_sample) {
      __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t");
      __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t");
    }

    phaseA += PHASE_PER_TIC_FOR_FREQ(FREQ_C2);
    outA = sineAtPhase(phaseA);

    phaseB += PHASE_PER_TIC_FOR_FREQ(FREQ_G3);
    outB = sineAtPhase(phaseB);

    do_next_sample = false;
  }
}



