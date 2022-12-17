/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/string/fast_string_concatenator.hpp
 *
 * Purpose:     Fast string concatenator.
 *
 * Created:     4th November 2003 (the time added to STLSoft libraries)
 * Updated:     17th December 2022
 *
 * Thanks to:   Sean Kelly for picking up on my gratuitous use of pointers
 *              in the first implementation.
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2019-2021, Matthew Wilson and Synesis Information Systems
 * Copyright (c) 2003-2019, Matthew Wilson and Synesis Software
 * Copyright (c) 2022, wiluite
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * - Neither the name(s) of Matthew Wilson and Synesis Information Systems
 *   nor the names of any contributors may be used to endorse or promote
 *   products derived from this software without specific prior written
 *   permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 * IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ////////////////////////////////////////////////////////////////////// */


#ifndef STLSOFT_INCL_STLSOFT_STRING_HPP_FAST_STRING_CONCATENATOR
#define STLSOFT_INCL_STLSOFT_STRING_HPP_FAST_STRING_CONCATENATOR
#include <cassert>
#include <cstring>
#include <memory>
#include "short_alloc.h"

namespace stlsoft
{
    template<   class S
            ,   class C
    >
    class fast_string_concatenator;

    template<   class S
            ,   class C = typename S::value_type
    >
    using fast_string_concatenator_sptr = std::shared_ptr<fast_string_concatenator<S,C>>;

    template <class S, std::size_t N = 50>
    constexpr const std::size_t concat_alloc_size = sizeof(fast_string_concatenator<S,typename S::value_type>) * N;

    template<class S>
    using concat_allocator = short_alloc<fast_string_concatenator<S, typename S::value_type>, concat_alloc_size<S>>;

    template<typename S>
    using concat_arena = typename concat_allocator<S>::arena_type;

    template<   class S
    >
    struct concat_ptr_and_alloc
    {
        fast_string_concatenator_sptr<S> concat_ptr;
        concat_allocator<S> alloc;
        operator S() const
        {
            return concat_ptr->operator S();
        }
    };
} /* namespace stlsoft */


namespace stlsoft
{
    class fsc_seed
    {};

    template <class S>
    class fsc_seed_t
            : public fsc_seed
    {
    public:
        typedef S   string_type;
    };

    template <class T>
    class fsc_safe_seed
            : public fsc_seed
    {
        concat_arena<T> & arena_;
    public:
        explicit fsc_safe_seed(concat_arena<T> & arena) : arena_(arena) {}
        auto& get_arena() const
        {
            return arena_;
        }
    };


/** Expression template class which provides fast string concatenation
 *
 * \ingroup group__library__String
 */
    template<   class S
            ,   class C = typename S::value_type
    >
    class fast_string_concatenator
    {
/// \name Member types
/// @{
    public:
        typedef S                                   string_type;
        typedef C                                   char_type;
        typedef fast_string_concatenator<S, C>      class_type;
        using sptr_class_type = fast_string_concatenator_sptr<S,C>;
        typedef std::size_t                         size_type;
    private:
        typedef typename S::iterator      string_iterator_type;
/// @}

/// \name Construction
/// @{
    public:
        fast_string_concatenator(string_type const& lhs, string_type const& rhs);
        fast_string_concatenator(string_type const& lhs, char_type const* rhs);
        fast_string_concatenator(string_type const& lhs, char_type rhs);
        fast_string_concatenator(char_type const* lhs, string_type const& rhs);
        fast_string_concatenator(char_type lhs, string_type const& rhs);
        fast_string_concatenator(class_type const& lhs, string_type const& rhs);
        fast_string_concatenator(sptr_class_type const& lhs, string_type const& rhs);
        fast_string_concatenator(class_type const& lhs, char_type const* rhs);
        fast_string_concatenator(sptr_class_type const& lhs, char_type const* rhs);
        fast_string_concatenator(class_type const& lhs, char_type rhs);
        fast_string_concatenator(sptr_class_type const& lhs, char_type /*const*/ rhs);
        fast_string_concatenator(fsc_seed const& lhs, string_type const& rhs);
        fast_string_concatenator(fsc_seed const& lhs, char rhs) : m_lhs(lhs), m_rhs(rhs){}

