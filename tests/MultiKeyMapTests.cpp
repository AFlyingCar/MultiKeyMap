#include "gtest/gtest.h"

#include <iostream>

#define _MKM_DEBUG_OUTPUT std::cerr << "[   MKM   ] "

// TODO: Combine these into a single main Trie.h file
#include "trie.h"
#include "Index.h"

TEST(TrieTests, GetIndexOfTypeTests) {
    constexpr std::size_t I1 = getIndexOfType<int, int, float, char, int>();
    std::cout << "I1=" << I1 << std::endl;

    constexpr std::size_t I2 = getIndexOfType<float, int, float, char, int>();
    std::cout << "I2=" << I2 << std::endl;

    constexpr std::size_t I3 = getIndexOfType<char, int, float, char, int>();
    std::cout << "I3=" << I3 << std::endl;

    // Note: This will not compile. Leaving it in to showcase a failing case
    //   To verify this, uncomment the next 2 lines and try compiling, it should
    //   fail.
    // constexpr std::size_t I4 = getIndexOfType<std::string, int, float, char, int>();
    // std::cout << "I4=" << I4 << std::endl;

    constexpr std::size_t I5 = getIndexOfType<const int, int, float, char, int>();
    std::cout << "I5=" << I5 << std::endl;
}

TEST(TrieTests, ValidateSimpleTrieInsert) {
    // Validate inserting into a simple trie with 1 only key
    mkm::MultiKeyMap<float /* V */, int> trie;

    auto result = trie.insert({5}, 3.14159);
    ASSERT_EQ(result, 1);

    result = trie.insert({6}, 7);
    ASSERT_EQ(result, 1);

    result = trie.insert({6}, 7);
    ASSERT_EQ(result, 0);
}

TEST(TrieTests, ValidateSimpleTrieLookup) {
    // Validate finding values from a simple trie with 1 only key
    mkm::MultiKeyMap<float /* V */, int> trie;

    auto key1 = std::make_tuple(5);
    auto key2 = std::make_tuple(6);

    auto v1 = 3.14159;
    auto v2 = 7;

    auto result = trie.insert({5}, v1);
    ASSERT_EQ(result, 1);

    result = trie.insert({6}, v2);
    ASSERT_EQ(result, 1);

    // Lookup with variadic version
    auto it = trie.find(5);
    ASSERT_NE(it, trie.end()); // Test iterator comparison and that we actually found the value

    auto&& [it_key1, it_v1] = *it;

    ASSERT_EQ(it_key1, key1); // Verify that we got the correct key
    ASSERT_FLOAT_EQ(it_v1, v1); // Verify that we got the correct value

    // Advance the iterator
    ++it;
    ASSERT_TRUE(it.isEnd());
    ASSERT_EQ(it, trie.end()); // Test that the iterator got advanced and is at the end

    // Verify that we can still advance and that doing so just spins in-place
    ++it;
    ASSERT_TRUE(it.isEnd());
    ASSERT_EQ(it, trie.end());


    // Lookup other value
    auto it2 = trie.find(6);
    ASSERT_NE(it2, trie.end()); // Test iterator comparison and that we actually found the value

    auto&& [it_key2, it_v2] = *it2;

    ASSERT_EQ(it_key2, key2); // Verify that we got the correct key
    ASSERT_FLOAT_EQ(it_v2, v2); // Verify that we got the correct value

    // Advance the iterator
    ++it2;
    ASSERT_TRUE(it2.isEnd());
    ASSERT_EQ(it2, trie.end()); // Test that the iterator got advanced and is at the end


    // Lookup non-existant value
    auto it3 = trie.find(0);
    ASSERT_TRUE(it3.isEnd());
    ASSERT_EQ(it3, trie.end()); // Test iterator comparison and that we actually found the value


    // Make sure that iterators can be compared to each other.
    ASSERT_EQ(trie.find(5), trie.find(5));
}

TEST(TrieTests, ValidateComplexTrieInsert) {
    mkm::MultiKeyMap<float /* V */, int, char, bool> trie;

    auto key1 = std::make_tuple(5, 'c', true);

    auto v1 = 3.14159;
    auto v2 = 7;

    auto result = trie.insert({5, 'c', true}, v1);
    ASSERT_EQ(result, 1);

    auto it1 = trie.find(5, 'c', true);
    ASSERT_NE(it1, trie.end());

    auto&& [it_key1, it_v1] = *it1;

    ASSERT_EQ(it_key1, key1); // Verify that we got the correct key
    ASSERT_FLOAT_EQ(it_v1, v1); // Verify that we got the correct value

    // Advance the iterator
    ++it1;
    ASSERT_TRUE(it1.isEnd());
    ASSERT_EQ(it1, trie.end()); // Test that the iterator got advanced and is at the end

    // Verify that we can still advance and that doing so just spins in-place
    ++it1;
    ASSERT_TRUE(it1.isEnd());
    ASSERT_EQ(it1, trie.end());
}

