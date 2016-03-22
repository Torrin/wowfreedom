#include "ScriptMgr.h"
#include "ObjectMgr.h"
#include "MapManager.h"
#include "SupportMgr.h"
#include "Chat.h"
#include "Language.h"
#include "Player.h"
#include "Transport.h"

class fgoto_commandscript : public CommandScript
{
public:
    fgoto_commandscript() : CommandScript("fgoto_commandscript") { }

    std::vector<ChatCommand> GetCommands() const override
    {
        static std::vector<ChatCommand> goCommandTable =
        {
            { "creature",           rbac::RBAC_PERM_COMMAND_GO_CREATURE,            false, &HandleGoCreatureCommand,                    "" },
            { "graveyard",          rbac::RBAC_PERM_COMMAND_GO_GRAVEYARD,           false, &HandleGoGraveyardCommand,                   "" },
            { "grid",               rbac::RBAC_PERM_COMMAND_GO_GRID,                false, &HandleGoGridCommand,                        "" },
            { "object",             rbac::RBAC_PERM_COMMAND_GO_OBJECT,              false, &HandleGoObjectCommand,                      "" },
            { "quest",              rbac::RBAC_PERM_COMMAND_GO_QUEST,               false, &HandleGoQuestCommand,                       "" },
            { "taxinode",           rbac::RBAC_PERM_COMMAND_GO_TAXINODE,            false, &HandleGoTaxinodeCommand,                    "" },
            { "trigger",            rbac::RBAC_PERM_COMMAND_GO_TRIGGER,             false, &HandleGoTriggerCommand,                     "" },
            { "zonexy",             rbac::RBAC_PERM_COMMAND_GO_ZONEXY,              false, &HandleGoZoneXYCommand,                      "" },
            { "xyz",                rbac::RBAC_PERM_COMMAND_GO_XYZ,                 false, &HandleGoXYZCommand,                         "" },
            { "bugticket",          rbac::RBAC_PERM_COMMAND_GO_BUG_TICKET,          false, &HandleGoTicketCommand<BugTicket>,           "" },
            { "complaintticket",    rbac::RBAC_PERM_COMMAND_GO_COMPLAINT_TICKET,    false, &HandleGoTicketCommand<ComplaintTicket>,     "" },
            { "suggestionticket",   rbac::RBAC_PERM_COMMAND_GO_SUGGESTION_TICKET,   false, &HandleGoTicketCommand<SuggestionTicket>,    "" },
            { "",                   rbac::RBAC_PERM_COMMAND_GO,                     false, &HandleGoXYZCommand,                         "" },
        };

        static std::vector<ChatCommand> commandTable =
        {
            { "goto", rbac::RBAC_PERM_COMMAND_GO, false, NULL, "", goCommandTable },
        };
        return commandTable;
    }

    /** \brief Teleport the GM to the specified creature
    *
    * .gocreature <GUID>      --> TP using creature.guid
    * .gocreature azuregos    --> TP player to the mob with this name
    *                             Warning: If there is more than one mob with this name
    *                                      you will be teleported to the first one that is found.
    * .gocreature id 6109     --> TP player to the mob, that has this creature_template.entry
    *                             Warning: If there is more than one mob with this "id"
    *                                      you will be teleported to the first one that is found.
    */
    //teleport to creature
    static bool HandleGoCreatureCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        Player* player = handler->GetSession()->GetPlayer();

        // "id" or number or [name] Shift-click form |color|Hcreature_entry:creature_id|h[name]|h|r
        char* param1 = handler->extractKeyFromLink((char*)args, "Hcreature");
        if (!param1)
            return false;

        std::ostringstream whereClause;

        // User wants to teleport to the NPC's template entry
        if (strcmp(param1, "id") == 0)
        {
            // Get the "creature_template.entry"
            // number or [name] Shift-click form |color|Hcreature_entry:creature_id|h[name]|h|r
            char* tail = strtok(NULL, "");
            if (!tail)
                return false;
            char* id = handler->extractKeyFromLink(tail, "Hcreature_entry");
            if (!id)
                return false;

            int32 entry = atoi(id);
            if (!entry)
                return false;

            whereClause << "WHERE id = '" << entry << '\'';
        }
        else
        {
            int32 guid = atoi(param1);

            // Number is invalid - maybe the user specified the mob's name
            if (!guid)
            {
                std::string name = param1;
                WorldDatabase.EscapeString(name);
                whereClause << ", creature_template WHERE creature.id = creature_template.entry AND creature_template.name " _LIKE_" '" << name << '\'';
            }
            else
                whereClause << "WHERE guid = '" << guid << '\'';
        }

