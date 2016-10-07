/*
 * translate.c
 *
 *   Copyright (c) 2008, Ueda Laboratory LMNtal Group <lmntal@ueda.info.waseda.ac.jp>
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
 *    3. Neither the name of the Ueda Laboratory LMNtal Groupy LMNtal
 *       Group nor the names of its contributors may be used to
 *       endorse or promote products derived from this software
 *       without specific prior written permission.
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
 * $Id: translate.c,v 1.34 2008/10/16 18:12:27 sasaki Exp $
 */

#include "translate.h"
#include "element/lmntal_thread.h"
#include "verifier/verifier.h"
#include "loader/load.h"
#include "loader/syntax.h"
#include "arch.h"
#include "element/vector.h"
#include "element/error.h"
#include "so.h"

#ifdef PROFILE
# include "runtime_status.h"
#endif

/* just for debug ! */
static FILE *OUT;

static lmn_interned_str translating_rule_name = 0;
void set_translating_rule_name(lmn_interned_str rule_name)
{
  translating_rule_name = rule_name;
}

void tr_print_list(int indent, int argi, int list_num, const LmnWord *list)
{
  int i;

  print_indent(indent); fprintf(OUT, "int targ%d_num = %d;\n", argi, list_num);
  print_indent(indent); fprintf(OUT, "LmnWord targ%d[] = {", argi);
  for(i = 0; i < list_num; i++){
    if(i != 0) fprintf(OUT, ", ");
    fprintf(OUT, "%ld", list[i]);
  }
  fprintf(OUT, "};\n");
}

void tr_instr_commit_ready(LmnReactCxtRef      rc,
                           LmnRuleRef          rule,
                           lmn_interned_str rule_name,
                           LmnLineNum       line_num,
                           LmnMembraneRef      *ptmp_global_root,
                           LmnRegisterArray      *p_v_tmp,
                           unsigned int     *org_next_id)
{
  LMN_ASSERT(rule);
  lmn_rule_set_name(rule, rule_name);

  *org_next_id = env_next_id();

#ifdef KWBT_OPT
  if (lmn_env.opt_mode != OPT_NONE) {
    lmn_fatal("translter mode, optimize mode is not supported");
  }
#endif

  if (RC_GET_MODE(rc, REACT_PROPERTY)) {
    return;
  }

  if (RC_GET_MODE(rc, REACT_ND)) {
    if (RC_MC_USE_DMEM(rc)) {
      /* dmemインタプリタ(body命令)を書かないとだめだ */
      lmn_fatal("translater mode, delta-membrane execution is not supported.");
    } else {
      LmnRegisterArray v, tmp;
      ProcessTableRef copymap;
      LmnMembraneRef tmp_global_root;
      unsigned int i, warry_size_org, warry_use_org;

#ifdef PROFILE
      if (lmn_env.profile_level >= 3) {
        profile_start_timer(PROFILE_TIME__STATE_COPY_IN_COMMIT);
      }
#endif

      warry_size_org  = warry_size(rc);
      warry_use_org   = warry_use_size(rc);
      tmp_global_root = lmn_mem_copy_with_map(RC_GROOT_MEM(rc), &copymap);

      /** 変数配列および属性配列のコピー */
      v = lmn_register_make(warry_size_org);

      /** copymapの情報を基に変数配列を書換える */
      for (i = 0; i < warry_use_org; i++) {
        LmnWord t;
        LmnRegisterRef r = lmn_register_array_get(v, i);
        lmn_register_set_at(r, at(rc, i));
        lmn_register_set_tt(r, tt(rc, i));
        if (lmn_register_tt(r) == TT_ATOM) {
          if (LMN_ATTR_IS_DATA(lmn_register_at(r))) {
            lmn_register_set_wt(r, (LmnWord)lmn_copy_data_atom((LmnAtom)wt(rc, i), (LmnLinkAttr)lmn_register_at(r)));
          } else if (proc_tbl_get_by_atom(copymap, LMN_SATOM(wt(rc, i)), &t)) {
            lmn_register_set_wt(r, (LmnWord)t);
          } else {
            t = 0;
            lmn_fatal("implementation error");
          }
        }
        else if (lmn_register_tt(r) == TT_MEM) {
          if (wt(rc, i) == (LmnWord)RC_GROOT_MEM(rc)) { /* グローバルルート膜 */
            lmn_register_set_wt(r, (LmnWord)tmp_global_root);
          } else if (proc_tbl_get_by_mem(copymap, (LmnMembraneRef)wt(rc, i), &t)) {
            lmn_register_set_wt(r, (LmnWord)t);
          } else {
            t = 0;
            lmn_fatal("implementation error");
          }
        }
        else { /* TT_OTHER */
          lmn_register_set_wt(r, wt(rc, i));
        }
      }
      proc_tbl_free(copymap);

      /** SWAP */
      tmp = rc_warry(rc);
      rc_warry_set(rc, v);
#ifdef PROFILE
      if (lmn_env.profile_level >= 3) {
        profile_finish_timer(PROFILE_TIME__STATE_COPY_IN_COMMIT);
      }
#endif

      /* 処理中の変数を外へ持ち出す */
      *ptmp_global_root = tmp_global_root;
      *p_v_tmp = tmp;
    }
  }
}