TEST(TrieTests, ValidateComplexTriePartialKeyLookup) {
    mkm::MultiKeyMap<float /* V */, int, char, bool> trie;

    auto key1 = std::make_tuple(5, 'c', true);
    auto key2 = std::make_tuple(5, 'c', false);
    auto key3 = std::make_tuple(5, 'b', true);
    auto key4 = std::make_tuple(5, 'd', false);
    auto key5 = std::make_tuple(6, 'd', false);

    // Should specify all values for key1 and key2
    // TODO: Looking up keys by tuple is broken
    auto partial_key1 = std::make_tuple(5, 'c');

    auto v1 = 1;
    auto v2 = 2;
    auto v3 = 3;
    auto v4 = 4;
    auto v5 = 5;

    auto result = trie.insert(key1, v1);
    ASSERT_EQ(result, 1);
    result = trie.insert(key2, v2);
    ASSERT_EQ(result, 1);
    result = trie.insert(key3, v3);
    ASSERT_EQ(result, 1);
    result = trie.insert(key4, v4);
    ASSERT_EQ(result, 1);
    result = trie.insert(key5, v5);
    ASSERT_EQ(result, 1);

    {
        std::cout << "Lookup {5, 'c'}" << std::endl;
        auto it1 = trie.find(5, 'c');
        ASSERT_NE(it1, trie.end());

        auto&& [it_key1, it_v1] = *it1;

        ASSERT_EQ(it_key1, key1); // Verify that we got the correct key
        ASSERT_FLOAT_EQ(it_v1, v1); // Verify that we got the correct value

        // It1 should return us 2 values
        ++it1;
        ASSERT_NE(it1, trie.end());

        auto&& [it_key2, it_v2] = *it1;

        ASSERT_EQ(it_key2, key2); // Verify that we got the correct key
        ASSERT_FLOAT_EQ(it_v2, v2); // Verify that we got the correct value

        // Advance the iterator
        ++it1;
        ASSERT_TRUE(it1.isEnd());
        ASSERT_EQ(it1, trie.end()); // Test that the iterator got advanced and is at the end

        // Verify that we can still advance and that doing so just spins in-place
        ++it1;
        ASSERT_TRUE(it1.isEnd());
        ASSERT_EQ(it1, trie.end());
    }

    {
        std::cout << "Lookup {5}" << std::endl;
        auto it2 = trie.find(5);
        ASSERT_NE(it2, trie.end());

        auto&& [it_key1, it_v1] = *it2;
        ASSERT_EQ(it_key1, key1); // Verify that we got the correct key
        ASSERT_FLOAT_EQ(it_v1, v1); // Verify that we got the correct value

        // It1 should return us 4 values
        ++it2;
        ASSERT_NE(it2, trie.end());

        auto&& [it_key2, it_v2] = *it2;
        ASSERT_EQ(it_key2, key2); // Verify that we got the correct key
        ASSERT_FLOAT_EQ(it_v2, v2); // Verify that we got the correct value

        // It1 should return us 4 values
        ++it2;
        ASSERT_NE(it2, trie.end());

        auto&& [it_key3, it_v3] = *it2;
        ASSERT_EQ(it_key3, key3); // Verify that we got the correct key
        ASSERT_FLOAT_EQ(it_v3, v3); // Verify that we got the correct value

        // It1 should return us 4 values
        ++it2;
        ASSERT_NE(it2, trie.end());

        auto&& [it_key4, it_v4] = *it2;
        ASSERT_EQ(it_key4, key4); // Verify that we got the correct key
        ASSERT_FLOAT_EQ(it_v4, v4); // Verify that we got the correct value

        // Advance the iterator
        ++it2;
        ASSERT_TRUE(it2.isEnd());
        ASSERT_EQ(it2, trie.end()); // Test that the iterator got advanced and is at the end

        // Verify that we can still advance and that doing so just spins in-place
        ++it2;
        ASSERT_TRUE(it2.isEnd());
        ASSERT_EQ(it2, trie.end());
    }
}

