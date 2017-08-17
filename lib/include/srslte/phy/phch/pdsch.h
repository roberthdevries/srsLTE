/**
 *
 * \section COPYRIGHT
 *
 * Copyright 2013-2015 Software Radio Systems Limited
 *
 * \section LICENSE
 *
 * This file is part of the srsLTE library.
 *
 * srsLTE is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * srsLTE is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * A copy of the GNU Affero General Public License can be found in
 * the LICENSE file in the top-level directory of this distribution
 * and at http://www.gnu.org/licenses/.
 *
 */

/******************************************************************************
 *  File:         pdsch.h
 *
 *  Description:  Physical downlink shared channel
 *
 *  Reference:    3GPP TS 36.211 version 10.0.0 Release 10 Sec. 6.4
 *****************************************************************************/

#ifndef PDSCH_
#define PDSCH_

#ifndef SRSLTE_SINGLE_THREAD

#include <pthread.h>
#include <semaphore.h>

#endif /* SRSLTE_SINGLE_THREAD */

#include "srslte/config.h"
#include "srslte/phy/common/phy_common.h"
#include "srslte/phy/mimo/precoding.h"
#include "srslte/phy/mimo/layermap.h"
#include "srslte/phy/modem/mod.h"
#include "srslte/phy/modem/demod_soft.h"
#include "srslte/phy/scrambling/scrambling.h"
#include "srslte/phy/phch/dci.h"
#include "srslte/phy/phch/regs.h"
#include "srslte/phy/phch/sch.h"
#include "srslte/phy/phch/pdsch_cfg.h"

typedef struct {
  srslte_sequence_t seq[SRSLTE_MAX_CODEWORDS][SRSLTE_NSUBFRAMES_X_FRAME];
  bool sequence_generated;
} srslte_pdsch_user_t;

#ifndef SRSLTE_SINGLE_THREAD

typedef struct {
  /* Thread identifier: they must set before thread creation */
  uint32_t codeword_idx;
  void *pdsch_ptr;

  /* Configuration Encoder/Decoder: they must be set before posting start semaphore */
  srslte_pdsch_cfg_t *cfg;
  uint16_t rnti;

  /* Encoder/Decoder data pointers: they must be set before posting start semaphore  */
  uint8_t *data;
  void *softbuffer;

  /* Execution status */
  int ret_status;

  /* Semaphores */
  sem_t start;
  sem_t finish;

  /* Thread kill flag */
  bool quit;
} srslte_pdsch_thread_args_t;

#endif /* SRSLTE_SINGLE_THREAD */

/* PDSCH object */
typedef struct SRSLTE_API {
  srslte_cell_t cell;
  
  uint32_t nof_rx_antennas;
  
  uint32_t max_re;
  
  /* buffers */
  // void buffers are shared for tx and rx
  cf_t *ce[SRSLTE_MAX_PORTS][SRSLTE_MAX_PORTS]; /* Channel estimation (Rx only) */
  cf_t *symbols[SRSLTE_MAX_PORTS];              /* PDSCH Encoded/Decoded Symbols */
  cf_t *x[SRSLTE_MAX_LAYERS];                   /* Layer mapped */
  cf_t *d[SRSLTE_MAX_CODEWORDS];                /* Modulated/Demodulated codewords */
  void *e[SRSLTE_MAX_CODEWORDS];

  /* tx & rx objects */
  srslte_modem_table_t mod[4];
  
  // This is to generate the scrambling seq for multiple CRNTIs
  srslte_pdsch_user_t **users;
  
  srslte_sch_t dl_sch[SRSLTE_MAX_CODEWORDS];

#ifndef SRSLTE_SINGLE_THREAD

  pthread_t threads[SRSLTE_MAX_CODEWORDS];
  srslte_pdsch_thread_args_t thread_args[SRSLTE_MAX_CODEWORDS];

#endif /* SRSLTE_SINGLE_THREAD */

} srslte_pdsch_t;

SRSLTE_API int srslte_pdsch_init(srslte_pdsch_t *q, 
                                 srslte_cell_t cell);

