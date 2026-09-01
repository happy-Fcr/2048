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

    for (i = 0U; i < RANK_COUNT; i++) checksum ^= data->scores[i];
    return checksum;
}

static uint8_t RankStorage_IsValid(const rank_flash_data_t *data)
{
    uint8_t i;

    if (data->magic != RANK_MAGIC || data->version != RANK_VERSION ||
        data->checksum != RankStorage_Checksum(data)) return 0U;

    for (i = 1U; i < RANK_COUNT; i++) {
        if (data->scores[i - 1U] < data->scores[i]) return 0U;
    }
    return 1U;
}

void RankStorage_Load(uint32_t scores[RANK_COUNT])
{
    uint8_t i;
    const rank_flash_data_t *data = (const rank_flash_data_t *)RANK_FLASH_ADDR;

    if (scores == 0) return;

    if (RankStorage_IsValid(data) != 0U) {
        for (i = 0U; i < RANK_COUNT; i++) scores[i] = data->scores[i];
    } else {
        for (i = 0U; i < RANK_COUNT; i++) scores[i] = 0U;
    }
}

uint8_t RankStorage_Save(const uint32_t scores[RANK_COUNT])
{
    uint8_t i;
    uint8_t verify_index;
    uint8_t success = 0U;
    uint32_t address = RANK_FLASH_ADDR;
    rank_flash_data_t data;
    const rank_flash_data_t *verify = (const rank_flash_data_t *)RANK_FLASH_ADDR;
    const uint32_t *words = (const uint32_t *)&data;

    if (scores == 0) return 0U;

    data.magic = RANK_MAGIC;
    data.version = RANK_VERSION;
    for (i = 0U; i < RANK_COUNT; i++) data.scores[i] = scores[i];
    data.checksum = RankStorage_Checksum(&data);

    fmc_unlock();
    fmc_flag_clear(FMC_FLAG_END | FMC_FLAG_OPERR | FMC_FLAG_WPERR |
                   FMC_FLAG_PGMERR | FMC_FLAG_PGSERR | FMC_FLAG_RDDERR);

    if (fmc_page_erase(RANK_FLASH_ADDR) == FMC_READY) {
        for (i = 0U; i < sizeof(rank_flash_data_t) / sizeof(uint32_t); i++) {
            if (fmc_word_program(address, words[i]) != FMC_READY) break;
            address += 4U;
        }
        if (i == sizeof(rank_flash_data_t) / sizeof(uint32_t) &&
            RankStorage_IsValid(verify) != 0U) {
            success = 1U;
            for (verify_index = 0U; verify_index < RANK_COUNT; verify_index++) {
                if (verify->scores[verify_index] != scores[verify_index]) {
                    success = 0U;
                    break;
                }
            }
        }
    }

    fmc_lock();
    return success;
}
