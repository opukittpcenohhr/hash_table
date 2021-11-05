#define CATCH_CONFIG_MAIN
#include <catch2/catch_test_macros.hpp>

#include <iostream>
#include <iomanip>

#include "hash_table.h"

TEST_CASE("Hash Table Test") {
    HashTable<std::string, int> hash_table;
    hash_table["a"] = 3;
    REQUIRE(hash_table["a"] == 3);
    hash_table["a"]++;
    REQUIRE(hash_table["a"] == 4);
    hash_table["b"] += hash_table["a"];
    REQUIRE(hash_table["b"] == 4);
    hash_table["a"] += hash_table["b"];
    REQUIRE(hash_table["a"] == 8);
    hash_table["b"] = 8;
    for (auto &[key, value] : hash_table) {
        REQUIRE(!key.empty());
        REQUIRE(value == 8);
    }
    hash_table.clear();
    REQUIRE(hash_table.empty());
    REQUIRE(hash_table["a"] == 0);
    REQUIRE(hash_table["b"] == 0);
    hash_table.clear();
    {
        const auto& [it, inserted] = hash_table.insert({"c", 3});
        REQUIRE(inserted);
        REQUIRE(it->first == "c");
        REQUIRE(it->second == 3);
    }
    {
        const auto& [it, inserted] = hash_table.insert({"c", 3});
        REQUIRE(!inserted);
        REQUIRE(it->first == "c");
        REQUIRE(it->second == 3);
    }
    auto new_hash_table = hash_table;
    hash_table.clear();
    REQUIRE(hash_table.empty());
    REQUIRE(!new_hash_table.empty());
    REQUIRE(new_hash_table["c"] == 3);
}

TEST_CASE("Hash Table Efficency Test") {
    auto start = clock();
    const size_t EXPECTED_SIZE = 2e6;
    HashTable<int, int> hash_table;
    for (size_t i = 0; i < EXPECTED_SIZE; ++i) {
        hash_table[i] = i;
    }
    REQUIRE(EXPECTED_SIZE == hash_table.size());
    for (const auto& [key, value] : hash_table) {
        REQUIRE(key == value);
        REQUIRE(hash_table.at(key) == value);
        REQUIRE(hash_table.find(key)->second == value);
    }
    auto end = clock();
    std::cerr << "Inserted " << EXPECTED_SIZE << " elements in " << std::fixed << std::setprecision(4) << (double)(end - start) / CLOCKS_PER_SEC << std::endl;
}
