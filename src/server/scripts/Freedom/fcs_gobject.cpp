#include "ScriptMgr.h"
#include "GameEventMgr.h"
#include "ObjectMgr.h"
#include "PoolMgr.h"
#include "MapManager.h"
#include "Chat.h"
#include "Language.h"
#include "Player.h"
#include "Opcodes.h"

class fgobject_commandscript : public CommandScript
{
public:
    fgobject_commandscript() : CommandScript("fgobject_commandscript") { }

    std::vector<ChatCommand> GetCommands() const override
    {
        static std::vector<ChatCommand> gobjectAddCommandTable =
        {
            { "temp", rbac::RBAC_PERM_COMMAND_GOBJECT_ADD_TEMP, false, &HandleGameObjectAddTempCommand,     "" },
            { "",     rbac::RBAC_PERM_COMMAND_GOBJECT_ADD,      false, &HandleGameObjectAddCommand,         "" },
        };
        static std::vector<ChatCommand> gobjectSetCommandTable =
        {
            { "phase", rbac::RBAC_PERM_COMMAND_GOBJECT_SET_PHASE, false, &HandleGameObjectSetPhaseCommand,      "" },
            { "state", rbac::RBAC_PERM_COMMAND_GOBJECT_SET_STATE, false, &HandleGameObjectSetStateCommand,      "" },
        };
        static std::vector<ChatCommand> gobjectCommandTable =
        {
            { "activate",   rbac::RBAC_PERM_COMMAND_GOBJECT_ACTIVATE,   false, &HandleGameObjectActivateCommand,    "" },
            { "delete",     rbac::RBAC_PERM_COMMAND_GOBJECT_DELETE,     false, &HandleGameObjectDeleteCommand,      "" },
            { "info",       rbac::RBAC_PERM_COMMAND_GOBJECT_INFO,       false, &HandleGameObjectInfoCommand,        "" },
            { "move",       rbac::RBAC_PERM_COMMAND_GOBJECT_MOVE,       false, &HandleGameObjectMoveCommand,        "" },
            { "near",       rbac::RBAC_PERM_COMMAND_GOBJECT_NEAR,       false, &HandleGameObjectNearCommand,        "" },
            { "turn",       rbac::RBAC_PERM_COMMAND_GOBJECT_TURN,       false, &HandleGameObjectTurnCommand,        "" },
            { "select",     rbac::RBAC_PERM_COMMAND_GOBJECT,            false, &HandleGameObjectSelectCommand,      "" },
            { "add",        rbac::RBAC_PERM_COMMAND_GOBJECT_ADD,        false, NULL,            "", gobjectAddCommandTable },
            { "spawn",      rbac::RBAC_PERM_COMMAND_GOBJECT_ADD,        false, NULL,            "", gobjectAddCommandTable },
            { "set",        rbac::RBAC_PERM_COMMAND_GOBJECT_SET,        false, NULL,            "", gobjectSetCommandTable },
        };
        static std::vector<ChatCommand> commandTable =
        {
            { "gobject", rbac::RBAC_PERM_COMMAND_GOBJECT, false, NULL, "", gobjectCommandTable },
        };
        return commandTable;
    }

    //set phasemask for selected object
    static bool HandleGameObjectSetPhaseCommand(ChatHandler* /*handler*/, char const* /*args*/)
    {
        /*// number or [name] Shift-click form |color|Hgameobject:go_id|h[name]|h|r
        char* id = handler->extractKeyFromLink((char*)args, "Hgameobject");
        if (!id)
        return false;

        uint32 guidLow = atoi(id);
        if (!guidLow)
        return false;

        GameObject* object = NULL;

        // by DB guid
        if (GameObjectData const* gameObjectData = sObjectMgr->GetGOData(guidLow))
        object = handler->GetObjectGlobalyWithGuidOrNearWithDbGuid(guidLow, gameObjectData->id);

        if (!object)
        {
        handler->PSendSysMessage(LANG_COMMAND_OBJNOTFOUND, guidLow);
        handler->SetSentErrorMessage(true);
        return false;
        }

        char* phase = strtok (NULL, " ");
        uint32 phaseMask = phase ? atoi(phase) : 0;
        if (phaseMask == 0)
        {
        handler->SendSysMessage(LANG_BAD_VALUE);
        handler->SetSentErrorMessage(true);
        return false;
        }

        object->SetPhaseMask(phaseMask, true);
        object->SaveToDB();*/
        return true;
    }

    static bool HandleGameObjectSelectCommand(ChatHandler* handler, char const* args)
    {
        uint32 entryId = 0;
        Player* source = handler->GetSession()->GetPlayer();
        float x, y, z;
        source->GetPosition(x, y, z);
        uint32 mapId = source->GetMapId();
        int index = 0;
        PreparedQueryResult result;

        if (*args)
        {
            char* token = strtok((char*)args, " ");
            entryId = atoul(token);
        }

        if (entryId)
        {
            PreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_SEL_NEAREST_GAMEOBJECT_BY_EID);
            stmt->setFloat(index++, x);
            stmt->setFloat(index++, y);
            stmt->setFloat(index++, z);
            stmt->setUInt16(index++, mapId);
            stmt->setUInt32(index++, entryId);

            result = WorldDatabase.Query(stmt);
        }
        else
        {
            PreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_SEL_NEAREST_GAMEOBJECT);
            stmt->setFloat(index++, x);
            stmt->setFloat(index++, y);
            stmt->setFloat(index++, z);
            stmt->setUInt16(index++, mapId);

