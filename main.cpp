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
        
      // Сортировка по размеру с помощью Boost
      boost::range::sort(finfo, 
          [](const FileInfo& a, const FileInfo& b) {
              return a.size < b.size;  // Сортировка по возрастанию
          });
        
      // Выводим путь к каждому элементу
      std::cout << "\nall directories:"<< std::endl;
       for (const auto& inf : finfo) {
          // Выводим путь к каждому элементу
          std::cout << inf.path.string()<<"  "<< inf.size << std::endl;
      }
    }
  
  return 0;
}}
