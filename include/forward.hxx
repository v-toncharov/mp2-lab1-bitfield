#pragma once
#include "bitspan_word.hxx"

struct indices;

template<bitspan_word W = default_bitspan_word> struct bitspan;
template<bitspan_word W = default_bitspan_word> struct bitspan_words;
template<bool in, bitspan_word W = default_bitspan_word>
    requires (!std::is_const_v<W>) struct bitspan_iter;
template<bitspan_word W = default_bitspan_word>
    requires (!std::is_const_v<W>) struct bitvec;
template<bool mut, bitspan_word W = default_bitspan_word>
    requires (!std::is_const_v<W>) struct bitvec_words;
