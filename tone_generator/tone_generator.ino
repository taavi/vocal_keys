#define NOTE_C2 428
#define NOTE_Db2 454
#define NOTE_D2 481
#define NOTE_Eb2 509
#define NOTE_E2 540
#define NOTE_F2 572
#define NOTE_Gb2 606
#define NOTE_G2 642
#define NOTE_Ab2 680
#define NOTE_A3 720
#define NOTE_Bb3 763
#define NOTE_B3 809
#define NOTE_C3 857
#define NOTE_Db3 908
#define NOTE_D3 962
#define NOTE_Eb3 1019
#define NOTE_E3 1080
#define NOTE_F3 1144
#define NOTE_Gb3 1212
#define NOTE_G3 1284
#define NOTE_Ab3 1360
#define NOTE_A4 1441
#define NOTE_Bb4 1527
#define NOTE_B4 1618
#define NOTE_C4 1714

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


unsigned int phaseA;
unsigned int phaseB;

volatile unsigned char outA;
volatile unsigned char outB;
volatile int do_next_sample;

void MyISR() {
    OCR1A = outA;
    OCR1B = outB;
    do_next_sample = true;
}

void setup() {
  // put your setup code here, to run once:
  phaseA = 0;
  phaseB = 0;
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


void loop() {
  while (!do_next_sample);
  phaseA += NOTE_C2;
  phaseB += NOTE_G3;
  outA = sine[phaseA >> 9];
  outB = sine[phaseB >> 9];
  do_next_sample = false;
}



