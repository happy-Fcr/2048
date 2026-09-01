#ifndef __RANK_STORAGE_H
#define __RANK_STORAGE_H

#include <stdint.h>

#define RANK_COUNT 10U

void RankStorage_Load(uint32_t scores[RANK_COUNT]);
uint8_t RankStorage_Save(const uint32_t scores[RANK_COUNT]);

#endif
