//
//  KeyValueCollection.hpp
//  KeyValueParser
//
//  Created by Fedor Artemenkov on 28.10.24.
//

#pragma once

#include <string>
#include <unordered_map>
#include <vector>

class KeyValueEntry
{
public:
    void debugPrint() const;
    std::unordered_map<std::string, std::string> properties;
};

class KeyValueCollection
{
public:
    void initFromString(const std::string& input);
    void debugPrint() const;
    
    std::vector<KeyValueEntry> getAllWithKeyValue(const std::string& key, const std::string& value);
    
    
private:
    std::vector<KeyValueEntry> entries;
};

