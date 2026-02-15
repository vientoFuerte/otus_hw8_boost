
#define BOOST_TEST_MODULE BayanTests
#include <boost/test/included/unit_test.hpp>
#include "bayan.h"
#include <fstream>

fs::path createFile(const fs::path& dir, const std::string& name, size_t size, char content = 'a') {
    fs::path p = dir / name;
    std::ofstream ofs(p.string(), std::ios::binary);
    std::string data(size, content);
    ofs.write(data.c_str(), size);
    return p;
}

// 1. Тестирование matchesMask
BOOST_AUTO_TEST_CASE(test_matchesMask) {
    std::vector<std::string> masks = {"*.txt", "*.cpp"};
    BOOST_CHECK(matchesMask("file.txt", masks) == true);
    BOOST_CHECK(matchesMask("file.cpp", masks) == true);
    BOOST_CHECK(matchesMask("file.h", masks) == false);
    BOOST_CHECK(matchesMask("file.TXT", masks) == true); // регистронезависимо
    BOOST_CHECK(matchesMask("any", {"*"}) == true);
}

// 2. Тестирование сравнения идентичных файлов
BOOST_AUTO_TEST_CASE(test_areFilesIdentical_same) {
    fs::path root = fs::temp_directory_path() / fs::unique_path();
    fs::create_directories(root);
    auto p1 = createFile(root, "f1", 100, 'x');
    auto p2 = createFile(root, "f2", 100, 'x'); // такое же содержимое
    
    FileInfo fi1{p1, 100, {}};
    FileInfo fi2{p2, 100, {}};
    
    bool res = areFilesIdentical(fi1, fi2, 32); // размер блока 32
    BOOST_CHECK(res == true);
    
    fs::remove_all(root);
}
