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
    // clearing console output //
    printf("\033c");

    // high state command to wake card
    static t_cmd    CMDINIT = {{0xff,0xff,0xff,0xff, 0xff, 0xff}};
    
    offset_cmd = pio_add_program(pio, &cmd_asm_program);
    offset_dat = pio_add_program(pio, &dat_asm_program);
    uint8_t         *r1;

    sleep_ms(1000);
    printf("started\n");

    ft_sm_init();
    ft_sm_dat_init();

    //waking card
    ft_send_cmd(CMDINIT, 0);
    ft_send_cmd(CMDINIT, 0);
    ft_send_cmd(CMDINIT, 0);
    
    r1 = ft_send_cmd(ft_cmd(0,0), 0);
    r1 = ft_send_cmd(ft_cmd(8,0x1aa), 8);
    printf("\nSending cmd8... expected response 08 00 00 01 aa 13\n");
    ft_print_buffer(r1, 0, 6);
    printf("\nSending acmd41... expected response 3f c0 ...");

    for (int i = 0; i < 5; i++)
    {
        if (!(r1[1] & (1 << 7)))
        {
            r1 = ft_send_cmd(ft_cmd(55,0), 8);
            r1 = ft_send_cmd(ft_cmd(41, 0x40ff8000), 8);
            ft_print_buffer(r1, 0, 6);
        }
        else
            break;
    }
    if (!((r1[1] & (1 << 7))))
        ft_soft_reset();
    //cmd2 - cid request
    r1 = ft_send_cmd(ft_cmd(2, 0), 20);
    printf("Sending cmd2...");
    ft_print_buffer(r1, 1, 17);
    //cmd3 - relative card addres request
    r1 = ft_send_cmd(ft_cmd(3, 0), 8);
    printf("Sending cmd3..");
    ft_print_buffer(r1, 1, 8);
    rca = r1[1] << 8 | r1[2];
    printf("rca - %04x\n", rca);
    // r1 = ft_send_cmd(ft_cmd(55, (rca << 16)), 8);
    // r1 = ft_send_cmd(ft_cmd(41, 2), 8);
    
    //read card size
    r1 = ft_send_cmd(ft_cmd(9, (rca << 16)), 20);
    C_SIZE = (((r1[8] & 0x3F)<<16) | (r1 [9] << 8) | r1 [10]);
    printf("Card size %d blocks\n", (C_SIZE + 1) * 1024);

    //activate transfer mode
    r1 = ft_send_cmd(ft_cmd(7, (rca << 16)), 8);
    printf("Sending cmd7... received\n");

    if ((r1[1] << 8 | r1[2]) == 0)
        printf("Card initialized\n");
    else
        ft_soft_reset();
    tusb_init();
    ft_wait(1000000);
    printf("fin\n");
    reset_usb_boot(0,0);   
}