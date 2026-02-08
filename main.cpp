// main.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.

#include <iostream>
#include "bayan.h"



// Функция сбора файлов
 std::vector<FileInfo> collectFiles(const BayanConfig& conf) {
   std::vector<FileInfo> files;
   for (const auto& dir : conf.directories) {
     for (fs::directory_iterator it(dir), end; it != end; ++it) {
        // проверка минимального размера
        size_t size = fs::file_size(it->path());
         if (size >= conf.min_size) {
             files.push_back({it->path(),size});
         }
      }
    }
   return files;
 }
 
 
// Функция для выделения групп файлов одного размера
std::vector<std::vector<FileInfo>> extractSameSizeGroups(std::vector<FileInfo>& finfo) {
    std::vector<std::vector<FileInfo>> groups;
    
    // Используем два указателя для выделения групп
    auto group_start = finfo.begin();
    auto current = finfo.begin();
    
    while (current != finfo.end())
    {
        // Запоминаем размер текущей группы
        uintmax_t current_size = group_start->size;
        // Пока не достигли конца и размер одинаковый
        while (current != finfo.end() && current->size == current_size) {
            ++current;
        }   
        // Если в группе больше одного файла
        if (std::distance(group_start, current) > 1) {
            // Копируем группу
            groups.emplace_back(group_start, current);
        }      
        // Переходим к следующей группе
        group_start = current;
        
    }
    return groups;
}

// Функция для сравнения двух файлов одного размера по конкретному блоку
bool compareSingleBlock(const FileInfo& fi1, const FileInfo& fi2, size_t block_size, size_t blockNum) {
    
    std::ifstream f1(fi1.path, std::ios::binary);
    std::ifstream f2(fi2.path, std::ios::binary);
    
    if (!f1.is_open() || !f2.is_open()) {
        return false;
    }
    
    // Вычисляем смещение для текущего блока
    size_t offset = blockNum * block_size;
    
    // Проверяем, не выходим ли мы за пределы файла
    if (offset >= fi1.size) {
        // Блок за пределами файла - считаем их идентичными
        // (оба файла имеют одинаковый размер, поэтому оба за пределами)
        return true;
    }
    
    // Вычисляем сколько байт нужно прочитать
    size_t bytes_to_read = std::min(block_size, fi1.size - offset);
    
    std::vector<char> buffer1(block_size, 0);
    std::vector<char> buffer2(block_size, 0);
    
    // Читаем блоки
    f1.seekg(offset, std::ios::beg);
    f2.seekg(offset, std::ios::beg);
    
    f1.read(buffer1.data(), bytes_to_read);
    f2.read(buffer2.data(), bytes_to_read);
    
    // Проверяем, сколько байт было фактически прочитано
    size_t actually_read1 = f1.gcount();
    size_t actually_read2 = f2.gcount();
    
    if (actually_read1 != actually_read2 || actually_read1 != bytes_to_read) {
        return false; // Ошибка чтения
    }
    
    // Дополняем нулями если нужно (для последнего блока)
    if (bytes_to_read < block_size) {
        std::fill(buffer1.begin() + bytes_to_read, buffer1.end(), 0);
        std::fill(buffer2.begin() + bytes_to_read, buffer2.end(), 0);
    }
    
    // Вычисляем CRC32
    boost::crc_32_type crc1, crc2;
    crc1.process_bytes(buffer1.data(), block_size);
    crc2.process_bytes(buffer2.data(), block_size);
    
    return crc1.checksum() == crc2.checksum();
}

// Функция для полного сравнения файлов по всем блокам
bool areFilesIdentical(const FileInfo& fi1, const FileInfo& fi2, size_t block_size) {
     
    size_t total_blocks = (fi1.size + block_size - 1) / block_size;
    
    for (size_t blockNum = 0; blockNum < total_blocks; ++blockNum) {
        if (!compareSingleBlock(fi1, fi2, block_size, blockNum)) {
            return false;
        }
    }
    
    return true;
}

