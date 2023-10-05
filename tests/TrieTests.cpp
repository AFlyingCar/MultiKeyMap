#include "gtest/gtest.h"

#include <iostream>

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
    generic_trie::KTrie<float /* V */, int> trie;

    auto result = trie.insert({5}, 3.14159);
    ASSERT_EQ(result, 1);

    result = trie.insert({6}, 7);
    ASSERT_EQ(result, 1);

    result = trie.insert({6}, 7);
    ASSERT_EQ(result, 0);
}

TEST(TrieTests, ValidateSimpleTrieLookup) {
    // Validate finding values from a simple trie with 1 only key
    generic_trie::KTrie<float /* V */, int> trie;

    auto v1 = 3.14159;
    auto v2 = 7;

    auto result = trie.insert({5}, v1);
    ASSERT_EQ(result, 1);

    result = trie.insert({6}, v2);
    ASSERT_EQ(result, 1);

    // Lookup with variadic version
    auto it = trie.find(5);
    ASSERT_NE(it, trie.end()); // Test iterator comparison and that we actually found the value
    ASSERT_FLOAT_EQ(*it, v1); // Verify that we got the correct value

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
    ASSERT_FLOAT_EQ(*it2, v2); // Verify that we got the correct value

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
    generic_trie::KTrie<float /* V */, int, char, bool> trie;

    auto v1 = 3.14159;
    auto v2 = 7;

    auto result = trie.insert({5, 'c', true}, v1);
}

