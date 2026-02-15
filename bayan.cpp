#include "bayan.h"

// Функция проверки, нужно ли исключить путь
bool isExcluded(const fs::path& p, const std::vector<std::string>& exclude_dirs) {
    // Нормализуем путь (убираем ".", ".." и приводим к абсолютному виду при необходимости)
    fs::path normalized = fs::absolute(p).lexically_normal();
    for (const auto& excl : exclude_dirs) {
        fs::path excl_path = fs::absolute(excl).lexically_normal();
        // Если текущий путь является поддиректорией исключённой, возвращаем true
        if (fs::equivalent(normalized, excl_path) || 
            normalized.string().find(excl_path.string()) == 0) {
            return true;
        }
    }
    return false;
}

// Функция сбора файлов
std::vector<FileInfo> collectFiles(const BayanConfig& conf) {
    std::vector<FileInfo> files;
    
    for (const auto& start_dir_str : conf.directories) {
        fs::path start_dir(start_dir_str);
        
        // Пропускаем, если стартовая директория исключена
        if (isExcluded(start_dir, conf.exclude_dirs)) {
            continue;
        }
        
        fs::recursive_directory_iterator it(start_dir), end;
        
        while (it != end) {
            // Ограничение глубины (если задано)
            if (conf.scan_depth >= 0 && it.depth() >= conf.scan_depth) {
                it.disable_recursion_pending();
            }
            
            const auto& path = it->path();
            
            // Если текущий элемент - исключённая директория, не заходим в неё
            if (fs::is_directory(path) && isExcluded(path, conf.exclude_dirs)) {
                it.disable_recursion_pending();
                ++it;
                continue;
            }
            
            // Обрабатываем обычные файлы
            if (fs::is_regular_file(path)) {
                uintmax_t size = fs::file_size(path);
                std::string filename = path.filename().string();
                if (size >= conf.min_size && matchesMask(filename, conf.file_masks)) {
                    files.push_back({path, size});
                }
            }
            
            ++it; // переход к следующему элементу
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
bool compareSingleBlock(std::ifstream& f1, std::ifstream& f2, size_t file_size, size_t block_size, size_t blockNum) {
     
   // Вычисляем смещение для текущего блока
    size_t offset = blockNum * block_size;
    
    // Проверяем, не выходим ли мы за пределы файла
    if (offset >= file_size) {
        return true;
    }
    
    // Вычисляем сколько байт нужно прочитать
    size_t bytes_to_read = std::min(block_size, file_size - offset);
    
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
     
    // Открываем файлы для чтения
    std::ifstream f1(fi1.path, std::ios::binary);
    std::ifstream f2(fi2.path, std::ios::binary);
    
    // Проверяем, успешно ли открыты файлы
    if (!f1.is_open() || !f2.is_open()) {
        return false;
    }
    
    size_t total_blocks = (fi1.size + block_size - 1) / block_size;
    
    for (size_t blockNum = 0; blockNum < total_blocks; ++blockNum) {
        if (!compareSingleBlock(f1, f2, fi1.size, block_size, blockNum)) {
            return false;
        }
    }

    return true;
}


bool matchesMask(const std::string& filename, const std::vector<std::string>& masks) {
    // Если масок нет - файл проходит
    if (masks.empty()) {
        return true;
    }
    
    // Если есть специальная маска "*" - все файлы проходят
    for (const auto& mask : masks) {
        if (mask == "*") {
            return true;
        }
    }
    
    // Проверяем каждую маску
    for (const auto& mask : masks) {
        // Используем FNM_CASEFOLD для регистронезависимого сравнения
        // и FNM_PATHNAME чтобы '*' не совпадал с '/' (для путей)
        if (fnmatch(mask.c_str(), filename.c_str(), FNM_CASEFOLD | FNM_PATHNAME) == 0) {
            return true;
        }
    }
    
    return false;
}

