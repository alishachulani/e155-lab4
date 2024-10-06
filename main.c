// Code for E155 Lab 4 to play songs
// Written by Alisha Chulani, October 2024, achulani@g.hmc.edu



#include "/Users/alishachulani/Desktop/STM32L432KC_RCC.h" // provided to us from E155 Github, you will need to change path for this to work 

// Register define statements
#define RCC_BASE          (0x40021000)
#define RCC_AHB2ENR       (RCC_BASE + 0x4C)
#define RCC_APB2ENR       (RCC_BASE + 0x60)
#define TIM16_BASE        (0x40014400)
#define TIM16_CR1         (TIM16_BASE + 0x00)
#define TIM16_EGR         (TIM16_BASE + 0x14)
#define TIM16_CCMR1       (TIM16_BASE + 0x18)
#define TIM16_CCER        (TIM16_BASE + 0x20)
#define TIM16_PSC         (TIM16_BASE + 0x28)
#define TIM16_ARR         (TIM16_BASE + 0x2C)
#define TIM16_CCR1        (TIM16_BASE + 0x34)
#define TIM16_BDTR        (TIM16_BASE + 0x44)
#define GPIOA_BASE        (0x48000000)
#define GPIOA_MODER       (GPIOA_BASE + 0x00)
#define GPIOA_OSPEEDR	  (GPIOA_BASE + 0x08)
#define GPIOA_AFRL        (GPIOA_BASE + 0x20)


const int notes[][2] = {
{659,	125},
{623,	125},
{659,	125},
{623,	125},
{659,	125},
{494,	125},
{587,	125},
{523,	125},
{440,	250},
{  0,	125},
{262,	125},
{330,	125},
{440,	125},
{494,	250},
{  0,	125},
{330,	125},
{416,	125},
{494,	125},
{523,	250},
{  0,	125},
{330,	125},
{659,	125},
{623,	125},
{659,	125},
{623,	125},
{659,	125},
{494,	125},
{587,	125},
{523,	125},
{440,	250},
{  0,	125},
{262,	125},
{330,	125},
{440,	125},
{494,	250},
{  0,	125},
{330,	125},
{523,	125},
{494,	125},
{440,	250},
{  0,	125},
{494,	125},
{523,	125},
{587,	125},
{659,	375},
{392,	125},
{699,	125},
{659,	125},
{587,	375},
{349,	125},
{659,	125},
{587,	125},
{523,	375},
{330,	125},
{587,	125},
{523,	125},
{494,	250},
{  0,	125},
{330,	125},
{659,	125},
{  0,	250},
{659,	125},
{1319,	125},
{  0,	250},
{623,	125},
{659,	125},
{  0,	250},
{623,	125},
{659,	125},
{623,	125},
{659,	125},
{623,	125},
{659,	125},
{494,	125},
{587,	125},
{523,	125},
{440,	250},
{  0,	125},
{262,	125},
{330,	125},
{440,	125},
{494,	250},
{  0,	125},
{330,	125},
{416,	125},
{494,	125},
{523,	250},
{  0,	125},
{330,	125},
{659,	125},
{623,	125},
{659,	125},
{623,	125},
{659,	125},
{494,	125},
{587,	125},
{523,	125},
{440,	250},
{  0,	125},
{262,	125},
{330,	125},
{440,	125},
{494,	250},
{  0,	125},
{330,	125},
{523,	125},
{494,	125},
{440,	500},
{  0,	0}};


const int twinkle[][2] = {

    {262, 500},  // C4 (Twin)
    {262, 500},  // C4 (kle)
    {392, 500},  // G4 (twin)
    {392, 500},  // G4 (kle)
    {440, 500},  // A4 (lit)
    {440, 500},  // A4 (tle)
    {392, 1000},  // G4 (star)
    {349, 500},  // F4 (How)
    {349, 500},  // F4 (I)
    {330, 500},  // E4 (won)
    {330, 500},  // E4 (der)
    {294, 1000},  // D4 (what)
    {262, 500},  // C4 (you)
    {262, 500},  // C4 (are)
    
    {0, 0}       // End


};

// Delay for input ms - use a calculated number of nops (which corresponds to 8MHz internal clock speed) to delay an accurate amount of time
void delay(int ms) {
   while (ms-- > 0) {
      volatile int x=887;
      while (x-- > 0)
         __asm("nop");
   }
}

