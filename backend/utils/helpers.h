#pragma once
#include <string>
#include <sstream>

// Конвертация строки с запятыми в PostgreSQL массив
inline std::string toPgArray(const std::string& input) {
    if (input.empty()) return "{}";
    
    std::string result = "{";
    std::stringstream ss(input);
    std::string item;
    bool first = true;
    
    while (std::getline(ss, item, ',')) {
        // Убираем пробелы
        item.erase(0, item.find_first_not_of(" "));
        item.erase(item.find_last_not_of(" ") + 1);
        
        if (!item.empty()) {
            if (!first) result += ",";
            result += "\"" + item + "\"";
            first = false;
        }
    }
    result += "}";
    return result;
}
