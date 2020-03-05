/* sdsl - succinct data structures library
    Copyright (C) 2011-2013 Simon Gog

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see http://www.gnu.org/licenses/ .
*/
/*! \file rrr_helper.hpp
   \brief rrr_helper.hpp contains the sdsl::binomial struct,
          a struct which contains informations about the binomial coefficients
   \author Simon Gog, Matthias Petri, Stefan Arnold
*/
#ifndef SDSL_RRR_HELPER
#define SDSL_RRR_HELPER

#ifdef RRR_NO_OPT
#ifndef RRR_NO_BS
#define RRR_NO_BS
#endif
#endif

#include "bits.hpp"
#include "uint128_t.hpp"
#include "uint256_t.hpp"

namespace sdsl
{

//! Trait struct for the binomial coefficient struct to handle different type of integers.
/*! This generic implementation works for 64-bit integers.
 */
template<uint16_t log_n>
struct binomial_coefficients_trait {
    typedef uint64_t number_type;
    static inline uint16_t hi(number_type x) {
        return bits::hi(x);
    }

    //! Read a \f$len\f$-bit integer of type number_type from a bitvector.
    /*!
     * \param bv   A bit_vector of int_vector from which we extract the integer.
     * \param pos  Position of the least significant bit of the integer which should be read.
     * \param len  bit-width of the integer which should be read.
     * \return The len-bit integer.
     */
    template<class bit_vector_type>
    static inline number_type get_int(const bit_vector_type& bv,
                                      typename bit_vector_type::size_type pos,
                                      uint16_t len) {
        return bv.get_int(pos, len);
    }

    //! Write a \f$len\f$-bit integer x of type number_type to a bitvector.
    /*!
     * \param bv     A bit_vecor or int_vector in which we write the integer.
     * \param pos    Position of the least significant bit of the integer which should be written.
     * \param x    The integer x which should be written.
     * \param len  Bit-width of x.
     */
    template<class bit_vector_type>
    static void set_int(bit_vector_type& bv, typename bit_vector_type::size_type pos,
                        number_type x, uint16_t len) {
        bv.set_int(pos, x, len);
    }

    //! Count the number of set bits in x.
    /*!
     *  \param x The integer x.
     */
    static inline uint16_t popcount(number_type x) {
        return bits::cnt(x);
    }
};

//! Specialization of binomial_coefficients_trait for 128-bit integers.
template<>
struct binomial_coefficients_trait<7> {
    typedef uint128_t number_type;
    static inline uint16_t hi(number_type x) {
        if ((x >> 64)) {
            return bits::hi(x >> 64) + 64;
        } else {
            return bits::hi(x);
        }
    }

    template<class bit_vector_type>
    static inline number_type get_int(const bit_vector_type& bv,
                                      typename bit_vector_type::size_type pos,
                                      uint16_t len) {
        if (len <= 64) {
            return bv.get_int(pos, len);
        } else {
            return (static_cast<number_type>(bv.get_int(pos+64, len-64))<<64) + bv.get_int(pos, 64);
        }
    }

    template<class bit_vector_type>
    static void set_int(bit_vector_type& bv,
                        typename bit_vector_type::size_type pos,
                        number_type x, uint16_t len) {
        if (len <= 64) {
            bv.set_int(pos, x, len);
        } else {
            bv.set_int(pos, x, 64);
            bv.set_int(pos+64, x>>64, len-64);
        }
    }

    static inline uint16_t popcount(number_type x) {
        return bits::cnt(x >> 64) + bits::cnt(x);
    }
};

//! Specialization of binomial_coefficients_trait for 256-bit integers.
template<>
struct binomial_coefficients_trait<8> {
    typedef uint256_t number_type;
    static inline uint16_t hi(number_type x) {
        return x.hi();
    }

    template<class bit_vector_type>
    static inline number_type get_int(const bit_vector_type& bv,
                                      typename bit_vector_type::size_type pos,
                                      uint16_t len) {
        if (len <= 64) {
            return number_type(bv.get_int(pos, len));
        } else if (len <= 128) {
            return number_type(bv.get_int(pos, 64), bv.get_int(pos+64, len-64), 0);
        } else if (len <= 192) {
            return number_type(bv.get_int(pos, 64), bv.get_int(pos + 64, 64),
                               (uint128_t)bv.get_int(pos + 128, len-128));
        } else { // > 192
            return number_type(bv.get_int(pos, 64), bv.get_int(pos+64, 64),
                               (((uint128_t)bv.get_int(pos+192, len-192))<<64) | bv.get_int(pos+128, 64));
        }
    }

