/*
 * memstack.c - Membrane Stack implementation
 *
 *   Copyright (c) 2008, Ueda Laboratory LMNtal Group
 *                                         <lmntal@ueda.info.waseda.ac.jp>
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions are
 *   met:
 *
 *    1. Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *
 *    3. Neither the name of the Ueda Laboratory LMNtal Group nor the
 *       names of its contributors may be used to endorse or promote
 *       products derived from this software without specific prior
 *       written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * $Id$
 */

#ifndef LMN_MEMSTACK_H
#define LMN_MEMSTACK_H


#include "vector.h"
#include "../react_context.h"

struct MemReactCxtData {
  LmnMemStack memstack; /* 膜主導実行時に使用 */
};

#define RC_MEMSTACK(rc)  (((struct MemReactCxtData *)(rc)->v)->memstack)

void mem_react_cxt_init(struct ReactCxt *cxt);
void mem_react_cxt_destroy(struct ReactCxt *cxt);

inline LmnMemStack lmn_memstack_make(void);
inline void lmn_memstack_free(LmnMemStack memstack);
inline BOOL lmn_memstack_isempty(LmnMemStack memstack);
inline void lmn_memstack_push(LmnMemStack memstack, LmnMembrane *mem);
inline LmnMembrane *lmn_memstack_pop(LmnMemStack memstack);
inline LmnMembrane *lmn_memstack_peek(LmnMemStack memstack);
void lmn_memstack_delete(LmnMemStack memstack, LmnMembrane *mem);

#endif