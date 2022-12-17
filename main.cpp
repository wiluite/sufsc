//#define STLSOFT_ALLOCATOR_SELECTOR_USE_STLSOFT_NEW_ALLOCATOR

#include <iostream>
#include "fast_string_concatenator.hpp"
#include <string>
#include "config.h"

using namespace std;
using namespace stlsoft;

#if !defined(SANITY_CHECK)

std::size_t memory = 0;
std::size_t alloc = 0;

void* operator new(std::size_t s) noexcept(false) //noexcept(noexcept(malloc(s)))
{
    memory += s;
    ++alloc;
    return malloc(s);
}

void operator delete(void* p) noexcept
{
    --alloc;
    free(p);
}

#endif

void memuse()
{
#if !defined(SANITY_CHECK)
    std::cout << "memory = " << memory << '\n';
    std::cout << "alloc = " << alloc << '\n';
#endif
}


using string_class = std::string;

template <class T>
auto transport (T s)
{
    static_assert(!std::is_same_v<string_class, T>);
    return s;
}

int main() {

    using saving_stack = concat_arena<string_class>;

    string_class s1 = "Goodbye";
    string_class s2 = "Cruel";
    string_class s3 = "World";

    // Correct Old Use-Case
    string_class res_string = fsc_seed()+s1+','+s2+' '+s3+",oh-oh!";
    std::cout << res_string << '\n';

    // Incorrect Old Use-Case
    auto tmp = stlsoft::fsc_seed()+s1+','+s2+' '+s3+",oh-oh!";
    // Don't! Stack-use-after-scope!
    // string_class str = tmp;
    // std::cout << str << std::endl;

    // New Use-Case
    saving_stack arena;
    memuse();
    auto const tmp_fsc = fsc_safe_seed(arena)+s1+','+s2+' '+s3+",oh-oh!";
    // do something special
    //...
    //....
    // assign to a string at *any* time
    string_class result_string = transport(tmp_fsc);
    memuse(); // dynamic memory allocation (if any) only at result_string initialization

    // Backward compatible case
    string_class result_string2 = fsc_safe_seed(arena)+s1+','+s2+' '+s3+",oh-oh!";

    std::cout << result_string << '\n' << result_string2 << '\n';

    return 0;
}