int main(int argc, char** argv)
{
    BayanConfig conf;
    
    // объект хранит информацию о том, какие параметры поддерживает программа
    po::options_description desc("Bayan program - file duplicate search");
    
    // добавляем параметры, которые будем парсить
    desc.add_options()
            ("help,h", "Show help message")
            ("directories,d", po::value<std::vector<std::string>>(&conf.directories)
                ->multitoken()->required(), 
                "Directories to scan (can be multiple, required)")
            ("exclude,e", po::value<std::vector<std::string>>(&conf.exclude_dirs)
                ->multitoken(), 
                "Directories to exclude from scanning (can be multiple)")
            ("level,l", po::value<int>(&conf.scan_depth)->default_value(-1), 
                "Scan depth (-1=unlimited, 0=current only, 1=one level deep, etc.)")
            ("min-size,s", po::value<size_t>(&conf.min_size)->default_value(1), 
                 "Minimum file size in bytes")
            ("block-size,b", po::value<size_t>(&conf.block_size)->default_value(1024),
            "Block size for file reading in bytes (default: 1024)")
            ("mask,m", po::value<std::vector<std::string>>(&conf.file_masks) 
                ->multitoken()
                ->default_value(std::vector<std::string>{"*"}, "*"),  // Значение по умолчанию
                "File mask filter (e.g., *.txt, *.cpp, can be multiple)")
            ("algorithm,a", po::value<std::string>(&conf.hash_algorithm)->default_value("md5"), 
            "Hash algorithm (md5, sha1, crc32, etc.)") ;
        
    // контейнер для хранения распарсенных значений параметров
    po::variables_map vm;
    
    try{
          // сохраняем распарсенные данные в переменную vm
          po::store(po::parse_command_line(argc, argv, desc), vm);
          
          
          // Проверка help до notify, чтобы не требовать обязательные параметры
          if (vm.count("help")) {
              std::cout << desc << "\n";
              return 0;
          }
      
          // применяем значения к переменным (например, к filename)
          po::notify(vm);  
    
    }

    catch(const po::error& e)
    {
      std::cerr<<"Error parsing command line: "<<e.what()<<std::endl;
      return 1;
    }
    
    
    // обращение к аргументам.
    if (vm.count("directories")) { 
        std::cout << "Directories to scan:\n";  
        for (const auto& dir : conf.directories) {  // цикл по всем директориям
            std::cout << "  - " << dir << "\n";
        }
    } else {
        std::cout << "Параметр 'directories' не указан\n";
    }
    
    if (vm.count("exclude")) {
        std::cout << "Directories to exclude:\n";
        for (const auto& dir : conf.exclude_dirs) {
            std::cout << "  - " << dir << "\n";
        }
    }
    
    if (vm.count("level")) {
        std::cout << "Scan level: " << conf.scan_depth << "\n";
    }
    
    if (vm.count("block-size")) {
        std::cout << "Block size: " << conf.block_size << "\n";
    }
    
     if (vm.count("mask")) {
        std::cout << "File masks:\n";
        for (const auto& mask : conf.file_masks) {
            std::cout << "  - " << mask << "\n";
        }
    }
    
    if (vm.count("algorithm")) {
        std::cout << "Hash algorithm: " << conf.hash_algorithm << "\n";
    }
    
    if (vm.count("min-size")) {
        std::cout << "Min file size: " << conf.min_size << "\n";
    }
    
    // Выводим все аргументы командной строки (сырые)
    /*std::cout << "Всего аргументов: " << argc << "\n";
    for (int i = 0; i < argc; i++) {
        std::cout << "argv[" << i << "] = " << argv[i] << "\n";
    }
    std::cout << "\n";*/
  
  //поиск дубликатов
  std::map<unsigned int, std::vector<std::string>> filesByHash;
   if (vm.count("directories")) { 
        for (const auto& dir : conf.directories) {  // цикл по всем директориям
            std::cout << "  - " << dir << "\n";
        }
  
    if (!conf.directories.empty()) {
      std::vector<FileInfo> finfo = collectFiles(conf);
        
      // Сортировка по размеру с помощью Boost (файлы разного размера не могут быть одинаковыми)
      boost::range::sort(finfo, 
          [](const FileInfo& a, const FileInfo& b) {
              return a.size < b.size;  // Сортировка по возрастанию
          });
      // Разбили на группы одинакового размера (в каждой больше одного файла).
      auto same_size_groups = extractSameSizeGroups(finfo);
        
      // Выводим все полученные группы
      std::cout << "\nall directories:"<< std::endl;
       for (const auto& group : same_size_groups) {
       
        // Список найденных дубликатов
        std::vector<std::string>duplicates;

          //в группе каждый файл сравниваем с остальными
          for (size_t i = 0; i < group.size(); ++i) {
              for (size_t j = i + 1; j < group.size(); ++j) {
                  // Сравниваем файлы group[i] и group[j]
                  bool files_equal = areFilesIdentical(group[i], group[j], conf.block_size);
                  
                  if (files_equal) {
                      duplicates.push_back(group[i].path.string());
                      duplicates.push_back(group[j].path.string());
                  }
              }
          }
          
          
                std::cout << "Duplicate found: " << std::endl;
          for (const auto& path : duplicates) {
              // Сравнить конкретный блок
              //bool block_equal = compareSingleBlock(file1, file2, block_size, 0);
    
              // Полное сравнение файлов
              //bool files_equal = areFilesIdentical(file1, file2, block_size);
              // Выводим путь к каждому элементу
              std::cout << path;
          }
          
          
          std::cout << std::endl;  
          //std::ifstream file(filename, std::ios::binary);
         // std::vector<std::string> block_hashes;
          //std::vector<char> buffer(block_size);

      }
    }
 }
  return 0;
}
