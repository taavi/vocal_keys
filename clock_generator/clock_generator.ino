void setup() {
  // First...
  GTCCR = 0xE0;
  
  TCCR0A = 0;
  TCCR0B = 0;

  // TCCR1 = CTC1 : PWM1A : COM1A[1:0] : CS1[3:0]
  // CTC1: count to OCR1C, or just keep counting = 1
  // PWM1A: PWM mode for OCR1A with counter to 0 after OCR1C = 1
  // COM1A: PWM mode = 0b10
  // CS1[3:0] = 0b0110 = 6 = CLK/32 => ~2MHz
  // TCCR1 = 0xE6
  TCCR1 = 0xE6;
  
  // GTCCR = TSM : PWM1B : COM1B[1:0] : FOC1B : FOC1A : PSR1 : PSR0
  // TSM: set to halt timers so you can set them up, clear to commence counting
  // PWM1B: PWM mode = 1
  // COM1B: PWM mode = 0b10
  // FOC1B = 0
  // FOC1A = 0
  // PSR1 = just write 0
  // PSR0 = just write 0
  // GTCCR = 0xE0 to start, then 0x60 to start on our merry way
  
  // TCNT1 = the counter
  // OCR1A = PWM data!
  // OCR1B = PWM data!
  // OCR1C = 255
  OCR1C = 110; // @ 2MHz that means 9986Hz is the PWM frequency
  OCR1A = 55; // 50% duty

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

  // Last...
  GTCCR = 0x60;

  // REMEMBER TO SET THE OUTPUT PIN DIRECTION
  // DDRB x x 5 4 3 2 1 0
  //            0     1
  DDRB = 0x02;
}

void loop() {
  // Nothing to do here. Just let Timer/Counter1 do its work.
}