    template<class bit_vector_type>
    static void set_int(bit_vector_type& bv,
                        typename bit_vector_type::size_type pos,
                        number_type x,
                        uint16_t len) {
        if (len <= 64) {
            bv.set_int(pos, x, len);
        } else if (len <= 128) {
            bv.set_int(pos, x, 64);
            bv.set_int(pos+64, x>>64, len-64);
        } else if (len <= 192) {
            bv.set_int(pos, x, 64);
            bv.set_int(pos+64, x>>64, 64);
            bv.set_int(pos+128, x>>128, len-128);
        } else { // > 192
            bv.set_int(pos, x, 64);
            bv.set_int(pos+64, x>>64, 64);
            bv.set_int(pos+128, x>>128, 64);
            bv.set_int(pos+192, x>>192, len-192);
        }
    }

    static inline uint16_t popcount(number_type x) {
        return x.popcount();
    }
};

template<uint16_t n, class number_type>
struct binomial_table {
    static struct impl {
        number_type table[n+1][n+1];
        number_type table_tr[n+1][n]; // table[][] transposed, without the last column, for faster column acceess.

        impl() {
            for (uint16_t k=0; k <= n; ++k) {
                table[k][k] = 1;    // initialize diagonal
            }
            for (uint16_t k=0; k <= n; ++k) {
                table[0][k] = 0;    // initialize first row
            }
            for (uint16_t nn=0; nn <= n; ++nn) {
                table[nn][0] = 1;    // initialize first column
            }
            for (int nn=1; nn<=n; ++nn) {
                for (int k=1; k<=n; ++k) {
                    table[nn][k] = table[nn-1][k-1] + table[nn-1][k];
                }
            }
            for (int nn=0; nn<n; ++nn) {
                for (int k=0; k<=n; ++k) {
                    table_tr[k][nn] = table[nn][k];
                }
            }
        }
    } data;
};

template<uint16_t n, class number_type>
typename binomial_table<n,number_type>::impl binomial_table<n,number_type>::data;

//! A struct for the binomial coefficients \f$ n \choose k \f$.
/*!
 * data.table[m][k] contains the number \f${m \choose k}\f$ for \f$ k\leq m\leq \leq n\f$.
 * Size of data.table :
 *   Let \f$ maxlog = \lceil \log n \rceil \f$ and \f$ maxsize = 2^{maxlog} \f$
 *   then the tables requires \f$ maxsize^2\times \lceil n/8 rceil \f$ bytes space.
 *    Examples:
 *          n <=  64: 64*64*8 bytes    =   4 kB *  8 = 32 kB
 *     64 < n <= 128: 128*128*16 bytes =  16 kB * 16 = 256 kB
 *    128 < n <= 256: 256*256*32 bytes =  64 kB * 32 = 2048 kB = 2 MB
 *  The table is shared now for all n's in on of these ranges.
 *
 * data.space[k] returns the bits needed to encode a value between [0..data.table[n][k]], given n and k.
 * Size of data.space is  \f$ (n+1) \times \lceil n/8 \rceil \f$ bytes. E.g. 64*8=512 bytes for n=63,
 * 2kB for n=127, and 8kB for n=255.
 *
 * BINARY_SEARCH_THRESHOLD is equal to \f$ n/\lceil\log{n+1}\rceil \f$
 * \pre The template parameter n should be in the range [7..256].
 */
template<uint16_t n>
struct binomial_coefficients {
    enum {MAX_LOG = (n>128 ? 8 : (n > 64 ? 7 : 6))};
    static const uint16_t MAX_SIZE = (1 << MAX_LOG);
    typedef binomial_coefficients_trait<MAX_LOG> trait;
    typedef typename trait::number_type number_type;
    typedef binomial_table<MAX_SIZE,number_type> tBinom;

    static struct impl {
        const number_type(&table_tr)[MAX_SIZE+1][MAX_SIZE] = tBinom::data.table_tr;  // table for the binomial coefficients
        uint16_t space[n+1];    // for entry i,j \lceil \log( {i \choose j}+1 ) \rceil
#ifndef RRR_NO_BS
        static const uint16_t BINARY_SEARCH_THRESHOLD = n/MAX_LOG;
#else
        static const uint16_t BINARY_SEARCH_THRESHOLD = 0;
#endif
        // L1Mask contains a word with the n least significant bits set to 1.
        number_type L1Mask = (~(number_type)0) >> (sizeof(number_type) * 8 - n);

