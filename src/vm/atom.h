/*
 * atom.h
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
 * $Id: atom.h,v 1.8 2008/09/19 05:18:17 taisuke Exp $
 */

#ifndef LMN_ATOM_H
#define LMN_ATOM_H

/* cldoc:begin-category(Lmntal::Atom) */

#include "lmntal.h"
#include "special_atom.h"
#include "functor.h"
#include "element/element.h"
#include "symbol.h"
#include "hyperlink.h"

/** Atom Structure
 *
 *  * Atom
 *      1st Word      : アトムリストにおけるprevポインタ
 *      2nd Word      : アトムリストにおけるnextポインタ
 *      3rd Word      : アトムと膜に割り当てる一意な整数ID
 *      aligned Word  : 以下のByte要素の合計をWordサイズへアラインメント
 *        next 2 Bytes: アトムの種類(アトム名とリンク数の組)を表すfunctorに対応した整数ID
 *        N Bytes     : リンク属性   (1 Byte * N本)
 *      N Words       : リンクデータ (1 Word * N本)
 *    (Nはリンクの数)
 *
 *  * Link Attribute
 *     リンク属性は, 先頭1ビットが立っていない場合は, 下位7bitが接続先リンクの番号を記録しており,
 *                先頭1ビットが立っている場合は, Primitiveデータの種類を記録する。
 *     [Link Number]  0-------
 *     [int]          1000 0000
 *     [double]       1000 0001
 *     [special]      1000 0011
 *     [string]       1000 0011
 *     [const string] 1000 0100
 *     [const double] 1000 0101
 *     [hyper link]   1000 1010
 *
 *     We are going to support some primitive data types.
 *     (signed/unsigned) int, short int, long int, byte, long long int,
 *     float, double, long double,
 *     bool, string, character,
 *     ground array, ground with membrane array, primitive arrays
 *
 *     But, incompletely-specified.
 *
 */


/* プロキシの3番目の引数番号の領域を remove_proxy, insert_proxyで利用中。
 * 所属する膜へのポインタを持っている */

#define LMN_ATOM_ATTR(X)                ((LmnLinkAttr)(X))
#define LMN_ATTR_BYTES                  (sizeof(LmnLinkAttr))
#define LMN_ATTR_MASK                   (0x7fU)
#define LMN_ATTR_FLAG                   (0x80U)

#define LMN_ATOM(X)                     ((LmnAtom)(X))
#define LMN_SATOM(X)                    ((LmnSAtom)(X))

/* アトムリストからATOMのprev/nextアトムを取得/設定する.
 * アトムリストから履歴アトムを読み飛ばさないので, 呼び出し側で適宜なんとかする */
#define LMN_SATOM_PPREV(ATOM)           (((LmnWord *)(ATOM)))
#define LMN_SATOM_PNEXT(ATOM)           (((LmnWord *)(ATOM)) + 1)
#define LMN_SATOM_GET_PREV(ATOM)        (LMN_SATOM(*LMN_SATOM_PPREV(LMN_SATOM(ATOM))))
#define LMN_SATOM_SET_PREV(ATOM, X)     (*LMN_SATOM_PPREV(LMN_SATOM(ATOM)) = LMN_ATOM((X)))
#define LMN_SATOM_GET_NEXT_RAW(ATOM)    (LMN_SATOM(*LMN_SATOM_PNEXT(LMN_SATOM(ATOM))))
#define LMN_SATOM_SET_NEXT(ATOM, X)     (*LMN_SATOM_PNEXT(LMN_SATOM(ATOM)) = LMN_ATOM((X)))

/* リンク番号のタグのワード数。ファンクタと同じワードにある分は数えない */
#define LMN_ATTR_WORDS(ARITY)           (((ARITY + (LMN_WORD_BYTES - 1))) >> LMN_WORD_SHIFT)

/* ファンクタIDの取得/設定, ファンクタIDからリンク数の取得のユーティリティ（プロキシはリンク1本分余分にデータ領域があるので分岐する） */

/* アトムATOMのプロセスIDを取得/設定 */
#define LMN_SATOM_ID(ATOM)              (*(((LmnWord *)(ATOM)) + 2))
#define LMN_SATOM_SET_ID(ATOM, ID)      (LMN_SATOM_ID(ATOM) = (ID))
#define LMN_FUNCTOR_SHIFT               (3)
#define LMN_LINK_SHIFT                  (4)

