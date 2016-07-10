#ifndef _FREEDOMMGR_H
#define _FREEDOMMGR_H

#include "ObjectMgr.h"

class AdvancedArgumentTokenizer
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
    AdvancedArgumentTokenizer(const std::string &src, bool preserveQuotes = false);
    ~AdvancedArgumentTokenizer();

    void LoadModifier(std::string modifier, uint32 paramCount);
    std::string GetModifierValue(std::string modifier, uint32 index);
    bool ModifierExists(std::string modifier) { return m_modifiers.find(modifier) != m_modifiers.end(); }
    std::string TryGetParam(uint32 index) { return (m_storage.size() > index) ? m_storage[index] : ""; }

    ModifierStorageType const* modifiers() { return &m_modifiers; }
    const_iterator begin() const { return m_storage.begin(); }
    const_iterator end() const { return m_storage.end(); }
    bool empty() { return m_storage.empty(); }

    size_type size() const { return m_storage.size(); }

    reference operator [] (size_type i) { return m_storage[i]; }
    const_reference operator [] (size_type i) const { return m_storage[i]; }
protected:
    bool IsModifier(std::string str) { return (str.length() > 1 && str[0] == '-' && std::isalpha(str[1])); }
protected:
    TokenStorage m_storage;
    ModifierStorageType m_modifiers;
};

struct ItemTemplateExtraData
{
    bool hidden;
};

typedef std::unordered_map<uint32, ItemTemplateExtraData> ItemTemplateExtraContainer;

struct PublicTeleData
{
    std::string name;
    float x;
    float y;
    float z;
    float o;
    uint32 map;
    uint32 gmBnetAccId;
};

typedef std::vector<PublicTeleData> PublicTeleContainer;

struct PrivateTeleData
{
    std::string name;
    float x;
    float y;
    float z;
    float o;
    uint32 map;
};

typedef std::vector<PrivateTeleData> PrivateTeleVector;
typedef std::map<uint32, PrivateTeleVector> PrivateTeleContainer;

struct PublicSpellData
{
    std::string name;
    uint8 targetOthers;
    uint32 gmBnetAccId;
};

typedef std::map<uint32, PublicSpellData> PublicSpellContainer;

struct MorphData
{
    std::string name;
    uint32 displayId;
    uint32 gmBnetAccId;
};

typedef std::vector<MorphData> MorphDataContainer;

struct PlayerExtraData
{
    ObjectGuid::LowType selectedGameobjectGuid;
    ObjectGuid::LowType selectedCreatureGuid;
    MorphDataContainer morphDataStore;
};

typedef std::unordered_map<ObjectGuid::LowType, PlayerExtraData> PlayerExtraDataContainer;

class Map;
class GameObject;

class FreedomMgr
{
    private:
        FreedomMgr();
        ~FreedomMgr();
    public:
        static FreedomMgr* instance();
        void LoadAllTables();

        // Gameobject
        void SetGameobjectSelectionForPlayer(ObjectGuid::LowType playerId, ObjectGuid::LowType gameobjectId);
        ObjectGuid::LowType GetSelectedGameobjectGuidFromPlayer(ObjectGuid::LowType playerId);
        GameObject* GetAnyGameObject(Map* objMap, ObjectGuid::LowType lowguid, uint32 entry);

        // Creature
        void SetCreatureSelectionForPlayer(ObjectGuid::LowType playerId, ObjectGuid::LowType creatureId);
        ObjectGuid::LowType GetSelectedCreatureGuidFromPlayer(ObjectGuid::LowType playerId);

        // Public teleports
        void LoadPublicTeleports();
        PublicTeleData const* GetPublicTeleport(std::string const& name);
        PublicTeleData const* GetFirstClosestPublicTeleport(std::string const& name);
        void AddPublicTeleport(PublicTeleData const& data);
        void DeletePublicTeleport(std::string const& name);
        PublicTeleContainer const& GetPublicTeleportContainer() { return _publicTeleStore; }

        // Private teleports
        void LoadPrivateTeleports();
        PrivateTeleData const* GetPrivateTeleport(uint32 bnetAccountId, std::string const& name);
        PrivateTeleData const* GetFirstClosestPrivateTeleport(uint32 bnetAccountId, std::string const& name);
        void AddPrivateTeleport(uint32 bnetAccountId, PrivateTeleData const& data);
        void DeletePrivateTeleport(uint32 bnetAccountId, std::string const& name);
        PrivateTeleVector const& GetPrivateTeleportContainer(uint32 bnetAccountId) { return _privateTeleStore[bnetAccountId]; }

        // Public spells
        void LoadPublicSpells();
        PublicSpellData const* GetPublicSpell(uint32 spellId);
        void AddPublicSpell(uint32 spellId, PublicSpellData const& data);
        void DeletePublicSpell(uint32 spellId);
        PublicSpellContainer const& GetPublicSpellContainer() { return _publicSpellStore; }

        // Morphs
        void LoadMorphs();
        MorphData const* GetMorphByName(ObjectGuid::LowType playerId, std::string const& name);
        MorphData const* GetMorphByDisplayId(ObjectGuid::LowType playerId, uint32 displayId);
        void AddMorph(ObjectGuid::LowType playerId, MorphData const& data);
        void DeleteMorphByName(ObjectGuid::LowType playerId, std::string const& name);
        MorphDataContainer const& GetMorphContainer(ObjectGuid::LowType playerId) { return _playerExtraDataStore[playerId].morphDataStore; }

        // Items
        void LoadItemTemplateExtras();
        ItemTemplateExtraData const* GetItemTemplateExtraById(uint32 itemId);
        void SetItemTemplateExtraHiddenFlag(uint32 itemId, bool hidden);
        ItemTemplateExtraContainer const& GetItemTemplateExtraContainer() { return _itemTemplateExtraStore; }

        // Misc
        std::string GetMapName(uint32 mapId);
        std::string ToChatLink(std::string type, uint64 key, std::string name) { return "|cffffffff|" + type + ":" + fmt::sprintf("%llu", key) + "|h[" + name + "]|h|r"; }
        std::string GetChatLinkKey(std::string const &src, std::string type);

        // Player
        void RemoveHoverFromPlayer(Player* player);
        void RemoveWaterwalkFromPlayer(Player* player);
        void RemoveFlyFromPlayer(Player* player);

    protected:
        PlayerExtraDataContainer _playerExtraDataStore;
        PublicTeleContainer _publicTeleStore;
        PublicSpellContainer _publicSpellStore;
        PrivateTeleContainer _privateTeleStore;
        ItemTemplateExtraContainer _itemTemplateExtraStore;
};

#define sFreedomMgr FreedomMgr::instance()

#endif