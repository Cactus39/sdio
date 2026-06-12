#include "tusb.h"
#include "sdio.h"

void    ft_wait(int ms)
{
    uint32_t    start = board_millis();
    tud_task();
    while (board_millis() - start < ms)
        tud_task();
}


// 1. Информация о диске (ОС запрашивает её при подключении)
void tud_msc_inquiry_cb(uint8_t lun, uint8_t vendor_id[8], uint8_t product_id[16], uint8_t product_rev[4]) {
    (void) lun;
    const char vid[] = "Pico";
    const char pid[] = "SDIO Reader";
    const char rev[] = "1.0";

    memcpy(vendor_id,   vid, strlen(vid) > 8 ? 8 : strlen(vid));
    memcpy(product_id,  pid, strlen(pid) > 16 ? 16 : strlen(pid));
    memcpy(product_rev, rev, strlen(rev) > 4 ? 4 : strlen(rev));
}

// 2. Проверка готовности (вставлена ли карта)
bool tud_msc_test_unit_ready_cb(uint8_t lun) {
    (void) lun;
    // Тут должна быть проверка от твоего SDIO-драйвера (карта на месте?)
    // Для теста пока возвращаем true
    return true; 
}

// 3. Чтение объема диска
void tud_msc_capacity_cb(uint8_t lun, uint32_t *block_count, uint16_t *block_size) {
    (void) lun;
    *block_size = 512;
    // uint8_t *r1;
    // r1 = ft_send_cmd(ft_cmd(9, (rca << 16)), 20);
    // int C_SIZE = (((r1[8] & 0x3F)<<16) | (r1 [9] << 8) | r1 [10]);
    *block_count = (C_SIZE + 1) * 1024;
    // *block_count = 61071360;
    // printf("bc %d\n", *block_count);
    // sleep_ms(1000);
    // if (C_SIZE)
        // ft_send_cmd(ft_cmd(7, (rca << 16)), 8);
}

bool    tud_msc_is_writable_cb(uint8_t lun)
{
    return false;
}

// 4. Колбэк ЧТЕНИЯ (Вызывается, когда ПК читает данные)
int32_t tud_msc_read10_cb(uint8_t lun, uint32_t lba, uint32_t offset, void* buffer, uint32_t bufsize) {
    (void) lun;

    static uint32_t cached_lba = 0xffffffff;
    // static uint8_t  cached_sec[512 + 16];
    static uint8_t  *cached_sec;
    uint32_t current_lba = lba + (offset / 512);
    uint32_t internal_offset = offset % 512;
    // uint8_t *temp;
    
    if (cached_lba != current_lba)
    // if (0)
    {
        cached_sec = ft_read_sd(current_lba);
        // memcpy(cached_sec, temp, 512);
        cached_lba = current_lba;
    }
    
    memcpy(buffer, cached_sec + internal_offset, 64);   
    // printf("in of %d bs %d\n", internal_offset, bufsize);
    // buffer = ft_read_sd(lba + offset);

    return bufsize;
}

// 4. Колбэк ЧТЕНИЯ (Вызывается, когда ПК читает данные)
// int32_t tud_msc_read10_cb(uint8_t lun, uint32_t lba, uint32_t offset, void* buffer, uint32_t bufsize) {
//     (void) lun;

//     static uint32_t cached_lba = 0xffffffff;
//     static uint8_t  *cached_sec = NULL;
    
//     uint8_t* dst = (uint8_t*) buffer;     // Указатель, куда складывать данные для ПК
//     uint32_t bytes_to_copy = bufsize;    // Сколько всего байт просит ПК (может быть > 512)
//     uint32_t current_offset = offset;    // Текущее смещение внутри транзакции SCSI

//     while (bytes_to_copy > 0) {
//         // Вычисляем LBA и смещение внутри сектора для текущего кусочка данных
//         uint32_t current_lba = lba + (current_offset / 512);
//         uint32_t internal_offset = current_offset % 512;
        
