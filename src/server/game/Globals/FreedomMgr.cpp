#include "FreedomMgr.h"
#include "Player.h"
#include "ObjectMgr.h"
#include "AccountMgr.h"

FreedomMgr::FreedomMgr()
{
}

FreedomMgr::~FreedomMgr()
{
}

void FreedomMgr::LoadAllTables()
{
    uint32 oldMSTime = getMSTime();

    LoadPublicTeleports();
    LoadPrivateTeleports();
    LoadPublicSpells();
    LoadMorphs();

    TC_LOG_INFO("server.loading", ">> Loaded FreedomMgr tables in %u ms", GetMSTimeDiffToNow(oldMSTime));
}

void FreedomMgr::LoadPublicTeleports()
{
    // clear current storage
    _publicTeleStore.clear();

    // id, position_x, position_y, position_z, orientation, map, name, id_bnet_gm
    PreparedStatement* stmt = FreedomDatabase.GetPreparedStatement(FREEDOM_SEL_PUBLIC_TELE);
    PreparedQueryResult result = FreedomDatabase.Query(stmt);

    if (!result)
        return;

    do
    {
        Field * fields = result->Fetch();
        PublicTeleData data;
        data.x = fields[1].GetFloat();
        data.y = fields[2].GetFloat();
        data.z = fields[3].GetFloat();
        data.o = fields[4].GetFloat();
        data.map = fields[5].GetUInt32();
        data.name = fields[6].GetString();
        data.gmBnetAccId = fields[7].GetUInt32();

        _publicTeleStore.push_back(data);
    } while (result->NextRow());
}

void FreedomMgr::LoadPrivateTeleports()
{
    // clear current storage
    _privateTeleStore.clear();

    // id, position_x, position_y, position_z, orientation, map, name, id_bnet_account
    PreparedStatement* stmt = FreedomDatabase.GetPreparedStatement(FREEDOM_SEL_PRIVATE_TELE);
    PreparedQueryResult result = FreedomDatabase.Query(stmt);

    if (!result)
        return;

    do
    {
        Field * fields = result->Fetch();
        PrivateTeleData data;
        data.x = fields[1].GetFloat();
        data.y = fields[2].GetFloat();
        data.z = fields[3].GetFloat();
        data.o = fields[4].GetFloat();
        data.map = fields[5].GetUInt32();
        data.name = fields[6].GetString();
        uint32 bnetAccId = fields[7].GetUInt32();

        _privateTeleStore[bnetAccId] = data;
    } while (result->NextRow());
}

void FreedomMgr::LoadPublicSpells()
{
    // clear current storage
    _publicSpellStore.clear();

    // spell_id, allow_targeting, name, id_bnet_gm
    PreparedStatement* stmt = FreedomDatabase.GetPreparedStatement(FREEDOM_SEL_PUBLIC_SPELL);
    PreparedQueryResult result = FreedomDatabase.Query(stmt);

    if (!result)
        return;

    do
    {
        Field * fields = result->Fetch();
        PublicSpellData data;
        uint32 spellId = fields[0].GetUInt32();
        data.targetOthers = fields[1].GetUInt8();
        data.name = fields[2].GetUInt32();
        data.gmBnetAccId = fields[3].GetUInt32();

        _publicSpellStore[spellId] = data;
    } while (result->NextRow());
}

void FreedomMgr::LoadMorphs()
{
    // clear current Morph storage for each player
    for (auto it : _playerExtraDataStore)
    {
        it.second.morphDataStore.clear();
    }

    // guid, name, id_display, id_bnet_gm
    PreparedStatement* stmt = FreedomDatabase.GetPreparedStatement(FREEDOM_SEL_MORPHS);
    PreparedQueryResult result = FreedomDatabase.Query(stmt);

    if (!result)
        return;

    do
    {
        Field * fields = result->Fetch();
        MorphData data;
        ObjectGuid::LowType charGuid = fields[0].GetUInt64();
        data.name = fields[1].GetString();
        data.displayId = fields[2].GetUInt32();
        data.gmBnetAccId = fields[3].GetUInt32();        

        _playerExtraDataStore[charGuid].morphDataStore.push_back(data);
    } while (result->NextRow());
}

void FreedomMgr::SetCreatureSelectionForPlayer(ObjectGuid::LowType playerId, ObjectGuid::LowType creatureId)
{
    _playerExtraDataStore[playerId].selectedCreatureGuid = creatureId;
}

ObjectGuid::LowType FreedomMgr::GetSelectedCreatureGuidFromPlayer(ObjectGuid::LowType playerId)
{
    return _playerExtraDataStore[playerId].selectedCreatureGuid;
}

void FreedomMgr::SetGameobjectSelectionForPlayer(ObjectGuid::LowType playerId, ObjectGuid::LowType gameobjectId)
{
    _playerExtraDataStore[playerId].selectedGameobjectGuid = gameobjectId;
}

ObjectGuid::LowType FreedomMgr::GetSelectedGameobjectGuidFromPlayer(ObjectGuid::LowType playerId)
{
    return _playerExtraDataStore[playerId].selectedGameobjectGuid;
}

FreedomMgr* FreedomMgr::instance()
{
    static FreedomMgr instance;
    return &instance;
}