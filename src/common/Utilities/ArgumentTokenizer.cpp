#include "ArgumentTokenizer.h"
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include "Util.h"

ArgumentTokenizer::ArgumentTokenizer(const std::string &src, bool preserveQuotes)
{
    bool trim = true;
    bool quoteMode = false;
    bool chatLinkMode = false;
    char cPrev = '\0';
    std::string token = "";
    normalParamString = src;
    boost::trim(normalParamString);

    for (auto it = src.begin(); it != src.end(); it++)
    {
        char c = *it;

        if (c == ' ' || c == '\t' || c == '\b') // whitespace case
        {
            if (trim)
            {
                cPrev = c;
                continue;
            }

            if (quoteMode || chatLinkMode)
            {
                token += c;
            }
            else
            {
                if (token != "")
                    m_storage.push_back(token);
                token = "";
                trim = true;
            }
        }
        else if (c == '\"') // quote case
        {
            if (quoteMode)
            {
                if (token != "")
                    m_storage.push_back(token);
                token = "";
                trim = true;
                quoteMode = false;
            }
            else
            {
                quoteMode = true;
                trim = false;
                if (preserveQuotes)
                    token += c;
            }
        }
        else if (c == 'c')
        {
            trim = false;
            token += c;

            if (cPrev == '|' && !quoteMode)
                chatLinkMode = true;
        }
        else if (c == 'r')
        {
            trim = false;
            token += c;

            if (cPrev == '|' && !quoteMode)
            {
                chatLinkMode = false;
                m_storage.push_back(token);
                token = "";
                trim = true;
            }
        }
        else
        {
            trim = false;
            token += c;
        }

        cPrev = c;
    }

    if (token != "")
        m_storage.push_back(token);
}

ArgumentTokenizer::~ArgumentTokenizer()
{
}

void ArgumentTokenizer::LoadModifier(std::string modifier, uint32 paramCount)
{
    // Do not override previously defined modifier due to potential missing parameters from normal token storage
    if (m_modifiers.find(modifier) != m_modifiers.end())
        return;

    ModifierValueStorageType valueStorage;
    valueStorage.first = paramCount;
    valueStorage.second = TokenStorage(paramCount);

    bool modifierCheck = true;
    bool extract = false;
    int extractCount = paramCount;
    int modifierParamIndex = 0;
    int paramIndex = 0;
    std::vector<uint32> removeIndexes;

    // Copy modifier and its paramaters (if it has any) to modifier storage
    for (auto param : m_storage)
    {
        if (!IsModifier(param) && modifierCheck)
            break;
        else
            modifierCheck = false;

        if (param == modifier && !extract)
        {
            extract = true;
            removeIndexes.push_back(paramIndex);

            // remove from normalParamString
            boost::replace_first(normalParamString, param + ((int)m_storage.size() > (paramIndex + 1) ? " " : ""), "");

            if (extractCount == 0)
                break;
        }
        else if (extract && extractCount > 0)
        {
            valueStorage.second[modifierParamIndex] = param;
            removeIndexes.push_back(paramIndex);

            // remove from normalParamString
            boost::replace_first(normalParamString, param + ((int)m_storage.size() > (paramIndex + 1) ? " " : ""), "");

            modifierParamIndex++;
            extractCount--;
        }

        paramIndex++;
    }

    boost::trim(normalParamString);

    // If extract is still false, then given modifier doesn't exist, remove it from modifier storage
    if (!extract)
    {
        return;
    }

    m_modifiers[modifier] = valueStorage;

    // Remove modifier and its parameters (if it has any) from normal token storage
    int indexOffset = 0;
    for (auto index : removeIndexes)
    {
        m_storage.erase(m_storage.begin() + index - indexOffset);
        indexOffset++;
    }
}

std::string ArgumentTokenizer::GetModifierValue(std::string modifier, uint32 index)
{
    auto it = m_modifiers.find(modifier);
    if (it != m_modifiers.end())
    {
        if (index < it->second.second.size())
            return it->second.second[index];
    }

    return "";
}

std::string ArgumentTokenizer::ExtractChatLinkKey(std::string src, std::string type)
{
    if (src.empty())
        return "";

    std::string typePart = "|" + type + ":";
    std::string key = "";
    std::size_t pos = src.find(typePart); // find start pos of "|Hkeytype:" fragment first

                                          // Check for plain number first
    std::string plainNumber = src;
    boost::trim(plainNumber); // trim spaces
    plainNumber = plainNumber.substr(0, plainNumber.find(' ')); // get first token in case src had multiple ones
    if (isNumeric(plainNumber.c_str()))
        return plainNumber;

    // Do ChatLink check
    if (pos != std::string::npos)
    {
        auto it = src.begin();
        std::advance(it, pos + typePart.length());

        // if key part iteration ends without encountering ':' or '|', 
        // then link was malformed and we return empty string later on
        for (; it != src.end(); it++)
        {
            char c = *it;

            if (c == ':' || c == '|')
                return key;

            key += c;
        }
    }

    return "";
}