BOOL tr_instr_commit_finish(LmnReactCxtRef      rc,
                            LmnRuleRef          rule,
                            lmn_interned_str rule_name,
                            LmnLineNum       line_num,
                            LmnMembraneRef      *ptmp_global_root,
                            LmnRegisterArray      *p_v_tmp,
                            unsigned int     warry_use_org,
                            unsigned int     warry_size_org)
{
  if(RC_GET_MODE(rc, REACT_ND)) {
    /* 処理中の変数を外から持ち込む */
    LmnMembraneRef tmp_global_root;
    LmnRegisterArray v;


    tmp_global_root = *ptmp_global_root;
    v = *p_v_tmp;

    mc_react_cxt_add_expanded(rc, tmp_global_root, rule); /* このruleはNULLではまずい気がする */

    if (lmn_rule_get_history_tbl(rule) && lmn_rule_get_pre_id(rule) != 0) {
      st_delete(lmn_rule_get_history_tbl(rule), lmn_rule_get_pre_id(rule), 0);
    }

    /* 変数配列および属性配列を元に戻す */

    lmn_register_free(rc_warry(rc));
    rc_warry_set(rc, v);
    warry_size_set(rc, warry_size_org);
    warry_use_size_set(rc, warry_use_org);

    return FALSE;
  } else {
    return TRUE;
  }
}

BOOL tr_instr_jump(LmnTranslated   f,
                   LmnReactCxtRef     rc,
                   LmnMembraneRef     thisisrootmembutnotused,
                   LmnRuleRef         rule,
                   int             newid_num,
                   const int       *newid)
{
  LmnRegisterArray v, tmp;
  unsigned int org_use, org_size;
  BOOL ret;
  int i;

  org_use  = warry_use_size(rc);
  org_size = warry_size(rc);
  v = lmn_register_make(org_size);
  for (i = 0; i < newid_num; i++){
    LmnRegisterRef r = lmn_register_array_get(v, i);
    lmn_register_set_wt(r, wt(rc, newid[i]));
    lmn_register_set_at(r, at(rc, newid[i]));
    lmn_register_set_tt(r, tt(rc, newid[i]));
  }

  tmp = rc_warry(rc);
  rc_warry_set(rc, v);

  ret = (*f)(rc, thisisrootmembutnotused, rule);

  lmn_register_free(rc_warry(rc));
  rc_warry_set(rc, tmp);
  warry_size_set(rc, org_size);
  warry_use_size_set(rc, org_use);

  return ret;
}

Vector vec_const_temporary_from_array(int size, const LmnWord *w)
{
  Vector v;
  v.num = size;
  v.cap = size;
  v.tbl = (LmnWord*)w;
  return v; /* コピーして返す tblはwをそのまま使うのでvec_freeしてはいけない */
}

int vec_inserted_index(Vector *v, LmnWord w)
{
  int i;
  for (i = 0; i < vec_num(v); i++){
    if (vec_get(v, i) == w) return i;
  }
  vec_push(v, w);
  return vec_num(v) - 1;
}

char *automalloc_sprintf(const char *format, ...)
{
  char trush[2];
  va_list ap;
  int buf_len;
  char *buf;

  va_start(ap, format);
  buf_len = vsnprintf(trush, 2, format, ap);
  va_end(ap);

  buf = (char *)lmn_malloc(buf_len + 1);

  va_start(ap, format);
  vsnprintf(buf, buf_len+1, format, ap);
  va_end(ap);

  return buf;
}

