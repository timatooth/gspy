#include <gst/gst.h>

#ifndef GSPY_AUDIO_SAMPLES_H_
#define GSPY_AUDIO_SAMPLES_H_

typedef struct _AudioSamples *AudioSamples;

extern double audiosamples_average(AudioSamples as);
extern void audiosamples_add(AudioSamples as, gdouble sample);
extern AudioSamples audiosamples_new(int sampleSize);
extern AudioSamples audiosamples_free(AudioSamples as);
/* Get the sample standard deviation of data */
extern gdouble audiosamples_sd(AudioSamples as);
extern void audiosamples_print(AudioSamples as);
extern guint audiosamples_size(AudioSamples as);

#endif
