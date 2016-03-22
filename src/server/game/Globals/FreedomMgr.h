#ifndef _FREEDOMMGR_H
#define _FREEDOMMGR_H

#include "ObjectMgr.h"

struct PlayerExtraData
{
    ObjectGuid::LowType selectedGameobjectGuid;
    ObjectGuid::LowType selectedCreatureGuid;
};

typedef std::unordered_map<ObjectGuid::LowType, PlayerExtraData> PlayerExtraDataContainer;

class FreedomMgr
{
    private:
        FreedomMgr();
        ~FreedomMgr();
    public:
        static FreedomMgr* instance();

        // Creature/Gobject selection GUIDs
        void SetCreatureSelectionForPlayer(ObjectGuid::LowType playerId, ObjectGuid::LowType creatureId);
        ObjectGuid::LowType GetSelectedCreatureGuidFromPlayer(ObjectGuid::LowType playerId);
        void SetGameobjectSelectionForPlayer(ObjectGuid::LowType playerId, ObjectGuid::LowType gameobjectId);
        ObjectGuid::LowType GetSelectedGameobjectGuidFromPlayer(ObjectGuid::LowType playerId);

    protected:
        PlayerExtraDataContainer _playerExtraDataStore;
};

#define sFreedomMgr FreedomMgr::instance()

#endif