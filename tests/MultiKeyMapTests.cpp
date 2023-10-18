#include "gtest/gtest.h"

#include <iostream>

// #define _MKM_DEBUG_OUTPUT std::cerr << "[   MKM   ] "

#include "MultiKeyMap.h"

using ComplexMultiKeyMapType = mkm::MultiKeyMap<int, char, bool, float /* V */>;

TEST(MultiKeyMapTests, GetIndexOfTypeTests) {
    constexpr std::size_t I1 = mkm::detail::getIndexOfType<int, int, float, char, int>();
    std::cout << "I1=" << I1 << std::endl;

    constexpr std::size_t I2 = mkm::detail::getIndexOfType<float, int, float, char, int>();
    std::cout << "I2=" << I2 << std::endl;

    constexpr std::size_t I3 = mkm::detail::getIndexOfType<char, int, float, char, int>();
    std::cout << "I3=" << I3 << std::endl;

    // Note: This will not compile. Leaving it in to showcase a failing case
    //   To verify this, uncomment the next 2 lines and try compiling, it should
    //   fail.
    // constexpr std::size_t I4 = detail::getIndexOfType<std::string, int, float, char, int>();
    // std::cout << "I4=" << I4 << std::endl;

    constexpr std::size_t I5 = mkm::detail::getIndexOfType<const int, int, float, char, int>();
    std::cout << "I5=" << I5 << std::endl;
}

TEST(MultiKeyMapTests, PrintTupleTests) {
    std::tuple<int, char, std::string> t{
        5, 'c', "foobar"
    };

    mkm::detail::printTuple(std::cout, t, std::make_index_sequence<3>());
    std::cout << std::endl;
}

TEST(MultiKeyMapTests, ValidateSimpleMultiKeyMapInsert) {
    // Validate inserting into a simple multiKeyMap with 1 only key
    mkm::MultiKeyMap<int, float> multiKeyMap;

    auto result = multiKeyMap.insert({5}, 3.14159);
    ASSERT_EQ(result, 1);

    result = multiKeyMap.insert({6}, 7);
    ASSERT_EQ(result, 1);

    result = multiKeyMap.insert({6}, 7);
    ASSERT_EQ(result, 0);
}

TEST(MultiKeyMapTests, ValidateSimpleMultiKeyMapLookup) {
    // Validate finding values from a simple multiKeyMap with 1 only key
    mkm::MultiKeyMap<int, float> multiKeyMap;

    auto key1 = std::make_tuple(5);
    auto key2 = std::make_tuple(6);

    auto v1 = 3.14159;
    auto v2 = 7;

    auto result = multiKeyMap.insert({5}, v1);
    ASSERT_EQ(result, 1);

    result = multiKeyMap.insert({6}, v2);
    ASSERT_EQ(result, 1);

    // Lookup with variadic version
    auto it = multiKeyMap.find(key1);
    ASSERT_NE(it, multiKeyMap.end()); // Test iterator comparison and that we actually found the value

    auto&& [it_key1, it_v1] = *it;

    ASSERT_EQ(it_key1, key1); // Verify that we got the correct key
    ASSERT_FLOAT_EQ(it_v1, v1); // Verify that we got the correct value

    // Advance the iterator
    ++it;
    ASSERT_TRUE(it.isEnd());
    ASSERT_EQ(it, multiKeyMap.end()); // Test that the iterator got advanced and is at the end

    // Verify that we can still advance and that doing so just spins in-place
    ++it;
    ASSERT_TRUE(it.isEnd());
    ASSERT_EQ(it, multiKeyMap.end());


    // Lookup other value
    auto it2 = multiKeyMap.find(6);
    ASSERT_NE(it2, multiKeyMap.end()); // Test iterator comparison and that we actually found the value

    auto&& [it_key2, it_v2] = *it2;

    ASSERT_EQ(it_key2, key2); // Verify that we got the correct key
    ASSERT_FLOAT_EQ(it_v2, v2); // Verify that we got the correct value

    // Advance the iterator
    ++it2;
    ASSERT_TRUE(it2.isEnd());
    ASSERT_EQ(it2, multiKeyMap.end()); // Test that the iterator got advanced and is at the end


    // Lookup non-existant value
    auto it3 = multiKeyMap.find(0);
    ASSERT_TRUE(it3.isEnd());
    ASSERT_EQ(it3, multiKeyMap.end()); // Test iterator comparison and that we actually found the value


    // Make sure that iterators can be compared to each other.
    ASSERT_EQ(multiKeyMap.find(5), multiKeyMap.find(5));
}