        QueryResult result = WorldDatabase.PQuery("SELECT position_x, position_y, position_z, orientation, map FROM creature %s", whereClause.str().c_str());
        if (!result)
        {
            handler->SendSysMessage(LANG_COMMAND_GOCREATNOTFOUND);
            handler->SetSentErrorMessage(true);
            return false;
        }
        if (result->GetRowCount() > 1)
            handler->SendSysMessage(LANG_COMMAND_GOCREATMULTIPLE);

        Field* fields = result->Fetch();
        float x = fields[0].GetFloat();
        float y = fields[1].GetFloat();
        float z = fields[2].GetFloat();
        float o = fields[3].GetFloat();
        uint32 mapId = fields[4].GetUInt16();

        if (!MapManager::IsValidMapCoord(mapId, x, y, z, o) || sObjectMgr->IsTransportMap(mapId))
        {
            handler->PSendSysMessage(LANG_INVALID_TARGET_COORD, x, y, mapId);
            handler->SetSentErrorMessage(true);
            return false;
        }

        // stop flight if need
        if (player->IsInFlight())
        {
            player->GetMotionMaster()->MovementExpired();
            player->CleanupAfterTaxiFlight();
        }
        // save only in non-flight case
        else
            player->SaveRecallPosition();

        player->TeleportTo(mapId, x, y, z, o);