        // These constructors are for handling embedded braces in the concatenation sequences, and represent the pathological case
        fast_string_concatenator(class_type const& lhs, class_type const& rhs);
        fast_string_concatenator(string_type const& lhs, class_type const& rhs);
        fast_string_concatenator(char_type const* lhs, class_type const& rhs);
        fast_string_concatenator(char_type lhs, class_type const& rhs);
/// @}

/// \name Accessors
/// @{
    public:
        operator S() const;
/// @}

/// \name Implementation
/// @{
    private:
        [[nodiscard]] size_type length() const
        {
            return m_lhs.length() + m_rhs.length();
        }

        string_iterator_type write(string_iterator_type s) const
        {
            return m_rhs.write(m_lhs.write(s));
        }

    private:
        struct Data;

        friend struct Data;

        struct Data
        {
            enum DataType
            {
                seed    // Argument was the seed type
                ,   single  // Argument was a single character
                ,   cstring // Argument was a C-string or a string object
                ,   concat  // Argument was another concatenator
                ,   concat_ptr
            };

            /// Represents a C-style string
            struct CString
            {
                std::size_t         len;
                char_type const     *s;
            };
            struct DataRef
            {
                union
                {
                    CString             cstring;
                    char_type           ch;
                    class_type  const   *concat;
                } u;
                sptr_class_type  concat_ptr;
            };

            explicit Data(string_type const& str)
                    : type(cstring)
            {
                ref.u.cstring.len = str.length();
                ref.u.cstring.s   = str.data();
            }
            explicit Data(char_type const* s)
                    : type(cstring)
            {
                ref.u.cstring.len = strlen(s);
                ref.u.cstring.s   = s;
            }
            explicit Data(char_type const ch)
                    : type(single)
            {
                ref.u.ch = ch;
            }

            explicit Data(class_type const & fc)
                    : type(concat)
            {
                ref.u.concat = &fc;
            }
            explicit Data(fsc_seed const&)
                    : type(seed)
            {}
            explicit Data(sptr_class_type const & fc)
                    : type(concat_ptr)
            {
                ref.concat_ptr = fc;
            }

            [[nodiscard]] size_type length() const
            {
                size_type  len;

                // Note that a default is not used in the switch statement because, even on very high
                // optimisations, it caused a 1-4% hit on most of the compilers
                assert(type == cstring || type == single || type == concat || type == seed || type == concat_ptr);

                switch(type)
                {
                    case    seed:
                        len = 0;
                        break;
                    case    single:
                        len = 1;
                        break;
                    case    cstring:
                        len = ref.u.cstring.len;
                        break;
                    case    concat:
                        len = ref.u.concat->length();
                        break;
                    case    concat_ptr:
                        len = ref.concat_ptr->length();
                        break;
                }

                assert(!(len < 0));

                return len;
            }

            [[nodiscard]] auto write(string_iterator_type s) const
            {
                size_type  len;

                // Note that a default is not used in the switch statement because, even on very high
                // optimisations, it caused a 1-4% hit on most of the compilers
                //STLSOFT_ASSERT(type == cstring || type == single || type == concat || type == seed);
                assert(type == cstring || type == single || type == concat || type == seed || type == concat_ptr);

                // Check that iterator is random access
                assert(std::addressof(s[1]) == &*(s + 1));

                switch(type)
                {
                    case    seed:
                        break;
                    case    single:
                        *(s++) = ref.u.ch;
                        break;
                    case    cstring:
                        len = ref.u.cstring.len;
                        std::copy(&ref.u.cstring.s[0], &ref.u.cstring.s[0] + len, s);
                        s += len;
                        break;
                    case    concat:
                        s = ref.u.concat->write(s);
                        break;
                    case    concat_ptr:
                        s = ref.concat_ptr->write(s);
                        break;
                }

                return s;
            }

            DataRef         ref;
            DataType const  type;
        };
/// @}
/// \name Construction
/// @{
    private:
        Data    m_lhs;
        Data    m_rhs;
/// @}

// Not to be implemented
    public:
        fast_string_concatenator& operator =(class_type const&) = delete;
    };


/* /////////////////////////////////////////////////////////////////////////
 * implementation
 */

    template<   class S
            ,   class C
    >
    inline fast_string_concatenator<S, C>::fast_string_concatenator(S const& lhs, S const& rhs)
            : m_lhs(lhs)
            , m_rhs(rhs)
    {}

