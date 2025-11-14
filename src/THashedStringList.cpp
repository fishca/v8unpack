#include "THashedStringList.h"
#include <stdexcept>
#include <algorithm>

THashedStringList::THashedStringList()
{
    m_lines.reserve(16);
}

void THashedStringList::Add(const std::string& line)
{
    auto pair = SplitLine(line);
    const std::string& key = pair.first;
    const std::string& value = pair.second;

    auto it = m_map.find(key);
    if (it != m_map.end())
    {
        // Ключ уже существует — обновляем значение
        it->second = value;
        // Обновляем и в векторе (для сохранения порядка)
        for (auto& entry : m_lines)
        {
            if (entry.first == key)
            {
                entry.second = value;
                break;
            }
        }
    }
    else
    {
        // Новый ключ — добавляем
        m_lines.emplace_back(key, value);
        m_map[key] = value;
    }
}

void THashedStringList::SetValue(const std::string& key, const std::string& value)
{
    auto it = m_map.find(key);
    if (it != m_map.end())
    {
        // Обновляем существующее значение
        it->second = value;
        for (auto& entry : m_lines)
        {
            if (entry.first == key)
            {
                entry.second = value;
                break;
            }
        }
    }
    else
    {
        // Добавляем новую запись
        m_lines.emplace_back(key, value);
        m_map[key] = value;
    }
}

std::string THashedStringList::GetValue(const std::string& key) const
{
    auto it = m_map.find(key);
    if (it != m_map.end())
        return it->second;
    return std::string(); // пустая строка
}

bool THashedStringList::HasKey(const std::string& key) const
{
    return m_map.find(key) != m_map.end();
}

bool THashedStringList::RemoveKey(const std::string& key)
{
    auto it = m_map.find(key);
    if (it == m_map.end())
        return false;

    // Удаляем из хеш-карты
    m_map.erase(it);

    // Удаляем из вектора (сохраняем порядок, но удаляем элемент)
    m_lines.erase(
        std::remove_if(m_lines.begin(), m_lines.end(),
            [&key](const std::pair<std::string, std::string>& entry) {
                return entry.first == key;
            }),
        m_lines.end()
    );

    return true;
}

void THashedStringList::Clear()
{
    m_lines.clear();
    m_map.clear();
}

size_t THashedStringList::Count() const
{
    return m_lines.size();
}

std::string THashedStringList::GetLine(size_t index) const
{
    if (index >= m_lines.size())
        throw std::out_of_range("Index out of range");
    return m_lines[index].first + "=" + m_lines[index].second;
}

std::string THashedStringList::GetKey(size_t index) const
{
    if (index >= m_lines.size())
        throw std::out_of_range("Index out of range");
    return m_lines[index].first;
}

std::string THashedStringList::GetValueByIndex(size_t index) const
{
    if (index >= m_lines.size())
        throw std::out_of_range("Index out of range");
    return m_lines[index].second;
}

// Статический метод для разбора строки "Key=Value"
std::pair<std::string, std::string> THashedStringList::SplitLine(const std::string& line)
{
    size_t pos = line.find('=');
    if (pos == std::string::npos)
        return std::make_pair(line, ""); // Нет '=', считаем value пустым

    std::string key = line.substr(0, pos);
    std::string value = line.substr(pos + 1);
    return std::make_pair(key, value);
}