SRSLTE_API int srslte_pdsch_init_tx_multi(srslte_pdsch_t *q,
                                          srslte_cell_t cell);

SRSLTE_API int srslte_pdsch_init_rx_multi(srslte_pdsch_t *q,
                                          srslte_cell_t cell,
                                          uint32_t nof_antennas);

SRSLTE_API void srslte_pdsch_free(srslte_pdsch_t *q);

SRSLTE_API int srslte_pdsch_set_rnti(srslte_pdsch_t *q, 
                                     uint16_t rnti);

SRSLTE_API void srslte_pdsch_free_rnti(srslte_pdsch_t *q, 
                                      uint16_t rnti);

SRSLTE_API float srslte_pdsch_coderate(uint32_t tbs, 
                                       uint32_t nof_re); 

SRSLTE_API int srslte_pdsch_cfg(srslte_pdsch_cfg_t *cfg, 
                                srslte_cell_t cell, 
                                srslte_ra_dl_grant_t *grant, 
                                uint32_t cfi, 
                                uint32_t sf_idx, 
                                uint32_t rvidx); 

SRSLTE_API int srslte_pdsch_cfg_multi(srslte_pdsch_cfg_t *cfg,
                                      srslte_cell_t cell,
                                      srslte_ra_dl_grant_t *grant,
                                      uint32_t cfi,
                                      uint32_t sf_idx,
                                      uint32_t rvidx,
                                      uint32_t rvidx2,
                                      srslte_mimo_type_t mimo_type,
                                      uint32_t pmi);

SRSLTE_API int srslte_pdsch_encode(srslte_pdsch_t *q,
                                   srslte_pdsch_cfg_t *cfg,
                                   srslte_softbuffer_tx_t *softbuffer,
                                   uint8_t *data, 
                                   uint16_t rnti,
                                   cf_t *sf_symbols[SRSLTE_MAX_PORTS]);

SRSLTE_API int srslte_pdsch_encode_multi(srslte_pdsch_t *q,
                                         srslte_pdsch_cfg_t *cfg,
                                         srslte_softbuffer_tx_t softbuffers[SRSLTE_MAX_CODEWORDS],
                                         uint8_t *data[SRSLTE_MAX_CODEWORDS],
                                         uint16_t rnti,
                                         cf_t *sf_symbols[SRSLTE_MAX_PORTS]);

SRSLTE_API int srslte_pdsch_decode(srslte_pdsch_t *q, 
                                   srslte_pdsch_cfg_t *cfg, 
                                   srslte_softbuffer_rx_t *softbuffer,
                                   cf_t *sf_symbols, 
                                   cf_t *ce[SRSLTE_MAX_PORTS],
                                   float noise_estimate, 
                                   uint16_t rnti,
                                   uint8_t *data);

SRSLTE_API int srslte_pdsch_decode_multi(srslte_pdsch_t *q, 
                                         srslte_pdsch_cfg_t *cfg, 
                                         srslte_softbuffer_rx_t softbuffers[SRSLTE_MAX_CODEWORDS],
                                         cf_t *sf_symbols[SRSLTE_MAX_PORTS], 
                                         cf_t *ce[SRSLTE_MAX_PORTS][SRSLTE_MAX_PORTS],
                                         float noise_estimate, 
                                         uint16_t rnti,
                                         uint8_t *data[SRSLTE_MAX_CODEWORDS]);

SRSLTE_API int srslte_pdsch_ri_pmi_select(srslte_pdsch_t *q,
                                          srslte_pdsch_cfg_t *cfg,
                                          cf_t *ce[SRSLTE_MAX_PORTS][SRSLTE_MAX_PORTS],
                                          float noise_estimate,
                                          uint32_t nof_ce,
                                          uint32_t *ri,
                                          uint32_t *pmi,
                                          float *current_sinr);

SRSLTE_API void srslte_pdsch_set_max_noi(srslte_pdsch_t *q, int max_iter);

SRSLTE_API float srslte_pdsch_average_noi(srslte_pdsch_t *q);

SRSLTE_API uint32_t srslte_pdsch_last_noi(srslte_pdsch_t *q); 

#endif
