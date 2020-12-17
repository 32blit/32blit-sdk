#include <string>

#include "fw-flash.hpp"
#include "firmware.hpp"

using namespace blit;

// trimmed FLASH registers
typedef struct
{
  volatile uint32_t ACR;
  volatile uint32_t KEYR1;
  volatile uint32_t OPTKEYR;
  volatile uint32_t CR1;
  volatile uint32_t SR1;
  volatile uint32_t CCR1;
} FLASH_TypeDef;

#define FLASH ((FLASH_TypeDef *) 0x52002000)

#define FLASH_KEY1 0x45670123U
#define FLASH_KEY2 0xCDEF89ABU

#define FLASH_CR_LOCK                        (1 << 0)
#define FLASH_CR_PG                          (1 << 1)
#define FLASH_CR_BER                         (1 << 3)
#define FLASH_CR_PSIZE                       (3 << 4)
#define FLASH_CR_START                       (1 << 7)

#define FLASH_SR_BSY                         (1 << 0)
#define FLASH_SR_QW                          (1 << 2)
#define FLASH_SR_EOP                         (1 << 16)

// SCB regs
#define SCB_AIRCR *(volatile uint32_t *)(0xE000ED0C)
#define SCB_CCR *(volatile uint32_t *)(0xE000ED14)
#define SCB_CCSIDR  *(volatile uint32_t *)(0xE000ED80)
#define SCB_CSSELR *(volatile uint32_t *)(0xE000ED84)
#define SCB_DCCSW *(volatile uint32_t *)(0xE000EF6C)

__attribute__((section(".persist"))) uint32_t persist_magic;

int rendered = 0;

void init() {
}

void render(uint32_t time_ms) {
  screen.pen = Pen(0, 0, 0);
  screen.clear();

  screen.pen = Pen(255, 255, 255);
  screen.text("Updating firmware...", minimal_font, Point(screen.bounds.w / 2, screen.bounds.h / 2), false, TextAlign::center_center);

  rendered++;
}

void update(uint32_t time_ms) {

  if(rendered < 2) return;

  // disable interrupts
  asm volatile ("cpsid i" : : : "memory");

  // unlock
  while(FLASH->SR1 & FLASH_SR_BSY) {}

  if(FLASH->CR1 & FLASH_CR_LOCK) {
    FLASH->KEYR1 = FLASH_KEY1;
    FLASH->KEYR1 = FLASH_KEY2;
  }

  // erase
  FLASH->CR1 |= FLASH_CR_PSIZE;
  FLASH->CR1 |= FLASH_CR_BER | FLASH_CR_START;

  // wait
  while(FLASH->SR1 & FLASH_SR_BSY) {}

  if(FLASH->SR1 & FLASH_SR_EOP) {
    FLASH->CCR1 |= FLASH_SR_EOP;
  }

  FLASH->CR1 &= ~FLASH_CR_BER;

  // write
  auto flash_ptr = reinterpret_cast<volatile uint32_t *>(0x8000000);
  auto ptr = reinterpret_cast<const uint32_t *>(firmware_data);
  unsigned int length = firmware_data_length;

  FLASH->CR1 |= FLASH_CR_PG;

  // 8 words at a time
  for(unsigned offset = 0; offset < length; offset += 8 * 4) {

    asm volatile("dsb 0xF":::"memory");

    for(int i = 0; i < 8; i++)
      *flash_ptr++ = *ptr++;

    asm volatile("dsb 0xF":::"memory");
  
    // wait
    while(FLASH->SR1 & FLASH_SR_QW) {}

    if(FLASH->SR1 & FLASH_SR_EOP) {
      FLASH->CCR1 |= FLASH_SR_EOP;
    }
  }

  FLASH->CR1 &= ~FLASH_CR_PG;

  // lock
  FLASH->CR1 |= FLASH_CR_LOCK;

  persist_magic = 0; // reset all the things

  //clean cache
  SCB_CSSELR = 0;
  asm volatile("dsb 0xF":::"memory");

  auto ccsidr = SCB_CCSIDR;

  auto sets = (uint32_t)((ccsidr >> 13) & 0x7FFF);
  do {
    auto ways = (uint32_t)(((ccsidr >> 3) & 0x3FF));
    do {
      SCB_DCCSW = ((sets & 0x1FF) << 5) | ((ways & 3) << 30);
    } while (ways-- != 0U);
  } while(sets-- != 0U);

  asm volatile("dsb 0xF":::"memory");
  asm volatile("isb 0xF":::"memory");

  // reset
  asm volatile("dsb 0xF":::"memory");
  SCB_AIRCR = (0x5FA << 16) /*VECTKEY*/ | (SCB_AIRCR & (7 << 8)) /*PRIGROUP*/ | (1 << 2) /*SYSRESETREQ*/;
  asm volatile("dsb 0xF":::"memory");

  while(true);
}