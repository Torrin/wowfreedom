#include "ScriptMgr.h"
#include "GameEventMgr.h"
#include "ObjectMgr.h"
#include "PoolMgr.h"
#include "MapManager.h"
#include "Chat.h"
#include "Language.h"
#include "Player.h"
#include "Opcodes.h"
#include "Config.h"
#include "BattlenetAccountMgr.h"

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
            { "state", rbac::RBAC_PERM_COMMAND_GOBJECT_SET_STATE, false, &HandleGameObjectSetStateCommand,      "" },
        };
        static std::vector<ChatCommand> gobjectCommandTable =
        {
            { "activate",   rbac::RBAC_PERM_COMMAND_GOBJECT_ACTIVATE,   false, &HandleGameObjectActivateCommand,    "" },
            { "delete",     rbac::RBAC_PERM_COMMAND_GOBJECT_DELETE,     false, &HandleGameObjectDeleteCommand,      "" },
            { "info",       rbac::RBAC_PERM_COMMAND_GOBJECT_INFO,       false, &HandleGameObjectInfoCommand,        "" },
            { "move",       rbac::RBAC_PERM_COMMAND_GOBJECT_MOVE,       false, &HandleGameObjectMoveCommand,        "" },
            { "near",       rbac::RBAC_PERM_COMMAND_GOBJECT_NEAR,       false, &HandleGameObjectNearCommand,        "" },
            { "phase",      rbac::RBAC_PERM_COMMAND_GOBJECT,            false, &HandleGameObjectPhaseCommand,       "" },
            { "turn",       rbac::RBAC_PERM_COMMAND_GOBJECT_TURN,       false, &HandleGameObjectTurnCommand,        "" },
            { "select",     rbac::RBAC_PERM_COMMAND_GOBJECT,            false, &HandleGameObjectSelectCommand,      "" },
            { "scale",      rbac::RBAC_PERM_COMMAND_GOBJECT,            false, &HandleGameObjectScaleCommand,       "" },
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

    static bool HandleGameObjectInfoCommand(ChatHandler* handler, char const* args)
    {
        Player* source = handler->GetSession()->GetPlayer();
        ObjectGuid::LowType guidLow = sFreedomMgr->GetSelectedGameobjectGuidFromPlayer(source->GetGUID().GetCounter());        
        bool historyInfo = false;
        bool positionInfo = false;
        bool advancedInfo = false;

        AdvancedArgumentTokenizer tokenizer(*args ? args : "");
        tokenizer.LoadModifier("-full", 0);
        tokenizer.LoadModifier("-pos", 0);
        tokenizer.LoadModifier("-history", 0);
        tokenizer.LoadModifier("-advanced", 0);

        std::string id = tokenizer.TryGetParam(0);
        if (!id.empty())
            guidLow = atoul(id.c_str());

        if (!guidLow)
        {
            handler->PSendSysMessage(FREEDOM_CMDE_GAMEOBJECT_NOT_FOUND);
            return true;
        }

        if (tokenizer.ModifierExists("-full"))
        {
            historyInfo = true;
            positionInfo = true;
            advancedInfo = true;
        }

        if (tokenizer.ModifierExists("-history"))
            historyInfo = true;

        if (tokenizer.ModifierExists("-pos"))
            positionInfo = true;

        if (tokenizer.ModifierExists("-advanced"))
            advancedInfo = true;

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

        // Gather data
        float px, py, pz, po;
        source->GetPosition(px, py, pz, po);
        std::string guidString = object->GetGUID().ToString();
        uint32 entryId = goTemplate->entry;
        uint32 displayId = goTemplate->displayId;
        uint32 scriptId = object->GetScriptId();
        std::string name = goTemplate->name;
        std::string aiName = object->GetAIName();
        uint32 phaseMask = object->GetPhaseMask();
        auto phaseList = object->GetPhases();
        float ox, oy, oz, oo;
        object->GetPosition(ox, oy, oz, oo);
        uint32 mapId = object->GetMapId();
        float distance = std::sqrt(std::pow(ox - px, 2) + std::pow(oy - py, 2) + std::pow(oz - pz, 2));
        float scale = object->GetObjectScale();
        GOState state = object->GetGoState();
        GameObjectExtraData const* extraData = sFreedomMgr->GetGameObjectExtraData(object->GetSpawnId());

        // Display data
        handler->PSendSysMessage(FREEDOM_CMDI_GAMEOBJECT_INFO, sFreedomMgr->ToChatLink("Hgameobject", guidLow, name));
        if (advancedInfo)
            handler->PSendSysMessage(FREEDOM_CMDI_GAMEOBJECT_INFO_LI_GUID_FULL, guidString);
        handler->PSendSysMessage(FREEDOM_CMDI_GAMEOBJECT_INFO_LI_GUID, guidLow);
        handler->PSendSysMessage(FREEDOM_CMDI_GAMEOBJECT_INFO_LI_ENTRY, entryId, sFreedomMgr->ToChatLink("Hgameobject_entry", entryId, "Spawn link"));
        handler->PSendSysMessage(FREEDOM_CMDI_GAMEOBJECT_INFO_LI_DISPLAY_ID, displayId);

        if (advancedInfo)
        {
            handler->PSendSysMessage(FREEDOM_CMDI_GAMEOBJECT_INFO_LI_SCRIPT_ID, scriptId);
            handler->PSendSysMessage(FREEDOM_CMDI_GAMEOBJECT_INFO_LI_NAME, name);
            handler->PSendSysMessage(FREEDOM_CMDI_GAMEOBJECT_INFO_LI_AI_NAME, aiName);
            handler->PSendSysMessage(FREEDOM_CMDI_GAMEOBJECT_INFO_LI_PHASEMASK, phaseMask);

            std::string stateEnumName;

            switch (state)
            {
            case GO_STATE_ACTIVE:
                stateEnumName = "GO_STATE_ACTIVE";
                break;
            case GO_STATE_READY:
                stateEnumName = "GO_STATE_READY";
                break;
            case GO_STATE_ACTIVE_ALTERNATIVE:
                stateEnumName = "GO_STATE_ACTIVE_ALTERNATIVE";
                break;
            case GO_STATE_TRANSPORT_ACTIVE:
                stateEnumName = "GO_STATE_TRANSPORT_ACTIVE";
                break;
            case GO_STATE_TRANSPORT_STOPPED:
                stateEnumName = "GO_STATE_TRANSPORT_STOPPED";
                break;
            default:
                stateEnumName = "UNKNOWN";
                break;
            }

            handler->PSendSysMessage(FREEDOM_CMDI_GAMEOBJECT_INFO_LI_STATE, (uint32)state, stateEnumName);
        }

        if (positionInfo)
        {
            handler->PSendSysMessage(FREEDOM_CMDI_GAMEOBJECT_INFO_LI_POS_X, ox);
            handler->PSendSysMessage(FREEDOM_CMDI_GAMEOBJECT_INFO_LI_POS_Y, oy);
            handler->PSendSysMessage(FREEDOM_CMDI_GAMEOBJECT_INFO_LI_POS_Z, oz);
            handler->PSendSysMessage(FREEDOM_CMDI_GAMEOBJECT_INFO_LI_ORIENTATION, oo);
        }

        handler->PSendSysMessage(FREEDOM_CMDI_GAMEOBJECT_INFO_LI_DISTANCE, distance);
        handler->PSendSysMessage(FREEDOM_CMDI_GAMEOBJECT_INFO_LI_SCALE, scale);

        if (!phaseList.empty())
        {
            handler->PSendSysMessage(FREEDOM_CMDI_GAMEOBJECT_INFO_LI_PHASELIST);

            for (uint32 phaseId : phaseList)
            {
                handler->PSendSysMessage(FREEDOM_CMDI_GAMEOBJECT_INFO_LI_PHASE_LI, phaseId, sFreedomMgr->GetPhaseMask(phaseId));
            }
        }

        if (historyInfo)
        {
            if (!extraData)
            {
                handler->PSendSysMessage(FREEDOM_CMDI_GAMEOBJECT_INFO_LI_NO_HISTORY);
            }
            else
            {
                // Gather history data
                uint64 creatorPlayerId = extraData->creatorPlayerId;
                std::string creatorPlayerName = "(UNKNOWN)";       
                uint32 creatorBnetId = extraData->creatorBnetAccId;
                std::string creatorAccountName = "(UNKNOWN)";
                uint64 modifierPlayerId = extraData->modifierPlayerId;
                std::string modifierPlayerName = "(UNKNOWN)";
                uint32 modifierBnetId = extraData->modifierBnetAccId;
                std::string modifierAccountName = "(UNKNOWN)";
                time_t created = extraData->created;
                std::string createdTimestamp = "(UNKNOWN)";
                time_t modified = extraData->modified;
                std::string modifiedTimestamp = "(UNKNOWN)";

                // Set names
                if (auto creatorPlayerInfo = sWorld->GetCharacterInfo(ObjectGuid::Create<HighGuid::Player>(creatorPlayerId)))
                    creatorPlayerName = creatorPlayerInfo->Name;
                if (auto modifierPlayerInfo = sWorld->GetCharacterInfo(ObjectGuid::Create<HighGuid::Player>(modifierPlayerId)))
                    modifierPlayerName = modifierPlayerInfo->Name;

                Battlenet::AccountMgr::GetName(creatorBnetId, creatorAccountName);
                Battlenet::AccountMgr::GetName(modifierBnetId, modifierAccountName);

                // Set timestamps
                if (created)
                    createdTimestamp = sFreedomMgr->ToDateTimeString(created);

                if (modified)
                    modifiedTimestamp = sFreedomMgr->ToDateTimeString(modified);

                handler->PSendSysMessage(FREEDOM_CMDI_GAMEOBJECT_INFO_LI_CREATOR_PLAYER, creatorPlayerName, creatorPlayerId);
                if (handler->HasPermission(rbac::RBAC_FPERM_ADMINISTRATION))
                    handler->PSendSysMessage(FREEDOM_CMDI_GAMEOBJECT_INFO_LI_CREATOR_ACC, creatorAccountName, creatorBnetId);
                handler->PSendSysMessage(FREEDOM_CMDI_GAMEOBJECT_INFO_LI_MODIFIER_PLAYER, modifierPlayerName, modifierPlayerId);
                if (handler->HasPermission(rbac::RBAC_FPERM_ADMINISTRATION))
                    handler->PSendSysMessage(FREEDOM_CMDI_GAMEOBJECT_INFO_LI_MODIFIER_ACC, modifierAccountName, modifierBnetId);
                handler->PSendSysMessage(FREEDOM_CMDI_GAMEOBJECT_INFO_LI_CREATED, createdTimestamp);
                handler->PSendSysMessage(FREEDOM_CMDI_GAMEOBJECT_INFO_LI_MODIFIED, modifiedTimestamp);
            }
        }
        
        return true;
    }

    static bool HandleGameObjectScaleCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
        {
            handler->PSendSysMessage(FREEDOM_CMDH_GAMEOBJECT_SCALE);
            return true;
        }

        Player* source = handler->GetSession()->GetPlayer();
        ObjectGuid::LowType guidLow = sFreedomMgr->GetSelectedGameobjectGuidFromPlayer(source->GetGUID().GetCounter());

        AdvancedArgumentTokenizer tokenizer(args);
        std::string scaleToken = tokenizer.TryGetParam(0);
        std::string id = tokenizer.TryGetParam(1);
        float scale = atof(scaleToken.c_str());

        if (!id.empty())
        {
            guidLow = atoul(sFreedomMgr->GetChatLinkKey(id, "Hgameobject").c_str());
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

        float maxScale = sConfigMgr->GetFloatDefault("Freedom.Gameobject.MaxScale", 15.0f);
        float minScale = sConfigMgr->GetFloatDefault("Freedom.Gameobject.MinScale", 0.0f);

        if (scale < minScale)
            scale = minScale;
        if (scale > maxScale)
            scale = maxScale;

        sFreedomMgr->GameObjectScale(object, scale);
        sFreedomMgr->GameObjectSetModifyHistory(object, source);
        sFreedomMgr->SaveGameObject(object);
        sFreedomMgr->GameObjectRefresh(object);

        handler->PSendSysMessage(FREEDOM_CMDI_GAMEOBJECT_SCALE, scale);

        return true;
    }

    static bool HandleGameObjectPhaseCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
        {
            handler->PSendSysMessage(FREEDOM_CMDH_GAMEOBJECT_PHASE);
            return true;
        }

        Player* source = handler->GetSession()->GetPlayer();
        ObjectGuid::LowType guidLow = sFreedomMgr->GetSelectedGameobjectGuidFromPlayer(source->GetGUID().GetCounter());
        bool parseAsPhaseMasks = true;

        AdvancedArgumentTokenizer tokenizer(args);
        tokenizer.LoadModifier("-ids", 0);
        tokenizer.LoadModifier("-guid", 1);

        if (tokenizer.ModifierExists("-guid"))
        {
            std::string guidValue = tokenizer.GetModifierValue("-guid", 0);
            std::string guidKey = sFreedomMgr->GetChatLinkKey(guidValue, "Hgameobject");
            guidLow = atoul(guidKey.c_str());
        }

        if (tokenizer.ModifierExists("-ids"))
        {
            parseAsPhaseMasks = false;
        }

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

        std::string phases = "";
        uint32 phaseMask = 0;

        for (auto phaseElem : tokenizer)
        {
            uint32 phase = atoul(phaseElem.c_str());

            if (!phase)
                continue;

            if (parseAsPhaseMasks)
            {
                if (!sFreedomMgr->IsValidPhaseMask(phase))
                    continue;

                phaseMask |= phase;
                phases += " " + phaseElem;
            }
            else
            {
                if (!sFreedomMgr->IsValidPhaseId(phase))
                    continue;

                phaseMask |= sFreedomMgr->GetPhaseMask(phase);
                phases += " " + phaseElem;
            }
        }

        if (phaseMask)
        {
            sFreedomMgr->GameObjectPhase(object, phaseMask);
            sFreedomMgr->GameObjectSetModifyHistory(object, source);
            sFreedomMgr->SaveGameObject(object);
            object = sFreedomMgr->GameObjectRefresh(object);
        }

        if (phases.empty())
            handler->PSendSysMessage(FREEDOM_CMDE_GAMEOBJECT_PHASE_NOT_SET);
        else if (parseAsPhaseMasks)
            handler->PSendSysMessage(FREEDOM_CMDI_GAMEOBJECT_PHASE_BITFIELDS, phases);
        else
            handler->PSendSysMessage(FREEDOM_CMDI_GAMEOBJECT_PHASE_PHASEIDS, phases);

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
            guidLow = atoul(id);

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

        if (auto extraData = sFreedomMgr->GetGameObjectTemplateExtraData(objectId))
        {
            if (extraData->disabled)
            {
                handler->PSendSysMessage(FREEDOM_CMDE_GAMEOBJECT_IS_BLACKLISTED);
                return true;
            }
        }

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

        if (auto extraData = sFreedomMgr->GetGameObjectTemplateExtraData(objectId))
        {
            if (extraData->disabled)
            {
                handler->PSendSysMessage(FREEDOM_CMDE_GAMEOBJECT_IS_BLACKLISTED);
                return true;
            }
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
        sFreedomMgr->GameObjectSetModifyHistory(object, source);
        sFreedomMgr->SaveGameObject(object);
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
        sFreedomMgr->GameObjectSetModifyHistory(object, source);
        sFreedomMgr->SaveGameObject(object);
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