//         // Считаем, сколько байт мы можем безопасно взять из текущего сектора
//         uint32_t chunk = 512 - internal_offset;
//         if (chunk > bytes_to_copy) {
//             chunk = bytes_to_copy;
//         }

//         // Если нужного сектора нет в кэше — обновляем кэш
//         if (cached_lba != current_lba) {
//             cached_sec = ft_read_sd(current_lba);
//             cached_lba = current_lba;
//         }

//         // Копируем только разрешенный кусочек (chunk), не выходя за рамки 512 байт кэша
//         memcpy(dst, cached_sec + internal_offset, chunk);   

//         // Сдвигаем указатели и уменьшаем счетчик оставшихся байт
//         dst += chunk;
//         current_offset += chunk;
//         bytes_to_copy -= chunk;
//     }

//     return bufsize;
// }

// 5. Колбэк ЗАПИСИ (Вызывается, когда ПК пишет данные)
int32_t tud_msc_write10_cb(uint8_t lun, uint32_t lba, uint32_t offset, uint8_t* buffer, uint32_t bufsize) {
    (void) lun;
    (void) offset;
    (void) lba;
    (void) buffer;
    (void) bufsize;

    // Твой вызов SDIO: например, sdio_write_blocks(buffer, lba, bufsize / 512);
    
    return -1; // Возвращаем количество успешно записанных БАЙТ
}

// 6. Обработка низкоуровневых команд SCSI (Обязателен!)
int32_t tud_msc_scsi_cb(
        uint8_t lun,
        uint8_t const scsi_cmd[16],
        void* buffer,
        uint16_t bufsize)
{
    (void)lun;
    (void)buffer;
    (void)bufsize;

    switch(scsi_cmd[0])
    {
        default:
            tud_msc_set_sense(
                lun,
                SCSI_SENSE_ILLEGAL_REQUEST,
                0x20,
                0x00
            );
            return -1;
    }
}

// Колбэк: получение емкости носителя
// void tud_msc_capacity_cb(uint8_t lun, uint32_t* block_count, uint16_t* block_size) {
//     t_pbr   pbr;

//     pbr = ft_read_pbr()
//     *block_size = 512; // Стандарт для SD-карт
//     *block_count = my_sdio_get_sector_count(); // Твоя функция, возвращающая объем в секторах
// }

// // Колбэк: чтение данных (ПК читает с SD-карты)
// int32_t tud_msc_read10_cb(uint8_t lun, uint32_t lba, uint32_t offset, void* buffer, uint32_t bufsize) {
//     // lba — номер стартового сектора
//     // bufsize — сколько байт нужно передать (обычно кратно 512)
//     uint32_t block_count = bufsize / 512;

//     // Вызываем твое чтение по SDIO
//     if (my_sdio_read_blocks(lba, buffer, block_count) == SDIO_OK) {
//         return bufsize; // Возвращаем количество прочитанных байт
//     }
    
//     return -1; // Ошибка чтения
// }

// // Колбэк: запись данных (ПК пишет на SD-карту)
// int32_t tud_msc_write10_cb(uint8_t lun, uint32_t lba, uint32_t offset, uint8_t* buffer, uint32_t bufsize) {
//     uint32_t block_count = bufsize / 512;

//     // Вызываем твою запись по SDIO
//     if (my_sdio_write_blocks(lba, buffer, block_count) == SDIO_OK) {
//         return bufsize;
//     }

//     return -1; // Ошибка записи
// }

// // Колбэк: базовая информация об устройстве (Inquiry)
// void tud_msc_inquiry_cb(uint8_t lun, uint8_t vendor_id[8], uint8_t product_id[16], uint8_t product_rev[4]) {
//     memcpy(vendor_id,  "PicoPIO ", 8);
//     memcpy(product_id, "SDIO Card Reader", 16);
//     memcpy(product_rev, "1.0 ", 4);
// }