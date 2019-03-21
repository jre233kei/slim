/*
 * vector.c
 *
 *   Copyright (c) 2008, Ueda Laboratory LMNtal Group
 * <lmntal@ueda.info.waseda.ac.jp> All rights reserved.
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
 * $Id: vector.c,v 1.8 2008/09/19 05:18:17 taisuke Exp $
 */

#include "vector.h"


Vector::Vector(){
  tbl = (LmnWord *)NULL;
}
Vector::Vector(unsigned int init_size){
  LMN_ASSERT(init_size > 0);
  init(init_size);
}
template <class T> Vector::Vector(const std::vector<T> &v){
  static_assert(std::is_scalar<T>::value && sizeof(T) <= sizeof(LmnWord),
              "vector elements must be scalars.");
  LMN_ASSERT(v.size() > 0);
  init(v.size());
  memcpy(tbl, v.data(), sizeof(T) * v.size());
  num = v.size();
}
Vector::Vector(const Vector &vec){
  int i;
  init(vec.get_num() > 0 ? vec.get_num() : 1);

  for (i = 0; i < vec.get_num(); i++) {
    this->tbl[i] = vec.tbl[i];
  }
  this->num = vec.get_num();
}
Vector::~Vector(){
  if(tbl != NULL){
    LMN_FREE(tbl);
  }
}
void Vector::init(unsigned int init_size){
  tbl = LMN_NALLOC(LmnWord, init_size);
  num = 0;
  cap = init_size;
}
void Vector::extend(){
  cap *= 2;
  tbl = LMN_REALLOC(LmnWord, tbl, cap);
}
unsigned int Vector::get_num() const{
  return num;
}
unsigned int Vector::get_cap() const{
  return cap;
}
bool Vector::is_empty() const{
  return num==0;
}
void Vector::push(LmnWord keyp){
  if (num == cap) {
    extend();
  }
  (tbl)[num] = keyp;
  num++;
}
void Vector::reduce(){
  cap /= 2;
  tbl = LMN_REALLOC(LmnWord, tbl, cap);
}
LmnWord Vector::pop(){
  LmnWord ret;
  LMN_ASSERT(num > 0);
  /* Stackとして利用する場合, reallocが頻繁に発生してしまう.
   * Stackなのでサイズの増減は頻繁に発生するものだが,
   * 頻繁なreallocはパフォーマンスに影響する.
   * >>とりあえず<< サイズの下限値を設ける. LmnStackなる構造を別に作るべきかも.
   * (gocho) */
  if (num <= cap / 2 && cap > 1024) {
    reduce();
  }
  ret = get(num - 1);
  num--;
  return ret;
}
//pop Nth element
LmnWord Vector::pop_n(unsigned int n){
  unsigned int i;
  LmnWord ret;
  LMN_ASSERT(num > 0 && n >= 0 && n < num);

  if (num <= cap / 2) {
    reduce();
  }
  ret = get(n);
  for (i = n; i < num - 1; ++i) {
    set(i, get(i + 1));
  }
  num--;
  return ret;
}
LmnWord Vector::peek() const{
  return get(num - 1);
}
void Vector::set(unsigned int index, LmnWord keyp){
  LMN_ASSERT(index < cap);
  tbl[index] = keyp;    
}
LmnWord Vector::get(unsigned int index) const {
  LMN_ASSERT(index < num);
  return (tbl[index]);
}
LmnWord Vector::last() const{
  return tbl[num-1];
}
/* pop all elements from vec */
void Vector::clear(){
  num = 0;
}
void Vector::destroy(){
  LMN_FREE(tbl);
  tbl = (LmnWord *)NULL;
}
unsigned long space_inner() const{
  return get_cap() * sizeof(vec_data_t);
}
BOOL Vector::contains(LmnWord keyp) const{
  unsigned int i = 0;
  while (i < get_num()) {
    if (get(i++) == keyp) {
      return TRUE;
    }
  }
  return FALSE;
}
void Vector::reverse(){ /* Vectorに詰んだ要素を逆順に並べ直す */
  unsigned int r, l;
  r = 0;
  l = get_num() - 1;

  while (r < l) {
    vec_data_t tmp = tbl[r];
    set(r, tbl[l]);
    set(l, tmp);
    r++;
    l--;
  }
}
void Vector::resize(unsigned int size, vec_data_t val){ 
/* ベクタのサイズを size に変更し、新規に追加された項目を val に設定する*/
  unsigned int i;
  while (size > cap) {
    extend();
  }
   /* 追加された項目を val に設定 */
  for (i = num; i < size; i++) {
    tbl[i] = val;
  }
  num = size;
}
void Vector::sort(int (*compare)(const void *, const void *)){
  qsort(tbl, num, sizeof(vec_data_t), compare);
}