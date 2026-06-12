#include "tusb.h"
#include "pico/unique_id.h"

/* ====================================================================
    1. ИДЕНТИФИКАТОРЫ И СТРОКОВЫЕ ОПИСАНИЯ
   ==================================================================== */
#define USBD_VID      0x2E8A // Официальный VID Raspberry Pi
#define USBD_PID      0x000A // Пример PID для Mass Storage

enum {
    STRID_LANGID = 0,
    STRID_MANUFACTURER,
    STRID_PRODUCT,
    STRID_SERIAL,
};

// Массив строк, которые увидит ОС при подключении устройства
static char const* string_desc_arr[] = {
    [STRID_LANGID]       = (const char[]) { 0x09, 0x04 }, // Поддержка английского языка (0x0409)
    [STRID_MANUFACTURER] = "Raspberry Pi",                // Производитель
    [STRID_PRODUCT]      = "Pico SDIO Card Reader",       // Название устройства
    [STRID_SERIAL]       = "000000000000",                // Заглушка для серийника (перезапишем ниже)
};

/* ====================================================================
    2. ДЕСКРИПТОР УСТРОЙСТВА (DEVICE DESCRIPTOR)
   ==================================================================== */
tusb_desc_device_t const desc_device = {
    .bLength            = sizeof(tusb_desc_device_t),
    .bDescriptorType    = TUSB_DESC_DEVICE,
    .bcdUSB             = 0x0200, // USB 2.0 (Pico работает в Full Speed 1.1, но тут пишем 2.0)
    .bDeviceClass       = 0x00,   // Класс задается в дескрипторе интерфейса
    .bDeviceSubClass    = 0x00,
    .bDeviceProtocol    = 0x00,
    .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,

    .idVendor           = USBD_VID,
    .idProduct          = USBD_PID,
    .bcdDevice          = 0x0100, // Версия устройства (1.0)

    .iManufacturer      = STRID_MANUFACTURER,
    .iProduct           = STRID_PRODUCT,
    .iSerialNumber      = STRID_SERIAL,
    .bNumConfigurations = 0x01
};

// Колбэк TinyUSB для выдачи дескриптора устройства
uint8_t const * tud_descriptor_device_cb(void) {
    return (uint8_t const *) &desc_device;
}

/* ====================================================================
    3. ДЕСКРИПТОР КОНФИГУРАЦИИ (CONFIGURATION DESCRIPTOR)
   ==================================================================== */
enum {
    ITF_NUM_MSC = 0,
    ITF_NUM_TOTAL
};

// Номера конечных точек (Endpoints). Для MSC нужны Bulk Out и Bulk In
#define EPNUM_MSC_OUT    0x01
#define EPNUM_MSC_IN     0x81

#define CONFIG_TOTAL_LEN (TUD_CONFIG_DESC_LEN + TUD_MSC_DESC_LEN)

uint8_t const desc_configuration[] = {
    // Конфигурация: номер, число интерфейсов, строковый индекс, общая длина, параметры питания
    TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, CONFIG_TOTAL_LEN, 0x00, 100),

    // Интерфейс Mass Storage: номер интерфейса, строка, точки OUT и IN, размер буфера (512)
    TUD_MSC_DESCRIPTOR(ITF_NUM_MSC, 0, EPNUM_MSC_OUT, EPNUM_MSC_IN, CFG_TUD_MSC_EP_BUFSIZE)
};

// Колбэк TinyUSB для выдачи дескриптора конфигурации
uint8_t const * tud_descriptor_configuration_cb(uint8_t index) {
    (void) index; // Игнорируем, так как у нас всего 1 конфигурация
    return desc_configuration;
}

/* ====================================================================
    4. РАБОТА СО СТРОКАМИ И УНИКАЛЬНЫМ СЕРИЙНЫМ НОМЕРОМ
   ==================================================================== */
static uint16_t _desc_str[32];

// Функция динамической генерации серийного номера из уникального ID чипа Pico
static void msc_id_init(void) {
    static bool initialized = false;
    if (initialized) return;

    pico_unique_board_id_t board_id;
    pico_get_unique_board_id(&board_id);
    
    // Переводим бинарный ID в HEX-строку для USB-серийника
    static char serial_str[17];
    for (int i = 0; i < 8; i++) {
        sprintf(&serial_str[i * 2], "%02X", board_id.id[i]);
    }
    string_desc_arr[STRID_SERIAL] = serial_str;
    initialized = true;
}

// Колбэк TinyUSB для выдачи строковых дескрипторов (в формате UTF-16)
uint16_t const* tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
    (void) langid;
    
    // Инициализируем уникальный серийник при первом запросе строки серийника
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
        if (chr_count > 31) chr_count = 31; // Ограничение буфера

        // Конвертируем ASCII в UTF-16 (требование USB стандарта)
        for (uint8_t i = 0; i < chr_count; i++) {
            _desc_str[1 + i] = str[i];
        }
    }

    // Первый байт — длина дескриптора, второй — тип (STRING)
    _desc_str[0] = (TUSB_DESC_STRING << 8) | (2 * chr_count + 2);

    return _desc_str;
}