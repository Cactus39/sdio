
#include "sdio.h"
PIO             pio = pio0;
uint            sm_cmd = 0;
uint            sm_dat = 1;
uint            offset_cmd;
uint            offset_dat;
uint16_t        rca;
int             C_SIZE;
int main()
{
    stdio_init_all();
    // tusb_init();
    printf("\033c");
    // printf("\033[2J\033[H");
    static t_cmd    CMDINIT = {{0xff,0xff,0xff,0xff, 0xff, 0xff}};
    offset_cmd = pio_add_program(pio, &cmd_asm_program);
    offset_dat = pio_add_program(pio, &dat_asm_program);
    uint8_t         *r1;

    // ft_wait(1000);
    sleep_ms(1000);
    // ft_wait(2000);
    printf("started\n");

    ft_sm_init();
    ft_sm_dat_init();
    // ft_wait(1000);
    // sleep_ms(1000);
    ft_send_cmd(CMDINIT, 0);
    ft_send_cmd(CMDINIT, 0);
    ft_send_cmd(CMDINIT, 0);
    
    
    r1 = ft_send_cmd(ft_cmd(0,0), 0);
    r1 = ft_send_cmd(ft_cmd(8,0x1aa), 8);

    while (!(r1[1] & (1 << 7)))
    {
        r1 = ft_send_cmd(ft_cmd(55,0), 8);
        r1 = ft_send_cmd(ft_cmd(41, 0x40ff8000), 8);
    }   
    // ft_print_buffer(r1, 0, 8);
    
    r1 = ft_send_cmd(ft_cmd(2, 0), 20);
    // ft_print_buffer(r1, 1, 20);
    r1 = ft_send_cmd(ft_cmd(3, 0), 8);
    rca = r1[1] << 8 | r1[2];
    
    r1 = ft_send_cmd(ft_cmd(9, (rca << 16)), 20);
    C_SIZE = (((r1[8] & 0x3F)<<16) | (r1 [9] << 8) | r1 [10]);
    ft_send_cmd(ft_cmd(7, (rca << 16)), 8);
    
    // ft_print_buffer(r1, 0, 20);
    // ft_wait(1000);
    // r1 = ft_send_cmd(ft_cmd(7, (rca << 16)), 8);
    // pio_sm_set_clkdiv(pio, sm_dat, 1.25f);
    // pio_sm_set_clkdiv(pio, sm_cmd, 1.25f);
    tusb_init();
    // pio_sm_set_clkdiv(pio, sm_cmd, 6.250f);
    // pio_sm_set_clkdiv(pio, sm_cmd, 6.250f);
    // sleep_ms(300);
    
    
    // r1 = ft_read_sd(0);
    // ft_print_buffer(r1, 0, 512 + 4);
    // uint lbr_addr = ft_get_uint32(r1, 446 + 8);
    // r1 = ft_read_sd(lbr_addr);
    // ft_print_buffer(r1, 1, 512);
    // t_pbr pbr = ft_read_pbr(my_pio, sm_cmd, offset_cmd, sm_dat, offset_dat, lbr_addr);
    ft_wait(1000);
    printf("fin\n");
    reset_usb_boot(0,0);   
}