void print_indent(int n)
{
  int i;
  for(i=0; i<n*2; ++i){
    fprintf(OUT, " ");
  }
}

/* 常に失敗する translate_generatorまわりがおかしくなったらこれをコメントイン */
/*
const BYTE *translate_instruction_generated(const BYTE *p, Vector *jump_points, const char *header, const char *successcode, const char *failcode, int indent, int *finishflag)
{
  *finishflag = -1;
  return p;
}
*/

const BYTE *translate_instruction(const BYTE *instr,
                                  Vector     *jump_points,
                                  const char *header,
                                  const char *successcode,
                                  const char *failcode,
                                  int         indent,
                                  int        *finishflag)
{
  LmnInstrOp op;
  /* const BYTE *op_address = instr; */

  READ_VAL(LmnInstrOp, instr, op);

  switch (op) {
  case INSTR_JUMP:{
    /* 残念ながら引数読み込み途中のinstrからオフセットジャンプするため */
    /* 先に全部読み込んでしまうと場所を忘れてしまう */
    LmnInstrVar   num, i, n;
    LmnJumpOffset offset;
    LmnRuleInstr  next;
    int           next_index;

    READ_VAL(LmnJumpOffset, instr, offset);
    next       = (BYTE*)instr + offset; /*ワーニング抑制 */
    next_index = vec_inserted_index(jump_points, (LmnWord)next);

    print_indent(indent); fprintf(OUT, "{\n");
    print_indent(indent); fprintf(OUT, "  static const int newid[] = {");

    i = 0;
    /* atom */
    READ_VAL(LmnInstrVar, instr, num);
    for (; num--; i++) {
      READ_VAL(LmnInstrVar, instr, n);
      if(i != 0) fprintf(OUT, ",");
      fprintf(OUT, "%d", n);
    }

    /* mem */
    READ_VAL(LmnInstrVar, instr, num);
    for (; num--; i++) {
      READ_VAL(LmnInstrVar, instr, n);
      if(i != 0) fprintf(OUT, ",");
      fprintf(OUT, "%d", n);
    }

    /* vars */
    READ_VAL(LmnInstrVar, instr, num);
    for (; num--; i++) {
      READ_VAL(LmnInstrVar, instr, n);
      if(i != 0) fprintf(OUT, ",");
      fprintf(OUT, "%d", n);
    }

    fprintf(OUT, "};\n");
    print_indent(indent); fprintf(OUT, "  extern BOOL %s_%d();\n", header, next_index);
    print_indent(indent); fprintf(OUT, "  if (tr_instr_jump(%s_%d, rc, thisisrootmembutnotused, rule, %d, newid))\n"
        , header, next_index, i);
    print_indent(indent); fprintf(OUT, "    %s;\n", successcode);
    print_indent(indent); fprintf(OUT, "  else\n");
    print_indent(indent); fprintf(OUT, "    %s;\n", failcode);
    print_indent(indent); fprintf(OUT, "}\n");

    *finishflag = 0;
    return instr;
  }

  default:
    *finishflag = -1; /* 常に失敗,終了 */
    return instr;
  }
}

/*
  pの先頭から出力して行き,その階層のjump/proceedが出てくるまでを変換する
  jump先は中間命令ではアドレスになっているが,
  そのポインタ値が何個めのjump先として現れたか(index)を,その関数のシグネチャに使う
  物理的に次の読み込み場所を返す
  (変換するスタート地点, 変換する必要のある部分の記録, ルールのシグネチャ:trans_**_**_**, 成功時コード, 失敗時コード, インデント)
*/
const BYTE *translate_instructions(const BYTE *p, Vector *jump_points, const char *header, const char *successcode, const char *failcode, int indent)
{
  while(1){
    /* 自動生成で変換可能な中間命令をトランスレートする */
    /* 終了フラグ: 正のとき変換成功+次を変換, 0のとき変換成功+jump/proceed等なので終了, 負のとき変換失敗 */
    const BYTE *next;
    int finishflag;

    next = translate_instruction_generated(p, jump_points, header, successcode, failcode, indent, &finishflag);

    if (finishflag > 0){
      p = next;
      continue;
    } else if(finishflag == 0) {
      return next;
    } else {
      /* 自動生成で対処できない中間命令をトランスレートする */
      next = translate_instruction(p, jump_points, header, successcode, failcode, indent, &finishflag);
      if (finishflag > 0) {
        p = next;
        continue;
      } else if(finishflag == 0) {
        return next;
      } else {
        LmnInstrOp op;
        READ_VAL(LmnInstrOp, p, op);
        fprintf(stderr, "translator: unknown instruction: %d\n", op);
        exit(1);
      }
    }
  }
}