            result = WorldDatabase.Query(stmt);
        }

        if (result) 
        {
            Field * fields = result->Fetch();

            uint64 guidLow = fields[0].GetUInt64();
            uint32 entryId = fields[1].GetUInt32();
            double objDist = fields[2].GetDouble();

            GameObjectTemplate const* objTemplate = sObjectMgr->GetGameObjectTemplate(entryId);

            if (!objTemplate)
            {
                handler->PSendSysMessage(FREEDOM_CMDE_GAMEOBJECT_SELECT_ENTRY_ID, entryId, guidLow);
                return true;
            }

            GameObject* obj = sFreedomMgr->GetAnyGameObject(source->GetMap(), guidLow, entryId);

            if (!obj)
            {
                handler->PSendSysMessage(FREEDOM_CMDE_GAMEOBJECT_SELECT_NOT_IN_WORLD, guidLow, entryId);
                return true;
            }

            sFreedomMgr->SetGameobjectSelectionForPlayer(source->GetGUID().GetCounter(), guidLow);
            handler->PSendSysMessage(FREEDOM_CMDI_GAMEOBJECT_SELECT,
                sFreedomMgr->ToChatLink("Hgameobject", guidLow, objTemplate->name),
                guidLow,
                entryId,
                objDist);

            return true;
        }
        else
        {
            handler->PSendSysMessage(FREEDOM_CMDE_GAMEOBJECT_NOT_FOUND);
            return true;
        }        
    }

    static bool HandleGameObjectActivateCommand(ChatHandler* handler, char const* args)
    {
        Player* source = handler->GetSession()->GetPlayer();
        ObjectGuid::LowType guidLow = sFreedomMgr->GetSelectedGameobjectGuidFromPlayer(source->GetGUID().GetCounter());

        // number or [name] Shift-click form |color|Hgameobject:go_guid|h[name]|h|r
        char* id = handler->extractKeyFromLink((char*)args, "Hgameobject");
        if (id)
            guidLow = strtoull(id, nullptr, 10);

        if (!guidLow)
        {
            handler->PSendSysMessage(FREEDOM_CMDE_GAMEOBJECT_NOT_FOUND);
            return true;
        }

        GameObject* object = NULL;
        GameObjectData const* goData = NULL;

        // by DB guid
        if (goData = sObjectMgr->GetGOData(guidLow))
            object = sFreedomMgr->GetAnyGameObject(source->GetMap(), guidLow, goData->id);

        if (!object)
        {
            handler->PSendSysMessage(FREEDOM_CMDE_GAMEOBJECT_GUID_NOT_EXISTS, guidLow);
            return true;
        }

        GameObjectTemplate const* goTemplate = sObjectMgr->GetGameObjectTemplate(goData->id);

        if (!goTemplate)
        {
            handler->PSendSysMessage(FREEDOM_CMDE_TEMPLATE_DOES_NOT_EXIST, guidLow, goData->id);
            return true;
        }

        // Activate
        object->SetLootState(GO_READY);
        object->UseDoorOrButton(10000, false, handler->GetSession()->GetPlayer());

        handler->PSendSysMessage(FREEDOM_CMDI_GAMEOBJECT_ACTIVATED, sFreedomMgr->ToChatLink("Hgameobject", guidLow, goTemplate->name));

        return true;
    }

    //spawn go
    static bool HandleGameObjectAddCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
        {
            handler->PSendSysMessage(FREEDOM_CMDH_GAMEOBJECT_SPAWN);
            return true;
        }

        // number or [name] Shift-click form |color|Hgameobject_entry:go_id|h[name]|h|r
        std::string id = sFreedomMgr->GetChatLinkKey(args, "Hgameobject_entry");
        uint32 objectId = atoul(id.c_str());

        if (!objectId)
        {
            handler->PSendSysMessage(FREEDOM_CMDE_GAMEOBJECT_SPAWN_INVALID_ID);
            return true;
        }

        char* spawnTimeSecsToken = strtok(NULL, " ");
        uint32 spawnTimeSecs = 0;
        if (spawnTimeSecsToken)
            spawnTimeSecs = atoul(spawnTimeSecsToken);

        const GameObjectTemplate* objectInfo = sObjectMgr->GetGameObjectTemplate(objectId);

        if (!objectInfo)
        {
            handler->PSendSysMessage(FREEDOM_CMDE_GAMEOBJECT_ENTRY_NOT_EXISTS, objectId);
            return true;
        }

        if (objectInfo->displayId && !sGameObjectDisplayInfoStore.LookupEntry(objectInfo->displayId))
        {
            handler->PSendSysMessage(FREEDOM_CMDE_GAMEOBJECT_SPAWN_INVALID_DISPLAY, objectId, objectInfo->displayId);
            return true;
        }

        Player* source = handler->GetSession()->GetPlayer();
        float x, y, z, o;
        source->GetPosition(x, y, z, o);
        Map* map = source->GetMap();

        GameObject* object = sFreedomMgr->GameObjectCreate(source, objectInfo, spawnTimeSecs);

        if (!object)
        {
            handler->PSendSysMessage(FREEDOM_CMDE_GAMEOBJECT_SPAWN_FAIL, objectId);
            return true;
        }

        handler->PSendSysMessage(FREEDOM_CMDI_GAMEOBJECT_SPAWN, 
            sFreedomMgr->ToChatLink("Hgameobject", object->GetSpawnId(), objectInfo->name),
            object->GetSpawnId(),
            objectInfo->entry, x, y, z);
        sFreedomMgr->SetGameobjectSelectionForPlayer(source->GetGUID().GetCounter(), object->GetSpawnId());
        return true;
    }

    // add go, temp only
    static bool HandleGameObjectAddTempCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
        {
            handler->PSendSysMessage(FREEDOM_CMDH_GAMEOBJECT_SPAWN);
            return true;
        }

        char* id = strtok((char*)args, " ");
        if (!id)
        {
            handler->PSendSysMessage(FREEDOM_CMDE_GAMEOBJECT_SPAWN_INVALID_ID);
            return true;
        }

        Player* player = handler->GetSession()->GetPlayer();

        char* spawntime = strtok(NULL, " ");
        uint32 spawntm = 300;

        if (spawntime)
            spawntm = atoi((char*)spawntime);

        float x = player->GetPositionX();
        float y = player->GetPositionY();
        float z = player->GetPositionZ();
        float ang = player->GetOrientation();

        float rot2 = std::sin(ang / 2);
        float rot3 = std::cos(ang / 2);

        uint32 objectId = atoi(id);

        if (!sObjectMgr->GetGameObjectTemplate(objectId))
        {
            handler->PSendSysMessage(FREEDOM_CMDE_GAMEOBJECT_ENTRY_NOT_EXISTS, objectId);
            return true;
        }

        player->SummonGameObject(objectId, x, y, z, ang, 0, 0, rot2, rot3, spawntm);

        return true;
    }

    //delete object by selection or guid
    static bool HandleGameObjectDeleteCommand(ChatHandler* handler, char const* args)
    {
        Player* source = handler->GetSession()->GetPlayer();
        ObjectGuid::LowType guidLow = sFreedomMgr->GetSelectedGameobjectGuidFromPlayer(source->GetGUID().GetCounter());

        // number or [name] Shift-click form |color|Hgameobject:go_guid|h[name]|h|r
        char* id = handler->extractKeyFromLink((char*)args, "Hgameobject");
        if (id)
            guidLow = strtoull(id, nullptr, 10);

        if (!guidLow)
        {
            handler->PSendSysMessage(FREEDOM_CMDE_GAMEOBJECT_NOT_FOUND);
            return true;
        }

        GameObject* object = NULL;

        // by DB guid
        if (GameObjectData const* gameObjectData = sObjectMgr->GetGOData(guidLow))
            object = sFreedomMgr->GetAnyGameObject(source->GetMap(), guidLow, gameObjectData->id);

        if (!object)
        {
            handler->PSendSysMessage(FREEDOM_CMDE_GAMEOBJECT_GUID_NOT_EXISTS, guidLow);
            return true;
        }

        ObjectGuid ownerGuid = object->GetOwnerGUID();
        if (!ownerGuid.IsEmpty())
        {
            Unit* owner = ObjectAccessor::GetUnit(*handler->GetSession()->GetPlayer(), ownerGuid);
            if (!owner || !ownerGuid.IsPlayer())
            {
                handler->PSendSysMessage(LANG_COMMAND_DELOBJREFERCREATURE, ownerGuid.ToString().c_str(), object->GetGUID().ToString().c_str());
                return true;
            }

            owner->RemoveGameObject(object, false);
        }

        sFreedomMgr->GameObjectDelete(object);

        handler->PSendSysMessage(FREEDOM_CMDI_GAMEOBJECT_DELETE);

        return true;
    }

    //turn selected object
    static bool HandleGameObjectTurnCommand(ChatHandler* handler, char const* args)
    {
        Player* source = handler->GetSession()->GetPlayer();
        ObjectGuid::LowType guidLow = sFreedomMgr->GetSelectedGameobjectGuidFromPlayer(source->GetGUID().GetCounter());
        float o = source->GetOrientation();
        
        // Prepare tokenizer with command modifiers
        AdvancedArgumentTokenizer tokenizer(*args ? args : "");
        tokenizer.LoadModifier("-guid", 1);
        tokenizer.LoadModifier("-adeg", 1);
        tokenizer.LoadModifier("-sdeg", 1);

        if (tokenizer.ModifierExists("-guid"))
        {
            std::string guidValue = tokenizer.GetModifierValue("-guid", 0);
            std::string guidKey = sFreedomMgr->GetChatLinkKey(guidValue, "Hgameobject");
            guidLow = atoul(guidKey.c_str());
        }

        if (!guidLow)
        {
            handler->PSendSysMessage(FREEDOM_CMDE_GAMEOBJECT_NOT_FOUND);
            return true;
        }

        GameObject* object = NULL;

        // by DB guid
        if (GameObjectData const* gameObjectData = sObjectMgr->GetGOData(guidLow))
            object = sFreedomMgr->GetAnyGameObject(source->GetMap(), guidLow, gameObjectData->id);

        if (!object)
        {
            handler->PSendSysMessage(FREEDOM_CMDE_GAMEOBJECT_GUID_NOT_EXISTS, guidLow);
            return true;
        }

        if (tokenizer.ModifierExists("-adeg"))
        {
            std::string addDeg = tokenizer.GetModifierValue("-adeg", 0);
            o = ((float)atof(addDeg.c_str())) * M_PI / 180.0f + object->GetOrientation();
        }

        if (tokenizer.ModifierExists("-sdeg"))
        {
            std::string setDeg = tokenizer.GetModifierValue("-sdeg", 0);
            o = ((float)atof(setDeg.c_str())) * M_PI / 180.0f;
        }

        sFreedomMgr->GameObjectTurn(object, o);
        object->SaveToDB();
        object = sFreedomMgr->GameObjectRefresh(object);

        handler->PSendSysMessage(FREEDOM_CMDI_GAMEOBJECT_TURNED,
            sFreedomMgr->ToChatLink("Hgameobject", guidLow, object->GetGOInfo()->name),
            guidLow);

        return true;
    }

    //move selected object
    static bool HandleGameObjectMoveCommand(ChatHandler* handler, char const* args)
    {
        Player* source = handler->GetSession()->GetPlayer();
        ObjectGuid::LowType guidLow = sFreedomMgr->GetSelectedGameobjectGuidFromPlayer(source->GetGUID().GetCounter());

        // Prepare tokenizer with command modifiers
        AdvancedArgumentTokenizer tokenizer(*args ? args : "");
        tokenizer.LoadModifier("-guid", 1);
        tokenizer.LoadModifier("-adeg", 1);
        tokenizer.LoadModifier("-sdeg", 1);
        tokenizer.LoadModifier("-po", 0);

        if (tokenizer.ModifierExists("-guid"))
        {
            std::string guidValue = tokenizer.GetModifierValue("-guid", 0);
            std::string guidKey = sFreedomMgr->GetChatLinkKey(guidValue, "Hgameobject");
            guidLow = atoul(guidKey.c_str());
        }

        if (!guidLow)
        {
            handler->PSendSysMessage(FREEDOM_CMDE_GAMEOBJECT_NOT_FOUND);
            return true;
        }

        GameObject* object = NULL;

        // by DB guid
        if (GameObjectData const* gameObjectData = sObjectMgr->GetGOData(guidLow))
            object = sFreedomMgr->GetAnyGameObject(source->GetMap(), guidLow, gameObjectData->id);

        if (!object)
        {
            handler->PSendSysMessage(FREEDOM_CMDE_GAMEOBJECT_GUID_NOT_EXISTS, guidLow);
            return true;
        }

        float o = object->GetOrientation();
        float oldO = o;

        if (tokenizer.ModifierExists("-adeg"))
        {
            std::string addDeg = tokenizer.GetModifierValue("-adeg", 0);
            o = ((float)atof(addDeg.c_str())) * M_PI / 180.0f + object->GetOrientation();
        }

        if (tokenizer.ModifierExists("-sdeg"))
        {
            std::string setDeg = tokenizer.GetModifierValue("-sdeg", 0);
            o = ((float)atof(setDeg.c_str())) * M_PI / 180.0f;
        }

        if (tokenizer.ModifierExists("-po"))
        {
            o = source->GetOrientation();
        }

        float x, y, z;
        if (tokenizer.empty())
        {
            Player* player = handler->GetSession()->GetPlayer();
            player->GetPosition(x, y, z);
        }
        else
        {
            float add_x = (float)atof(tokenizer[0].c_str());;
            float add_y = (float)atof(tokenizer.TryGetParam(1).c_str());
            float add_z = (float)atof(tokenizer.TryGetParam(2).c_str());
            // rotation matrix
            x = add_x*cos(oldO) - add_y*sin(oldO) + object->GetPositionX();
            y = add_x*sin(oldO) + add_y*cos(oldO) + object->GetPositionY();
            z = add_z + object->GetPositionZ();

            if (!MapManager::IsValidMapCoord(object->GetMapId(), x, y, z))
            {
                handler->PSendSysMessage(FREEDOM_CMDE_INVALID_TARGET_COORDS, x, y, z, object->GetMapId());
                return true;
            }
        }

        sFreedomMgr->GameObjectMove(object, x, y, z, o);
        object->SaveToDB();
        object = sFreedomMgr->GameObjectRefresh(object);

        handler->PSendSysMessage(FREEDOM_CMDI_GAMEOBJECT_MOVE, 
            sFreedomMgr->ToChatLink("Hgameobject", guidLow, object->GetGOInfo()->name),
            guidLow);

        return true;
    }

    static bool HandleGameObjectNearCommand(ChatHandler* handler, char const* args)
    {
        float distance = (!*args) ? 10.0f : (float)(atof(args));
        uint32 count = 0;

        Player* player = handler->GetSession()->GetPlayer();
        uint32 maxResults = sWorld->getIntConfig(CONFIG_MAX_RESULTS_LOOKUP_COMMANDS);

        PreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_SEL_NEAREST_GAMEOBJECTS);
        stmt->setFloat(0, player->GetPositionX());
        stmt->setFloat(1, player->GetPositionY());
        stmt->setFloat(2, player->GetPositionZ());
        stmt->setUInt32(3, player->GetMapId());
        stmt->setFloat(4, player->GetPositionX());
        stmt->setFloat(5, player->GetPositionY());
        stmt->setFloat(6, player->GetPositionZ());
        stmt->setFloat(7, distance * distance);
        stmt->setUInt32(8, maxResults);
        PreparedQueryResult result = WorldDatabase.Query(stmt);

        if (result)
        {
            do
            {
                Field* fields = result->Fetch();
                ObjectGuid::LowType guid = fields[0].GetUInt64();
                uint32 entry = fields[1].GetUInt32();
                double objDist = fields[2].GetDouble();

                GameObjectTemplate const* gameObjectInfo = sObjectMgr->GetGameObjectTemplate(entry);

                if (!gameObjectInfo)
                    continue;

                handler->PSendSysMessage(FREEDOM_CMDI_GAMEOBJECT_NEAR_LIST_ITEM, 
                    objDist, 
                    entry,
                    sFreedomMgr->ToChatLink("Hgameobject", guid, gameObjectInfo->name));

                ++count;
            } while (result->NextRow());
        }

        if (count == maxResults)
            handler->PSendSysMessage(FREEDOM_CMDI_GAMEOBJECT_NEAR_MAX_RESULT_COUNT, count, distance);
        else
            handler->PSendSysMessage(FREEDOM_CMDI_GAMEOBJECT_NEAR, count, distance);
        return true;
    }

    //show info of gameobject
    static bool HandleGameObjectInfoCommand(ChatHandler* handler, char const* args)
    {
        uint32 entry = 0;
        uint32 type = 0;
        uint32 displayId = 0;
        std::string name;
        uint32 lootId = 0;
        Player* source = handler->GetSession()->GetPlayer();

        if (!*args)
        {
            uint32 guidLow = sFreedomMgr->GetSelectedGameobjectGuidFromPlayer(source->GetGUID().GetCounter());

            // by DB guid
            if (GameObjectData const* gameObjectData = sObjectMgr->GetGOData(guidLow))
            {
                entry = gameObjectData->id;
            }                
        }
        else
        {
            entry = atoul(args);
        }

        GameObjectTemplate const* gameObjectInfo = sObjectMgr->GetGameObjectTemplate(entry);

        if (!gameObjectInfo)
        {
            handler->PSendSysMessage(FREEDOM_CMDE_GAMEOBJECT_ENTRY_NOT_EXISTS, entry);
            return true;
        }

        type = gameObjectInfo->type;
        displayId = gameObjectInfo->displayId;
        name = gameObjectInfo->name;
        lootId = gameObjectInfo->GetLootId();

        handler->PSendSysMessage(LANG_GOINFO_ENTRY, entry);
        handler->PSendSysMessage(LANG_GOINFO_TYPE, type);
        handler->PSendSysMessage(LANG_GOINFO_LOOTID, lootId);
        handler->PSendSysMessage(LANG_GOINFO_DISPLAYID, displayId);
        handler->PSendSysMessage(LANG_GOINFO_NAME, name.c_str());

        return true;
    }

    static bool HandleGameObjectSetStateCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
        {
            handler->PSendSysMessage(FREEDOM_CMDH_GAMEOBJECT_SET_STATE);
            return true;
        }

        Player* source = handler->GetSession()->GetPlayer();
        ObjectGuid::LowType guidLow = sFreedomMgr->GetSelectedGameobjectGuidFromPlayer(source->GetGUID().GetCounter());

        AdvancedArgumentTokenizer tokenizer(args);
        std::string type = tokenizer.TryGetParam(0);
        std::string state = tokenizer.TryGetParam(1);
        std::string id = tokenizer.TryGetParam(2);

        if (!id.empty())
            guidLow = atoul(sFreedomMgr->GetChatLinkKey(id, "Hgameobject").c_str());

        if (!guidLow)
        {
            handler->PSendSysMessage(FREEDOM_CMDE_GAMEOBJECT_NOT_FOUND);
            return true;
        }

        GameObject* object = NULL;

        // by DB guid
        if (GameObjectData const* gameObjectData = sObjectMgr->GetGOData(guidLow))
            object = sFreedomMgr->GetAnyGameObject(source->GetMap(), guidLow, gameObjectData->id);

        if (!object)
        {
            handler->PSendSysMessage(FREEDOM_CMDE_GAMEOBJECT_GUID_NOT_EXISTS, guidLow);
            return true;
        }       

        int32 objectType = atoi(type.c_str());
        if (objectType < 0)
        {
            if (objectType == -1)
                object->SendObjectDeSpawnAnim(object->GetGUID());
            else if (objectType == -2)
                handler->PSendSysMessage(FREEDOM_CMDH_GAMEOBJECT_SET_STATE);
            return true;
        }
        
        if (state.empty())
        {
            handler->PSendSysMessage(FREEDOM_CMDH_GAMEOBJECT_SET_STATE);
            return true;
        }

        int32 objectState = atoi(state.c_str());

        if (objectType < 4)
            object->SetByteValue(GAMEOBJECT_BYTES_1, objectType, objectState);
        else if (objectType == 4)
            object->SendCustomAnim(objectState);

        handler->PSendSysMessage(FREEDOM_CMDI_GAMEOBJECT_SET_STATE, objectType, objectState);
        return true;
    }
};