        return true;
    }

    static bool HandleGoGraveyardCommand(ChatHandler* handler, char const* args)
    {
        Player* player = handler->GetSession()->GetPlayer();

        if (!*args)
            return false;

        char* gyId = strtok((char*)args, " ");
        if (!gyId)
            return false;

        int32 graveyardId = atoi(gyId);

        if (!graveyardId)
            return false;

        WorldSafeLocsEntry const* gy = sWorldSafeLocsStore.LookupEntry(graveyardId);
        if (!gy)
        {
            handler->PSendSysMessage(LANG_COMMAND_GRAVEYARDNOEXIST, graveyardId);
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (!MapManager::IsValidMapCoord(gy->MapID, gy->Loc.X, gy->Loc.Y, gy->Loc.Z))
        {
            handler->PSendSysMessage(LANG_INVALID_TARGET_COORD, gy->Loc.X, gy->Loc.Y, gy->MapID);
            handler->SetSentErrorMessage(true);
            return false;
        }

        // stop flight if need
        if (player->IsInFlight())
        {
            player->GetMotionMaster()->MovementExpired();
            player->CleanupAfterTaxiFlight();
        }
        // save only in non-flight case
        else
            player->SaveRecallPosition();

        player->TeleportTo(gy->MapID, gy->Loc.X, gy->Loc.Y, gy->Loc.Z, (gy->Facing * M_PI) / 180); // Orientation is initially in degrees
        return true;
    }

    //teleport to grid
    static bool HandleGoGridCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        Player* player = handler->GetSession()->GetPlayer();

        char* gridX = strtok((char*)args, " ");
        char* gridY = strtok(NULL, " ");
        char* id = strtok(NULL, " ");

        if (!gridX || !gridY)
            return false;

        uint32 mapId = id ? (uint32)atoi(id) : player->GetMapId();

        // center of grid
        float x = ((float)atof(gridX) - CENTER_GRID_ID + 0.5f) * SIZE_OF_GRIDS;
        float y = ((float)atof(gridY) - CENTER_GRID_ID + 0.5f) * SIZE_OF_GRIDS;

        if (!MapManager::IsValidMapCoord(mapId, x, y))
        {
            handler->PSendSysMessage(LANG_INVALID_TARGET_COORD, x, y, mapId);
            handler->SetSentErrorMessage(true);
            return false;
        }

        // stop flight if need
        if (player->IsInFlight())
        {
            player->GetMotionMaster()->MovementExpired();
            player->CleanupAfterTaxiFlight();
        }
        // save only in non-flight case
        else
            player->SaveRecallPosition();

        Map const* map = sMapMgr->CreateBaseMap(mapId);
        float z = std::max(map->GetHeight(x, y, MAX_HEIGHT), map->GetWaterLevel(x, y));

        player->TeleportTo(mapId, x, y, z, player->GetOrientation());
        return true;
    }

    //teleport to gameobject
    static bool HandleGoObjectCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        Player* player = handler->GetSession()->GetPlayer();

        // number or [name] Shift-click form |color|Hgameobject:go_guid|h[name]|h|r
        char* id = handler->extractKeyFromLink((char*)args, "Hgameobject");
        if (!id)
            return false;

        ObjectGuid::LowType guid = strtoull(id, nullptr, 10);
        if (!guid)
            return false;

        float x, y, z, o;
        uint32 mapId;

        // by DB guid
        if (GameObjectData const* goData = sObjectMgr->GetGOData(guid))
        {
            x = goData->posX;
            y = goData->posY;
            z = goData->posZ;
            o = goData->orientation;
            mapId = goData->mapid;
        }
        else
        {
            handler->SendSysMessage(LANG_COMMAND_GOOBJNOTFOUND);
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (!MapManager::IsValidMapCoord(mapId, x, y, z, o) || sObjectMgr->IsTransportMap(mapId))
        {
            handler->PSendSysMessage(LANG_INVALID_TARGET_COORD, x, y, mapId);
            handler->SetSentErrorMessage(true);
            return false;
        }

        // stop flight if need
        if (player->IsInFlight())
        {
            player->GetMotionMaster()->MovementExpired();
            player->CleanupAfterTaxiFlight();
        }
        // save only in non-flight case
        else
            player->SaveRecallPosition();

        player->TeleportTo(mapId, x, y, z, o);
        return true;
    }

    static bool HandleGoQuestCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        Player* player = handler->GetSession()->GetPlayer();

        char* id = handler->extractKeyFromLink((char*)args, "Hquest");
        if (!id)
            return false;

        uint32 questID = atoi(id);
        if (!questID)
            return false;

        if (!sObjectMgr->GetQuestTemplate(questID))
        {
            handler->PSendSysMessage(LANG_COMMAND_QUEST_NOTFOUND, questID);
            handler->SetSentErrorMessage(true);
            return false;
        }

        float x, y, z;
        uint32 mapId;

        if (QuestPOIVector const* poiData = sObjectMgr->GetQuestPOIVector(questID))
        {
            auto data = poiData->front();

            mapId = data.MapID;

            x = data.points.front().X;
            y = data.points.front().Y;
        }
        else
        {
            handler->PSendSysMessage(LANG_COMMAND_QUEST_NOTFOUND, questID);
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (!MapManager::IsValidMapCoord(mapId, x, y) || sObjectMgr->IsTransportMap(mapId))
        {
            handler->PSendSysMessage(LANG_INVALID_TARGET_COORD, x, y, mapId);
            handler->SetSentErrorMessage(true);
            return false;
        }

        // stop flight if need
        if (player->IsInFlight())
        {
            player->GetMotionMaster()->MovementExpired();
            player->CleanupAfterTaxiFlight();
        }
        // save only in non-flight case
        else
            player->SaveRecallPosition();

        Map const* map = sMapMgr->CreateBaseMap(mapId);
        z = std::max(map->GetHeight(x, y, MAX_HEIGHT), map->GetWaterLevel(x, y));

        player->TeleportTo(mapId, x, y, z, 0.0f);
        return true;
    }

    static bool HandleGoTaxinodeCommand(ChatHandler* handler, char const* args)
    {
        Player* player = handler->GetSession()->GetPlayer();

        if (!*args)
            return false;

        char* id = handler->extractKeyFromLink((char*)args, "Htaxinode");
        if (!id)
            return false;

        int32 nodeId = atoi(id);
        if (!nodeId)
            return false;

        TaxiNodesEntry const* node = sTaxiNodesStore.LookupEntry(nodeId);
        if (!node)
        {
            handler->PSendSysMessage(LANG_COMMAND_GOTAXINODENOTFOUND, nodeId);
            handler->SetSentErrorMessage(true);
            return false;
        }

        if ((node->Pos.X == 0.0f && node->Pos.Y == 0.0f && node->Pos.Z == 0.0f) ||
            !MapManager::IsValidMapCoord(node->MapID, node->Pos.X, node->Pos.Y, node->Pos.Z))
        {
            handler->PSendSysMessage(LANG_INVALID_TARGET_COORD, node->Pos.X, node->Pos.Y, node->MapID);
            handler->SetSentErrorMessage(true);
            return false;
        }

        // stop flight if need
        if (player->IsInFlight())
        {
            player->GetMotionMaster()->MovementExpired();
            player->CleanupAfterTaxiFlight();
        }
        // save only in non-flight case
        else
            player->SaveRecallPosition();

        player->TeleportTo(node->MapID, node->Pos.X, node->Pos.Y, node->Pos.Z, player->GetOrientation());
        return true;
    }

    static bool HandleGoTriggerCommand(ChatHandler* handler, char const* args)
    {
        Player* player = handler->GetSession()->GetPlayer();

        if (!*args)
            return false;

        char* id = strtok((char*)args, " ");
        if (!id)
            return false;

        int32 areaTriggerId = atoi(id);

        if (!areaTriggerId)
            return false;

        AreaTriggerEntry const* at = sAreaTriggerStore.LookupEntry(areaTriggerId);
        if (!at)
        {
            handler->PSendSysMessage(LANG_COMMAND_GOAREATRNOTFOUND, areaTriggerId);
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (!MapManager::IsValidMapCoord(at->MapID, at->Pos.X, at->Pos.Y, at->Pos.Z))
        {
            handler->PSendSysMessage(LANG_INVALID_TARGET_COORD, at->Pos.X, at->Pos.Y, at->MapID);
            handler->SetSentErrorMessage(true);
            return false;
        }

        // stop flight if need
        if (player->IsInFlight())
        {
            player->GetMotionMaster()->MovementExpired();
            player->CleanupAfterTaxiFlight();
        }
        // save only in non-flight case
        else
            player->SaveRecallPosition();

        player->TeleportTo(at->MapID, at->Pos.X, at->Pos.Y, at->Pos.Z, player->GetOrientation());
        return true;
    }

    //teleport at coordinates
    static bool HandleGoZoneXYCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        Player* player = handler->GetSession()->GetPlayer();

        char* zoneX = strtok((char*)args, " ");
        char* zoneY = strtok(NULL, " ");
        char* tail = strtok(NULL, "");

        char* id = handler->extractKeyFromLink(tail, "Harea");       // string or [name] Shift-click form |color|Harea:area_id|h[name]|h|r

        if (!zoneX || !zoneY)
            return false;

        float x = (float)atof(zoneX);
        float y = (float)atof(zoneY);

        // prevent accept wrong numeric args
        if ((x == 0.0f && *zoneX != '0') || (y == 0.0f && *zoneY != '0'))
            return false;

        uint32 areaId = id ? (uint32)atoi(id) : player->GetZoneId();

        AreaTableEntry const* areaEntry = sAreaTableStore.LookupEntry(areaId);

        if (x < 0 || x > 100 || y < 0 || y > 100 || !areaEntry)
        {
            handler->PSendSysMessage(LANG_INVALID_ZONE_COORD, x, y, areaId);
            handler->SetSentErrorMessage(true);
            return false;
        }

        // update to parent zone if exist (client map show only zones without parents)
        AreaTableEntry const* zoneEntry = areaEntry->ParentAreaID ? sAreaTableStore.LookupEntry(areaEntry->ParentAreaID) : areaEntry;
        ASSERT(zoneEntry);

        Map const* map = sMapMgr->CreateBaseMap(zoneEntry->MapID);

        if (map->Instanceable())
        {
            handler->PSendSysMessage(LANG_INVALID_ZONE_MAP, areaEntry->ID, areaEntry->AreaName_lang, map->GetId(), map->GetMapName());
            handler->SetSentErrorMessage(true);
            return false;
        }

        Zone2MapCoordinates(x, y, zoneEntry->ID);

        if (!MapManager::IsValidMapCoord(zoneEntry->MapID, x, y))
        {
            handler->PSendSysMessage(LANG_INVALID_TARGET_COORD, x, y, zoneEntry->MapID);
            handler->SetSentErrorMessage(true);
            return false;
        }

        // stop flight if need
        if (player->IsInFlight())
        {
            player->GetMotionMaster()->MovementExpired();
            player->CleanupAfterTaxiFlight();
        }
        // save only in non-flight case
        else
            player->SaveRecallPosition();

        float z = std::max(map->GetHeight(x, y, MAX_HEIGHT), map->GetWaterLevel(x, y));

        player->TeleportTo(zoneEntry->MapID, x, y, z, player->GetOrientation());
        return true;
    }

    //teleport at coordinates, including Z and orientation
    static bool HandleGoXYZCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        Player* player = handler->GetSession()->GetPlayer();

        char* goX = strtok((char*)args, " ");
        char* goY = strtok(NULL, " ");
        char* goZ = strtok(NULL, " ");
        char* id = strtok(NULL, " ");
        char* port = strtok(NULL, " ");

        if (!goX || !goY)
            return false;

        float x = (float)atof(goX);
        float y = (float)atof(goY);
        float z;
        float ort = port ? (float)atof(port) : player->GetOrientation();
        uint32 mapId = id ? (uint32)atoi(id) : player->GetMapId();

        if (goZ)
        {
            z = (float)atof(goZ);
            if (!MapManager::IsValidMapCoord(mapId, x, y, z))
            {
                handler->PSendSysMessage(LANG_INVALID_TARGET_COORD, x, y, mapId);
                handler->SetSentErrorMessage(true);
                return false;
            }
        }
        else
        {
            if (!MapManager::IsValidMapCoord(mapId, x, y))
            {
                handler->PSendSysMessage(LANG_INVALID_TARGET_COORD, x, y, mapId);
                handler->SetSentErrorMessage(true);
                return false;
            }
            Map const* map = sMapMgr->CreateBaseMap(mapId);
            z = std::max(map->GetHeight(x, y, MAX_HEIGHT), map->GetWaterLevel(x, y));
        }

        // stop flight if need
        if (player->IsInFlight())
        {
            player->GetMotionMaster()->MovementExpired();
            player->CleanupAfterTaxiFlight();
        }
        // save only in non-flight case
        else
            player->SaveRecallPosition();

        player->TeleportTo(mapId, x, y, z, ort);
        return true;
    }

    template<typename T>
    static bool HandleGoTicketCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        char* id = strtok((char*)args, " ");
        if (!id)
            return false;

        uint32 ticketId = atoi(id);
        if (!ticketId)
            return false;

        T* ticket = sSupportMgr->GetTicket<T>(ticketId);
        if (!ticket)
        {
            handler->SendSysMessage(LANG_COMMAND_TICKETNOTEXIST);
            return true;
        }

        Player* player = handler->GetSession()->GetPlayer();
        if (player->IsInFlight())
        {
            player->GetMotionMaster()->MovementExpired();
            player->CleanupAfterTaxiFlight();
        }
        else
            player->SaveRecallPosition();

        ticket->TeleportTo(player);
        return true;
    }
};