static void translate_rule(LmnRuleRef rule, const char *header)
{
  Vector *jump_points = vec_make(4);
  int i;

  vec_push(jump_points, (LmnWord)lmn_rule_get_inst_seq(rule));

  for (i = 0; i < vec_num(jump_points) /*変換中にjump_pointsは増えていく*/; i++){
    BYTE *p = (BYTE*)vec_get(jump_points, i);
    fprintf(OUT, "BOOL %s_%d(LmnReactCxt* rc, LmnMembraneRef thisisrootmembutnotused, LmnRule rule)\n", header, i); /* TODO m=wt[0]なのでmは多分いらない */
    fprintf(OUT, "{\n");
    /* (変換するスタート地点, 変換する必要のある部分の記録, ルールのシグネチャ:trans_**_**_**, 成功時コード, 失敗時コード, インデント) */
    translate_instructions(p, jump_points, header, "return TRUE", "return FALSE", 1);
    fprintf(OUT, "}\n");
  }

  /* 各関数の前方宣言をすることができないので,関数を呼ぶ時には自分で前方宣言をする */
  /* trans_***(); ではなく { extern trans_***(); trans_***(); } と書くだけ */
}

static void translate_ruleset(LmnRuleSetRef ruleset, const char *header)
{
  char *buf;
  lmn_interned_str *rule_names;
  int i, buf_len, rules_count;

  buf_len     = strlen(header) + 50; /* 適当. これだけあれば足りるはず */
  buf         = (char *)lmn_malloc(buf_len + 1);
  rules_count = lmn_ruleset_rule_num(ruleset);
  if (rules_count > 0) {
    rule_names = LMN_CALLOC(lmn_interned_str, rules_count);
  } else {
    rule_names = NULL;
  }

  for (i = 0; i < lmn_ruleset_rule_num(ruleset); i++) {
    snprintf(buf, buf_len, "%s_%d", header, i); /* ルールのシグネチャ */
    translate_rule(lmn_ruleset_get_rule(ruleset, i), buf);
    rule_names[i] = translating_rule_name;
  }
  fprintf(OUT, "struct trans_rule %s_rules[%d] = {", header, lmn_ruleset_rule_num(ruleset));

  for(i = 0; i < lmn_ruleset_rule_num(ruleset); i++){
    if(i != 0) fprintf(OUT, ", ");
    fprintf(OUT, "{%d, %s_%d_0}", rule_names[i], header, i); /* 各ルールの名前と先頭関数を配列に */
  }
  fprintf(OUT, "};\n\n");

  LMN_FREE(rule_names);
  lmn_free(buf);
}

static void print_trans_header(const char *filename)
{
  fprintf(OUT, "/* this .c source is generated by slim --translate */\n");
  fprintf(OUT, "/* compile: gcc -o %s.so ---.c -shared -Wall -fPIC -I SlimSrcPath*/\n", filename);
  fprintf(OUT, "/* run    : LD_LIBRARY_PATH=\".\" slim ./%s.so */\n", filename);
  fprintf(OUT, "/* .so file name must be \"%s\". */\n", filename);
  fprintf(OUT, "\n");
  fprintf(OUT, "#include \"so.h\"\n");
  fprintf(OUT, "\n");
  fprintf(OUT, "#define TR_GSID(x) (trans_%s_maindata.symbol_exchange[x])\n", filename);
  fprintf(OUT, "#define TR_GFID(x) (trans_%s_maindata.functor_exchange[x])\n", filename);
  fprintf(OUT, "#define TR_GRID(x) (trans_%s_maindata.ruleset_exchange[x])\n", filename);
  fprintf(OUT, "\n");
  fprintf(OUT, "extern struct trans_maindata trans_%s_maindata;\n", filename);
  fprintf(OUT, "\n");
}

