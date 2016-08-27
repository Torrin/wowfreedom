#include <string>
#include <vector>
#include <unordered_map>
#include <cctype> 
#include "Define.h"

class ArgumentTokenizer
{
public:
    typedef std::vector<std::string> TokenStorage;
    typedef std::pair<uint32, TokenStorage> ModifierValueStorageType;
    typedef std::unordered_map<std::string, ModifierValueStorageType> ModifierStorageType;

    typedef TokenStorage::size_type size_type;
    typedef TokenStorage::const_iterator const_iterator;
    typedef TokenStorage::reference reference;
    typedef TokenStorage::const_reference const_reference;

public:
    ArgumentTokenizer(const std::string &src, bool preserveQuotes = false);
    ~ArgumentTokenizer();

    void LoadModifier(std::string modifier, uint32 paramCount);
    bool ModifierExists(std::string modifier) { return m_modifiers.find(modifier) != m_modifiers.end(); }

    std::string GetModifierValue(std::string modifier, uint32 index);

    std::string GetUntokenizedString() { return normalParamString; }

    template<typename T = std::string>
    T TryGetParam(uint32 index) { }

    template<>
    std::string TryGetParam<std::string>(uint32 index) { return (m_storage.size() > index) ? m_storage[index] : ""; }

    template<>
    float TryGetParam<float>(uint32 index) { return atof(TryGetParam<std::string>(index).c_str()); }

    template<>
    uint32 TryGetParam<uint32>(uint32 index) { return strtoul(TryGetParam<std::string>(index).c_str(), nullptr, 10); }

    template<>
    uint64 TryGetParam<uint64>(uint32 index) { return strtoull(TryGetParam<std::string>(index).c_str(), nullptr, 10); }

    template<typename T = std::string>
    T TryGetParam(uint32 index, std::string keyType) { }

    template<>
    std::string TryGetParam<std::string>(uint32 index, std::string keyType) { return ExtractChatLinkKey(TryGetParam<std::string>(index), keyType); }

    template<>
    uint32 TryGetParam<uint32>(uint32 index, std::string keyType) { return strtoul(ExtractChatLinkKey(TryGetParam<std::string>(index), keyType).c_str(), nullptr, 10); }

    template<>
    uint64 TryGetParam<uint64>(uint32 index, std::string keyType) { return strtoull(ExtractChatLinkKey(TryGetParam<std::string>(index), keyType).c_str(), nullptr, 10); }

    ModifierStorageType const* modifiers() { return &m_modifiers; }
    const_iterator begin() const { return m_storage.begin(); }
    const_iterator end() const { return m_storage.end(); }
    bool empty() { return m_storage.empty(); }

    size_type size() const { return m_storage.size(); }

    reference operator [] (size_type i) { return m_storage[i]; }
    const_reference operator [] (size_type i) const { return m_storage[i]; }
protected:
    bool IsModifier(std::string str) { return (str.length() > 1 && str[0] == '-' && std::isalpha(str[1])); }
    std::string ExtractChatLinkKey(std::string src, std::string type);
protected:
    TokenStorage m_storage;
    ModifierStorageType m_modifiers;
    std::string normalParamString;
};