void AddSC_fgoto_commandscript()
{
    new fgoto_commandscript();
}


#ifdef FREEDOM_MOP_548_CODE

class fgoto_commandscript : public CommandScript
{
public:
    fgoto_commandscript() : CommandScript("fgoto_commandscript") { }

    ChatCommand* GetCommands() const OVERRIDE
    {
        static ChatCommand goCommandTable[] =
        {
            { "creature",   rbac::RBAC_PERM_COMMAND_GO_CREATURE,            false, &HandleGoCreatureCommand,                        "", NULL },
            { "grid",       rbac::RBAC_PERM_COMMAND_GO_GRID,                false, &HandleGoGridCommand,                            "", NULL },
            { "object",     rbac::RBAC_PERM_COMMAND_GO_OBJECT,              false, &HandleGoObjectCommand,                          "", NULL },
            { "gobject",    rbac::RBAC_PERM_COMMAND_GO_OBJECT,              false, &HandleGoObjectCommand,                          "", NULL },
            { "zonexy",     rbac::RBAC_PERM_COMMAND_GO_ZONEXY,              false, &HandleGoZoneXYCommand,                          "", NULL },
            { "xyz",        rbac::RBAC_PERM_COMMAND_GO_XYZ,                 false, &HandleGoXYZCommand,                             "", NULL },
            { "ticket",     rbac::RBAC_PERM_COMMAND_GO_TICKET,              false, &HandleGoTicketCommand,                          "", NULL },
            { "local",      rbac::RBAC_PERM_COMMAND_GO_LOCAL,               false, &HandleGoLocalCommand,                           "", NULL },
            { "relative",   rbac::RBAC_PERM_COMMAND_GO_LOCAL,               false, &HandleGoLocalCommand,                           "", NULL },
            { "",           rbac::RBAC_PERM_COMMAND_GO_LOCAL,               false, &HandleGoLocalCommand,                           "", NULL },
            { NULL, 0, false, NULL, "", NULL }
        };

        static ChatCommand commandTable[] =
        {
            { "goto",       rbac::RBAC_PERM_COMMAND_GO,                     false, NULL,                                            "", goCommandTable },
            { NULL, 0, false, NULL, "", NULL }
        };
        return commandTable;
    }