        impl() {
            static typename binomial_table<n,number_type>::impl tmp_data;
            for (int k=0; k<=n; ++k) {
                space[k] = (tmp_data.table[n][k] == 1ULL) ? 0 : trait::hi(tmp_data.table[n][k]) + 1;
            }
        }
    } data;
};

template<uint16_t n>
typename binomial_coefficients<n>::impl binomial_coefficients<n>::data;

//! Class to encode and decode binomial coefficients on the fly.
/*!
 * The basic encoding and decoding process is described in
 * Gonzalo Navarro and Eliana Providel: Fast, Small, Simple Rank/Select on Bitmaps, SEA 2012
 *
 * Implemented optimizations in the decoding process:
 *   - Constant time handling for uniform blocks (only zeros or ones in the block)
 *   - Constant time handling for blocks contains only a single one bit.
 *   - Decode blocks with at most \f$ k<n\log(n) \f$ by a binary search for the ones.
 *   - For operations decode_popcount, decode_select, and decode_bit a block
 *     is only decoded as long as the query is not answered yet.
 */
template<uint16_t n>
struct rrr_helper {
    typedef binomial_coefficients<n> binomial; //!< The struct containing the binomial coefficients
    typedef typename binomial::number_type number_type; //!< The used number type, e.g. uint64_t, uint128_t,...
    typedef typename binomial::trait trait; //!< The number trait

    //! Returns the space usage in bits of the binary representation of the number \f${n \choose k}\f$
    static inline uint16_t space_for_bt(uint16_t i) {
        return binomial::data.space[i];
    }

    template<class bit_vector_type>
    static inline number_type decode_btnr(const bit_vector_type& bv,
                                          typename bit_vector_type::size_type btnrp, uint16_t btnrlen) {
        return trait::get_int(bv, btnrp, btnrlen);
    }

    template<class bit_vector_type>
    static void set_bt(bit_vector_type& bv, typename bit_vector_type::size_type pos,
                       number_type bt, uint16_t space_for_bt) {
        trait::set_int(bv, pos, bt, space_for_bt);
    }

    template<class bit_vector_type>
    static inline uint16_t get_bt(const bit_vector_type& bv, typename bit_vector_type::size_type pos,
                                  uint16_t block_size) {
        return trait::popcount(trait::get_int(bv, pos, block_size));
    }

    static inline number_type bin_to_nr(number_type bin, uint16_t k) {
        if (!bin or bin == binomial::data.L1Mask) {  // handle special case
            return 0;
        }
        number_type nr = 0;
        assert(k == trait::popcount(bin));
        uint16_t  nn = n; // size of the block
        while (bin) {
            if (bin & 1ULL) {
                nr += binomial::data.table_tr[k][nn-1];
                --k; // go to the case (n-1, k-1)
            }// else go to the case (n-1, k)
            bin >>= 1;
            --nn;
        }
        return nr;
    }

    //! Decode the bit at position \f$ off \f$ of the block encoded by the pair (k, nr).
    static inline bool decode_bit(uint16_t k, number_type nr, uint16_t off) {
#ifndef RRR_NO_OPT
        assert(k != 0 && k != n); // this must have already been checked in the caller
        if (k == 1) { // if k==1 then the encoded block contains exactly one set bit
            return (n-static_cast<uint16_t>(nr)-1) == off; // position n-nr-1
        }
#endif
        uint16_t nn = n;
        // if k < n \log n, it is better to do a binary search for each of the on bits
        if (k+1 < binomial::data.BINARY_SEARCH_THRESHOLD+1) {
            while (k > 1) {
                uint16_t nn_lb = k, nn_rb = nn+1; // invariant nr >= binomial::data.table[nn_lb-1][k]
                while (nn_lb < nn_rb) {
                    uint16_t nn_mid = (nn_lb + nn_rb) / 2;
                    if (nr >= binomial::data.table_tr[k][nn_mid-1]) {
                        nn_lb = nn_mid+1;
                    } else {
                        nn_rb = nn_mid;
                    }
                }
                nn = nn_lb-1;
                if (n-nn >= off) {
                    return (n-nn) == off;
                }
                nr -= binomial::data.table_tr[k][nn-1];
                --k;
                --nn;
            }
        } else { // else do a linear decoding
            int i = 0;
            while (k > 1) {
                if (i > off) {
                    return 0;
                }
                if (nr >= binomial::data.table_tr[k][nn-1]) {
                    nr -= binomial::data.table_tr[k][nn-1];
                    --k;
                    if (i == off)
                        return 1;
                }
                --nn;
                ++i;
            }
        }
        return (n-static_cast<uint16_t>(nr)-1) == off;
    }

