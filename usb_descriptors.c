#include "tusb.h"
#include "pico/unique_id.h"

/* ====================================================================
    1. DEVICE INFO
   ==================================================================== */
#define USBD_VID      0x2E8A // VID Raspberry Pi
#define USBD_PID      0x000A // PID Mass Storage

enum {
    STRID_LANGID = 0,
    STRID_MANUFACTURER,
    STRID_PRODUCT,
    STRID_SERIAL,
};

static char const* string_desc_arr[] = {
    [STRID_LANGID]       = (const char[]) { 0x09, 0x04 }, // lang support (0x0409)
    [STRID_MANUFACTURER] = "Raspberry Pi",                
    [STRID_PRODUCT]      = "Pico SDIO Card Reader",       
    [STRID_SERIAL]       = "000000000000",                
};

/* ====================================================================
    2. DEVICE DESCRIPTOR
   ==================================================================== */
tusb_desc_device_t const desc_device = {
    .bLength            = sizeof(tusb_desc_device_t),
    .bDescriptorType    = TUSB_DESC_DEVICE,
    .bcdUSB             = 0x0200, 
    .bDeviceClass       = 0x00,  
    .bDeviceSubClass    = 0x00,
    .bDeviceProtocol    = 0x00,
    .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,

    .idVendor           = USBD_VID,
    .idProduct          = USBD_PID,
    .bcdDevice          = 0x0100,

    .iManufacturer      = STRID_MANUFACTURER,
    .iProduct           = STRID_PRODUCT,
    .iSerialNumber      = STRID_SERIAL,
    .bNumConfigurations = 0x01
};


uint8_t const * tud_descriptor_device_cb(void) {
    return (uint8_t const *) &desc_device;
}

/* ====================================================================
    3. CONFIGURATION DESCRIPTOR
   ==================================================================== */
enum {
    ITF_NUM_MSC = 0,
    ITF_NUM_TOTAL
};


#define EPNUM_MSC_OUT    0x01
#define EPNUM_MSC_IN     0x81

#define CONFIG_TOTAL_LEN (TUD_CONFIG_DESC_LEN + TUD_MSC_DESC_LEN)

uint8_t const desc_configuration[] = {
    TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, CONFIG_TOTAL_LEN, 0x00, 100),
    TUD_MSC_DESCRIPTOR(ITF_NUM_MSC, 0, EPNUM_MSC_OUT, EPNUM_MSC_IN, CFG_TUD_MSC_EP_BUFSIZE)
};

uint8_t const * tud_descriptor_configuration_cb(uint8_t index) {
    (void) index;
    return desc_configuration;
}

/* ====================================================================
    4. STRINGS AND SERIAL NUM
   ==================================================================== */
static uint16_t _desc_str[32];


static void msc_id_init(void) {
    static bool initialized = false;
    if (initialized) return;

    pico_unique_board_id_t board_id;
    pico_get_unique_board_id(&board_id);
    
    static char serial_str[17];
    for (int i = 0; i < 8; i++) {
        sprintf(&serial_str[i * 2], "%02X", board_id.id[i]);
    }
    string_desc_arr[STRID_SERIAL] = serial_str;
    initialized = true;
}

uint16_t const* tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
    (void) langid;
    
    if (index == STRID_SERIAL) {
        msc_id_init();
    }

    uint8_t chr_count;

    if (index == STRID_LANGID) {
        _desc_str[1] = string_desc_arr[STRID_LANGID][0] | (string_desc_arr[STRID_LANGID][1] << 8);
        chr_count = 1;
    } else {
        if (!(index < sizeof(string_desc_arr) / sizeof(string_desc_arr[0]))) return NULL;

        const char* str = string_desc_arr[index];
        chr_count = strlen(str);
        if (chr_count > 31) chr_count = 31;

        for (uint8_t i = 0; i < chr_count; i++) {
            _desc_str[1 + i] = str[i];
        }
    }
    _desc_str[0] = (TUSB_DESC_STRING << 8) | (2 * chr_count + 2);

    return _desc_str;
}