    static bool HandleGoLocalCommand(ChatHandler* handler, char const* args)
    {
        if (!*args) {
            handler->PSendSysMessage("Please specify parameters. Syntax: .goto [local/relative] $local_x $local_y [$local_z [$local_deg]]");
            return true;
        }

        Player* source = handler->GetSession()->GetPlayer();
        char* token_x = strtok((char*)args, " ");
        char* token_y = strtok(NULL, " ");
        char* token_z = strtok(NULL, " ");
        char* token_deg = strtok(NULL, " ");
        float source_x = source->GetPositionX();
        float source_y = source->GetPositionY();
        float source_z = source->GetPositionZ();
        float source_o = source->GetOrientation();
        uint32 source_map_id = source->GetMapId();

        if (!token_x || !token_y) {
            handler->PSendSysMessage("Not enough parameters. Syntax: .goto [local/relative] $local_x $local_y [$local_z [$local_deg]]");
            return true;
        }

        float add_x = (float)atof(token_x);
        float add_y = (float)atof(token_y);
        float add_z = token_z ? (float)atof(token_z) : 0.0f;
        float add_deg = token_deg ? (float)atof(token_deg) : 0.0f;
        float new_x = add_x*cos(source_o) - add_y*sin(source_o) + source_x; // rotation matrix
        float new_y = add_x*sin(source_o) + add_y*cos(source_o) + source_y; // rotation matrix
        float new_z = add_z + source_z;
        float new_o = add_deg * 3.14159265f / 180.0f + source_o;

        if (!MapManager::IsValidMapCoord(source_map_id, new_x, new_y, new_z))
        {
            handler->PSendSysMessage("Invalid destination [X: %f Y: %f Z: %f MapID: %u].", new_x, new_y, new_z, source_map_id);
            return true;
        }

        // stop flight if need
        if (source->IsInFlight())
        {
            source->GetMotionMaster()->MovementExpired();
            source->CleanupAfterTaxiFlight();
        }
        // save only in non-flight case
        else
        {
            source->SaveRecallPosition();
        }

        source->TeleportTo(source_map_id, new_x, new_y, new_z, new_o);
        return true;
    }