// Initialize GPIOA6 for the purpose of outputting PWM signal from TIM16
  void initializeGPIO() {
  // Enable clock to GPIOA
  volatile uint32_t *RCC_AHB2ENR_PTR = (volatile uint32_t *)(RCC_AHB2ENR);
  *RCC_AHB2ENR_PTR |= (1 << 0);

  // Set A6 to Alternate Function 14
  volatile uint32_t *GPIOA_AFRL_PTR = (volatile uint32_t *)(GPIOA_AFRL);
  *GPIOA_AFRL_PTR &= ~(0b1111 << 24);
  *GPIOA_AFRL_PTR |= (0b1110 << 24);


  // Set A6 speed to very fast
  volatile uint32_t *GPIOA_OSPEEDR_PTR = (volatile uint32_t *)(GPIOA_OSPEEDR);
  *GPIOA_OSPEEDR_PTR |= (0b11 << 12);

  // Set A6 to alternate function mode
  volatile uint32_t *GPIOA_MODER_PTR = (volatile uint32_t *)(GPIOA_MODER);
  *GPIOA_MODER_PTR &= ~(0b11 << 12);
  *GPIOA_MODER_PTR |= (0b10 << 12);
}

// Initialize TIM16 for PWM output
  void initializeTIMER() {
  // Enable clock to TIM16
  volatile uint32_t *RCC_APB2ENR_PTR = (volatile uint32_t *)(RCC_APB2ENR);
  *RCC_APB2ENR_PTR |= (1 << 17);

  // Set to PWM mode
  volatile uint32_t *TIM16_CCMR1_PTR = (volatile uint32_t *)(TIM16_CCMR1);
  *TIM16_CCMR1_PTR &= ~(0b111 << 4);
  *TIM16_CCMR1_PTR |= (0b110 << 4);

  // Enable preload register
  *TIM16_CCMR1_PTR |= (1 << 3);

  // Set ARPE bit in CR1 register
  volatile uint16_t *TIM16_CR1_PTR = (volatile uint16_t *)(TIM16_CR1);
  *TIM16_CR1_PTR |= (1 << 7);

  // Initialize registers by setting UG bit in EGR
  volatile uint16_t *TIM16_EGR_PTR = (volatile uint16_t *)(TIM16_EGR);
  *TIM16_EGR_PTR |= (1 << 0);

  // Set duty cycle
  volatile uint16_t *TIM16_CCR1_PTR = (volatile uint16_t *)(TIM16_CCR1);
  *TIM16_CCR1_PTR |= (32767);

  // Clear and Set PSC
  volatile uint16_t *TIM16_PSC_PTR = (volatile uint16_t *)(TIM16_PSC);
  *TIM16_PSC_PTR &= ~(65535);
  *TIM16_PSC_PTR |= (255);

  // Set MOE
  volatile uint32_t *TIM16_BDTR_PTR = (volatile uint32_t *)(TIM16_BDTR);
  *TIM16_BDTR_PTR |= (1 << 15);

  // Set CC1E
  volatile uint16_t *TIM16_CCER_PTR = (volatile uint16_t *)(TIM16_CCER);
  *TIM16_CCER_PTR |= (1 << 0);
}

// Since clock frequency is at 8 MHz and we are using PSC of 255, we calculate ARR value with the following:
  void setFrequency(int frequency) {
  volatile uint16_t *TIM16_ARR_PTR = (volatile uint16_t *)(TIM16_ARR);
  int newArr = (31250/frequency) - 1;
  *TIM16_ARR_PTR &= ~(65535);
  *TIM16_ARR_PTR |= (newArr);

  volatile uint16_t *TIM16_CCR1_PTR = (volatile uint16_t *)(TIM16_CCR1);
  *TIM16_CCR1_PTR &= ~(65535);
  *TIM16_CCR1_PTR |= (newArr/2);
}

// Enable counter
  void enableCounter() {
  volatile uint16_t *TIM16_CR1_PTR = (volatile uint16_t *)(TIM16_CR1);
  *TIM16_CR1_PTR |= (1 << 0);
}

// Disable counter
  void disableCounter() {
  volatile uint16_t *TIM16_CR1_PTR = (volatile uint16_t *)(TIM16_CR1);
  *TIM16_CR1_PTR &= ~(1 << 0);
}

// Play note by setting PWM output and then delaying for some amount of time
  void playNote(int frequency, int time) {
  if (frequency != 0) {
    setFrequency(frequency);
    enableCounter();
    delay(time);
    disableCounter();
  } else {
    delay(time);
  }
}

// Play both songs, comment and uncomment as necessary based on which song you want to play
int main(void) {
  configureClock();
  initializeGPIO();
  initializeTIMER();

  delay(1000);
  //delay(5000);

  //for (int i = 0; i < (sizeof(notes) / sizeof(notes[0])); i++) {
  //  playNote(notes[i][0], notes[i][1]);
  //}

  delay(1000);
  for (int i = 0; i < (sizeof(twinkle) / sizeof(twinkle[0])); i++) {
    playNote(twinkle[i][0], twinkle[i][1]);
  }

  enableCounter();
  disableCounter();
	
}