void AddSC_fgobject_commandscript()
{
    new fgobject_commandscript();
}

#ifdef FREEDOM_MOP_548_CODE
#define MATH_PI 3.14159265f

class fgobject_commandscript : public CommandScript
{
public:
    fgobject_commandscript() : CommandScript("fgob_commandscript") { }

    ChatCommand* GetCommands() const
    {
        static ChatCommand gobjectCommandTable[] =
        {
            { "select",     rbac::RBAC_PERM_COMMAND_GOBJECT_SELECT,         false, &HandleGameObjectSelectCommand,                  "", NULL },
            { "phase",      rbac::RBAC_PERM_COMMAND_GOBJECT_PHASE,          false, &HandleGameObjectPhaseCommand,                   "", NULL },
            { "scale",      rbac::RBAC_PERM_COMMAND_GOBJECT_SCALE,          false, &HandleGameObjectScaleCommand,                   "", NULL },
            { "activate",   rbac::RBAC_PERM_COMMAND_GOBJECT_ACTIVATE,       false, &HandleGameObjectActivateCommand,                "", NULL },
            { "delete",     rbac::RBAC_PERM_COMMAND_GOBJECT_DELETE,         false, &HandleGameObjectDeleteCommand,                  "", NULL },
            { "info",       rbac::RBAC_PERM_COMMAND_GOBJECT_INFO,           false, &HandleGameObjectInfoCommand,                    "", NULL },
            { "move",       rbac::RBAC_PERM_COMMAND_GOBJECT_MOVE,           false, &HandleGameObjectMoveCommand,                    "", NULL },
            { "near",       rbac::RBAC_PERM_COMMAND_GOBJECT_NEAR,           false, &HandleGameObjectNearCommand,                    "", NULL },
            { "turn",       rbac::RBAC_PERM_COMMAND_GOBJECT_TURN,           false, &HandleGameObjectTurnCommand,                    "", NULL },
            { "add",        rbac::RBAC_PERM_COMMAND_GOBJECT_ADD,            false, &HandleGameObjectAddCommand,                     "", NULL },
            { "spawn",      rbac::RBAC_PERM_COMMAND_GOBJECT_ADD,            false, &HandleGameObjectAddCommand,                     "", NULL },
            { "disable",    rbac::RBAC_PERM_COMMAND_GOBJECT_DISABLE,        false, &HandleGameObjectDisableCommand,                 "", NULL },
            { "enable",     rbac::RBAC_PERM_COMMAND_GOBJECT_ENABLE,         false, &HandleGameObjectEnableCommand,                  "", NULL },
            { NULL, 0, false, NULL, "", NULL }
        };

        static ChatCommand commandTable[] =
        {
            { "gobject",    rbac::RBAC_PERM_COMMAND_GOBJECT,                false, NULL,                                            "", gobjectCommandTable },
            { NULL, 0, false, NULL, "", NULL }
        };

        return commandTable;
    }

