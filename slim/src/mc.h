#ifndef MC_H
#define MC_H

#include "st.h"
#include "internal_hash.h"
#include "membrane.h"
#include "vector.h"

/**
 * task.cのinterpret()で用いられるフラグを集めたもの
 *
 * nd_exec: 非決定的実行フラグ
 *   初期化ルール適用以前：FALSE
 *   初期化ルール適用以降：TRUE
 * system_rule_committed: ボディ実行中フラグ
 *   ボディ実行中のみTRUE (i.e. ルールのボディ部冒頭のCOMMIT命令の処理開始時にTRUEとなり、PROCEED命令の処理終了時にFALSEになる)
 *   左辺に出現するPROCEED（GROUP終了を表す）と右辺に出現するPROCEEDを区別するために使用
 * system_rule_proceeded: システムルール適用成功フラグ
 *   システムルール適用成功時：TRUE
 *   システムルール実行時はinterpret()が常にFALSEを返す仕様となっているため、システムルール適用成功を表すフラグとして代わりにこれを用いる
 * property_rule: 性質ルール実行中フラグ
 *   性質ルール適用成功時にTRUEを返す目的で使用
 */
typedef struct McFlags {
  BOOL nd_exec;
  BOOL system_rule_committed;
  BOOL system_rule_proceeded;
  BOOL property_rule;
} McFlags;

typedef struct State State;

struct State {
  LmnMembrane *mem; /* グローバルルート膜 */
  int hash;         /* ハッシュ値 */
  BOOL flags;       /* flags (unsigned char) */
  Vector successor; /* successor nodes */
};

LMN_EXTERN State *state_make(LmnMembrane *mem);
LMN_EXTERN inline void state_succ_init(State *s, int init_size);
LMN_EXTERN void state_free(State *s);

/* flag of the first DFS (nested DFS, on-stack state) */
#define FST_MASK (0x01U)
/* flag of the second DFS (nested DFS, visited state) */
#define SND_MASK (0x02U)
/* macros for nested DFS */
#define set_fst(S)    ((S)->flags |= FST_MASK)
#define unset_fst(S)  ((S)->flags &= (~FST_MASK))
#define is_fst(S)     ((S)->flags & FST_MASK)
#define set_snd(S)    ((S)->flags |= SND_MASK)
#define unset_snd(S)  ((S)->flags &= (~SND_MASK))
#define is_snd(S)     ((S)->flags & SND_MASK)

LMN_EXTERN int state_hash(LmnWord s);
LMN_EXTERN int state_cmp(HashKeyType s1, HashKeyType s2);

LMN_EXTERN inline void activate_ancestors(LmnMembrane *mem);

#endif