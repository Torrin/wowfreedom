#ifndef _FREEDOMMGR_H
#define _FREEDOMMGR_H

#include "ObjectMgr.h"
#include "GameObject.h"
#include "Creature.h"

struct GameObjectTemplateExtraData
{
    GameObjectTemplateExtraData() : disabled(false), modelName("(UNKNOWN)"), modelType("MDX"), isDefault(false) { }

    bool disabled;
    std::string modelName;
    std::string modelType;
    bool isDefault;
};

typedef std::unordered_map<uint32, GameObjectTemplateExtraData> GameObjectTemplateExtraContainer;

struct GameObjectExtraData
{
    GameObjectExtraData() : scale(-1.0f), creatorBnetAccId(0), creatorPlayerId(0),
        modifierBnetAccId(0), modifierPlayerId(0), created(0), modified(0), phaseMask(1) { }

    float scale;
    uint32 creatorBnetAccId;
    uint64 creatorPlayerId;
    uint32 modifierBnetAccId;
    uint64 modifierPlayerId;
    time_t created;
    time_t modified;
    uint32 phaseMask;
};

typedef std::unordered_map<uint64, GameObjectExtraData> GameObjectExtraContainer;

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
    uint32 phaseMask;
};

typedef std::unordered_map<ObjectGuid::LowType, PlayerExtraData> PlayerExtraDataContainer;

struct CreatureExtraData
{
    CreatureExtraData() : scale(-1.0f), creatorBnetAccId(0), creatorPlayerId(0),
        modifierBnetAccId(0), modifierPlayerId(0), created(0), modified(0), phaseMask(1),
        displayLock(false), displayId(0), nativeDisplayId(0), genderLock(false), gender(0),
        swim(true), gravity(true), fly(false) { }

    float scale;
    uint32 creatorBnetAccId;
    uint64 creatorPlayerId;
    uint32 modifierBnetAccId;
    uint64 modifierPlayerId;
    time_t created;
    time_t modified;
    uint32 phaseMask;
    bool displayLock;
    uint32 displayId;
    uint32 nativeDisplayId;
    bool genderLock;
    uint8 gender;
    bool swim;
    bool gravity;
    bool fly;
};

typedef std::unordered_map<uint64, CreatureExtraData> CreatureExtraContainer;

struct CreatureTemplateExtraData
{
    bool disabled;
};

typedef std::unordered_map<uint32, CreatureTemplateExtraData> CreatureTemplateExtraContainer;

typedef std::vector<std::pair<uint32, uint32>> PhaseListContainer;

class Map;

class FreedomMgr
{
    private:
        FreedomMgr();
        ~FreedomMgr();
    public:
        static FreedomMgr* instance();
        void LoadAllTables();

        // Phasing
        int GetPhaseMask(uint32 phaseId);
        int GetPhaseId(uint32 phaseMask);
        bool IsValidPhaseId(uint32 phaseId);
        bool IsValidPhaseMask(uint32 phaseMask);
        void CreaturePhase(Creature* creature, uint32 phaseMask);
        void GameObjectPhase(GameObject* go, uint32 phaseMask);
        void PlayerPhase(Player* player, uint32 phaseMask);
        uint32 GetPlayerPhase(Player* player);

        // Gameobject
        void LoadGameObjectExtras();
        void LoadGameObjectTemplateExtras();
        GameObjectTemplateExtraData const* GetGameObjectTemplateExtraData(uint32 entry);
        void SetGameobjectTemplateExtraDisabledFlag(uint32 entryId, bool disabled);
        void SaveGameObject(GameObject* go);
        void SetGameobjectSelectionForPlayer(ObjectGuid::LowType playerId, ObjectGuid::LowType gameobjectId);
        ObjectGuid::LowType GetSelectedGameobjectGuidFromPlayer(ObjectGuid::LowType playerId);
        GameObject* GetAnyGameObject(Map* objMap, ObjectGuid::LowType lowguid, uint32 entry);
        GameObject* GameObjectRefresh(GameObject* go);
        void GameObjectMove(GameObject* go, float x, float y, float z, float o);
        void GameObjectTurn(GameObject* go, float o);
        void GameObjectScale(GameObject* go, float scale);
        void GameObjectDelete(GameObject* go);
        void GameObjectSetModifyHistory(GameObject* go, Player* modifier);
        GameObjectExtraData const* GetGameObjectExtraData(uint64 guid);
        GameObject* GameObjectCreate(Player* creator, GameObjectTemplate const* gobTemplate, uint32 spawnTimeSecs = 0);

        // Creature        
        void LoadCreatureExtras();
        void LoadCreatureTemplateExtras();
        void SetCreatureTemplateExtraDisabledFlag(uint32 entryId, bool disabled);
        void SaveCreature(Creature* creature);
        void CreatureSetModifyHistory(Creature* creature, Player* modifier);
        void CreatureMove(Creature* creature, float x, float y, float z, float o);
        void CreatureTurn(Creature* creature, float o);
        void CreatureScale(Creature* creature, float scale);
        void CreatureDelete(Creature* creature);
        void CreatureSetEmote(Creature* creature, uint32 emoteId);
        void CreatureSetMount(Creature* creature, uint32 mountId);
        void CreatureSetAuraToggle(Creature* creature, uint32 auraId, bool toggle);
        void CreatureSetBytes1(Creature* creature, uint32 bytes1);
        void CreatureSetBytes2(Creature* creature, uint32 bytes2);
        void CreatureSetGravity(Creature* creature, bool toggle);
        void CreatureSetSwim(Creature* creature, bool toggle);
        void CreatureSetFly(Creature* creature, bool toggle);
        bool CreatureCanSwim(Creature const* creature);
        bool CreatureCanWalk(Creature const* creature);
        bool CreatureCanFly(Creature const* creature);
        Creature* CreatureCreate(Player* creator, CreatureTemplate const* creatureTemplate);
        void CreatureRefresh(Creature* creature);
        CreatureExtraData const* GetCreatureExtraData(uint64 guid);
        CreatureTemplateExtraData const* GetCreatureTemplateExtraData(uint32 entry);
        void SetCreatureSelectionForPlayer(ObjectGuid::LowType playerId, ObjectGuid::LowType creatureId);
        ObjectGuid::LowType GetSelectedCreatureGuidFromPlayer(ObjectGuid::LowType playerId);
        Creature* GetAnyCreature(ObjectGuid::LowType lowguid);
        Creature* GetAnyCreature(Map* map, ObjectGuid::LowType lowguid, uint32 entry);

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
        std::string ToDateTimeString(time_t t);
        std::string ToDateString(time_t t);

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
        GameObjectExtraContainer _gameObjectExtraStore;
        GameObjectTemplateExtraContainer _gameObjectTemplateExtraStore;
        PhaseListContainer _phaseListStore;
        CreatureExtraContainer _creatureExtraStore;
        CreatureTemplateExtraContainer _creatureTemplateExtraStore;
};

#define sFreedomMgr FreedomMgr::instance()

#endif