#define LMN_SATOM_GET_FUNCTOR(ATOM)     LMN_FUNCTOR(*(((LmnWord *)LMN_SATOM(ATOM)) + LMN_FUNCTOR_SHIFT))
#define LMN_SATOM_SET_FUNCTOR(ATOM, X)  (*(LmnFunctor*)((LmnWord*)(ATOM) + LMN_FUNCTOR_SHIFT) = (X))
#define LMN_SATOM_GET_ARITY(ATOM)       (LMN_FUNCTOR_ARITY(LMN_SATOM_GET_FUNCTOR(LMN_SATOM(ATOM))))
#define LMN_FUNCTOR_GET_LINK_NUM(F)     ((LMN_FUNCTOR_ARITY(F)) - (LMN_IS_PROXY_FUNCTOR(F) ? 1U : 0U))
#define LMN_SATOM_GET_LINK_NUM(ATOM)    (LMN_FUNCTOR_GET_LINK_NUM(LMN_SATOM_GET_FUNCTOR(LMN_SATOM(ATOM))))

/* アトムATOMのN番目のリンク属性/リンクデータを取得 */
#define LMN_SATOM_PATTR(ATOM, N)        ((LmnLinkAttr *)(((BYTE *)(((LmnWord *)(ATOM)) + LMN_LINK_SHIFT)) + (N) * LMN_ATTR_BYTES))
#define LMN_SATOM_PLINK(ATOM,N)         (((LmnWord *)(ATOM)) + LMN_LINK_SHIFT + LMN_ATTR_WORDS(LMN_SATOM_GET_ARITY(ATOM)) + (N))
#define LMN_SATOM_GET_ATTR(ATOM, N)     (*LMN_SATOM_PATTR(LMN_SATOM(ATOM), N))
#define LMN_SATOM_SET_ATTR(ATOM, N, X)  ((*LMN_SATOM_PATTR(LMN_SATOM(ATOM), N)) = (X))
#define LMN_SATOM_GET_LINK(ATOM, N)     (LMN_ATOM(*LMN_SATOM_PLINK(LMN_SATOM(ATOM), N)))
#define LMN_SATOM_SET_LINK(ATOM, N, X)  (*LMN_SATOM_PLINK(LMN_SATOM(ATOM), N) = (LmnWord)(X))
#define LMN_HLATOM_SET_LINK(ATOM, X)    (LMN_SATOM_SET_LINK(ATOM, 0, X))

/* word size of atom の加算は prev, next, id, functorのワード */
#define LMN_SATOM_WORDS(ARITY)          (LMN_LINK_SHIFT + LMN_ATTR_WORDS(ARITY) + (ARITY))

/* リンク属性ATTRであるアトムATOMのファンクタがFUNCならばTRUEを返す */
#define LMN_HAS_FUNCTOR(ATOM, ATTR, FUNC) \
          (LMN_ATTR_IS_DATA(ATTR) ? FALSE \
                                  : LMN_SATOM_GET_FUNCTOR(LMN_SATOM(ATOM)) == (FUNC))

/* operations for link attribute */
#define LMN_ATTR_IS_DATA(X)             ((X) & ~LMN_ATTR_MASK)
/* make data/link link attribute from value */
#define LMN_ATTR_MAKE_DATA(X)           (0x80U | (X))
#define LMN_ATTR_MAKE_LINK(X)           (X)
/* get link attribute value (remove tag) */
#define LMN_ATTR_GET_VALUE(X)           ((X) & LMN_ATTR_MASK)
/* set link attribute value. Tag is not changed. */
#define LMN_ATTR_SET_VALUE(PATTR, X)    (*(PATTR) = ((((X) & ~LMN_ATTR_MASK)) | X))

/* get/set membrane of proxy */
#define LMN_SATOM_IS_PROXY(ATOM)        (LMN_IS_PROXY_FUNCTOR(LMN_SATOM_GET_FUNCTOR((ATOM))))
#define LMN_PROXY_GET_MEM(PROXY_ATM)    ((LmnMembraneRef)LMN_SATOM_GET_LINK((PROXY_ATM), 2))
#define LMN_PROXY_SET_MEM(PROXY_ATM, X) LMN_SATOM_SET_LINK((PROXY_ATM), 2, (LmnWord)(X))
#define LMN_PROXY_FUNCTOR_NUM           (3)
#define LMN_IS_PROXY_FUNCTOR(FUNC)      ((FUNC) < LMN_PROXY_FUNCTOR_NUM)
#define LMN_IS_SYMBOL_FUNCTOR(FUNC)     ((FUNC) >= LMN_PROXY_FUNCTOR_NUM)

