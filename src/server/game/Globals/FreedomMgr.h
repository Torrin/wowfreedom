#ifndef _FREEDOMMGR_H
#define _FREEDOMMGR_H

#include "ObjectMgr.h"

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

class FreedomMgr
{
    private:
        FreedomMgr();
        ~FreedomMgr();
    public:
        static FreedomMgr* instance();
        void LoadAllTables();

        // Creature/Gobject selection GUIDs
        void SetCreatureSelectionForPlayer(ObjectGuid::LowType playerId, ObjectGuid::LowType creatureId);
        ObjectGuid::LowType GetSelectedCreatureGuidFromPlayer(ObjectGuid::LowType playerId);
        void SetGameobjectSelectionForPlayer(ObjectGuid::LowType playerId, ObjectGuid::LowType gameobjectId);
        ObjectGuid::LowType GetSelectedGameobjectGuidFromPlayer(ObjectGuid::LowType playerId);

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

        // Misc
        std::string GetMapName(uint32 mapId);
        std::string ToChatLink(std::string type, uint32 key, std::string name) { return "|cffffffff|" + type + ":" + fmt::sprintf("%u", key) + "|h[" + name + "]|h|r"; }

        // Player
        void RemoveHoverFromPlayer(Player* player);
        void RemoveWaterwalkFromPlayer(Player* player);
        void RemoveFlyFromPlayer(Player* player);

    protected:
        PlayerExtraDataContainer _playerExtraDataStore;
        PublicTeleContainer _publicTeleStore;
        PublicSpellContainer _publicSpellStore;
        PrivateTeleContainer _privateTeleStore;
};

#define sFreedomMgr FreedomMgr::instance()

#endif