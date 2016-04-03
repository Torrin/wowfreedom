#include "FreedomMgr.h"
#include "Player.h"
#include "ObjectMgr.h"
#include "AccountMgr.h"
#include "MovementPackets.h"
#include "MoveSpline.h"
#include <boost/algorithm/string/predicate.hpp>

FreedomMgr::FreedomMgr()
{
}

FreedomMgr::~FreedomMgr()
{
}

FreedomMgr* FreedomMgr::instance()
{
    static FreedomMgr instance;
    return &instance;
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

#pragma region PLAYERS
void FreedomMgr::RemoveHoverFromPlayer(Player* player)
{
    Unit* source_unit = player->ToUnit();
    float hoverHeight = source_unit->GetFloatValue(UNIT_FIELD_HOVERHEIGHT);

    source_unit->RemoveUnitMovementFlag(MOVEMENTFLAG_HOVER);

    if (hoverHeight)
    {
        float newZ = source_unit->GetPositionZ() - hoverHeight;
        source_unit->UpdateAllowedPositionZ(source_unit->GetPositionX(), source_unit->GetPositionY(), newZ);
        source_unit->UpdateHeight(newZ);
    }

    WorldPackets::Movement::MoveSetFlag packet(SMSG_MOVE_UNSET_HOVERING);
    packet.MoverGUID = source_unit->GetGUID();
    source_unit->SendMessageToSet(packet.Write(), true);

    // Force player on the ground after removing hover
    source_unit->SetUnitMovementFlags(MOVEMENTFLAG_FALLING);
    WorldPackets::Movement::MoveUpdate moveUpdate;
    moveUpdate.movementInfo = &source_unit->m_movementInfo;
    source_unit->SendMessageToSet(moveUpdate.Write(), true);
}

void FreedomMgr::RemoveWaterwalkFromPlayer(Player* player)
{
    Unit* source_unit = player->ToUnit();
    source_unit->RemoveUnitMovementFlag(MOVEMENTFLAG_WATERWALKING);

    WorldPackets::Movement::MoveSetFlag packet(SMSG_MOVE_SET_LAND_WALK);
    packet.MoverGUID = source_unit->GetGUID();
    source_unit->SendMessageToSet(packet.Write(), true);
}

void FreedomMgr::RemoveFlyFromPlayer(Player* player)
{
    Unit* source_unit = player->ToUnit();
    source_unit->RemoveUnitMovementFlag(MOVEMENTFLAG_CAN_FLY | MOVEMENTFLAG_MASK_MOVING_FLY);

    if (!source_unit->IsLevitating())
        source_unit->SetFall(true);

    WorldPackets::Movement::MoveSetFlag packet(SMSG_MOVE_UNSET_CAN_FLY);
    packet.MoverGUID = source_unit->GetGUID();
    source_unit->SendMessageToSet(packet.Write(), true);
}
#pragma endregion

#pragma region MISC

std::string FreedomMgr::GetMapName(uint32 mapId)
{
    const MapEntry* map = sMapStore.LookupEntry(mapId);

    if (map)
        return map->MapName_lang;
    else
        return "Unknown";
}

#pragma endregion

#pragma region PUBLIC_TELEPORT

PublicTeleData const* FreedomMgr::GetPublicTeleport(std::string const& name)
{
    auto it = std::find_if(_publicTeleStore.begin(), _publicTeleStore.end(), [name](PublicTeleData t)->bool { return boost::iequals(name, t.name); });

    if (it != _publicTeleStore.end())
        return &(*it);
    else
        return nullptr;
}

PublicTeleData const* FreedomMgr::GetFirstClosestPublicTeleport(std::string const& name)
{
    // Priority to exact match
    auto it = std::find_if(_publicTeleStore.begin(), _publicTeleStore.end(), [name](PublicTeleData t)->bool { return boost::iequals(name, t.name); });

    if (it != _publicTeleStore.end())
    {
        return &(*it);
    }
    else // Find first starts-with match, if exact match fails
    {
        it = std::find_if(_publicTeleStore.begin(), _publicTeleStore.end(), [name](PublicTeleData t)->bool { return boost::istarts_with(t.name, name); });

        if (it != _publicTeleStore.end())
            return &(*it);
        else
            return nullptr;
    }
}

void FreedomMgr::AddPublicTeleport(PublicTeleData const& data)
{
    _publicTeleStore.push_back(data);

    // name, position_x, position_y, position_z, orientation, map, id_bnet_gm
    int index = 0;
    PreparedStatement* stmt = FreedomDatabase.GetPreparedStatement(FREEDOM_INS_PUBLIC_TELE);
    stmt->setString(index++, data.name);
    stmt->setFloat(index++, data.x);
    stmt->setFloat(index++, data.y);
    stmt->setFloat(index++, data.z);
    stmt->setFloat(index++, data.o);
    stmt->setUInt32(index++, data.map);
    stmt->setUInt32(index++, data.gmBnetAccId);
    FreedomDatabase.Execute(stmt);
}

void FreedomMgr::DeletePublicTeleport(std::string const& name)
{
    auto it = std::find_if(_publicTeleStore.begin(), _publicTeleStore.end(), [name](PublicTeleData t)->bool { return boost::iequals(name, t.name); });

    if (it != _publicTeleStore.end())
    {             
        // WHERE name = ?
        PreparedStatement* stmt = FreedomDatabase.GetPreparedStatement(FREEDOM_DEL_PUBLIC_TELE_NAME);
        stmt->setString(0, name);
        FreedomDatabase.Execute(stmt);

        _publicTeleStore.erase(it);
    }
}

void FreedomMgr::LoadPublicTeleports()
{
    // clear current storage
    _publicTeleStore.clear();

    // name, position_x, position_y, position_z, orientation, map, id_bnet_gm
    PreparedStatement* stmt = FreedomDatabase.GetPreparedStatement(FREEDOM_SEL_PUBLIC_TELE);
    PreparedQueryResult result = FreedomDatabase.Query(stmt);

    if (!result)
        return;

    do
    {
        Field * fields = result->Fetch();
        PublicTeleData data;
        data.name = fields[0].GetString();
        data.x = fields[1].GetFloat();
        data.y = fields[2].GetFloat();
        data.z = fields[3].GetFloat();
        data.o = fields[4].GetFloat();
        data.map = fields[5].GetUInt32();        
        data.gmBnetAccId = fields[6].GetUInt32();

        _publicTeleStore.push_back(data);
    } while (result->NextRow());
}

#pragma endregion

#pragma region PRIVATE_TELEPORT

PrivateTeleData const* FreedomMgr::GetPrivateTeleport(uint32 bnetAccountId, std::string const& name)
{
    auto it = std::find_if(_privateTeleStore[bnetAccountId].begin(), _privateTeleStore[bnetAccountId].end(), [name](PrivateTeleData t)->bool { return boost::iequals(name, t.name); });

    if (it != _privateTeleStore[bnetAccountId].end())
        return &(*it);
    else
        return nullptr;
}

PrivateTeleData const* FreedomMgr::GetFirstClosestPrivateTeleport(uint32 bnetAccountId, std::string const& name)
{
    // Priority to exact match
    auto it = std::find_if(_privateTeleStore[bnetAccountId].begin(), _privateTeleStore[bnetAccountId].end(), [name](PrivateTeleData t)->bool { return boost::iequals(name, t.name); });

    if (it != _privateTeleStore[bnetAccountId].end())
    {
        return &(*it);
    }
    else // Find first starts-with match, if exact match fails
    {
        it = std::find_if(_privateTeleStore[bnetAccountId].begin(), _privateTeleStore[bnetAccountId].end(), [name](PrivateTeleData t)->bool { return boost::istarts_with(t.name, name); });

        if (it != _privateTeleStore[bnetAccountId].end())
            return &(*it);
        else
            return nullptr;
    }
}

void FreedomMgr::AddPrivateTeleport(uint32 bnetAccountId, PrivateTeleData const& data)
{
    _privateTeleStore[bnetAccountId].push_back(data);

    // position_x, position_y, position_z, orientation, map, name, id_bnet_gm
    int index = 0;
    PreparedStatement* stmt = FreedomDatabase.GetPreparedStatement(FREEDOM_INS_PRIVATE_TELE);
    stmt->setString(index++, data.name);
    stmt->setFloat(index++, data.x);
    stmt->setFloat(index++, data.y);
    stmt->setFloat(index++, data.z);
    stmt->setFloat(index++, data.o);
    stmt->setUInt32(index++, data.map);
    stmt->setUInt32(index++, bnetAccountId);
    FreedomDatabase.Execute(stmt);
}

void FreedomMgr::DeletePrivateTeleport(uint32 bnetAccountId, std::string const& name)
{
    auto it = std::find_if(_privateTeleStore[bnetAccountId].begin(), _privateTeleStore[bnetAccountId].end(), [name](PrivateTeleData t)->bool { return boost::iequals(name, t.name); });

    if (it != _privateTeleStore[bnetAccountId].end())
    {
        // WHERE name = ? AND id_bnet_account = ?
        PreparedStatement* stmt = FreedomDatabase.GetPreparedStatement(FREEDOM_DEL_PRIVATE_TELE_NAME);
        stmt->setString(0, name);
        stmt->setUInt32(1, bnetAccountId);
        FreedomDatabase.Execute(stmt);

        _privateTeleStore[bnetAccountId].erase(it);
    }
}

void FreedomMgr::LoadPrivateTeleports()
{
    // clear current storage
    _privateTeleStore.clear();

    // name, position_x, position_y, position_z, orientation, map, id_bnet_account
    PreparedStatement* stmt = FreedomDatabase.GetPreparedStatement(FREEDOM_SEL_PRIVATE_TELE);
    PreparedQueryResult result = FreedomDatabase.Query(stmt);

    if (!result)
        return;

    do
    {
        Field * fields = result->Fetch();
        PrivateTeleData data;
        data.name = fields[0].GetString();
        data.x = fields[1].GetFloat();
        data.y = fields[2].GetFloat();
        data.z = fields[3].GetFloat();
        data.o = fields[4].GetFloat();
        data.map = fields[5].GetUInt32();
        uint32 bnetAccId = fields[6].GetUInt32();

        _privateTeleStore[bnetAccId].push_back(data);
    } while (result->NextRow());
}

#pragma endregion

#pragma region PUBLIC_SPELL

PublicSpellData const* FreedomMgr::GetPublicSpell(uint32 spellId)
{
    auto it = _publicSpellStore.find(spellId);
    if (it != _publicSpellStore.end())
        return &it->second;
    else
        return nullptr;
}

void FreedomMgr::AddPublicSpell(uint32 spellId, PublicSpellData const& data)
{
    _publicSpellStore[spellId] = data;

    // spell_id, allow_targeting, name, id_bnet_gm
    int index = 0;
    PreparedStatement* stmt = FreedomDatabase.GetPreparedStatement(FREEDOM_INS_PUBLIC_SPELL);
    stmt->setUInt32(index++, spellId);
    stmt->setUInt8(index++, data.targetOthers);
    stmt->setString(index++, data.name);
    stmt->setUInt32(index++, data.gmBnetAccId);
    FreedomDatabase.Execute(stmt);
}

void FreedomMgr::DeletePublicSpell(uint32 spellId)
{
    if (_publicSpellStore.find(spellId) != _publicSpellStore.end())
    {
        // WHERE spell_id = ?
        PreparedStatement* stmt = FreedomDatabase.GetPreparedStatement(FREEDOM_DEL_PUBLIC_SPELL_ID);
        stmt->setUInt32(0, spellId);
        FreedomDatabase.Execute(stmt);

        _publicSpellStore.erase(spellId);
    }
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

#pragma endregion

#pragma region MORPHS

MorphData const* FreedomMgr::GetMorphByName(ObjectGuid::LowType playerId, std::string const& name)
{
    auto it = std::find_if(_playerExtraDataStore[playerId].morphDataStore.begin(), _playerExtraDataStore[playerId].morphDataStore.end(), [name](MorphData m)->bool { return boost::iequals(name, m.name); });

    if (it != _playerExtraDataStore[playerId].morphDataStore.end())
        return &(*it);
    else
        return nullptr;
}

MorphData const* FreedomMgr::GetMorphByDisplayId(ObjectGuid::LowType playerId, uint32 displayId)
{
    auto it = std::find_if(_playerExtraDataStore[playerId].morphDataStore.begin(), _playerExtraDataStore[playerId].morphDataStore.end(), [displayId](MorphData m)->bool { return m.displayId == displayId; });

    if (it != _playerExtraDataStore[playerId].morphDataStore.end())
        return &(*it);
    else
        return nullptr;
}

void FreedomMgr::AddMorph(ObjectGuid::LowType playerId, MorphData const& data)
{
    _playerExtraDataStore[playerId].morphDataStore.push_back(data);

    // guid, name, id_display, id_bnet_gm
    int index = 0;
    PreparedStatement* stmt = FreedomDatabase.GetPreparedStatement(FREEDOM_INS_MORPHS);
    stmt->setUInt64(index++, playerId);
    stmt->setString(index++, data.name);
    stmt->setUInt32(index++, data.displayId);
    stmt->setUInt32(index++, data.gmBnetAccId);
    FreedomDatabase.Execute(stmt);
}

void FreedomMgr::DeleteMorphByName(ObjectGuid::LowType playerId, std::string const& name)
{
    auto it = std::find_if(_playerExtraDataStore[playerId].morphDataStore.begin(), _playerExtraDataStore[playerId].morphDataStore.end(), [name](MorphData m)->bool { return boost::iequals(name, m.name); });

    if (it != _playerExtraDataStore[playerId].morphDataStore.end())
    {
        PreparedStatement* stmt = FreedomDatabase.GetPreparedStatement(FREEDOM_DEL_MORPHS_NAME);
        stmt->setString(0, name);
        stmt->setUInt64(1, playerId);
        FreedomDatabase.Execute(stmt);

        _playerExtraDataStore[playerId].morphDataStore.erase(it);
    }
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

#pragma endregion MORPHS

#pragma region SELECTION

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

#pragma endregion