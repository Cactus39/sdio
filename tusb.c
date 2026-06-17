#include "tusb.h"
#include "sdio.h"

//wait ms while run usb task
void    ft_wait(int ms)
{
    uint32_t    start = board_millis();
    tud_task();
    while (board_millis() - start < ms)
        tud_task();
}

// callback for inquiry
void tud_msc_inquiry_cb(uint8_t lun, uint8_t vendor_id[8], uint8_t product_id[16], uint8_t product_rev[4]) {
    (void) lun;
    const char vid[] = "Pico";
    const char pid[] = "SDIO Reader";
    const char rev[] = "1.0";

    memcpy(vendor_id,   vid, strlen(vid) > 8 ? 8 : strlen(vid));
    memcpy(product_id,  pid, strlen(pid) > 16 ? 16 : strlen(pid));
    memcpy(product_rev, rev, strlen(rev) > 4 ? 4 : strlen(rev));
}

bool tud_msc_test_unit_ready_cb(uint8_t lun) {
    (void) lun;
    
    return true; 
}

void tud_msc_capacity_cb(uint8_t lun, uint32_t *block_count, uint16_t *block_size) {
    (void) lun;
    *block_size = 512;
    *block_count = (C_SIZE + 1) * 1024;
}

bool    tud_msc_is_writable_cb(uint8_t lun)
{
    return false;
}

//callback for read preset for buffsize 64
int32_t tud_msc_read10_cb(uint8_t lun, uint32_t lba, uint32_t offset, void* buffer, uint32_t bufsize) {
    (void) lun;

    static uint32_t cached_lba = 0xffffffff;
    static uint8_t  *cached_sec;
    uint32_t current_lba = lba + (offset / 512);
    uint32_t internal_offset = offset % 512;
    
    if (cached_lba != current_lba)
    {
        cached_sec = ft_read_sd(current_lba);
        cached_lba = current_lba;
    }
    
    memcpy(buffer, cached_sec + internal_offset, 64);   
    return bufsize;
}

int32_t tud_msc_write10_cb(uint8_t lun, uint32_t lba, uint32_t offset, uint8_t* buffer, uint32_t bufsize) {
    (void) lun;
    (void) offset;
    (void) lba;
    (void) buffer;
    (void) bufsize;
    
    return -1; 
}

int32_t tud_msc_scsi_cb(uint8_t lun, uint8_t const scsi_cmd[16], void* buffer, uint16_t bufsize)
{
    (void)lun;
    (void)buffer;
    (void)bufsize;

    switch(scsi_cmd[0])
    {
        default:
            tud_msc_set_sense(lun, SCSI_SENSE_ILLEGAL_REQUEST, 0x20, 0x00);
            return -1;
    }
}
