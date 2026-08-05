#include "sound.h"
#include "gd32f4xx.h"
#include "systick.h"

/*
 * TianKuixing expansion board routes DAC_AUDIO to P2-24 (PA3), followed by
 * an audio amplifier and speaker. A timer-driven square wave is sufficient
 * for the monophonic game melody and does not need an SD card.
 */
#define SOUND_PORT              GPIOA
#define SOUND_PIN               GPIO_PIN_3
#define SOUND_RCU               RCU_GPIOA
#define SOUND_TIMER             TIMER6
#define SOUND_TIMER_RCU         RCU_TIMER6
#define SOUND_TIMER_IRQ         TIMER6_IRQn
#define SOUND_TIMER_BASE_HZ     1000000U

typedef struct {
    uint16_t frequency;
    uint16_t duration_ms;
} sound_note_t;

enum {
    SOUND_IDLE = 0,
    SOUND_MUSIC,
    SOUND_EFFECT
};

/* A short, looping 2048-style melody. frequency 0 is a rest. */
static const sound_note_t game_music[] = {
    {659, 180}, {784, 180}, {988, 180}, {784, 180},
    {659, 180}, {587, 180}, {523, 360}, {0, 90},
    {587, 180}, {659, 180}, {784, 180}, {659, 180},
    {587, 180}, {523, 180}, {494, 360}, {0, 180}
};

static const sound_note_t win_effect[] = {
    {784, 100}, {988, 100}, {1175, 120}, {1568, 280}
};

static const sound_note_t game_over_effect[] = {
    {494, 160}, {392, 160}, {330, 180}, {262, 420}
};

static const sound_note_t *active_notes;
static uint8_t active_count;
static uint8_t note_index;
static uint8_t sound_mode;
static uint8_t loop_enabled;
static uint32_t next_note_ms;
static volatile uint8_t sound_volume = 3U;
static volatile uint8_t gate_phase;

static void Sound_SetTone(uint16_t frequency)
{
    timer_parameter_struct timer_initpara;
    uint32_t period;
    uint32_t prescaler;

    timer_disable(SOUND_TIMER);
    gpio_bit_reset(SOUND_PORT, SOUND_PIN);
    gate_phase = 0U;

    if (frequency == 0U) {
        return;
    }

    prescaler = SystemCoreClock / SOUND_TIMER_BASE_HZ;
    period = SOUND_TIMER_BASE_HZ / ((uint32_t)frequency * 2U);

    if (prescaler == 0U || period == 0U) {
        return;
    }

    timer_deinit(SOUND_TIMER);
    timer_struct_para_init(&timer_initpara);
    timer_initpara.prescaler = (uint16_t)(prescaler - 1U);
    timer_initpara.alignedmode = TIMER_COUNTER_EDGE;
    timer_initpara.counterdirection = TIMER_COUNTER_UP;
    timer_initpara.period = (uint16_t)(period - 1U);
    timer_initpara.clockdivision = TIMER_CKDIV_DIV1;
    timer_initpara.repetitioncounter = 0U;
    timer_init(SOUND_TIMER, &timer_initpara);
    timer_interrupt_flag_clear(SOUND_TIMER, TIMER_INT_FLAG_UP);
    timer_interrupt_enable(SOUND_TIMER, TIMER_INT_UP);
    timer_enable(SOUND_TIMER);
}

static void Sound_StartSequence(const sound_note_t *notes, uint8_t count,
                                uint8_t loop, uint8_t mode)
{
    active_notes = notes;
    active_count = count;
    note_index = 0U;
    loop_enabled = loop;
    sound_mode = mode;
    next_note_ms = systick_get_ms();
    Sound_Task();
}

void Sound_Init(void)
{
    rcu_periph_clock_enable(SOUND_RCU);
    rcu_periph_clock_enable(SOUND_TIMER_RCU);

    gpio_mode_set(SOUND_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, SOUND_PIN);
    gpio_output_options_set(SOUND_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, SOUND_PIN);
    gpio_bit_reset(SOUND_PORT, SOUND_PIN);

    nvic_irq_enable(SOUND_TIMER_IRQ, 2U, 0U);
}

void Sound_StartMusic(void)
{
    Sound_StartSequence(game_music,
                        (uint8_t)(sizeof(game_music) / sizeof(game_music[0])),
                        1U, SOUND_MUSIC);
}

void Sound_Stop(void)
{
    sound_mode = SOUND_IDLE;
    active_notes = 0;
    active_count = 0U;
    note_index = 0U;
    timer_disable(SOUND_TIMER);
    gpio_bit_reset(SOUND_PORT, SOUND_PIN);
}

void Sound_SetVolume(uint8_t level)
{
    if (level > 5U) {
        level = 5U;
    }
    sound_volume = level;
}

void Sound_PlayWin(void)
{
    Sound_StartSequence(win_effect,
                        (uint8_t)(sizeof(win_effect) / sizeof(win_effect[0])),
                        0U, SOUND_EFFECT);
}

void Sound_PlayGameOver(void)
{
    Sound_StartSequence(game_over_effect,
                        (uint8_t)(sizeof(game_over_effect) / sizeof(game_over_effect[0])),
                        0U, SOUND_EFFECT);
}

void Sound_Task(void)
{
    uint32_t now;

    if (sound_mode == SOUND_IDLE) {
        return;
    }

    now = systick_get_ms();
    if ((int32_t)(now - next_note_ms) < 0) {
        return;
    }

    if (note_index >= active_count) {
        if (loop_enabled) {
            note_index = 0U;
        } else {
            Sound_Stop();
            return;
        }
    }

    Sound_SetTone(active_notes[note_index].frequency);
    next_note_ms = now + active_notes[note_index].duration_ms;
    note_index++;
}

void TIMER6_IRQHandler(void)
{
    if (timer_interrupt_flag_get(SOUND_TIMER, TIMER_INT_FLAG_UP) == SET) {
        timer_interrupt_flag_clear(SOUND_TIMER, TIMER_INT_FLAG_UP);
        gate_phase = (uint8_t)((gate_phase + 1U) % 5U);
        if (sound_volume == 0U || gate_phase >= sound_volume) {
            gpio_bit_reset(SOUND_PORT, SOUND_PIN);
        } else {
            gpio_bit_toggle(SOUND_PORT, SOUND_PIN);
        }
    }
}