TEST(MultiKeyMapTests, ValidateComplexMultiKeyMapInsert) {
    ComplexMultiKeyMapType multiKeyMap;

    auto key1 = std::make_tuple(5, 'c', true);

    auto v1 = 3.14159;
    auto v2 = 7;

    auto result = multiKeyMap.insert({5, 'c', true}, v1);
    ASSERT_EQ(result, 1);

    auto it1 = multiKeyMap.find(key1);
    ASSERT_NE(it1, multiKeyMap.end());

    auto&& [it_key1, it_v1] = *it1;

    ASSERT_EQ(it_key1, key1); // Verify that we got the correct key
    ASSERT_FLOAT_EQ(it_v1, v1); // Verify that we got the correct value

    // Advance the iterator
    ++it1;
    ASSERT_TRUE(it1.isEnd());
    ASSERT_EQ(it1, multiKeyMap.end()); // Test that the iterator got advanced and is at the end

    // Verify that we can still advance and that doing so just spins in-place
    ++it1;
    ASSERT_TRUE(it1.isEnd());
    ASSERT_EQ(it1, multiKeyMap.end());
}

TEST(MultiKeyMapTests, ValidateComplexMultiKeyMapKeyPrefixLookup) {
    ComplexMultiKeyMapType multiKeyMap;

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

    auto result = multiKeyMap.insert(key1, v1);
    ASSERT_EQ(result, 1);
    result = multiKeyMap.insert(key2, v2);
    ASSERT_EQ(result, 1);
    result = multiKeyMap.insert(key3, v3);
    ASSERT_EQ(result, 1);
    result = multiKeyMap.insert(key4, v4);
    ASSERT_EQ(result, 1);
    result = multiKeyMap.insert(key5, v5);
    ASSERT_EQ(result, 1);

    {
        std::cout << "Lookup {5, 'c'}" << std::endl;
        auto it1 = multiKeyMap.find(5, 'c');
        ASSERT_NE(it1, multiKeyMap.end());

        auto&& [it_key1, it_v1] = *it1;

        ASSERT_EQ(it_key1, key1); // Verify that we got the correct key
        ASSERT_FLOAT_EQ(it_v1, v1); // Verify that we got the correct value

        // It1 should return us 2 values
        ++it1;
        ASSERT_NE(it1, multiKeyMap.end());

        auto&& [it_key2, it_v2] = *it1;

        ASSERT_EQ(it_key2, key2); // Verify that we got the correct key
        ASSERT_FLOAT_EQ(it_v2, v2); // Verify that we got the correct value

        // Advance the iterator
        ++it1;
        ASSERT_TRUE(it1.isEnd());
        ASSERT_EQ(it1, multiKeyMap.end()); // Test that the iterator got advanced and is at the end

        // Verify that we can still advance and that doing so just spins in-place
        ++it1;
        ASSERT_TRUE(it1.isEnd());
        ASSERT_EQ(it1, multiKeyMap.end());
    }

    {
        std::cout << "Lookup {5}" << std::endl;
        auto it2 = multiKeyMap.find(5);
        ASSERT_NE(it2, multiKeyMap.end());

        auto&& [it_key1, it_v1] = *it2;
        ASSERT_EQ(it_key1, key1); // Verify that we got the correct key
        ASSERT_FLOAT_EQ(it_v1, v1); // Verify that we got the correct value

        // It1 should return us 4 values
        ++it2;
        ASSERT_NE(it2, multiKeyMap.end());

        auto&& [it_key2, it_v2] = *it2;
        ASSERT_EQ(it_key2, key2); // Verify that we got the correct key
        ASSERT_FLOAT_EQ(it_v2, v2); // Verify that we got the correct value

        // It1 should return us 4 values
        ++it2;
        ASSERT_NE(it2, multiKeyMap.end());

        auto&& [it_key3, it_v3] = *it2;
        ASSERT_EQ(it_key3, key3); // Verify that we got the correct key
        ASSERT_FLOAT_EQ(it_v3, v3); // Verify that we got the correct value

        // It1 should return us 4 values
        ++it2;
        ASSERT_NE(it2, multiKeyMap.end());

        auto&& [it_key4, it_v4] = *it2;
        ASSERT_EQ(it_key4, key4); // Verify that we got the correct key
        ASSERT_FLOAT_EQ(it_v4, v4); // Verify that we got the correct value

        // Advance the iterator
        ++it2;
        ASSERT_TRUE(it2.isEnd());
        ASSERT_EQ(it2, multiKeyMap.end()); // Test that the iterator got advanced and is at the end

        // Verify that we can still advance and that doing so just spins in-place
        ++it2;
        ASSERT_TRUE(it2.isEnd());
        ASSERT_EQ(it2, multiKeyMap.end());
    }
}