    static bool HandleGoCreatureCommand(ChatHandler* handler, char const* args)
    {
        Player* source = handler->GetSession()->GetPlayer();
        
        Creature * creature = NULL;
        

        // "id" or number or [name] Shift-click form |color|Hcreature_entry:creature_id|h[name]|h|r
        char* param_guid = handler->extractKeyFromLink((char*)args, "Hcreature");

        if (param_guid)
        {
            uint32 guid_low = (uint32)atol(param_guid);
            
            if (CreatureData const* cr_data = sObjectMgr->GetCreatureData(guid_low))
            {
                creature = sObjectAccessor->GetObjectInWorld(MAKE_NEW_GUID(guid_low, cr_data->id, HIGHGUID_UNIT), (Creature*)NULL);
                if (!creature)
                    creature = sObjectAccessor->GetObjectInWorld(MAKE_NEW_GUID(guid_low, cr_data->id, HIGHGUID_PET), (Creature*)NULL);
                if (!creature)
                    creature = sObjectAccessor->GetObjectInWorld(MAKE_NEW_GUID(guid_low, cr_data->id, HIGHGUID_VEHICLE), (Creature*)NULL);
            }
        }
        else
        {
            creature = handler->getSelectedCreature();
        }

        if (!creature)
        {
            handler->PSendSysMessage("Creature not found");
        }

        float pos_x = creature->GetPositionX();
        float pos_y = creature->GetPositionY();
        float pos_z = creature->GetPositionZ();
        float pos_o = creature->GetOrientation();
        int map_id = creature->GetMapId();

        if (!MapManager::IsValidMapCoord(map_id, pos_x, pos_y, pos_z, pos_o))
        {
            handler->PSendSysMessage(LANG_INVALID_TARGET_COORD, pos_x, pos_y, map_id);
            return true;
        }

        // stop flight if need
        if (source->IsInFlight())
        {
            source->GetMotionMaster()->MovementExpired();
            source->CleanupAfterTaxiFlight();
        }
        // save only in non-flight case
        else
        {
            source->SaveRecallPosition();
        }

        source->TeleportTo(map_id, pos_x, pos_y, pos_z, pos_o);
        return true;
    }