    //! Decode the len-bit integer starting at position \f$ off \f$ of the block encoded by the pair (k, nr).
    static inline uint64_t decode_int(uint16_t k, number_type nr, uint16_t off, uint16_t len) {
#ifndef RRR_NO_OPT
        assert(k != 0 && k != n); // this must have already been checked in the caller
        if (k == 1) { // if k==1 then the encoded block contains exactly one set bit
            uint16_t pos = (n-static_cast<uint16_t>(nr)-1);
            if (pos >= off and pos <= (off+len-1)) {
                return 1ULL << (pos-off);
            } else
                return 0;
        }
#endif
        uint64_t res = 0;
        uint16_t nn = n;
        int i = 0;
        while (k > 1) {
            if (i > off+len-1) {
                return res;
            }
            if (nr >= binomial::data.table_tr[k][nn-1]) {
                nr -= binomial::data.table_tr[k][nn-1];
                --k;
                if (i >= off)
                    res |= 1ULL << (i-off);
            }
            --nn;
            ++i;
        }
        uint16_t pos = (n-static_cast<uint16_t>(nr)-1);
        if (pos >= off and pos <= (off+len-1)) {
            res |= 1ULL << (pos-off);
        }
        return res;
    }


    //! Decode the first off bits bits of the block encoded by the pair (k, nr) and return the set bits.
    static inline uint16_t decode_popcount(uint16_t k, number_type nr, uint16_t off) {
#ifndef RRR_NO_OPT
        assert(k != 0 && k != n); // this must have already been checked in the caller
        if (k == 1) { // if k==1 then the encoded block contains exactly one set bit
            return (n-static_cast<uint16_t>(nr)-1) < off;
        }
#endif
        uint16_t result = 0;
        uint16_t nn = n;
        // if k < n \log n, it is better to do a binary search for each of the on bits
        if (k+1 < binomial::data.BINARY_SEARCH_THRESHOLD+1) {
            while (k > 1) {
                uint16_t nn_lb = k, nn_rb = nn+1; // invariant nr >= binomial::data.table[nn_lb-1][k]
                while (nn_lb < nn_rb) {
                    uint16_t nn_mid = (nn_lb + nn_rb) / 2;
                    if (nr >= binomial::data.table_tr[k][nn_mid-1]) {
                        nn_lb = nn_mid+1;
                    } else {
                        nn_rb = nn_mid;
                    }
                }
                nn = nn_lb-1;
                if (n-nn >= off) {
                    return result;
                }
                ++result;
                nr -= binomial::data.table_tr[k][nn-1];
                --k;
                --nn;
            }
        } else {
            int i = 0;
            while (k > 1) {
                if (i >= off) {
                    return result;
                }
                if (nr >= binomial::data.table_tr[k][nn-1]) {
                    nr -= binomial::data.table_tr[k][nn-1];
                    --k;
                    ++result;
                }
                --nn;
                ++i;
            }
        }
        return result + ((n-static_cast<uint16_t>(nr)-1) < off);
    }


    //! Decode the first off bits bits of the block encoded by the pair (k, nr) and return the set bits.
    static inline std::pair<bool, uint16_t>
    decode_bit_and_popcount(uint16_t k, number_type nr, uint16_t off) {
#ifndef RRR_NO_OPT
        assert(k != 0 && k != n); // this must have already been checked in the caller
        if (k == 1) { // if k==1 then the encoded block contains exactly one set bit
            uint16_t pos = (n-static_cast<uint16_t>(nr)-1);
            return std::make_pair(pos == off, pos < off);
        }
#endif
        uint16_t result = 0;
        uint16_t nn = n;
        // if k < n \log n, it is better to do a binary search for each of the on bits
        if (k+1 < binomial::data.BINARY_SEARCH_THRESHOLD+1) {
            while (k > 1) {
                uint16_t nn_lb = k, nn_rb = nn+1; // invariant nr >= binomial::data.table[nn_lb-1][k]
                while (nn_lb < nn_rb) {
                    uint16_t nn_mid = (nn_lb + nn_rb) / 2;
                    if (nr >= binomial::data.table_tr[k][nn_mid-1]) {
                        nn_lb = nn_mid+1;
                    } else {
                        nn_rb = nn_mid;
                    }
                }
                nn = nn_lb-1;
                if (n-nn >= off) {
                    return std::make_pair((n-nn) == off, result);
                }
                ++result;
                nr -= binomial::data.table_tr[k][nn-1];
                --k;
                --nn;
            }
        } else { // else do a linear decoding
            int i = 0;
            while (k > 1) {
                if (i > off) {
                    return std::make_pair(false, result);
                }
                if (nr >= binomial::data.table_tr[k][nn-1]) {
                    if (i == off)
                        return std::make_pair(true, result);
                    nr -= binomial::data.table_tr[k][nn-1];
                    --k;
                    ++result;
                }
                --nn;
                ++i;
            }
        }
        uint16_t pos = (n-static_cast<uint16_t>(nr)-1);
        return std::make_pair(pos == off, result + (pos < off));
    }


