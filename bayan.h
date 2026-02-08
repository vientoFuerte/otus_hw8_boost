#pragma once

#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <cstdint> // для std::uintmax_t
#include <boost/program_options.hpp> //для парсинга командной строки
#include <boost/filesystem.hpp>
#include <boost/crc.hpp>
#include <boost/range/algorithm/sort.hpp>  // Для boost::range::sort



namespace po = boost::program_options; // псевдоним для сокращения записей
namespace fs = boost::filesystem;

struct FileInfo {
    fs::path path;
    uintmax_t size;
};

struct BayanConfig {
    std::vector<std::string> directories;
    std::vector<std::string> exclude_dirs;
    std::vector<std::string> file_masks;      // Маски файлов (например, *.txt, *.cpp)
    std::string hash_algorithm = "crc32"; 
    int scan_depth = -1;
    size_t min_size = 1;                      // Минимальный размер файла (по умолчанию 1 байт)
    size_t block_size = 1024;                 // Размер блока для чтения файлов
  
};


// Функция для полного сравнения файлов по всем блокам
bool areFilesIdentical(const FileInfo& fi1, const FileInfo& fi2, size_t block_size);

// Функция для сравнения двух файлов одного размера по конкретному блоку
bool compareSingleBlock(const FileInfo& fi1, const FileInfo& fi2, size_t block_size, size_t blockNum);

// Функция для выделения групп файлов одного размера
std::vector<std::vector<FileInfo>> extractSameSizeGroups(std::vector<FileInfo>& finfo);

// Функция сбора файлов
 std::vector<FileInfo> collectFiles(const BayanConfig& conf);