    static bool HandleGameObjectDisableCommand(ChatHandler* handler, char const* args)
    {
        if (!*args) {
            handler->PSendSysMessage("Please specify Entry ID of the object to disable.");
            return true;
        }

        // number or [name] Shift-click form |color|Hgameobject_entry:go_id|h[name]|h|r
        char* id = handler->extractKeyFromLink((char*)args, "Hgameobject_entry");
        uint32 entry_id = atol(id);

        if (!entry_id)
        {
            handler->PSendSysMessage("Game object (Entry ID: %u) not found", entry_id);
            return true;
        }

        PreparedStatement * stmt = WorldDatabase.GetPreparedStatement(WORLD_UPD_GAMEOBJECT_DISABLED);
        stmt->setUInt8(0, 1);
        stmt->setUInt32(1, entry_id);
        WorldDatabase.Execute(stmt);

        handler->PSendSysMessage(">> Game object (Entry ID: %u) has been disabled from spawning and listing.", entry_id);

        return true;
    }

    static bool HandleGameObjectEnableCommand(ChatHandler* handler, char const* args)
    {
        if (!*args) {
            handler->PSendSysMessage("Please specify Entry ID of the object to enable.");
            return true;
        }

        // number or [name] Shift-click form |color|Hgameobject_entry:go_id|h[name]|h|r
        char* id = handler->extractKeyFromLink((char*)args, "Hgameobject_entry");
        uint32 entry_id = atol(id);

        if (!entry_id)
        {
            handler->PSendSysMessage("Game object (Entry ID: %u) not found", entry_id);
            return true;
        }

        PreparedStatement * stmt = WorldDatabase.GetPreparedStatement(WORLD_UPD_GAMEOBJECT_DISABLED);
        stmt->setUInt8(0, 0);
        stmt->setUInt32(1, entry_id);
        WorldDatabase.Execute(stmt);

        handler->PSendSysMessage(">> Game object (Entry ID: %u) has been enabled for spawning and listing.", entry_id);

        return true;
    }

