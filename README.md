
# MultiKeyMap

## Description

Small C++17 header-only library that implements a multi-key associative
container with key-prefix lookup. Each part of the full key for values can be a
unique hashable type.

Since value lookup can be done using only part of the key (assuming the key
prefix is provided in the same order as specified in the template parameters and
no parts are skipped), doing a `find` operation will return an iterator that
iterates over all possible values whose keys start with the given key prefix.

`MultiKeyMap` is implemented internally as a
[Trie](https://en.wikipedia.org/wiki/Trie) data structure with a compile-time
known depth, and allocates a `std::unordered_map` for each "part" of the key in
every node.

Only the following methods support providing key prefixes, while all others
require a full key:
* `find`
* `count`
* `contains`
* `erase`

## Examples

```cpp
#include <string>
#include <iostream>

#include "MultiKeyMap.h"

int main() {
    // Create the MultiKeyMap. The full key type here is <int, char, bool>, and
    //   the value type is <float>
    mkm::MultiKeyMap<int, char, bool, std::string> m;

    // Insert elements using both operator[] and insert
    m[{6, 'd', false}] = "first";
    m[std::make_tuple(6, 'f', false)] = "second";

    bool was_inserted = m.insert({7, 'd', false}, "third"); // returns true
    was_inserted = m.insert({7, 'd', false}, "fourth"); // returns false

    m[{6, 'd', true}] = "fifth";

    // Iterate over the entire map, will output every value
    for(auto p : m) {
        std::cout << "{" << std::get<0>(p.first) << ','
                         << std::get<1>(p.first) << ','
                         << std::get<2>(p.first) << ','
                         << "}: " << p.second << std::endl;
    }

    // Lookup a value by key prefix.
    // Will only print out values "first", "second", and "fifth"
    auto it = m.find(std::make_tuple(6));
    for(; it != m.end(); ++it) {
        std::cout << "{" << std::get<0>(it->first) << ','
                         << std::get<1>(it->first) << ','
                         << std::get<2>(it->first) << ','
                         << "}: " << it->second << std::endl;
    }

    // Erase a value using a key prefix
    m.erase(std::make_tuple(6, 'd'));
}
```

