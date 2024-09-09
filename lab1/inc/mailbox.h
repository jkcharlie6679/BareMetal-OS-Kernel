#include <stdint.h>
/* channels */
#define MBOX_CH_POWER   0
#define MBOX_CH_FB      1
#define MBOX_CH_VUART   2
#define MBOX_CH_VCHIQ   3
#define MBOX_CH_LEDS    4
#define MBOX_CH_BTNS    5
#define MBOX_CH_TOUCH   6
#define MBOX_CH_COUNT   7
#define MBOX_CH_PROP    8
/* tags */
#define MAILBOX_TAG_BOARD_VISION     0x10002
#define MAILBOX_TAG_MEMORY           0x10005

/*extern volatile unsigned int mbox[8];*/

int mailbox_property(unsigned int identifier, unsigned int buffer_size, unsigned char ch);

void mbox_board_ver(uint32_t *board_ver);
void mbox_mem_info(uint32_t *mem_start_addr, uint32_t *mem_size);