TEST(TrieTests, ValidateComplexTrieForEach) {
    mkm::MultiKeyMap<float /* V */, int, char, bool> trie;

    std::vector<std::tuple<int, char, bool>> keys = {
        std::make_tuple(5, 'c', true),
        std::make_tuple(5, 'c', false),
        std::make_tuple(5, 'b', true),
        std::make_tuple(5, 'd', false),
        std::make_tuple(6, 'd', false)
    };
    std::vector<float> vals = {
        1,
        2,
        3,
        4,
        5
    };

    auto result = trie.insert(keys[0], vals[0]);
    ASSERT_EQ(result, 1);
    result = trie.insert(keys[1], vals[1]);
    ASSERT_EQ(result, 1);
    result = trie.insert(keys[2], vals[2]);
    ASSERT_EQ(result, 1);
    result = trie.insert(keys[3], vals[3]);
    ASSERT_EQ(result, 1);
    result = trie.insert(keys[4], vals[4]);
    ASSERT_EQ(result, 1);

    auto count = 0;
    for(auto it = trie.find(5); it != trie.end(); ++it) {
        auto&& [k,v] = *it;

        std::cout << "{" << std::get<0>(k) << ", "
                         << std::get<1>(k) << ", "
                         << std::get<2>(k) << "} => " << v << std::endl;

        ASSERT_EQ(k, keys[count]);
        ASSERT_FLOAT_EQ(v, vals[count]);
        ++count;
    }
    ASSERT_EQ(count, 4);

    ////////
    std::cout << "------ FOREACH ------" << std::endl;
    count = 0;
    for(auto&& [k,v] : trie) {
        std::cout << "{" << std::get<0>(k) << ", "
                         << std::get<1>(k) << ", "
                         << std::get<2>(k) << "} => " << v << std::endl;

        ASSERT_EQ(k, keys[count]);
        ASSERT_FLOAT_EQ(v, vals[count]);
        ++count;
    }
    ASSERT_EQ(count, 5);
}

TEST(TrieTests, ValidateConstness) {
    mkm::MultiKeyMap<float /* V */, int, char, bool> trie;

    std::vector<std::tuple<int, char, bool>> keys = {
        std::make_tuple(5, 'c', true),
        std::make_tuple(5, 'c', false),
        std::make_tuple(5, 'b', true),
        std::make_tuple(5, 'd', false),
        std::make_tuple(6, 'd', false)
    };
    std::vector<float> vals = {
        1,
        2,
        3,
        4,
        5
    };

    auto result = trie.insert(keys[0], vals[0]);
    ASSERT_EQ(result, 1);
    result = trie.insert(keys[1], vals[1]);
    ASSERT_EQ(result, 1);
    result = trie.insert(keys[2], vals[2]);
    ASSERT_EQ(result, 1);
    result = trie.insert(keys[3], vals[3]);
    ASSERT_EQ(result, 1);
    result = trie.insert(keys[4], vals[4]);
    ASSERT_EQ(result, 1);

    const auto& const_trie = trie;
    {
        auto const_it = const_trie.find(5);

        auto count = 0;
        for(; const_it != const_trie.end(); ++const_it) {
            auto&& [k,v] = *const_it;

            std::cout << "{" << std::get<0>(k) << ", "
                             << std::get<1>(k) << ", "
                             << std::get<2>(k) << "} => " << v << std::endl;

            ASSERT_EQ(k, keys[count]);
            ASSERT_FLOAT_EQ(v, vals[count]);
            ++count;
        }
        ASSERT_EQ(count, 4);

        ////////

        std::cout << "------ FOREACH ------" << std::endl;
        count = 0;
        for(auto&& [k,v] : trie) {
            std::cout << "{" << std::get<0>(k) << ", "
                             << std::get<1>(k) << ", "
                             << std::get<2>(k) << "} => " << v << std::endl;

            ASSERT_EQ(k, keys[count]);
            ASSERT_FLOAT_EQ(v, vals[count]);
            ++count;
        }
        ASSERT_EQ(count, 5);

    }
}

