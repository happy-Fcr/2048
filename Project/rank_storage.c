#include "rank_storage.h"
#include "gd32f4xx_fmc.h"

/* GD32F470ZG has 1 MiB internal Flash; reserve the final 4 KiB page. */
#define RANK_FLASH_ADDR 0x080FF000U
#define RANK_MAGIC      0x52414E4BU
#define RANK_VERSION    0x00000001U

typedef struct {
    uint32_t magic;
    uint32_t version;
    uint32_t scores[RANK_COUNT];
    uint32_t checksum;
} rank_flash_data_t;

static uint32_t RankStorage_Checksum(const rank_flash_data_t *data)
{
    uint8_t i;
    uint32_t checksum = data->magic ^ data->version;

    for (i = 0; i < RANK_COUNT; i++) {
        checksum ^= data->scores[i];
    }

    return checksum;
}

void RankStorage_Load(uint32_t scores[RANK_COUNT])
{
    uint8_t i;
    const rank_flash_data_t *data = (const rank_flash_data_t *)RANK_FLASH_ADDR;

    if (scores == 0) return;

    if (data->magic == RANK_MAGIC &&
        data->version == RANK_VERSION &&
        data->checksum == RankStorage_Checksum(data) &&
        data->scores[0] >= data->scores[1] &&
        data->scores[1] >= data->scores[2] &&
        data->scores[2] >= data->scores[3] &&
        data->scores[3] >= data->scores[4] &&
        data->scores[4] >= data->scores[5] &&
        data->scores[5] >= data->scores[6] &&
        data->scores[6] >= data->scores[7] &&
        data->scores[7] >= data->scores[8] &&
        data->scores[8] >= data->scores[9]) {
        for (i = 0; i < RANK_COUNT; i++) {
            scores[i] = data->scores[i];
        }
    } else {
        for (i = 0; i < RANK_COUNT; i++) {
            scores[i] = 0;
        }
    }
}

void RankStorage_Save(const uint32_t scores[RANK_COUNT])
{
    uint8_t i;
    uint32_t address = RANK_FLASH_ADDR;
    rank_flash_data_t data;
    const uint32_t *words = (const uint32_t *)&data;

    if (scores == 0) return;

    data.magic = RANK_MAGIC;
    data.version = RANK_VERSION;
    for (i = 0; i < RANK_COUNT; i++) {
        data.scores[i] = scores[i];
    }
    data.checksum = RankStorage_Checksum(&data);

    fmc_unlock();
    fmc_flag_clear(FMC_FLAG_END | FMC_FLAG_OPERR | FMC_FLAG_WPERR |
                   FMC_FLAG_PGMERR | FMC_FLAG_PGSERR | FMC_FLAG_RDDERR);

    if (fmc_page_erase(RANK_FLASH_ADDR) == FMC_READY) {
        for (i = 0; i < sizeof(rank_flash_data_t) / sizeof(uint32_t); i++) {
            if (fmc_word_program(address, words[i]) != FMC_READY) {
                break;
            }
            address += 4U;
        }
    }

    fmc_lock();
}
