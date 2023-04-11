// ta.cpp - test out audio output

#include "../../stv/os.h"
#include <poll.h>
#include <alsa/asoundlib.h>

snd_pcm_t *AudO;
short Buf [4096];


int playback_callback (snd_pcm_sframes_t nfr)
{ int e;
DBG("playback_callback (`d)", (int)nfr);

// fill Buf with data

   if ((e = snd_pcm_writei (AudO, Buf, nfr)) < 0)
DBG("snd_pcm_write error=`s", snd_strerror (e));
   return e;
}


int main (int argc, char *argv [])
{  App.Init (CC("pcheetah"), CC("ta"), CC("ta"));
TRC("ta bgn");
  snd_pcm_hw_params_t *hw;
  snd_pcm_sw_params_t *sw;
  snd_pcm_sframes_t    nfr;
  int e;
   if ((e = snd_pcm_open (& AudO, argv [1], SND_PCM_STREAM_PLAYBACK, 0)) < 0)
      {DBG("`s", snd_strerror (e));   exit (1);}

   if ((e = snd_pcm_hw_params_malloc        (    & hw)) < 0)
      {DBG("`s", snd_strerror (e));   exit (1);}
   if ((e = snd_pcm_hw_params_any           (AudO, hw)) < 0)
      {DBG("`s", snd_strerror (e));   exit (1);}
   if ((e = snd_pcm_hw_params_set_access    (AudO, hw,
                                            SND_PCM_ACCESS_RW_INTERLEAVED)) < 0)
      {DBG("`s", snd_strerror (e));   exit (1);}
   if ((e = snd_pcm_hw_params_set_format    (AudO, hw,
                                                    SND_PCM_FORMAT_S16_LE)) < 0)
      {DBG("`s", snd_strerror (e));   exit (1);}
   if ((e = snd_pcm_hw_params_set_rate_near (AudO, hw, 44100, 0)) < 0)
      {DBG("`s", snd_strerror (e));   exit (1);}
   if ((e = snd_pcm_hw_params_set_channels  (AudO, hw, 2)) < 0)
      {DBG("`s", snd_strerror (e));   exit (1);}
   if ((e = snd_pcm_hw_params               (AudO, hw)) < 0)
      {DBG("`s", snd_strerror (e));   exit (1);}
   snd_pcm_hw_params_free                   (      hw);

// wake us up whenever 4096 or more frames of playback data
// can be delivered.  And we'll start the device ourselves.
   if ((e = snd_pcm_sw_params_malloc              (    & sw)) < 0)
      {DBG("`s", snd_strerror (e));   exit (1);}
   if ((e = snd_pcm_sw_params_current             (AudO, sw)) < 0)
      {DBG("`s", snd_strerror (e));   exit (1);}
   if ((e = snd_pcm_sw_params_set_avail_min       (AudO, sw, 4096)) < 0)
      {DBG("`s", snd_strerror (e));   exit (1);}
   if ((e = snd_pcm_sw_params_set_start_threshold (AudO, sw, 0U)) < 0)
      {DBG("`s", snd_strerror (e));   exit (1);}
   if ((e = snd_pcm_sw_params                     (AudO, sw)) < 0)
      {DBG("`s", snd_strerror (e));   exit (1);}

// interface will interrupt kernel every 4096 frames, and ALSA
// will wake up this program very soon after that.
   if ((e = snd_pcm_prepare (AudO)) < 0)
      {DBG("`s", snd_strerror (e));   exit (1);}

   for (;;) {
      if ((e = snd_pcm_wait (AudO, 1000)) < 0)
         {DBG("poll error=`s", strerror (errno));   break;}

   // find out how much space is available for playback data
      if ((nfr = snd_pcm_avail_update (AudO)) < 0) {
         if (nfr == -EPIPE)  DBG("snd_pcm xrun error");
         else                DBG("snd_pcm unknown rc=`d", (int)nfr);
         break;
      }
      nfr = nfr > 4096 ? 4096 : nfr;
      if (playback_callback (nfr) != nfr)   // deliver the data
         {DBG("playback_callback error");   break;}
   }

   snd_pcm_close (AudO);
TRC("ta end");
}
