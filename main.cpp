// main.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.

#include <iostream>
#include "bayan.h"
#include <boost/program_options.hpp> //для парсинга командной строки


namespace po = boost::program_options; // псевдоним для сокращения записей

int main(int argc, char** argv)
{
    std::string filename;
    
    // объект хранит информацию о том, какие параметры поддерживает программа
    po::options_description desc("Программа Bayan - поиск дубликатов файлов");
    
    // добавляем параметры, которые будем парсить
    desc.add_options()
        ("help,h", "Показать справку")
        ("file,f", po::value<std::string>(&filename), "Имя файла для обработки")
        ("directory,d", po::value<std::string>(), "Директория для сканирования")
        ("verbose,v", "Подробный вывод")
        ("level,l", po::value<int>()->default_value(1), "Уровень сканирования");
        
    // контейнер для хранения распарсенных значений параметров
    po::variables_map vm;
    
    try{
          // сохраняем распарсенные данные в переменную vm
          po::store(po::parse_command_line(argc, argv, desc), vm);
          // применяем значения к переменным (например, к filename)
          po::notify(vm);  
    
    }

    catch(const po::error& e)
    {
      std::cerr<<"Error parsing command line: "<<e.what()<<std::endl;
      return 1;
    }
    
    
    if (vm.count("help")) {
        std::cout << desc << "\n";
        return 0;
    }
    
    // обращение к аргументам.
    if (vm.count("directory")) {
        std::cout << "Параметр 'directory': " << vm["directory"].as<std::string>() << "\n";
    } else {
        std::cout << "Параметр 'directory' не указан\n";
    }
    
    // Выводим все аргументы командной строки (сырые)
    std::cout << "Всего аргументов: " << argc << "\n";
    for (int i = 0; i < argc; i++) {
        std::cout << "argv[" << i << "] = " << argv[i] << "\n";
    }
    std::cout << "\n";
  
}
