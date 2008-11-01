/*
 * rule.h - types and functions about rule, rule set, module
 *
 *   Copyright (c) 2008, Ueda Laboratory LMNtal Group <lang@ueda.info.waseda.ac.jp>
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
 * $Id: rule.h,v 1.6 2008/09/29 05:23:40 taisuke Exp $
 */

#ifndef LMN_RULE_H
#define LMN_RULE_H

#include "vector.h"

/*----------------------------------------------------------------------
 * Rule
 */

/* 関数によるルールの処理の表現。トランスレータにより、ルールを変換して
   生成された関数を想定している。戻り値は適用に成功した場合TRUE,失敗し
   た場合FALSEを返す */
typedef BOOL (*LmnTranslated)(LmnMembrane *);
typedef struct LmnRule *LmnRule;

LMN_EXTERN LmnRule lmn_rule_make(LmnRuleInstr instr, int instr_len, lmn_interned_str name);
LMN_EXTERN LmnRule lmn_rule_make_translated(LmnTranslated translated, lmn_interned_str name);
LMN_EXTERN LmnRule lmn_rule_copy(LmnRule rule);
LMN_EXTERN void lmn_rule_free(LmnRule rule);
LMN_EXTERN LmnTranslated lmn_rule_get_translated(LmnRule rule);
LMN_EXTERN lmn_interned_str lmn_rule_get_name(LmnRule rule);
LMN_EXTERN BYTE *lmn_rule_get_inst_seq(LmnRule rule);

/*----------------------------------------------------------------------
 * Rule Set
 */

typedef struct LmnRuleSet *LmnRuleSet;

LMN_EXTERN int lmn_gen_ruleset_id(void);
LMN_EXTERN LmnRuleSet lmn_ruleset_from_id(int id);
LMN_EXTERN LmnRuleSet lmn_ruleset_make(LmnRulesetId id, int init_size);
LMN_EXTERN void lmn_ruleset_free(LmnRuleSet ruleset);
LMN_EXTERN void lmn_ruleset_put(LmnRuleSet ruleset, LmnRule rule);
LMN_EXTERN int lmn_ruleset_get_id(LmnRuleSet ruleset);
LMN_EXTERN void lmn_set_ruleset(LmnRuleSet ruleset, int id);
LMN_EXTERN unsigned int lmn_ruleset_rule_num(LmnRuleSet ruleset);
LMN_EXTERN LmnRule lmn_ruleset_get_rule(LmnRuleSet ruleset, int i);

/*----------------------------------------------------------------------
 * System Rule Set
 */

extern LmnRuleSet system_ruleset;
void lmn_add_system_rule(LmnRule rule);

/*----------------------------------------------------------------------
 * Module
 */

LMN_EXTERN void lmn_set_module(lmn_interned_str module_name, LmnRuleSet ruleset);
LMN_EXTERN LmnRuleSet lmn_get_module_ruleset(lmn_interned_str module_name);

#endif /* LMN_RULE_H */