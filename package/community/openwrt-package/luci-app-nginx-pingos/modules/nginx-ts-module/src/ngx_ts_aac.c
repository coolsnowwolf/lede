
/*
 * Copyright (C) Roman Arutyunyan
 */


#include <ngx_config.h>
#include <ngx_core.h>

#include "ngx_ts_aac.h"


ngx_ts_aac_params_t *
ngx_ts_aac_decode_params(ngx_ts_stream_t *ts, u_char *adts, size_t adts_len)
{
    /*
     * XXX
     * https://wiki.multimedia.cx/index.php/ADTS
     */

    ngx_ts_aac_params_t  *aac;

    static ngx_uint_t  freq[] = {
        96000, 88200, 64000, 48000, 44100, 32000, 24000, 22050,
        16000, 12000, 11025,  8000,  7350,     0,     0,     0
    };

    if (adts_len < 7) {
        return NULL;
    }

    aac = ngx_pcalloc(ts->pool, sizeof(ngx_ts_aac_params_t));
    if (aac == NULL) {
        return NULL;
    }

    /* MPEG Version: 0 for MPEG-4, 1 for MPEG-2 */
    aac->version = (adts[1] & 0x08) ? 1 : 0;

    /*
     * MPEG-4 Audio Object Types:
     *   1: AAC Main
     *   2: AAC LC (Low Complexity)
     *   3: AAC SSR (Scalable Sample Rate)
     *   4: AAC LTP (Long Term Prediction)
     */
    aac->profile = (adts[2] >> 6) + 1;

    aac->freq_index = (adts[2] >> 2) & 0x0f;

    aac->freq = freq[aac->freq_index];
    if (aac->freq == 0) {
        goto failed;
    }

    aac->chan = ((adts[2] & 0x01) << 2) | (adts[3] >> 6);

    ngx_log_debug4(NGX_LOG_DEBUG_CORE, ts->log, 0,
                   "ts aac version:%ui, profile:%ui, freq:%ui, chan:%ui",
                   aac->version, aac->profile, aac->freq, aac->chan);

    return aac;

failed:

    ngx_log_error(NGX_LOG_ERR, ts->log, 0, "failed to parse AAC parameters");

    return NULL;
}
