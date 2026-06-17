#ifndef SDIO_H
# define SDIO_H
#include "pico/stdlib.h"
#include "pico/bootrom.h"
#include "tusb.h"
#include "cmd_asm.pio.h"
#include "dat_asm.pio.h"
#include "hardware/structs/usb.h" 
#include "bsp/board.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <ctype.h>
# define CMD_PIN 5
# define CLK_PIN 6
# define DAT0_PIN 7
# define DAT1_PIN 8
# define DAT2_PIN 9
# define DAT3_PIN 10

extern PIO             pio;
extern uint            sm_cmd;
extern uint            sm_dat;
extern uint            offset_cmd;
extern uint            offset_dat;
extern uint16_t         rca;
extern int              C_SIZE;

typedef struct s_cmd
{
    uint8_t     bytes[6];
}   t_cmd;

typedef struct s_pbr
{
    uint16_t    sec_size_bytes;
    uint8_t     sec_per_cluster;
    uint16_t    rsvd_sec_count;
    uint8_t     num_FATs;
    uint32_t    total_sec_count;
    uint32_t    FAT_size_sec;
    uint32_t    Root_cluster;
}   t_pbr;


t_cmd   ft_crc7(t_cmd CMD);
t_cmd   ft_cmd(int cmd, uint32_t arg);
void    ft_sm_init();
void    ft_sm_dat_init();

uint8_t *ft_send_cmd(t_cmd CMD, int i);
int ft_print_buffer(unsigned char *buffer, int flag, int count);
uint32_t    ft_get_uint32(uint8_t *buff, uint offset);
uint16_t    ft_get_uint16(uint8_t *buff, uint offset);
uint8_t     *ft_read_sd(uint arg);
void    ft_wait(int ms);




#endif