    template<   class S
            ,   class C
    >
    inline fast_string_concatenator<S, C>::fast_string_concatenator(S const& lhs, C const* rhs)
            : m_lhs(lhs)
            , m_rhs(rhs)
    {}

    template<   class S
            ,   class C
    >
    inline fast_string_concatenator<S, C>::fast_string_concatenator(S const& lhs, C const rhs)
            : m_lhs(lhs)
            , m_rhs(rhs)
    {}

    template<   class S
            ,   class C
    >
    inline fast_string_concatenator<S, C>::fast_string_concatenator(C const* lhs, S const& rhs)
            : m_lhs(lhs)
            , m_rhs(rhs)
    {}

    template<   class S
            ,   class C
    >
    inline fast_string_concatenator<S, C>::fast_string_concatenator(C const lhs, S const& rhs)
            : m_lhs(lhs)
            , m_rhs(rhs)
    {}

    template<   class S
            ,   class C
    >
    inline fast_string_concatenator<S, C>::fast_string_concatenator(fast_string_concatenator const& lhs, S const& rhs)
            : m_lhs(lhs)
            , m_rhs(rhs)
    {}

    template<   class S
            ,   class C
    >
    inline fast_string_concatenator<S, C>::fast_string_concatenator(sptr_class_type const& lhs, string_type const& rhs)
            : m_lhs(lhs)
            , m_rhs(rhs)
    {}

    template<   class S
            ,   class C
    >
    inline fast_string_concatenator<S, C>::fast_string_concatenator(fast_string_concatenator const& lhs, C const* rhs)
            : m_lhs(lhs)
            , m_rhs(rhs)
    {}
    template<   class S
            ,   class C
    >
    inline fast_string_concatenator<S, C>::fast_string_concatenator(sptr_class_type const& lhs, char_type const* rhs)
            : m_lhs(lhs)
            , m_rhs(rhs)
    {}

    template<   class S
            ,   class C
    >
    inline fast_string_concatenator<S, C>::fast_string_concatenator(fast_string_concatenator const& lhs, C const rhs)
            : m_lhs(lhs)
            , m_rhs(rhs)
    {}

    template<   class S
            ,   class C
    >
    inline fast_string_concatenator<S, C>::fast_string_concatenator(sptr_class_type const& lhs, char_type const rhs)
            : m_lhs(lhs)
            , m_rhs(rhs)
    {}

// These constructors are for handling embedded braces in the concatenation sequences, and represent the pathological case
    template<   class S
            ,   class C
    >
    inline fast_string_concatenator<S, C>::fast_string_concatenator(fast_string_concatenator const& lhs, fast_string_concatenator const& rhs)
            : m_lhs(lhs)
            , m_rhs(rhs)
    {}

    template<   class S
            ,   class C
    >
    inline fast_string_concatenator<S, C>::fast_string_concatenator(S const& lhs, fast_string_concatenator const& rhs)
            : m_lhs(lhs)
            , m_rhs(rhs)
    {}

    template<   class S
            ,   class C
    >
    inline fast_string_concatenator<S, C>::fast_string_concatenator(C const* lhs, fast_string_concatenator const& rhs)
            : m_lhs(lhs)
            , m_rhs(rhs)
    {}

    template<   class S
            ,   class C
    >
    inline fast_string_concatenator<S, C>::fast_string_concatenator(C const lhs, fast_string_concatenator const& rhs)
            : m_lhs(lhs)
            , m_rhs(rhs)
    {}

    template<   class S
            ,   class C
    >
    inline fast_string_concatenator<S, C>::fast_string_concatenator(fsc_seed const& lhs, S const& rhs)
            : m_lhs(lhs)
            , m_rhs(rhs)
    {}

    template<   class S
            ,   class C
    >
    inline fast_string_concatenator<S, C>::operator S() const
    {
        size_type   len = length();
        string_type s(len, '~');
        write(s.begin());
        assert(s.length() == strlen(s.c_str()));

        return s;
    }

/* /////////////////////////////////////////////////////////////////////////
 * operator +
 */

    template<   class S
            ,   class C
    >
    inline fast_string_concatenator<S, C> operator +(fsc_seed const& lhs, S const& rhs)
    {
        return fast_string_concatenator<S, C>(lhs, rhs);
    }

    template<   class  S>
    fast_string_concatenator<S> operator +(fsc_seed const& lhs, S const&  rhs)
    {
        return fast_string_concatenator<S>(lhs, rhs);
    }

