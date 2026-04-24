#ifndef PS5_SDK_PS5_SDK_AUDIO_HELPERS_H
#define PS5_SDK_PS5_SDK_AUDIO_HELPERS_H

#include "core.h"

#define PS5SDK_AUDIO_S16_STEREO 1
#define PS5SDK_AUDIO_SAMPLE_RATE 48000
#define PS5SDK_AUDIO_SAMPLES_PER_BUF 256

static void ps5_sdk_audio_fill_square_beep(s16 *samples, int frames, int phase, int period, s16 amp) {
    if (!samples) return;
    if (period < 8) period = 8;
    for (int i = 0; i < frames; i++) {
        int t = (i + phase) % period;
        s16 v = (t < (period / 2)) ? amp : (s16)-amp;
        samples[i * 2 + 0] = v;
        samples[i * 2 + 1] = v;
    }
}

static s32 ps5_sdk_audio_open_default(void *G, void *open_fn) {
    if (!open_fn) return -1;
    return (s32)NC(G, open_fn, 0xFF, 0, 0, PS5SDK_AUDIO_SAMPLES_PER_BUF, PS5SDK_AUDIO_SAMPLE_RATE, PS5SDK_AUDIO_S16_STEREO);
}

static void ps5_sdk_audio_close_known_handles(void *G, void *close_fn, int max_handles) {
    if (!close_fn) return;
    for (int h = 0; h < max_handles; h++) {
        NC(G, close_fn, (u64)h, 0, 0, 0, 0, 0);
    }
}

static s32 ps5_sdk_audio_play_square_tone(
    void *G,
    void *output_fn,
    s32 audio_h,
    s16 *samples,
    int chunks,
    int period,
    s16 amp
) {
    if (!output_fn || audio_h < 0 || !samples || chunks <= 0) return -1;
    s32 ret = 0;
    int phase = 0;
    for (int n = 0; n < chunks; n++) {
        ps5_sdk_audio_fill_square_beep(samples, PS5SDK_AUDIO_SAMPLES_PER_BUF, phase, period, amp);
        ret = (s32)NC(G, output_fn, (u64)audio_h, (u64)samples, 0, 0, 0, 0);
        phase += PS5SDK_AUDIO_SAMPLES_PER_BUF;
    }
    return ret;
}

#endif /* PS5_SDK_PS5_SDK_AUDIO_HELPERS_H */
