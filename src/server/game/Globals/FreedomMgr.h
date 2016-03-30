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

typedef std::map<uint32, PrivateTeleData> PrivateTeleContainer;

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

        // Private teleports
        void LoadPrivateTeleports();

        // Public spells
        void LoadPublicSpells();

        // Morphs
        void LoadMorphs();

    protected:
        PlayerExtraDataContainer _playerExtraDataStore;
        PublicTeleContainer _publicTeleStore;
        PublicSpellContainer _publicSpellStore;
        PrivateTeleContainer _privateTeleStore;
};

#define sFreedomMgr FreedomMgr::instance()

#endif