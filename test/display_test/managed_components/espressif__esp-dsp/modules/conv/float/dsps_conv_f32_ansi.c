// Copyright 2018-2019 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "dsps_conv.h"
#include "esp_log.h"

static const char *TAG = "dsps_conv";

esp_err_t dsps_conv_f32_ansi(const float *Signal, const int siglen, const float *Kernel, const int kernlen, float *convout)
{
    if (NULL == Signal) {
        return ESP_ERR_DSP_PARAM_OUTOFRANGE;
    }
    if (NULL == Kernel) {
        return ESP_ERR_DSP_PARAM_OUTOFRANGE;
    }
    if (NULL == convout) {
        return ESP_ERR_DSP_PARAM_OUTOFRANGE;
    }

    float *sig = (float *)Signal;
    float *kern = (float *)Kernel;
    int lsig = siglen;
    int lkern = kernlen;

    if (siglen < kernlen) {
        sig = (float *)Kernel;
        kern = (float *)Signal;
        lsig = kernlen;
        lkern = siglen;
    }

    for (int n = 0; n < lkern; n++) {
        size_t k;

        convout[n] = 0;

        for (k = 0; k <= n; k++) {
            convout[n] += sig[k] * kern[n - k];
        }
        ESP_LOGV(TAG, "L1 kmin = %i, kmax = %i , n-kmin = %i", 0, n, n);
    }
    for (int n = lkern; n < lsig; n++) {
        int kmin, kmax, k;

        convout[n] = 0;

        kmin = n - lkern + 1;
        kmax = n;
        ESP_LOGV(TAG, "L2 n=%i, kmin = %i, kmax = %i , n-kmin = %i", n, kmin, kmax, n - kmin);
        for (k = kmin; k <= kmax; k++) {
            convout[n] += sig[k] * kern[n - k];
        }
    }

    for (int n = lsig; n < lsig + lkern - 1; n++) {
        int kmin, kmax, k;

        convout[n] = 0;

        kmin = n - lkern + 1;
        kmax =  lsig - 1;

        for (k = kmin; k <= kmax; k++) {
            convout[n] += sig[k] * kern[n - k];
        }
        ESP_LOGV(TAG, "L3 n=%i, kmin = %i, kmax = %i , n-kmin = %i", n, kmin, kmax, n - kmin);
    }
    return ESP_OK;
}