    /*! \pre k >= sel, sel>0
     */
    static inline uint16_t decode_select(uint16_t k, number_type nr, uint16_t sel) {
#ifndef RRR_NO_OPT
        assert(k != 0 && k != n); // this must have already been checked in the caller
        if (k == 1) {
            assert(sel == 1);
            return n-static_cast<uint16_t>(nr)-1;
        }
#endif
        uint16_t nn = n;
        // if k < n \log n, it is better to do a binary search for each of the on bits
        if (sel+1 < binomial::data.BINARY_SEARCH_THRESHOLD+1) {
            while (sel > 0) {
                uint16_t nn_lb = k, nn_rb = nn+1; // invariant nr >= iii.m_coefficients[nn_lb-1]
                while (nn_lb < nn_rb) {
                    uint16_t nn_mid = (nn_lb + nn_rb) / 2;
                    if (nr >= binomial::data.table_tr[k][nn_mid-1]) {
                        nn_lb = nn_mid+1;
                    } else {
                        nn_rb = nn_mid;
                    }
                }
                nn = nn_lb-1;
                nr -= binomial::data.table_tr[k][nn-1];
                --sel;
                --nn;
                --k;
            }
            return n-nn-1;
        } else {
            int i = 0;
            while (sel > 0) {   // TODO: this condition only work if the precondition holds
                if (nr >= binomial::data.table_tr[k][nn-1]) {
                    nr -= binomial::data.table_tr[k][nn-1];
                    --sel;
                    --k;
                }
                --nn;
                ++i;
            }
            return i-1;
        }
    }

    /*! \pre k >= sel, sel>0
     */
    static inline uint16_t decode_select0(uint16_t k, number_type nr, uint16_t sel) {
#ifndef RRR_NO_OPT
        assert(k != 0 && k != n); // this must have already been checked in the caller
        if (k == 1) {
            assert(sel < n);
            return sel-1 + (n-static_cast<uint16_t>(nr)-1 < sel);
        }
#endif
        uint16_t nn = n;
        while (sel > 0) {   // TODO: this condition only work if the precondition holds
            if (nr >= binomial::data.table_tr[k][nn-1]) {
                nr -= binomial::data.table_tr[k][nn-1];
                // a one is decoded
                --k;
            } else {
                // a zero is decoded
                --sel;
            }
            --nn;
        }
        return n-nn-1; // return the starting position of $sel$th occurence of the pattern
    }

    /*! \pre k >= sel, sel>0
     */
    template<uint8_t pattern, uint8_t len>
    static inline uint16_t decode_select_bitpattern(uint16_t k, number_type nr, uint16_t sel) {
        uint8_t decoded_pattern = 0;
        uint8_t decoded_len     = 0;
        uint16_t nn = n;
        while (sel > 0) {   // TODO: this condition only work if the precondition holds
            decoded_pattern = decoded_pattern<<1;
            ++decoded_len;
            if (nr >= binomial::data.table_tr[k][nn-1]) {
                nr -= binomial::data.table_tr[k][nn-1];
                // a one is decoded
                decoded_pattern |= 1; // add to the pattern
                --k;
            }
            --nn;
            if (decoded_len == len) {  // if decoded pattern length equals len of the searched pattern
                if (decoded_pattern == pattern) {  // and pattern equals the searched pattern
                    --sel;
                }
                decoded_pattern = 0; decoded_len = 0; // reset pattern
            }
        }
        return n-nn-len; // return the starting position of $sel$th occurence of the pattern
    }

};

} // end namespace
#endif
