#include <stdio.h>
#include <stdint.h>
#include <stm32f10x.h>


void SystemCLK(void);
void gpio_init(void);
void timer_init(void);
void TIM1_UP_IRQHandler(void);

void SystemCLK(void){
	RCC->CR |= 1<<16;
	while(!(RCC->CR & RCC_CR_HSERDY));
	
	//configure flash memory
	FLASH->ACR |= FLASH_ACR_PRFTBE;
	while(!(FLASH->ACR & FLASH_ACR_PRFTBS));
	
	FLASH->ACR |= FLASH_ACR_LATENCY_2; //latency for HCLK 64mHZ
	//Disabled PLL for configure
//	RCC->CR &= ~ RCC_CR_PLLON;
//	while(RCC->CR & RCC_CR_PLLRDY);
	// configure PLL
	RCC->CFGR |= RCC_CFGR_PLLXTPRE_HSE_Div2;
	//MULL
	RCC->CFGR |=RCC_CFGR_PLLMULL16; //4x16=64 mHZ
	
	RCC->CR |= 1<<24; // PLL ON
	while(!(RCC->CR &RCC_CR_PLLRDY));
	
	// Set PLL for HCLK
	RCC->CFGR |= RCC_CFGR_SW_PLL;
	while(!(RCC->CFGR & RCC_CFGR_SWS_PLL));
	
}
void gpio_init(void){
	RCC->APB2ENR |= RCC_APB2ENR_IOPCEN; //ENable clock GPIOC
	GPIOC->CRH &= ~(GPIO_CRH_CNF13 | GPIO_CRH_MODE13);
	GPIOC->CRH |= (GPIO_CRH_MODE13_1 | GPIO_CRH_MODE13_0); // MODE output 50MHZ == 3<<20
	GPIOC->CRH &= ~(GPIO_CRH_CNF13);   // push-pull
	
}

void timer_init(void){
	RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;
	TIM1->CR1 =0;
	TIM1->CR1 |= TIM_CR1_DIR; // set dir to down-couting
	TIM1->PSC = 64000-1; // Ftimer= 1000HZ => 1ms =1 count
	TIM1->ARR = 1000-1;
	TIM1->DIER |= TIM_DIER_UIE; // enable interrupt 
	TIM1->CR1 |=TIM_CR1_CEN; //Turn on timer
	NVIC_EnableIRQ(TIM1_UP_IRQn);
	
}
	
void TIM1_UP_IRQHandler(void){
	if(TIM1->SR &TIM_SR_UIF){
		TIM1->SR &= ~TIM_SR_UIF;
		GPIOC->ODR ^= GPIO_ODR_ODR13;
	}
}

int main(void){
	SystemCLK();
	gpio_init();
	timer_init();
	while(1){
	}
}