    //teleport to grid
    static bool HandleGoGridCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        Player* source = handler->GetSession()->GetPlayer();

        char* gridX = strtok((char*)args, " ");
        char* gridY = strtok(NULL, " ");
        char* id = strtok(NULL, " ");

        if (!gridX || !gridY)
            return false;

        uint32 mapId = id ? (uint32)atoi(id) : source->GetMapId();

        // center of grid
        float x = ((float)atof(gridX) - CENTER_GRID_ID + 0.5f) * SIZE_OF_GRIDS;
        float y = ((float)atof(gridY) - CENTER_GRID_ID + 0.5f) * SIZE_OF_GRIDS;

        if (!MapManager::IsValidMapCoord(mapId, x, y))
        {
            handler->PSendSysMessage(LANG_INVALID_TARGET_COORD, x, y, mapId);
            handler->SetSentErrorMessage(true);
            return false;
        }

        // stop flight if need
        if (source->IsInFlight())
        {
            source->GetMotionMaster()->MovementExpired();
            source->CleanupAfterTaxiFlight();
        }
        // save only in non-flight case
        else
        {
            source->SaveRecallPosition();
        }

        Map const* map = sMapMgr->CreateBaseMap(mapId);
        float z = std::max(map->GetHeight(x, y, MAX_HEIGHT), map->GetWaterLevel(x, y));