/* 全ルールセットの総数を数える. ルールセット0番, 1番を数に含む (0番は使わない(番号合わせ),1番はsystem) */
/* 2番3番にinitial_rulesetが入った模様 なので4番から通常ルールセット */
static int count_rulesets()
{
  int i;

  for(i = FIRST_ID_OF_NORMAL_RULESET; ; i++){
    LmnRuleSetRef rs = lmn_ruleset_from_id(i);
    if(!rs) break;
  }

  return i;
}

static int count_modules()
{
  extern st_table_t module_table;
  return st_num(module_table);
}

static void print_trans_maindata(const char *filename)
{
  fprintf(OUT, "struct trans_maindata trans_%s_maindata = {\n", filename);

  /* シンボルの個数(0番anonymousも数える) */
  fprintf(OUT, "  %d, /*count of symbol*/\n", count_symbols());
  /* シンボルの配列 */
  fprintf(OUT, "  trans_%s_maindata_symbols, /*symboltable*/\n", filename);
  /* ファンクタの個数 */
  fprintf(OUT, "  %d, /*count of functor*/\n", lmn_functor_table.next_id);
  /* ファンクタの配列 */
  fprintf(OUT, "  trans_%s_maindata_functors, /*functortable*/\n", filename);
  /* ルールセットの個数 */
  fprintf(OUT, "  %d, /*count of ruleset*/\n", count_rulesets());
  /* ルールセットオブジェクトへのポインタの配列 */
  fprintf(OUT, "  trans_%s_maindata_rulesets, /*rulesettable*/\n", filename);
  /* モジュールの個数 */
  fprintf(OUT, "  %d, /*count of module*/\n", count_modules());
  /* モジュールの配列 */
  fprintf(OUT, "  trans_%s_maindata_modules, /*moduletable*/\n", filename);
  /* シンボルid変換テーブル */
  fprintf(OUT, "  trans_%s_maindata_symbolexchange, /*symbol id exchange table*/\n", filename);
  /* ファンクタid変換テーブル */
  fprintf(OUT, "  trans_%s_maindata_functorexchange, /*functor id exchange table*/\n", filename);
  /* ルールセットid変換テーブル */
  fprintf(OUT, "  trans_%s_maindata_rulesetexchange /*ruleset id exchange table*/\n", filename);
  fprintf(OUT, "};\n\n");
}

static void print_trans_symbols(const char *filename)
{
  int i;
  int count = count_symbols();

  fprintf(OUT, "const char *trans_%s_maindata_symbols[%d] = {\n", filename, count);
  for(i=0; i<count; ++i){
    fprintf(OUT, "  \"%s\"", lmn_id_to_name(i));
    if(i != count-1) fprintf(OUT, ",");
    fprintf(OUT, "\n");
  }
  fprintf(OUT, "};\n");

  fprintf(OUT, "int trans_%s_maindata_symbolexchange[%d];\n\n", filename, count);
}

static void print_trans_functors(const char *filename)
{
  int i;
  int count = lmn_functor_table.next_id;
  /* idは0から, next_idが1なら既に1個登録済み => count==next_id */

  fprintf(OUT, "struct LmnFunctorEntry trans_%s_maindata_functors[%d] = {\n", filename, count);
  for(i=0; i<count; ++i){
    fprintf(OUT,
            "  {%d, %d, %d, %d}",
            lmn_functor_table.entry[i].special,
            lmn_functor_table.entry[i].module,
            lmn_functor_table.entry[i].name,
            lmn_functor_table.entry[i].arity);
    if(i != count-1) fprintf(OUT, ",");
    fprintf(OUT, "\n");
  }
  fprintf(OUT, "};\n");

  fprintf(OUT, "int trans_%s_maindata_functorexchange[%d];\n\n", filename, count);
}

static void print_trans_rules(const char *filename)
{
  char *buf;
  int count, i, buf_len;

  count   = count_rulesets();
  buf_len = strlen(filename) + 50; /* 適当にこれだけあれば足りるはず */
  buf     = (char *)lmn_malloc(buf_len + 1);

  /* システムルールセットの出力 */
  snprintf(buf, buf_len, "trans_%s_1", filename);
  translate_ruleset(system_ruleset, buf);

  /* initial ruleset */
  snprintf(buf, buf_len, "trans_%s_2", filename);
  translate_ruleset(initial_ruleset, buf);

  /* initial systemruleset */
  snprintf(buf, buf_len, "trans_%s_3", filename);
  translate_ruleset(initial_system_ruleset, buf);

  /* 通常ルールセットの出力 */
  for(i=FIRST_ID_OF_NORMAL_RULESET; i<count; ++i){
    snprintf(buf, buf_len, "trans_%s_%d", filename, i);
    translate_ruleset(lmn_ruleset_from_id(i), buf);
  }

  free(buf);
}

