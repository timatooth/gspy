#include <stdlib.h>
#include <math.h>
#include <gst/gst.h>
#include "audiosamples.h"

struct _AudioSamples {
  guint maxSampleSize;
  gdouble *samples;
  guint itemcount;
  guint index;
};

AudioSamples audiosamples_new(int size){
  AudioSamples as = malloc(sizeof &as);
  if (NULL == as){
    g_printerr("Error couldn't allocate memory for AudioSample\n");
    exit(1);
  }
  as->maxSampleSize = size;
  as->samples = malloc(sizeof as->samples[0] * as->maxSampleSize);
  as->index = 0;
  as->itemcount = 0;
  return as;
}

void audiosamples_add(AudioSamples as, gdouble sample){
  as->samples[as->index] = sample;
  as->index = (as->index + 1) % as->maxSampleSize;
  if(as->itemcount < as->maxSampleSize){
    as->itemcount++;
  }
}

void audiosamples_print(AudioSamples as){
  int i;
  for(i = 0; i < as->itemcount; i++){
    g_print("[%d] \t=> %.5f\n", i, as->samples[i]);
  }
}

guint audiosamples_size(AudioSamples as){
  return as->itemcount;
}

AudioSamples audiosamples_free(AudioSamples as){
  if(NULL == as){
    g_printerr("Warning: attempted to free NULL AudioSample ptr");
    return NULL;
  }
  free(as->samples);
  free(as);
  return NULL;
}

gdouble audiosamples_average(AudioSamples as){
  int i;
  gdouble sum = 0;
  for(i = 0; i < as->itemcount; i++){
    sum += as->samples[i];
  }
  return sum / as->itemcount;
}

/* Sample standard deviation */
gdouble audiosamples_sd(AudioSamples as){
  int i;
  gdouble avg = audiosamples_average(as);
  gdouble ss;
  gdouble result;
  if(as->itemcount < 2){
    return 0;
  }
  
  for(i = 0; i < as->itemcount; i++){
    ss += (as->samples[i] - avg) * (as->samples[i] - avg);
  }
  result = ss * (1.0f / (as->itemcount - 1));
  return sqrt(result);
}