TEST(MultiKeyMapTests, ValidateComplexMultiKeyMapForEach) {
    ComplexMultiKeyMapType multiKeyMap;

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

    auto result = multiKeyMap.insert(keys[0], vals[0]);
    ASSERT_EQ(result, 1);
    result = multiKeyMap.insert(keys[1], vals[1]);
    ASSERT_EQ(result, 1);
    result = multiKeyMap.insert(keys[2], vals[2]);
    ASSERT_EQ(result, 1);
    result = multiKeyMap.insert(keys[3], vals[3]);
    ASSERT_EQ(result, 1);
    result = multiKeyMap.insert(keys[4], vals[4]);
    ASSERT_EQ(result, 1);

    auto count = 0;
    for(auto it = multiKeyMap.find(5); it != multiKeyMap.end(); ++it) {
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
    for(auto&& [k,v] : multiKeyMap) {
        std::cout << "{" << std::get<0>(k) << ", "
                         << std::get<1>(k) << ", "
                         << std::get<2>(k) << "} => " << v << std::endl;

        ASSERT_EQ(k, keys[count]);
        ASSERT_FLOAT_EQ(v, vals[count]);
        ++count;
    }
    ASSERT_EQ(count, 5);
}

TEST(MultiKeyMapTests, ValidateConstness) {
    ComplexMultiKeyMapType multiKeyMap;

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

    auto result = multiKeyMap.insert(keys[0], vals[0]);
    ASSERT_EQ(result, 1);
    result = multiKeyMap.insert(keys[1], vals[1]);
    ASSERT_EQ(result, 1);
    result = multiKeyMap.insert(keys[2], vals[2]);
    ASSERT_EQ(result, 1);
    result = multiKeyMap.insert(keys[3], vals[3]);
    ASSERT_EQ(result, 1);
    result = multiKeyMap.insert(keys[4], vals[4]);
    ASSERT_EQ(result, 1);

    const auto& const_multiKeyMap = multiKeyMap;
    {
        auto const_it = const_multiKeyMap.find(5);

        auto count = 0;
        for(; const_it != const_multiKeyMap.end(); ++const_it) {
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
        for(auto&& [k,v] : multiKeyMap) {
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

TEST(MultiKeyMapTests, ValidateComplexMultiKeyMapAt) {
    ComplexMultiKeyMapType multiKeyMap;

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

    auto result = multiKeyMap.insert(keys[0], vals[0]);
    ASSERT_EQ(result, 1);
    result = multiKeyMap.insert(keys[1], vals[1]);
    ASSERT_EQ(result, 1);
    result = multiKeyMap.insert(keys[2], vals[2]);
    ASSERT_EQ(result, 1);
    result = multiKeyMap.insert(keys[3], vals[3]);
    ASSERT_EQ(result, 1);
    result = multiKeyMap.insert(keys[4], vals[4]);
    ASSERT_EQ(result, 1);

    auto v = multiKeyMap.at(5, 'c', false);
    ASSERT_FLOAT_EQ(v, 2);

    v = multiKeyMap.at(6, 'd', false);
    ASSERT_FLOAT_EQ(v, 5);

    EXPECT_THROW(multiKeyMap.at(7, '\0', false), std::out_of_range);

    // Verify const at()
    {
        const auto& const_multiKeyMap = multiKeyMap;

        v = const_multiKeyMap.at(5, 'c', false);
        ASSERT_FLOAT_EQ(v, 2);

        v = const_multiKeyMap.at(6, 'd', false);
        ASSERT_FLOAT_EQ(v, 5);

        EXPECT_THROW(const_multiKeyMap.at(7, '\0', false), std::out_of_range);
    }
}

TEST(MultiKeyMapTests, ValidateComplexMultiKeyMapSize) {
    ComplexMultiKeyMapType multiKeyMap;

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

    auto size = multiKeyMap.size();
    ASSERT_EQ(size, 0);

    auto result = multiKeyMap.insert(keys[0], vals[0]);
    ASSERT_EQ(result, 1);

    size = multiKeyMap.size();
    ASSERT_EQ(size, 1);

    result = multiKeyMap.insert(keys[1], vals[1]);
    ASSERT_EQ(result, 1);

    size = multiKeyMap.size();
    ASSERT_EQ(size, 2);

    result = multiKeyMap.insert(keys[2], vals[2]);
    ASSERT_EQ(result, 1);

    size = multiKeyMap.size();
    ASSERT_EQ(size, 3);

    result = multiKeyMap.insert(keys[3], vals[3]);
    ASSERT_EQ(result, 1);

    size = multiKeyMap.size();
    ASSERT_EQ(size, 4);

    result = multiKeyMap.insert(keys[4], vals[4]);
    ASSERT_EQ(result, 1);

    size = multiKeyMap.size();
    ASSERT_EQ(size, 5);
}

TEST(MultiKeyMapTests, ValidateComplexMultiKeyMapOperatorBracket) {
    ComplexMultiKeyMapType multiKeyMap;

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

    auto result = multiKeyMap[keys[0]] = vals[0];
    ASSERT_FLOAT_EQ(result, vals[0]);
    result = multiKeyMap[keys[1]] = vals[1];
    ASSERT_FLOAT_EQ(result, vals[1]);
    result = multiKeyMap[keys[2]] = vals[2];
    ASSERT_FLOAT_EQ(result, vals[2]);
    result = multiKeyMap[keys[3]] = vals[3];
    ASSERT_FLOAT_EQ(result, vals[3]);
    result = multiKeyMap[{6, 'd', false}] = vals[4];
    ASSERT_FLOAT_EQ(result, vals[4]);

    auto size = multiKeyMap.size();
    ASSERT_EQ(size, 5);

    // Verify that the elements we expect to exist are all there
    auto count = 0;
    for(auto&& [k,v] : multiKeyMap) {
        std::cout << "{" << std::get<0>(k) << ", "
                         << std::get<1>(k) << ", "
                         << std::get<2>(k) << "} => " << v << std::endl;

        ASSERT_EQ(k, keys[count]);
        ASSERT_FLOAT_EQ(v, vals[count]);
        ++count;
    }

    // Reading only, make sure this doesn't do any insertions and doesn't modify
    //   the value that's already there
    result = multiKeyMap[keys[0]];
    ASSERT_FLOAT_EQ(result, vals[0]);

    size = multiKeyMap.size();
    ASSERT_EQ(size, 5);
}

TEST(MultiKeyMapTests, ValidateComplexMultiKeyMapEraseAndClear) {
    ComplexMultiKeyMapType multiKeyMap;

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

    auto result = multiKeyMap.insert(keys[0], vals[0]);
    ASSERT_EQ(result, 1);
    result = multiKeyMap.insert(keys[1], vals[1]);
    ASSERT_EQ(result, 1);
    result = multiKeyMap.insert(keys[2], vals[2]);
    ASSERT_EQ(result, 1);
    result = multiKeyMap.insert(keys[3], vals[3]);
    ASSERT_EQ(result, 1);
    result = multiKeyMap.insert(keys[4], vals[4]);
    ASSERT_EQ(result, 1);

    auto size = multiKeyMap.size();
    ASSERT_EQ(size, 5);

    multiKeyMap.erase(keys[1]);

    // Verify that the size has decreased
    size = multiKeyMap.size();
    ASSERT_EQ(size, 4);

    // Verify that the element cannot be found
    auto it = multiKeyMap.find(keys[1]);
    ASSERT_TRUE(it.isEnd());
    ASSERT_EQ(it, multiKeyMap.end());

    // Verify clear works
    multiKeyMap.clear();
    size = multiKeyMap.size();
    ASSERT_EQ(size, 0);
    ASSERT_EQ(multiKeyMap.begin(), multiKeyMap.end());
}

TEST(MultiKeyMapTests, ValidateComplexMultiKeyMapCount) {
    ComplexMultiKeyMapType multiKeyMap;

    std::vector<std::tuple<int, char, bool>> keys = {
        std::make_tuple(5, 'c', true),
        std::make_tuple(5, 'c', false),
        std::make_tuple(5, 'b', true),
        std::make_tuple(5, 'd', false),
        std::make_tuple(6, 'd', false),
        std::make_tuple(7, 'z', false) // fake key
    };
    std::vector<float> vals = {
        1,
        2,
        3,
        4,
        5
    };

    auto result = multiKeyMap.insert(keys[0], vals[0]);
    ASSERT_EQ(result, 1);
    result = multiKeyMap.insert(keys[1], vals[1]);
    ASSERT_EQ(result, 1);
    result = multiKeyMap.insert(keys[2], vals[2]);
    ASSERT_EQ(result, 1);
    result = multiKeyMap.insert(keys[3], vals[3]);
    ASSERT_EQ(result, 1);
    result = multiKeyMap.insert(keys[4], vals[4]);
    ASSERT_EQ(result, 1);

    // Full key
    auto count = multiKeyMap.count(keys[1]);
    ASSERT_EQ(count, 1);

    // Partial key
    count = multiKeyMap.count(5, 'c');
    ASSERT_EQ(count, 2);
    count = multiKeyMap.count(5);
    ASSERT_EQ(count, 4);
    count = multiKeyMap.count(6);
    ASSERT_EQ(count, 1);

    // Non-existant key
    count = multiKeyMap.count(7);
    ASSERT_EQ(count, 0);
    count = multiKeyMap.count(keys[5]);
    ASSERT_EQ(count, 0);
}

TEST(MultiKeyMapTests, ValidateComplexMultiKeyMapContains) {
    ComplexMultiKeyMapType multiKeyMap;

    std::vector<std::tuple<int, char, bool>> keys = {
        std::make_tuple(5, 'c', true),
        std::make_tuple(5, 'c', false),
        std::make_tuple(5, 'b', true),
        std::make_tuple(5, 'd', false),
        std::make_tuple(6, 'd', false),
        std::make_tuple(7, 'z', false) // fake key
    };
    std::vector<float> vals = {
        1,
        2,
        3,
        4,
        5
    };

    auto result = multiKeyMap.insert(keys[0], vals[0]);
    ASSERT_EQ(result, 1);
    result = multiKeyMap.insert(keys[1], vals[1]);
    ASSERT_EQ(result, 1);
    result = multiKeyMap.insert(keys[2], vals[2]);
    ASSERT_EQ(result, 1);
    result = multiKeyMap.insert(keys[3], vals[3]);
    ASSERT_EQ(result, 1);
    result = multiKeyMap.insert(keys[4], vals[4]);
    ASSERT_EQ(result, 1);

    // Full key
    auto contains = multiKeyMap.contains(keys[1]);
    ASSERT_EQ(contains, true);

    // Partial key
    contains = multiKeyMap.contains(5, 'c');
    ASSERT_EQ(contains, true);
    contains = multiKeyMap.contains(5);
    ASSERT_EQ(contains, true);
    contains = multiKeyMap.contains(6);
    ASSERT_EQ(contains, true);

    // Non-existant key
    contains = multiKeyMap.contains(7);
    ASSERT_EQ(contains, false);
    contains = multiKeyMap.contains(keys[5]);
    ASSERT_EQ(contains, false);
}

TEST(MultiKeyMapTests, ValidateComplexMultiKeyMapCopy) {
    ComplexMultiKeyMapType multiKeyMap;

    std::vector<std::tuple<int, char, bool>> keys = {
        std::make_tuple(5, 'c', true),
        std::make_tuple(5, 'c', false),
        std::make_tuple(5, 'b', true),
        std::make_tuple(5, 'd', false),
        std::make_tuple(6, 'd', false),
        std::make_tuple(7, 'z', false) // fake key
    };
    std::vector<float> vals = {
        1,
        2,
        3,
        4,
        5
    };

    auto result = multiKeyMap.insert(keys[0], vals[0]);
    ASSERT_EQ(result, 1);
    result = multiKeyMap.insert(keys[1], vals[1]);
    ASSERT_EQ(result, 1);
    result = multiKeyMap.insert(keys[2], vals[2]);
    ASSERT_EQ(result, 1);
    result = multiKeyMap.insert(keys[3], vals[3]);
    ASSERT_EQ(result, 1);
    result = multiKeyMap.insert(keys[4], vals[4]);
    ASSERT_EQ(result, 1);

    // operator=
    ComplexMultiKeyMapType multiKeyMap2;
    multiKeyMap2 = multiKeyMap;

    for(auto&& [k,v] : multiKeyMap) {
        ASSERT_TRUE(multiKeyMap2.contains(k));
        ASSERT_EQ(multiKeyMap2.at(k), v);
    }

    multiKeyMap[keys[2]] = -32;
    ASSERT_EQ(multiKeyMap.at(keys[2]), -32);
    ASSERT_NE(multiKeyMap2.at(keys[2]), -32);

    ASSERT_EQ(multiKeyMap.size(), multiKeyMap2.size());

    // copy constructor
    ComplexMultiKeyMapType multiKeyMap3(multiKeyMap);

    for(auto&& [k,v] : multiKeyMap) {
        ASSERT_TRUE(multiKeyMap3.contains(k));
        ASSERT_EQ(multiKeyMap3.at(k), v);
    }

    vals[2] = multiKeyMap[keys[2]] = -25;
    multiKeyMap2[keys[2]] = 17;
    ASSERT_EQ(multiKeyMap.at(keys[2]), -25);
    ASSERT_EQ(multiKeyMap2.at(keys[2]), 17);
    ASSERT_NE(multiKeyMap3.at(keys[2]), -25);
    ASSERT_NE(multiKeyMap3.at(keys[2]), 17);

    ASSERT_EQ(multiKeyMap.size(), multiKeyMap3.size());

    // move constructor
    auto mkm1_bkup = multiKeyMap; // now that we know copies work, take a backup
                                  //   for verification
    ComplexMultiKeyMapType multiKeyMap4(std::move(multiKeyMap));

    for(auto&& [k,v] : mkm1_bkup) {
        ASSERT_TRUE(multiKeyMap4.contains(k));
        ASSERT_EQ(multiKeyMap4.at(k), v);
    }

    // Make sure that old multiKeyMap is empty (size may still exist)
    ASSERT_EQ(multiKeyMap.begin(), multiKeyMap.end());

    multiKeyMap2[keys[2]] = 1024;
    multiKeyMap3[keys[2]] = -9999999;
    ASSERT_EQ(multiKeyMap4.at(keys[2]), -25);
    ASSERT_EQ(multiKeyMap2.at(keys[2]), 1024);
    ASSERT_EQ(multiKeyMap3.at(keys[2]), -9999999);

    ASSERT_EQ(multiKeyMap.size(), multiKeyMap3.size());
}

TEST(MultiKeyMapTests, ValidateComplexMultiKeyMapComparison) {
    ComplexMultiKeyMapType multiKeyMap;

    std::vector<std::tuple<int, char, bool>> keys = {
        std::make_tuple(5, 'c', true),
        std::make_tuple(5, 'c', false),
        std::make_tuple(5, 'b', true),
        std::make_tuple(5, 'd', false),
        std::make_tuple(6, 'd', false),
        std::make_tuple(7, 'z', false)
    };
    std::vector<float> vals = {
        1,
        2,
        3,
        4,
        5
    };

    auto result = multiKeyMap.insert(keys[0], vals[0]);
    ASSERT_EQ(result, 1);
    result = multiKeyMap.insert(keys[1], vals[1]);
    ASSERT_EQ(result, 1);
    result = multiKeyMap.insert(keys[2], vals[2]);
    ASSERT_EQ(result, 1);
    result = multiKeyMap.insert(keys[3], vals[3]);
    ASSERT_EQ(result, 1);
    result = multiKeyMap.insert(keys[4], vals[4]);
    ASSERT_EQ(result, 1);

    // map 2, make it the same
    ComplexMultiKeyMapType multiKeyMap2;

    result = multiKeyMap2.insert(keys[0], vals[0]);
    ASSERT_EQ(result, 1);
    result = multiKeyMap2.insert(keys[1], vals[1]);
    ASSERT_EQ(result, 1);
    result = multiKeyMap2.insert(keys[2], vals[2]);
    ASSERT_EQ(result, 1);
    result = multiKeyMap2.insert(keys[3], vals[3]);
    ASSERT_EQ(result, 1);
    result = multiKeyMap2.insert(keys[4], vals[4]);
    ASSERT_EQ(result, 1);

    // Make sure both maps are equal
    ASSERT_EQ(multiKeyMap, multiKeyMap2);

    // Modify map 2, and compare again (they should be different)
    result = multiKeyMap2.insert(keys[5], -3.14);
    ASSERT_EQ(result, 1);

    ASSERT_NE(multiKeyMap, multiKeyMap2);

    // erase value from map 2, and compare (they should be equal)
    multiKeyMap2.erase(keys[5]);

    // Make sure both maps are equal again
    ASSERT_EQ(multiKeyMap, multiKeyMap2);
}

TEST(MultiKeyMapTests, ValidateComplexMultiKeyMapSwap) {
    ComplexMultiKeyMapType multiKeyMap;

    std::vector<std::tuple<int, char, bool>> keys = {
        std::make_tuple(5, 'c', true),
        std::make_tuple(5, 'c', false),
        std::make_tuple(5, 'b', true),
        std::make_tuple(5, 'd', false),
        std::make_tuple(6, 'd', false),
        std::make_tuple(7, 'z', false)
    };
    std::vector<float> vals = {
        1,
        2,
        3,
        4,
        5
    };

    auto result = multiKeyMap.insert(keys[0], vals[0]);
    ASSERT_EQ(result, 1);
    result = multiKeyMap.insert(keys[1], vals[1]);
    ASSERT_EQ(result, 1);
    result = multiKeyMap.insert(keys[2], vals[2]);
    ASSERT_EQ(result, 1);
    result = multiKeyMap.insert(keys[3], vals[3]);
    ASSERT_EQ(result, 1);
    result = multiKeyMap.insert(keys[4], vals[4]);
    ASSERT_EQ(result, 1);

    ComplexMultiKeyMapType multiKeyMap2;

    multiKeyMap.swap(multiKeyMap2);

    auto count = 0;
    for(auto&& [k,v] : multiKeyMap2) {
        std::cout << "{" << std::get<0>(k) << ", "
                         << std::get<1>(k) << ", "
                         << std::get<2>(k) << "} => " << v << std::endl;

        ASSERT_EQ(k, keys[count]);
        ASSERT_FLOAT_EQ(v, vals[count]);
        ++count;
    }

    ASSERT_TRUE(multiKeyMap.empty());

    // Test std::swap

    std::swap(multiKeyMap, multiKeyMap2);

    count = 0;
    for(auto&& [k,v] : multiKeyMap) {
        std::cout << "{" << std::get<0>(k) << ", "
                         << std::get<1>(k) << ", "
                         << std::get<2>(k) << "} => " << v << std::endl;

        ASSERT_EQ(k, keys[count]);
        ASSERT_FLOAT_EQ(v, vals[count]);
        ++count;
    }

    ASSERT_TRUE(multiKeyMap2.empty());
}

TEST(MultiKeyMapTests, ValidateComplexMultiKeyMapMerge) {
    ComplexMultiKeyMapType multiKeyMap;

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

    std::vector<std::tuple<int, char, bool>> keys2 = {
        keys[0],
        keys[1],
        std::make_tuple(-15, 'd', true),
        std::make_tuple(7, 'z', false)
    };
    std::vector<float> vals2 = {
        1,
        2,
        3,
        5
    };

    auto result = multiKeyMap.insert(keys[0], vals[0]);
    ASSERT_EQ(result, 1);
    result = multiKeyMap.insert(keys[1], vals[1]);
    ASSERT_EQ(result, 1);
    result = multiKeyMap.insert(keys[2], vals[2]);
    ASSERT_EQ(result, 1);
    result = multiKeyMap.insert(keys[3], vals[3]);
    ASSERT_EQ(result, 1);
    result = multiKeyMap.insert(keys[4], vals[4]);
    ASSERT_EQ(result, 1);

    ComplexMultiKeyMapType multiKeyMap2;

    // Merge full map into empty map, result should be the same as a swap
    multiKeyMap2.merge(multiKeyMap);

    auto count = 0;
    for(auto&& [k,v] : multiKeyMap2) {
        std::cout << "{" << std::get<0>(k) << ", "
                         << std::get<1>(k) << ", "
                         << std::get<2>(k) << "} => " << v << std::endl;

        ASSERT_EQ(k, keys[count]);
        ASSERT_FLOAT_EQ(v, vals[count]);
        ++count;
    }

    ASSERT_TRUE(multiKeyMap.empty());

    // Merge map that only has overlaps. Should be no-op
    {
        auto mkm2_bkup = multiKeyMap2;

        ComplexMultiKeyMapType multiKeyMap3;

        // Only insert some values, so they're not equal
        result = multiKeyMap3.insert(keys[0], vals[0]);
        ASSERT_EQ(result, 1);
        result = multiKeyMap3.insert(keys[2], vals[2]);
        ASSERT_EQ(result, 1);

        auto mkm3_bkup = multiKeyMap3;

        // merge maps with only overlaps, nothing should happen
        multiKeyMap2.merge(multiKeyMap3);

        ASSERT_EQ(multiKeyMap2, mkm2_bkup);
        ASSERT_EQ(multiKeyMap3, mkm3_bkup);
    }

    // Merge map with some overlaps.
    ComplexMultiKeyMapType multiKeyMap4;

    // Only insert some values, so they're not equal
    result = multiKeyMap4.insert(keys2[0], vals2[0]);
    ASSERT_EQ(result, 1);
    result = multiKeyMap4.insert(keys2[1], vals2[1]);
    ASSERT_EQ(result, 1);
    result = multiKeyMap4.insert(keys2[2], vals2[2]);
    ASSERT_EQ(result, 1);
    result = multiKeyMap4.insert(keys2[3], vals2[3]);
    ASSERT_EQ(result, 1);

    multiKeyMap2.merge(multiKeyMap4);

    // Make sure that map 2 still has the original values and the new values
    // Size (number of unique values):
    ASSERT_EQ(multiKeyMap2.size(), 7);

    // Original:
    ASSERT_TRUE(multiKeyMap2.contains(keys[0]));
    ASSERT_FLOAT_EQ(multiKeyMap2.at(keys[0]), vals[0]);
    ASSERT_TRUE(multiKeyMap2.contains(keys[1]));
    ASSERT_FLOAT_EQ(multiKeyMap2.at(keys[1]), vals[1]);
    ASSERT_TRUE(multiKeyMap2.contains(keys[2]));
    ASSERT_FLOAT_EQ(multiKeyMap2.at(keys[2]), vals[2]);
    ASSERT_TRUE(multiKeyMap2.contains(keys[3]));
    ASSERT_FLOAT_EQ(multiKeyMap2.at(keys[3]), vals[3]);
    ASSERT_TRUE(multiKeyMap2.contains(keys[4]));
    ASSERT_FLOAT_EQ(multiKeyMap2.at(keys[4]), vals[4]);
    // New:
    ASSERT_TRUE(multiKeyMap2.contains(keys2[2]));
    ASSERT_FLOAT_EQ(multiKeyMap2.at(keys2[2]), vals2[2]);
    ASSERT_TRUE(multiKeyMap2.contains(keys2[3]));
    ASSERT_FLOAT_EQ(multiKeyMap2.at(keys2[3]), vals2[3]);

    // Make sure map 4 only has the 2 overlapping values
    ASSERT_EQ(multiKeyMap4.size(), 2);

    ASSERT_TRUE(multiKeyMap4.contains(keys2[0]));
    ASSERT_FLOAT_EQ(multiKeyMap4.at(keys2[0]), vals2[0]);
    ASSERT_TRUE(multiKeyMap4.contains(keys2[1]));
    ASSERT_FLOAT_EQ(multiKeyMap4.at(keys2[1]), vals2[1]);
}

TEST(MultiKeyMapTests, ValidateComplexMultiKeyMapInitializingConstructor) {
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

    std::vector<std::pair<std::tuple<int, char, bool>, float>> key_vals = {
        { keys[0], vals[0] },
        { keys[1], vals[1] },
        { keys[2], vals[2] },
        { keys[3], vals[3] },
        { keys[4], vals[4] }
    };

    ComplexMultiKeyMapType multiKeyMap {
        { keys[0], vals[0] },
        { keys[1], vals[1] },
        { keys[2], vals[2] },
        { keys[3], vals[3] },
        { keys[4], vals[4] }
    };

    auto count = 0;
    for(auto&& [k,v] : multiKeyMap) {
        std::cout << "{" << std::get<0>(k) << ", "
                         << std::get<1>(k) << ", "
                         << std::get<2>(k) << "} => " << v << std::endl;

        ASSERT_EQ(k, keys[count]);
        ASSERT_FLOAT_EQ(v, vals[count]);
        ++count;
    }

    ComplexMultiKeyMapType multiKeyMap2(
        key_vals.begin(),
        key_vals.end()
    );

    count = 0;
    for(auto&& [k,v] : multiKeyMap2) {
        std::cout << "{" << std::get<0>(k) << ", "
                         << std::get<1>(k) << ", "
                         << std::get<2>(k) << "} => " << v << std::endl;

        ASSERT_EQ(k, keys[count]);
        ASSERT_FLOAT_EQ(v, vals[count]);
        ++count;
    }
}

TEST(MultiKeyMapTests, PrintMapTests) {
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
    std::vector<std::pair<std::tuple<int, char, bool>, float>> key_vals = {
        { keys[0], vals[0] },
        { keys[1], vals[1] },
        { keys[2], vals[2] },
        { keys[3], vals[3] },
        { keys[4], vals[4] }
    };

    ComplexMultiKeyMapType multiKeyMap(
        key_vals.begin(),
        key_vals.end()
    );

    std::cout << multiKeyMap << std::endl;
}