static void print_trans_rulesets(const char *filename)
{
  int count, i;

  count = count_rulesets();

  /* ルールセットテーブルで各ルールセットのデータ名を参照するので、先に個々のデータを出力する */
  print_trans_rules(filename);

  fprintf(OUT, "struct trans_ruleset trans_%s_maindata_rulesets[%d] = {\n", filename, count);
  /* ruleset id is 2,3,4,5... ? 1:systemrulesetただし登録はされていない */
  /* ruleset0番は存在しないが数合わせに出力 */
  fprintf(OUT, "  {0,0},\n");
  /* ruleset1番はtableに登録されていないがsystemrulesetなので出力 */
  fprintf(OUT, "  {%d,trans_%s_1_rules},\n", lmn_ruleset_rule_num(system_ruleset), filename);
  /* ruleset2番,3番はinitial ruleset, initial system ruleset */
  fprintf(OUT, "  {%d,trans_%s_2_rules},\n", lmn_ruleset_rule_num(initial_ruleset), filename);
  fprintf(OUT, "  {%d,trans_%s_3_rules},\n", lmn_ruleset_rule_num(initial_system_ruleset), filename);
  /* 以降は普通のrulesetなので出力(どれが初期データルールかはload時に拾う) */
  for (i = FIRST_ID_OF_NORMAL_RULESET; i < count; i++) {
    LmnRuleSetRef rs = lmn_ruleset_from_id(i);
    LMN_ASSERT(rs); /* countで数えているからNULLにあたることはないはず */

    fprintf(OUT, "  {%d,trans_%s_%d_rules}", lmn_ruleset_rule_num(rs), filename, i);
    if(i != count - 1) {
      fprintf(OUT, ",");
    }
    fprintf(OUT, "\n");
  }
  fprintf(OUT, "};\n");

  fprintf(OUT, "int trans_%s_maindata_rulesetexchange[%d];\n\n", filename, count);
}

static int print_trans_module_f(st_data_t key, st_data_t value, st_data_t counter_p)
{
  lmn_interned_str m_key;
  LmnRuleSetRef       m_val;
  int             *m_counter;

  m_key     = (lmn_interned_str)key;
  m_val     = (LmnRuleSetRef)value;
  m_counter = (int *)counter_p; /* これが0なら最初の引数 */

  if (*m_counter > 0) {
    fprintf(OUT, "  ,");
  } else {
    fprintf(OUT, "   ");
  }

  fprintf(OUT, "{ %d, %d }\n", m_key, lmn_ruleset_get_id(m_val));

  ++*m_counter;
  return ST_CONTINUE;
}

static void print_trans_modules(const char *filename)
{
  extern st_table_t module_table;
  int count, counter;

  count   = count_modules();
  counter = 0;
  fprintf(OUT, "struct trans_module trans_%s_maindata_modules[%d] = {\n", filename, count);
  st_foreach(module_table, (st_iter_func)print_trans_module_f, (st_data_t)&counter);
  fprintf(OUT, "};\n\n");
}

static void print_trans_initfunction(const char *filename)
{
  fprintf(OUT, "void init_%s(void){\n", filename);

  /* fprintf(OUT, "  extern void helloworld(const char*);\n"); */
  /* fprintf(OUT, "  helloworld(\"%s\");\n", filename); */

  fprintf(OUT, "}\n\n");
}


void translate(char *filepath)
{
  char *filename;

  /* just for debug ! */
  /* OUT = stderr; */
  OUT = stdout;
  /* OUT = fopen("/dev/null", "w"); */

  if (filepath) {
    filename = create_formatted_basename(filepath);
  } else {
    filename = strdup("anonymous");
  }

  print_trans_header(filename);
  print_trans_symbols(filename);
  print_trans_functors(filename);
  print_trans_rulesets(filename);
  print_trans_modules(filename);
  print_trans_maindata(filename);
  print_trans_initfunction(filename);

  free(filename);
  if (OUT != stdout) fprintf(stderr, "--translate is under construction\n");
}