    static bool HandleGameObjectSelectCommand(ChatHandler* handler, char const* args)
    {
        Player* source = handler->GetSession()->GetPlayer();
        char * entry_id_string = handler->extractKeyFromLink((char*)args, "Hgameobject_entry");
        uint32 entry_id = entry_id_string ? atol(entry_id_string) : 0;
        PreparedQueryResult result;

        // get first nearest game object relative to player's position with or without given entry ID for narrowing results
        if (entry_id)
        {
            PreparedStatement * stmt = WorldDatabase.GetPreparedStatement(WORLD_SEL_GAMEOBJECT_NEAREST_SINGLE_EID);
            stmt->setFloat(0, source->GetPositionX());
            stmt->setFloat(1, source->GetPositionY());
            stmt->setFloat(2, source->GetPositionZ());
            stmt->setUInt16(3, uint16(source->GetMapId()));
            stmt->setUInt32(4, entry_id);
            result = WorldDatabase.Query(stmt);
        }
        else
        {
            PreparedStatement * stmt = WorldDatabase.GetPreparedStatement(WORLD_SEL_GAMEOBJECT_NEAREST_SINGLE);
            stmt->setFloat(0, source->GetPositionX());
            stmt->setFloat(1, source->GetPositionY());
            stmt->setFloat(2, source->GetPositionZ());
            stmt->setUInt16(3, uint16(source->GetMapId()));
            result = WorldDatabase.Query(stmt);
        }

        if (!result) {
            handler->PSendSysMessage("No game objects found!");
            return true;
        }

        // 0:guid, 1:id, 2:position_x, 3:position_y, 4:position_z, 5:map, 6:distance
        Field * fields = result->Fetch();
        uint32 guid_low = fields[0].GetUInt32();
        uint32 template_id = fields[1].GetUInt32();
        float pos_x = fields[2].GetFloat();
        float pos_y = fields[3].GetFloat();
        float pos_z = fields[4].GetFloat();
        // cant use distance value from query, as for some reason the float value from it is drastically incorrect (extremely huge value), therefore, recalculate the distance in server-code
        float distance = sqrt(pow(pos_x - source->GetPositionX(), 2) + pow(pos_y - source->GetPositionY(), 2) + pow(pos_z - source->GetPositionZ(), 2));

        GameObjectTemplate const* gameobject_info = sObjectMgr->GetGameObjectTemplate(template_id);

        if (!gameobject_info)
        {
            handler->PSendSysMessage("Selected game object (EID: %u) (GUID: %u) does not exist in gameobject_template table! Contact devs about this.", template_id, guid_low);
            return true;
        }

        // set game object to player's selection
        source->SetSelectedGameObject(guid_low);

        GameObject* object = NULL;

        // by DB guid
        if (GameObjectData const* go_data = sObjectMgr->GetGOData(guid_low))
            object = handler->GetObjectGlobalyWithGuidOrNearWithDbGuid(guid_low, go_data->id);

        if (!object) {
            handler->PSendSysMessage("Game object (ID: %u) exists in database but not in-game. Please, inform developers about this bug. Selection on this object still succeeds, however.", guid_low);
            return true;
        }

        // fill game object's history
        uint32 creator_id = object->GetCreator();
        uint32 editor_id = object->GetEditor();
        uint32 account_id = handler->GetSession()->GetAccountId();
        std::string created = TimeToTimestampStr(object->GetCreatedTimestamp());
        std::string modified = TimeToTimestampStr(object->GetModifiedTimestamp());

        // get account usernames if source has enough permission
        std::string creator_username = MSG_COLOR_RED "<HIDDEN>" "|r"; 
        std::string editor_username = MSG_COLOR_RED "<HIDDEN>" "|r";

        if (handler->HasPermission(rbac::RBAC_PERM_SPECIAL_SHOW_PRIVATE_INFO))
        {
            // get creator username
            if (!sAccountMgr->GetName(creator_id, creator_username))
            {
                creator_username = "<NONE>";
            }

            // get editor username
            if (!sAccountMgr->GetName(editor_id, editor_username))
            {
                editor_username = "<NONE>";
            }
        }

        if (creator_id == account_id)
        {
            creator_username += " [YOU]";
        }

        if (editor_id == account_id)
        {
            editor_username += " [YOU]";
        }

        // display information to the source
        // [name] Shift-click form: |color|Hgameobject_entry:go_id|h[name]|h|r
        handler->PSendSysMessage("SELECTED GAME OBJECT: |cFFFFFFFF|Hgameobject_entry:%u|h[%s]|h|r", template_id, gameobject_info->name.c_str());
        handler->PSendSysMessage("> GUID: %u", guid_low);
        handler->PSendSysMessage("> Entry: %u", template_id);
        handler->PSendSysMessage("> Scale: %.3f", object->GetFloatValue(OBJECT_FIELD_SCALE));
        handler->PSendSysMessage("> Created by: %s (ID: %u, TIMESTAMP: %s)", creator_username.c_str(), creator_id, created.c_str());
        handler->PSendSysMessage("> Last modified by: %s (ID: %u, TIMESTAMP: %s)", editor_username.c_str(), editor_id, modified.c_str());
        handler->PSendSysMessage("> Position: X: %f Y: %f Z: %f", pos_x, pos_y, pos_z);
        handler->PSendSysMessage("> Distance: %.2f yards", distance);

        return true;
    }

