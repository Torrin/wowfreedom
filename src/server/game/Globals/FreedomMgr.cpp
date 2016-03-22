#include "FreedomMgr.h"
#include "Player.h"
#include "ObjectMgr.h"

FreedomMgr::FreedomMgr()
{
}

FreedomMgr::~FreedomMgr()
{
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