    template<class S>
    concat_ptr_and_alloc<S> operator +(fsc_safe_seed<S> const& lhs, S const& rhs)

    {
        concat_allocator<S> al(lhs.get_arena());
        concat_ptr_and_alloc<S> ret {std::allocate_shared<fast_string_concatenator<S>>(al, lhs, rhs), al};
        return ret;
    }

    template<   class S
            ,   class C
    >
    inline fast_string_concatenator<S, C> const& operator +(fsc_seed const& /* lhs */, fast_string_concatenator<S, C/*, T*/> const& rhs)
    {
        return rhs;
    }

    template<   class S
            ,   class C
    >
    inline fast_string_concatenator<S, C> operator +(fast_string_concatenator<S, C> const& lhs, S const& rhs)
    {
        return fast_string_concatenator<S, C>(lhs, rhs);
    }

    template<   class S
    >
    inline auto operator +(concat_ptr_and_alloc<S> const & lhs, S const& rhs)
    {
        return concat_ptr_and_alloc<S> {std::allocate_shared<fast_string_concatenator<S>>(lhs.alloc, lhs.concat_ptr, rhs)
                , std::move(lhs.alloc)};
    }

    template<   class S
            ,   class C
    >
    inline fast_string_concatenator<S, C> operator +(fast_string_concatenator<S, C> const& lhs, C const* rhs)
    {
        return fast_string_concatenator<S, C>(lhs, rhs);
    }
    template<   class S
            ,   class C
    >
    inline auto operator +(concat_ptr_and_alloc<S> const & lhs, C const* rhs)
    {
        return concat_ptr_and_alloc<S> {std::allocate_shared<fast_string_concatenator<S>>(lhs.alloc, lhs.concat_ptr, rhs)
                , std::move(lhs.alloc)};
    }

    template<   class S
            ,   class C
    >
    inline fast_string_concatenator<S, C> operator +(fast_string_concatenator<S, C> const& lhs, C const rhs)
    {
        return fast_string_concatenator<S, C>(lhs, rhs);
    }

    template<   class S
            ,   class C
    >
    inline auto operator +(concat_ptr_and_alloc<S> const & lhs, C const rhs)
    {
        return concat_ptr_and_alloc<S> {std::allocate_shared<fast_string_concatenator<S>>(lhs.alloc, lhs.concat_ptr, rhs)
                , std::move(lhs.alloc)};
    }

// These operators are for handling embedded braces in the concatenation sequences, and represent the pathological case
    template<   class S
            ,   class C
    >
    inline fast_string_concatenator<S, C> operator +(fast_string_concatenator<S, C> const& lhs, fast_string_concatenator<S, C> const& rhs)
    {
        return fast_string_concatenator<S, C>(lhs, rhs);
    }

    template<   class S
            ,   class C
    >
    inline fast_string_concatenator<S, C> operator +(S const& lhs, fast_string_concatenator<S, C> const& rhs)
    {
        return fast_string_concatenator<S, C>(lhs, rhs);
    }

    template<   class S
            ,   class C
    >
    inline fast_string_concatenator<S, C> operator +(C const* lhs, fast_string_concatenator<S, C> const& rhs)
    {
        return fast_string_concatenator<S, C>(lhs, rhs);
    }

    template<   class S
            ,   class C
    >
    inline fast_string_concatenator<S, C> operator +(C const lhs, fast_string_concatenator<S, C> const& rhs)
    {
        return fast_string_concatenator<S, C>(lhs, rhs);
    }

/* ////////////////////////////////////////////////////////////////////// */

    static_assert(std::is_move_constructible_v<concat_allocator<std::string>>);
    static_assert(!std::is_move_assignable_v<concat_allocator<std::string>>);
    static_assert(std::is_copy_constructible_v<concat_allocator<std::string>>);
    static_assert(!std::is_copy_assignable_v<concat_allocator<std::string>>);

} /* namespace stlsoft */


/* /////////////////////////////////////////////////////////////////////////
 * inclusion control
 */

#ifdef STLSOFT_CF_PRAGMA_ONCE_SUPPORT
# pragma once
#endif /* STLSOFT_CF_PRAGMA_ONCE_SUPPORT */

#endif /* !STLSOFT_INCL_STLSOFT_STRING_HPP_FAST_STRING_CONCATENATOR */

/* ///////////////////////////// end of file //////////////////////////// */