    static bool HandleGameObjectScaleCommand(ChatHandler* handler, char const* args)
    {
        if (!*args) {
            handler->PSendSysMessage("Please specify new scale parameter.");
            return true;
        }

        char * params[2];
        params[0] = strtok((char *)args, " ");
        params[1] = strtok(NULL, " ");

        uint32 guid_low;
        float scale;
        Player* source = handler->GetSession()->GetPlayer();

        if (!params[1])
        {
            guid_low = source->GetSelectedGameObject();
            scale = atof(params[0]);
        }
        else
        {
            guid_low = atoi(handler->extractKeyFromLink(params[0], "Hgameobject"));
            scale = atof(params[1]);
        }

        if (!guid_low)
        {
            handler->PSendSysMessage("Game object (GUID: %u) not found", guid_low);
            return true;
        }

        if (scale > 30.0f || scale < 0.0f)
        {
            handler->PSendSysMessage("Scale must be between 0.0f and 30.0f.");
            return true;
        }

        GameObject* object = NULL;

        // by DB guid
        if (GameObjectData const* go_data = sObjectMgr->GetGOData(guid_low))
            object = handler->GetObjectGlobalyWithGuidOrNearWithDbGuid(guid_low, go_data->id);

        if (!object) {
            handler->PSendSysMessage(LANG_COMMAND_OBJNOTFOUND, guid_low);
            handler->SetSentErrorMessage(true);
            return false;
        }

        // set scale
        float old_scale = object->GetObjectScale();
        object->SetObjectScale(scale);

        // Update editor
        object->SetEditor(handler->GetSession()->GetAccountId());

        // Update modified datetime
        object->SetModifiedTimestamp(time(NULL));

        //TODO: (Azeroc) Fix object resetting to old state after moving prematurely after the command
        object->SaveToDB();
        object->RemoveFromWorld();
        object->AddToWorld();

        handler->PSendSysMessage(">> Object successfully scaled from %.3f to %.3f!", old_scale, scale);

        return true;
    }

    static bool HandleGameObjectActivateCommand(ChatHandler* handler, char const* args)
    {
        uint32 guid_low;

        if (!*args)
        {
            guid_low = handler->GetSession()->GetPlayer()->GetSelectedGameObject();
        } 
        else
        {
            char* id = handler->extractKeyFromLink((char*)args, "Hgameobject");
            guid_low = atol(id);
        }

        if (!guid_low)
        {
            handler->PSendSysMessage("Game object (GUID: %u) not found", guid_low);
            return true;
        }

        GameObject* object = NULL;

        // by DB guid
        if (GameObjectData const* go_data = sObjectMgr->GetGOData(guid_low))
            object = handler->GetObjectGlobalyWithGuidOrNearWithDbGuid(guid_low, go_data->id);

        if (!object)
        {
            handler->PSendSysMessage(LANG_COMMAND_OBJNOTFOUND, guid_low);
            handler->SetSentErrorMessage(true);
            return false;
        }

        // Activate
        object->SetLootState(GO_READY);
        object->UseDoorOrButton(10000, false, handler->GetSession()->GetPlayer());

        handler->PSendSysMessage(">> Object activated!");

        return true;
    }