TEST(TrieTests, ValidateComplexTrieAt) {
    mkm::MultiKeyMap<float /* V */, int, char, bool> trie;

    std::vector<std::tuple<int, char, bool>> keys = {
        std::make_tuple(5, 'c', true),
        std::make_tuple(5, 'c', false),
        std::make_tuple(5, 'b', true),
        std::make_tuple(5, 'd', false),
        std::make_tuple(6, 'd', false)
    };
    std::vector<float> vals = {
        1,
        2,
        3,
        4,
        5
    };

    auto result = trie.insert(keys[0], vals[0]);
    ASSERT_EQ(result, 1);
    result = trie.insert(keys[1], vals[1]);
    ASSERT_EQ(result, 1);
    result = trie.insert(keys[2], vals[2]);
    ASSERT_EQ(result, 1);
    result = trie.insert(keys[3], vals[3]);
    ASSERT_EQ(result, 1);
    result = trie.insert(keys[4], vals[4]);
    ASSERT_EQ(result, 1);

    auto v = trie.at(5, 'c', false);
    ASSERT_FLOAT_EQ(v, 2);

    v = trie.at(6, 'd', false);
    ASSERT_FLOAT_EQ(v, 5);

    EXPECT_THROW(trie.at(7, '\0', false), std::out_of_range);

    // Verify const at()
    {
        const auto& const_trie = trie;

        v = const_trie.at(5, 'c', false);
        ASSERT_FLOAT_EQ(v, 2);

        v = const_trie.at(6, 'd', false);
        ASSERT_FLOAT_EQ(v, 5);

        EXPECT_THROW(const_trie.at(7, '\0', false), std::out_of_range);
    }
}

TEST(TrieTests, ValidateComplexTrieSize) {
    mkm::MultiKeyMap<float /* V */, int, char, bool> trie;

    std::vector<std::tuple<int, char, bool>> keys = {
        std::make_tuple(5, 'c', true),
        std::make_tuple(5, 'c', false),
        std::make_tuple(5, 'b', true),
        std::make_tuple(5, 'd', false),
        std::make_tuple(6, 'd', false)
    };
    std::vector<float> vals = {
        1,
        2,
        3,
        4,
        5
    };

    auto size = trie.size();
    ASSERT_EQ(size, 0);

    auto result = trie.insert(keys[0], vals[0]);
    ASSERT_EQ(result, 1);

    size = trie.size();
    ASSERT_EQ(size, 1);

    result = trie.insert(keys[1], vals[1]);
    ASSERT_EQ(result, 1);

    size = trie.size();
    ASSERT_EQ(size, 2);

    result = trie.insert(keys[2], vals[2]);
    ASSERT_EQ(result, 1);

    size = trie.size();
    ASSERT_EQ(size, 3);

    result = trie.insert(keys[3], vals[3]);
    ASSERT_EQ(result, 1);

    size = trie.size();
    ASSERT_EQ(size, 4);

    result = trie.insert(keys[4], vals[4]);
    ASSERT_EQ(result, 1);

    size = trie.size();
    ASSERT_EQ(size, 5);
}

TEST(TrieTests, ValidateComplexTrieOperatorBracket) {
    mkm::MultiKeyMap<float /* V */, int, char, bool> trie;

    std::vector<std::tuple<int, char, bool>> keys = {
        std::make_tuple(5, 'c', true),
        std::make_tuple(5, 'c', false),
        std::make_tuple(5, 'b', true),
        std::make_tuple(5, 'd', false),
        std::make_tuple(6, 'd', false)
    };
    std::vector<float> vals = {
        1,
        2,
        3,
        4,
        5
    };

    auto result = trie[keys[0]] = vals[0];
    ASSERT_FLOAT_EQ(result, vals[0]);
    result = trie[keys[1]] = vals[1];
    ASSERT_FLOAT_EQ(result, vals[1]);
    result = trie[keys[2]] = vals[2];
    ASSERT_FLOAT_EQ(result, vals[2]);
    result = trie[keys[3]] = vals[3];
    ASSERT_FLOAT_EQ(result, vals[3]);
    result = trie[keys[4]] = vals[4];
    ASSERT_FLOAT_EQ(result, vals[4]);

    auto size = trie.size();
    ASSERT_EQ(size, 5);

    // Verify that the elements we expect to exist are all there
    auto count = 0;
    for(auto&& [k,v] : trie) {
        std::cout << "{" << std::get<0>(k) << ", "
                         << std::get<1>(k) << ", "
                         << std::get<2>(k) << "} => " << v << std::endl;

        ASSERT_EQ(k, keys[count]);
        ASSERT_FLOAT_EQ(v, vals[count]);
        ++count;
    }

    // Reading only, make sure this doesn't do any insertions and doesn't modify
    //   the value that's already there
    result = trie[keys[0]];
    ASSERT_FLOAT_EQ(result, vals[0]);

    size = trie.size();
    ASSERT_EQ(size, 5);
}

