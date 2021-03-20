# sufsc - Safe use Matthew Wilson's fast_string_concatenator
A Use-Case for safe use an effective string concatenator 
class at the cost of small object copy (and a bit) per an
operation.

# Version
1.0.0

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

    using string_class = std::string;

    string_class s1 = "Goodbye";
    string_class s2 = "Cruel";
    string_class s3 = "World";

    // Correct Old Use-Case 
    string_class res_string = fsc_seed() + s1 + ' ' + s2 + ' ' + s3 + ",ha-ha!";
    std::cout << res_string << '\n';

    // Incorrect Old Use-Case
    // Don't!
    // auto tmp = stlsoft::fsc_seed() + s1 + ' ' + s2 + ' ' + s3 + ",ha-ha!";
    // stack-use-after-scope!
    // string_class str = tmp;
    // std::cout << str << std::endl;

    // New Use-Case 
    concat_arena<string_class> stack_arena;
    auto tmp_fsc = fsc_safe_seed<string_class>(stack_arena) + s1 + ' ' + s2 + ' ' + s3 + ",ha-ha!";
    // do something special
    // ...
    // ....
    // assign to a string at *any* time
    string_class result_string = tmp_fsc;

    std::cout << result_string << '\n';

```