    static bool HandleGameObjectAddCommand(ChatHandler* handler, char const* args)
    {
        if (!*args) {
            handler->PSendSysMessage("Please specify Entry ID of the object to add.");
            return true;
        }

        // number or [name] Shift-click form |color|Hgameobject_entry:go_id|h[name]|h|r
        char* id = handler->extractKeyFromLink((char*)args, "Hgameobject_entry");
        uint32 entry_id = atol(id);

        if (!entry_id)
        {
            handler->PSendSysMessage("Game object (Entry ID: %u) not found", entry_id);
            return true;
        }

        // check if the game object entry ID is disabled
        PreparedStatement * disable_check_stmt = WorldDatabase.GetPreparedStatement(WORLD_SEL_GAMEOBJECT_IS_DISABLED);
        disable_check_stmt->setUInt32(0, entry_id);
        PreparedQueryResult disable_check_result = WorldDatabase.Query(disable_check_stmt);

        if (disable_check_result)
        {
            handler->PSendSysMessage("Game object (Entry ID: %u) is disabled. Contact the staff about its re-enabling.", entry_id);
            return true;
        }

        const GameObjectTemplate* object_info = sObjectMgr->GetGameObjectTemplate(entry_id);

        if (!object_info)
        {
            handler->PSendSysMessage(LANG_GAMEOBJECT_NOT_EXIST, entry_id);
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (object_info->displayId && !sGameObjectDisplayInfoStore.LookupEntry(object_info->displayId))
        {
            // report to DB errors log as in loading case
            TC_LOG_ERROR("sql.sql", "Gameobject (Entry %u GoType: %u) have invalid displayId (%u), not spawned.", entry_id, object_info->type, object_info->displayId);
            handler->PSendSysMessage(LANG_GAMEOBJECT_HAVE_INVALID_DATA, entry_id);
            handler->SetSentErrorMessage(true);
            return false;
        }

        Player* source = handler->GetSession()->GetPlayer();
        float x = float(source->GetPositionX());
        float y = float(source->GetPositionY());
        float z = float(source->GetPositionZ());
        float o = float(source->GetOrientation());
        Map* map = source->GetMap();

        GameObject* object = new GameObject;
        uint32 guid_low = sObjectMgr->GenerateLowGuid(HIGHGUID_GAMEOBJECT);

        if (!object->Create(guid_low, object_info->entry, map, source->GetPhaseMgr().GetPhaseMaskForSpawn(), x, y, z, o, 0.0f, 0.0f, 0.0f, 0.0f, 0, GO_STATE_READY))
        {
            delete object;
            return false;
        }

        uint32 account_id = handler->GetSession()->GetAccountId();

        // Update creator
        object->SetCreator(account_id);

        // Update editor
        object->SetEditor(account_id);

        // fill the gameobject data and save to the db
        object->SaveToDB(map->GetId(), (1 << map->GetSpawnMode()), source->GetPhaseMgr().GetPhaseMaskForSpawn());

        // this will generate a new guid if the object is in an instance
        if (!object->LoadGameObjectFromDB(guid_low, map))
        {
            delete object;
            return false;
        }

        /// @todo is it really necessary to add both the real and DB table guid here ?
        sObjectMgr->AddGameobjectToGrid(guid_low, sObjectMgr->GetGOData(guid_low));

        handler->PSendSysMessage(LANG_GAMEOBJECT_ADD, entry_id, object_info->name.c_str(), guid_low, x, y, z);

        // add created game object to player's selection
        source->SetSelectedGameObject(guid_low);

        return true;
    }

    static bool HandleGameObjectDeleteCommand(ChatHandler* handler, char const* args)
    {
        uint32 guid_low;

        // number or [name] Shift-click form |color|Hgameobject:go_guid|h[name]|h|r
        if (!*args) 
        {
            guid_low = handler->GetSession()->GetPlayer()->GetSelectedGameObject();
        }
        else
        {
            char* id = handler->extractKeyFromLink((char*)args, "Hgameobject");
            guid_low = atol(id);
        }

        if (!guid_low)
        {
            handler->PSendSysMessage("Game object (GUID: %u) not found", guid_low);
            return true;
        }

        GameObject* object = NULL;

        // by DB guid
        if (GameObjectData const* go_data = sObjectMgr->GetGOData(guid_low)) 
        {
            object = handler->GetObjectGlobalyWithGuidOrNearWithDbGuid(guid_low, go_data->id);
            if (!object) // get matching HIGHGUID_GAMEOBJECT type of GO in entire world
                object = sObjectAccessor->GetObjectInWorld(MAKE_NEW_GUID(guid_low, go_data->id, HIGHGUID_GAMEOBJECT), (GameObject*)NULL);
            if (!object) // get matching HIGHGUID_DYNAMICOBJECT type of GO in entire world
                object = sObjectAccessor->GetObjectInWorld(MAKE_NEW_GUID(guid_low, go_data->id, HIGHGUID_DYNAMICOBJECT), (GameObject*)NULL);
        }
            

        if (!object)
        {
            handler->PSendSysMessage(LANG_COMMAND_OBJNOTFOUND, guid_low);
            handler->SetSentErrorMessage(true);
            return false;
        }

        object->SetRespawnTime(0);                                 // not save respawn time
        object->Delete();
        object->DeleteFromDB();

        handler->PSendSysMessage(LANG_COMMAND_DELOBJMESSAGE, object->GetGUIDLow());

        return true;
    }

    //turn selected object
    static bool HandleGameObjectTurnCommand(ChatHandler* handler, char const* args)
    {
        uint32 guid_low = handler->GetSession()->GetPlayer()->GetSelectedGameObject();
        float new_o;
        float old_o;
        Player* source = handler->GetSession()->GetPlayer();

        if (!guid_low)
        {
            handler->PSendSysMessage("Game object (GUID: %u) not found", guid_low);
            return true;
        }

        GameObject* object = NULL;

        // by DB guid
        if (GameObjectData const* go_data = sObjectMgr->GetGOData(guid_low))
            object = handler->GetObjectGlobalyWithGuidOrNearWithDbGuid(guid_low, go_data->id);

        if (!object)
        {
            handler->PSendSysMessage(LANG_COMMAND_OBJNOTFOUND, guid_low);
            handler->SetSentErrorMessage(true);
            return false;
        }

        old_o = object->GetOrientation();

        if (!*args)        
            new_o = source->GetOrientation();
        else
            new_o = old_o + ((float)atof((char*)args) * (MATH_PI / 180.0f));

        object->Relocate(object->GetPositionX(), object->GetPositionY(), object->GetPositionZ(), new_o);
        object->UpdateRotationFields();

        // Update editor
        object->SetEditor(handler->GetSession()->GetAccountId());

        // Update modified datetime
        object->SetModifiedTimestamp(time(NULL));

        //TODO: (Azeroc) Fix object resetting to old state after moving prematurely after the command
        object->SaveToDB();
        object->RemoveFromWorld();
        object->AddToWorld();

        handler->PSendSysMessage("Game Object |cffffffff|Hgameobject:%d|h[%s]|h|r (GUID: %d) turned.", object->GetGUIDLow(), object->GetGOInfo()->name.c_str(), object->GetGUIDLow());

        return true;
    }

    //move selected object
    static bool HandleGameObjectMoveCommand(ChatHandler* handler, char const* args)
    {
        Player* source = handler->GetSession()->GetPlayer();
        uint32 guid_low = source->GetSelectedGameObject();
       
        if (!guid_low)
        {
            handler->PSendSysMessage("Game object (GUID: %u) not found", guid_low);
            return true;
        }

        GameObject* object = NULL;

        // by DB guid
        if (GameObjectData const* go_data = sObjectMgr->GetGOData(guid_low))
            object = handler->GetObjectGlobalyWithGuidOrNearWithDbGuid(guid_low, go_data->id);

        if (!object)
        {
            handler->PSendSysMessage(LANG_COMMAND_OBJNOTFOUND, guid_low);
            handler->SetSentErrorMessage(true);
            return false;
        }

        char* toX = strtok((char*)args, " ");
        char* toY = strtok(NULL, " ");
        char* toZ = strtok(NULL, " ");

        if (!toX)
        {
            object->Relocate(source->GetPositionX(), source->GetPositionY(), source->GetPositionZ(), source->GetOrientation());
        }
        else
        {
            if (!toY || !toZ)
            {
                handler->PSendSysMessage("Not enough arguments! You must specify full X, Y, Z coordinate set (e.g. .gob move 10 20 30). Do not specify any to move the object to your character's position.", guid_low);
                return true;
            }

            float x = (float)atof(toX);
            float y = (float)atof(toY);
            float z = (float)atof(toZ);

            if (!MapManager::IsValidMapCoord(object->GetMapId(), x, y, z))
            {
                handler->PSendSysMessage(LANG_INVALID_TARGET_COORD, x, y, object->GetMapId());
                handler->SetSentErrorMessage(true);
                return false;
            }

            object->Relocate(x, y, z, source->GetOrientation());
        }

        // Update editor
        object->SetEditor(handler->GetSession()->GetAccountId());

        // Update modified datetime
        object->SetModifiedTimestamp(time(NULL));

        //TODO: (Azeroc) Fix object resetting to old state after moving prematurely after the command
        object->SaveToDB();
        object->RemoveFromWorld();
        object->AddToWorld();

        handler->PSendSysMessage(LANG_COMMAND_MOVEOBJMESSAGE, object->GetGUIDLow(), object->GetGOInfo()->name.c_str(), object->GetGUIDLow());

        return true;
    }

    //set phasemask for selected object
    static bool HandleGameObjectPhaseCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
        {
            handler->PSendSysMessage("Not enough arguments!");
            return true;
        }

        Player* source = handler->GetSession()->GetPlayer();
        uint32 guid_low;
        uint32 phase_mask;
        char * params[2];
        params[0] = strtok((char*)args, " ");
        params[1] = strtok(NULL, " ");

        // number or [name] Shift-click form |color|Hgameobject:go_guid|h[name]|h|r

        if (!params[1])
        {
            guid_low = source->GetSelectedGameObject();
            phase_mask = atol(params[0]);
        }
        else
        {
            char* id = handler->extractKeyFromLink(params[0], "Hgameobject");
            guid_low = atol(id);
            phase_mask = atol(params[1]);
        }

        if (!guid_low)
        {
            handler->PSendSysMessage("Game object (GUID: %u) not found", guid_low);
            return true;
        }

        if (phase_mask == 0)
        {
            handler->SendSysMessage(LANG_BAD_VALUE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        GameObject* object = NULL;

        // by DB guid
        if (GameObjectData const* go_data = sObjectMgr->GetGOData(guid_low))
            object = handler->GetObjectGlobalyWithGuidOrNearWithDbGuid(guid_low, go_data->id);

        if (!object)
        {
            handler->PSendSysMessage(LANG_COMMAND_OBJNOTFOUND, guid_low);
            handler->SetSentErrorMessage(true);
            return false;
        }

        object->SetPhaseMask(phase_mask, true);

        // Update editor
        object->SetEditor(handler->GetSession()->GetAccountId());

        // Update modified datetime
        object->SetModifiedTimestamp(time(NULL));

        object->SaveToDB();

        // TODO: Display phase mask in binary for better phase-masks's visualization
        handler->PSendSysMessage(">> Game object (GUID: %u) successfully phased (PhaseMask: %u).", guid_low, phase_mask);

        return true;
    }

    static bool HandleGameObjectNearCommand(ChatHandler* handler, char const* args)
    {
        float distance = (!*args) ? 10.0f : (float)(atof(args));
        uint32 count = 0;

        Player* player = handler->GetSession()->GetPlayer();

        PreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_SEL_GAMEOBJECT_NEAREST);
        stmt->setFloat(0, player->GetPositionX());
        stmt->setFloat(1, player->GetPositionY());
        stmt->setFloat(2, player->GetPositionZ());
        stmt->setUInt32(3, player->GetMapId());
        stmt->setFloat(4, player->GetPositionX());
        stmt->setFloat(5, player->GetPositionY());
        stmt->setFloat(6, player->GetPositionZ());
        stmt->setFloat(7, distance * distance);
        PreparedQueryResult result = WorldDatabase.Query(stmt);

        if (result)
        {
            do
            {
                Field* fields = result->Fetch();
                uint32 guid = fields[0].GetUInt32();
                uint32 entry = fields[1].GetUInt32();
                float x = fields[2].GetFloat();
                float y = fields[3].GetFloat();
                float z = fields[4].GetFloat();
                uint16 mapId = fields[5].GetUInt16();

                GameObjectTemplate const* gameObjectInfo = sObjectMgr->GetGameObjectTemplate(entry);

                if (!gameObjectInfo)
                    continue;

                handler->PSendSysMessage(LANG_GO_LIST_CHAT, guid, entry, guid, gameObjectInfo->name.c_str(), x, y, z, mapId);

                ++count;
            } while (result->NextRow());
        }

        handler->PSendSysMessage(LANG_COMMAND_NEAROBJMESSAGE, distance, count);
        return true;
    }

    static bool HandleGameObjectInfoCommand(ChatHandler* handler, char const* args)
    {
        uint32 entry;
        uint32 type;
        uint32 displayId;
        std::string name;
        uint32 lootId = 0;

        if (!*args)
        {
            if (WorldObject* object = handler->getSelectedObject())
                entry = object->GetEntry();
            else
                entry = atoi((char*)args);
        }
        else
            entry = atoi((char*)args);

        GameObjectTemplate const* gameObjectInfo = sObjectMgr->GetGameObjectTemplate(entry);

        if (!gameObjectInfo)
            return false;

        type = gameObjectInfo->type;
        displayId = gameObjectInfo->displayId;
        name = gameObjectInfo->name;
        if (type == GAMEOBJECT_TYPE_CHEST)
            lootId = gameObjectInfo->chest.lootId;
        else if (type == GAMEOBJECT_TYPE_FISHINGHOLE)
            lootId = gameObjectInfo->fishinghole.lootId;

        handler->PSendSysMessage(LANG_GOINFO_ENTRY, entry);
        handler->PSendSysMessage(LANG_GOINFO_TYPE, type);
        handler->PSendSysMessage(LANG_GOINFO_LOOTID, lootId);
        handler->PSendSysMessage(LANG_GOINFO_DISPLAYID, displayId);
        handler->PSendSysMessage(LANG_GOINFO_NAME, name.c_str());

        return true;
    }
};

void AddSC_fgobject_commandscript()
{
    new fgobject_commandscript();
}

#endif