#ifndef __SOUND_H
#define __SOUND_H

#include <stdint.h>

void Sound_Init(void);
void Sound_StartMusic(void);
void Sound_Stop(void);
void Sound_PlayWin(void);
void Sound_PlayGameOver(void);
void Sound_Task(void);
void Sound_SetVolume(uint8_t level);

#endif /* __SOUND_H */