        source->TeleportTo(mapId, x, y, z, source->GetOrientation());
        return true;
    }

    //teleport to gameobject
    static bool HandleGoObjectCommand(ChatHandler* handler, char const* args)
    {

        Player* source = handler->GetSession()->GetPlayer();
        int32 guid = 0;

        // number or [name] Shift-click form |color|Hgameobject:go_guid|h[name]|h|r
        char* param_guid = handler->extractKeyFromLink((char*)args, "Hgameobject");
        
        if (param_guid)
        {
            guid = (uint32)atol(param_guid);
        }
        else
        {
            guid = source->GetSelectedGameObject();
        }

        GameObjectData const* goData = sObjectMgr->GetGOData(guid);

        if (!guid || !goData)
        {
            handler->PSendSysMessage("Game object not found");
            return true;
        }

        float pos_x = goData->posX;
        float pos_y = goData->posY;
        float pos_z = goData->posZ;
        float pos_o = goData->orientation;
        uint32 map_id = goData->mapid;

        if (!MapManager::IsValidMapCoord(map_id, pos_x, pos_y, pos_z, pos_o))
        {
            handler->PSendSysMessage(LANG_INVALID_TARGET_COORD, pos_x, pos_y, map_id);
            return true;
        }

        // stop flight if need
        if (source->IsInFlight())
        {
            source->GetMotionMaster()->MovementExpired();
            source->CleanupAfterTaxiFlight();
        }
        // save only in non-flight case
        else
        {
            source->SaveRecallPosition();
        }

        source->TeleportTo(map_id, pos_x, pos_y, pos_z, pos_o);
        return true;
    }

    //teleport at coordinates
    static bool HandleGoZoneXYCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        Player* source = handler->GetSession()->GetPlayer();

        char* zoneX = strtok((char*)args, " ");
        char* zoneY = strtok(NULL, " ");
        char* tail = strtok(NULL, "");

        char* id = handler->extractKeyFromLink(tail, "Harea");       // string or [name] Shift-click form |color|Harea:area_id|h[name]|h|r

        if (!zoneX || !zoneY)
            return false;

        float x = (float)atof(zoneX);
        float y = (float)atof(zoneY);

        // prevent accept wrong numeric args
        if ((x == 0.0f && *zoneX != '0') || (y == 0.0f && *zoneY != '0'))
            return false;

        uint32 areaId = id ? (uint32)atoi(id) : source->GetZoneId();

        AreaTableEntry const* areaEntry = GetAreaEntryByAreaID(areaId);

        if (x < 0 || x > 100 || y < 0 || y > 100 || !areaEntry)
        {
            handler->PSendSysMessage(LANG_INVALID_ZONE_COORD, x, y, areaId);
            handler->SetSentErrorMessage(true);
            return false;
        }

        // update to parent zone if exist (client map show only zones without parents)
        AreaTableEntry const* zoneEntry = areaEntry->zone ? GetAreaEntryByAreaID(areaEntry->zone) : areaEntry;

        Map const* map = sMapMgr->CreateBaseMap(zoneEntry->mapid);

        if (map->Instanceable())
        {
            handler->PSendSysMessage(LANG_INVALID_ZONE_MAP, areaEntry->ID, areaEntry->area_name, map->GetId(), map->GetMapName());
            handler->SetSentErrorMessage(true);
            return false;
        }

        Zone2MapCoordinates(x, y, zoneEntry->ID);

        if (!MapManager::IsValidMapCoord(zoneEntry->mapid, x, y))
        {
            handler->PSendSysMessage(LANG_INVALID_TARGET_COORD, x, y, zoneEntry->mapid);
            handler->SetSentErrorMessage(true);
            return false;
        }

        // stop flight if need
        if (source->IsInFlight())
        {
            source->GetMotionMaster()->MovementExpired();
            source->CleanupAfterTaxiFlight();
        }
        // save only in non-flight case
        else
        {
            source->SaveRecallPosition();
        }

        float z = std::max(map->GetHeight(x, y, MAX_HEIGHT), map->GetWaterLevel(x, y));

        source->TeleportTo(zoneEntry->mapid, x, y, z, source->GetOrientation());
        return true;
    }

    //teleport at coordinates, including Z and orientation
    static bool HandleGoXYZCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        Player* source = handler->GetSession()->GetPlayer();

        char* goX = strtok((char*)args, " ");
        char* goY = strtok(NULL, " ");
        char* goZ = strtok(NULL, " ");
        char* id = strtok(NULL, " ");
        char* port = strtok(NULL, " ");

        if (!goX || !goY)
            return false;

        float x = (float)atof(goX);
        float y = (float)atof(goY);
        float z;
        float ort = port ? (float)atof(port) : source->GetOrientation();
        uint32 mapId = id ? (uint32)atoi(id) : source->GetMapId();

        if (goZ)
        {
            z = (float)atof(goZ);
            if (!MapManager::IsValidMapCoord(mapId, x, y, z))
            {
                handler->PSendSysMessage(LANG_INVALID_TARGET_COORD, x, y, mapId);
                handler->SetSentErrorMessage(true);
                return false;
            }
        }
        else
        {
            if (!MapManager::IsValidMapCoord(mapId, x, y))
            {
                handler->PSendSysMessage(LANG_INVALID_TARGET_COORD, x, y, mapId);
                handler->SetSentErrorMessage(true);
                return false;
            }
            Map const* map = sMapMgr->CreateBaseMap(mapId);
            z = std::max(map->GetHeight(x, y, MAX_HEIGHT), map->GetWaterLevel(x, y));
        }

        // stop flight if need
        if (source->IsInFlight())
        {
            source->GetMotionMaster()->MovementExpired();
            source->CleanupAfterTaxiFlight();
        }
        // save only in non-flight case
        else
        {
            source->SaveRecallPosition();
        }

        source->TeleportTo(mapId, x, y, z, ort);
        return true;
    }

    static bool HandleGoTicketCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        char* id = strtok((char*)args, " ");
        if (!id)
            return false;

        uint32 ticketId = atoi(id);
        if (!ticketId)
            return false;

        GmTicket* ticket = sTicketMgr->GetTicket(ticketId);
        if (!ticket)
        {
            handler->SendSysMessage(LANG_COMMAND_TICKETNOTEXIST);
            return true;
        }

        Player* source = handler->GetSession()->GetPlayer();

        // stop flight if need
        if (source->IsInFlight())
        {
            source->GetMotionMaster()->MovementExpired();
            source->CleanupAfterTaxiFlight();
        }
        // save only in non-flight case
        else
        {
            source->SaveRecallPosition();
        }

        ticket->TeleportTo(source);
        return true;
    }
};

void AddSC_fgoto_commandscript()
{
    new fgoto_commandscript();
}

#endif