#define LMN_SATOM_STR(ATOM)             LMN_SYMBOL_STR(LMN_FUNCTOR_NAME_ID(LMN_SATOM_GET_FUNCTOR(LMN_SATOM(ATOM))))
#define LMN_FUNCTOR_STR(F)              LMN_SYMBOL_STR(LMN_FUNCTOR_NAME_ID(F))

/* operations for extended atom */
#define LMN_ATTR_IS_DATA_WITHOUT_EX(ATTR)  (LMN_ATTR_IS_DATA(ATTR) \
                                              && !LMN_ATTR_IS_HL(ATTR) )
#define LMN_ATTR_IS_EX(ATTR)               (LMN_ATTR_IS_DATA(ATTR) \
                                              && LMN_ATTR_IS_HL(ATTR) )
#define LMN_IS_EX_FUNCTOR(FUNC)            ((FUNC) == LMN_HL_FUNC)

/* link attribute of primitive data type */
/* low 7 bits of link attribute */

#define LMN_INT_ATTR                    (LMN_ATTR_FLAG | 0x00U)
#define LMN_DBL_ATTR                    (LMN_ATTR_FLAG | 0x01U)
#define LMN_SP_ATOM_ATTR                (LMN_ATTR_FLAG | 0x03U)
#define LMN_STRING_ATTR                 LMN_SP_ATOM_ATTR
/* 定数アトム */
#define LMN_CONST_STR_ATTR              (LMN_ATTR_FLAG | 0x04U)
#define LMN_CONST_DBL_ATTR              (LMN_ATTR_FLAG | 0x05U)
/* ハイパーリンクアトム (⊂ extended atom ⊂ data atom ⊂ unary) 
ハイパーリンクアトムはプロキシと同様シンボルアトムとしても扱われることに注意 */
#define LMN_HL_ATTR                     (LMN_ATTR_FLAG | 0x0aU)

/*----------------------------------------------------------------------
 * allocation
 */

LmnSAtom lmn_new_atom(LmnFunctor f);
void lmn_delete_atom(LmnSAtom ap);
void free_atom_memory_pools(void);



/*----------------------------------------------------------------------
 * functions
 */
void mpool_init(void);
LmnAtom lmn_copy_atom(LmnAtom atom, LmnLinkAttr attr);
LmnSAtom lmn_copy_satom(LmnSAtom atom);
LmnAtom lmn_copy_data_atom(LmnAtom atom, LmnLinkAttr attr);
LmnSAtom lmn_copy_satom_with_data(LmnSAtom atom, BOOL is_new_hl);
void lmn_free_atom(LmnAtom atom, LmnLinkAttr attr);
void free_symbol_atom_with_buddy_data(LmnSAtom atom);
BOOL lmn_eq_func(LmnAtom atom0, LmnLinkAttr attr0,
                 LmnAtom atom1,LmnLinkAttr attr1);
BOOL lmn_data_atom_is_ground(LmnAtom atom, LmnLinkAttr attr,
                             ProcessTableRef *hlinks);
BOOL lmn_data_atom_eq(LmnAtom atom1, LmnLinkAttr attr1,
                      LmnAtom atom2, LmnLinkAttr attr2);
double lmn_get_double(LmnAtom atom);
LmnAtom lmn_create_double_atom(double d);
void lmn_destroy_double_atom(LmnAtom atom);

#ifdef LMN_DOUBLE_IS_IMMEDIATE
# define LMN_GETREF_DOUBLE(Atom) ((double *)&Atom)
#else
# define LMN_GETREF_DOUBLE(Atom) ((double *)Atom)
#endif

#define LMN_COPY_DBL_ATOM(Dst, Src)                                            \
  do {                                                                         \
    (Dst) = (LmnWord)lmn_create_double_atom(lmn_get_double(Src));              \
  } while (0)


/* cldoc:end-category() */

#endif /* LMN_ATOM_H */

