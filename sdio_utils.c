#include "sdio.h"

//adding crc to the last cmd byte
t_cmd ft_crc7(t_cmd CMD)
{
    uint8_t     byte;
    uint8_t     crc = 0x0;

    for (int i = 0; i < 5; i++)
    {
        byte = CMD.bytes[i];
        for (int j = 7; j >= 0; j--)
        {
            uint8_t msb = (crc >> 6) & 0x01;
            crc = (crc << 1) & 0x7f;
            uint8_t curr = (byte >> j) & 0x01;
            if (msb ^ curr)
                crc = crc ^ 0x09;
        }
    }
    crc = (crc << 1) | 1;
    CMD.bytes[5] = (crc);
    return (CMD);
}

//creating cmd from int cmd number and argument
t_cmd   ft_cmd(int cmd_arg, uint32_t arg)
{
    t_cmd   cmd = {{(uint8_t)0x40 + cmd_arg, (uint8_t)(arg >> 24), \
    (uint8_t)(arg >> 16), (uint8_t)(arg >> 8), (uint8_t)arg, 0x01}};
    cmd = ft_crc7(cmd);
    return (cmd);   
}

//init state machine for cmd line
void    ft_sm_init()
{
    pio_gpio_init(pio, CMD_PIN);
    pio_gpio_init(pio, CLK_PIN);
    gpio_pull_up(CMD_PIN);
    pio_sm_set_consecutive_pindirs(pio, sm_cmd, CMD_PIN, 1, 0);
    pio_sm_set_consecutive_pindirs(pio, sm_cmd, CLK_PIN, 1, 1);
    pio_sm_config   cmd_c = cmd_asm_program_get_default_config(offset_cmd);

    sm_config_set_out_pins(&cmd_c, CMD_PIN, 1);
    sm_config_set_in_pins(&cmd_c, CMD_PIN);
    sm_config_set_jmp_pin(&cmd_c, CMD_PIN);
    sm_config_set_set_pins(&cmd_c, CMD_PIN, 1);
    sm_config_set_sideset_pins(&cmd_c, CLK_PIN);

    sm_config_set_out_shift(&cmd_c, false, false, 32);
    sm_config_set_in_shift(&cmd_c, false, false, 8);
    sm_config_set_clkdiv(&cmd_c, 6.250f);
    
    pio_sm_init(pio, sm_cmd, offset_cmd, &cmd_c);
}

// send command and receive answer r bytes
uint8_t *ft_send_cmd(t_cmd CMD, int r)
{
    static uint8_t buff[32];
    pio_sm_set_enabled(pio, sm_cmd, false);
    pio_sm_clear_fifos(pio, sm_cmd);
    pio_sm_restart(pio, sm_cmd);
    
    pio_sm_exec(pio, sm_cmd, pio_encode_jmp(offset_cmd));
    pio_sm_set_enabled(pio, sm_cmd, true);
    // printf("[%08x", (CMD.bytes[0]<<24 | CMD.bytes[1]<<16 | CMD.bytes[2]<<8 | CMD.bytes[3]));
    // printf("%04x]", (CMD.bytes[4]<<24 | CMD.bytes[5]<<16));

    pio_sm_put_blocking(pio, sm_cmd, (CMD.bytes[0]<<24 | CMD.bytes[1]<<16 | CMD.bytes[2]<<8 | CMD.bytes[3]));
    pio_sm_put_blocking(pio, sm_cmd, (CMD.bytes[4]<<24 | CMD.bytes[5]<<16));
    while (!pio_sm_is_tx_fifo_empty(pio, sm_cmd));
    for (int i = 0; i < r; i++)
        buff[i] = pio_sm_get_blocking(pio, sm_cmd);
    while (!pio_sm_is_rx_fifo_empty(pio, sm_cmd));
    pio_sm_exec(pio, sm_cmd, pio_encode_irq_clear(0, 4));
    sleep_ms(2);
    pio_sm_set_enabled(pio, sm_cmd, false);
    return (buff);
}

//print buffer flag == if_printable, count - bytes
int ft_print_buffer(unsigned char *buffer, int flag, int count)
{
    int i = 0;
    if (flag)
    {
        for (i = 0; i < count; i++)
        {
            if (isprint(buffer[i]))
                printf("%c", buffer[i]);
            else
                printf("%02x ", buffer[i]);
            if (i > count)
                break ;
        }
        printf("\n");
        return (i);
    }
    for (i = 0; i < count; i++)
    {
        if (i % 32 == 0)
            printf("\n");
        printf("%02x ", buffer[i]);
        if (i > count)
        break ;
    }
    printf("\n");
    return (0);
}

//init state machine for data line
void    ft_sm_dat_init()
{
    pio_gpio_init(pio, DAT0_PIN);
    gpio_pull_up(DAT0_PIN);
    
    pio_sm_set_consecutive_pindirs(pio, sm_dat, DAT0_PIN, 1, false);
    pio_sm_set_consecutive_pindirs(pio, sm_dat, CLK_PIN, 1, true);

    pio_sm_config dat_c = dat_asm_program_get_default_config(offset_dat);
    sm_config_set_sideset_pins(&dat_c, CLK_PIN);
    sm_config_set_in_pins(&dat_c, DAT0_PIN);
    sm_config_set_jmp_pin(&dat_c, DAT0_PIN);
    
    sm_config_set_out_shift(&dat_c, false, false, 8);
    sm_config_set_in_shift(&dat_c, false, false, 32);
    sm_config_set_clkdiv(&dat_c, 6.250f);
    pio_sm_init(pio, sm_dat, offset_dat, &dat_c);

}

//send cmd 17 with lbr as argument
uint8_t *ft_read_sd(uint arg)
{
    static uint8_t  buff[512 + 16];
    uint32_t        temp;

    pio_sm_clear_fifos(pio, sm_dat);
    pio_sm_restart(pio, sm_dat);

    pio_sm_exec(pio, sm_dat, pio_encode_irq_set(0, 4));
    pio_sm_exec(pio, sm_dat, pio_encode_jmp(offset_dat));
    pio_sm_set_enabled(pio, sm_dat, true);
    ft_send_cmd(ft_cmd(17, arg), 0);
    // bzero(buff, 512 + 16);

    for (int i = 0; i < (512); i += 4)
    {
        temp = pio_sm_get_blocking(pio, sm_dat);
        buff[i] = temp >> 24 & 0xFF;
        buff[i + 1] = temp >> 16 & 0xFF;
        buff[i + 2] = temp >> 8 & 0xFF;
        buff[i + 3] = temp & 0xFF;
    }
    // while(!pio_sm_is_rx_fifo_empty(pio, sm_dat));
        // ft_wait(1);
    ft_wait(1);
    pio_sm_set_enabled(pio, sm_dat, false);
    return (buff);
}

void ft_soft_reset(void)
{
    printf("restart system\n");
    sleep_ms(1000);
    watchdog_reboot(0, 0, 0);
    while (1)
        __breakpoint();
}