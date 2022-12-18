# sufsc - Safe use Matthew Wilson's fast_string_concatenator
A Use-Case for safe use an effective string concatenator 
class at the cost of small object copy (and a bit) per an
operation.

# Version
1.0.2

# Features
- free from STLSoft dependency

# Problem
While using fast_string_concatenator you must construct a 
target string before the end of the current expression. You 
can not construct a composite concatenator and set it aside 
for a while. This is because all inner concatenations will 
live until the nearest semicolon, and there will be 
dangling references to them.

# Solution
- shared pointers to prolong lifetimes
- stack allocations to avoid heap overhead

# Example
```cpp

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
    string_class result_string = tmp_fsc;
    memuse(); // dynamic memory allocation (if any) only at result_string initialization

    // Backward compatible case
    string_class result_string2 = fsc_safe_seed(arena)+s1+','+s2+' '+s3+",oh-oh!";

    std::cout << result_string << '\n' << result_string2 << '\n';

```
