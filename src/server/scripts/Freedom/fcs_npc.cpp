#include "ScriptMgr.h"
#include "ObjectMgr.h"
#include "Chat.h"
#include "Transport.h"
#include "CreatureGroups.h"
#include "Language.h"
#include "TargetedMovementGenerator.h"                      // for HandleNpcUnFollowCommand
#include "CreatureAI.h"
#include "Player.h"
#include "Pet.h"

template<typename E, typename T = char const*>
struct EnumName
{
    E Value;
    T Name;
};

#define CREATE_NAMED_ENUM(VALUE) { VALUE, STRINGIZE(VALUE) }

#define NPCFLAG_COUNT   24
#define FLAGS_EXTRA_COUNT 16

EnumName<NPCFlags, int32> const npcFlagTexts[NPCFLAG_COUNT] =
{
    { UNIT_NPC_FLAG_AUCTIONEER,         LANG_NPCINFO_AUCTIONEER },
    { UNIT_NPC_FLAG_BANKER,             LANG_NPCINFO_BANKER },
    { UNIT_NPC_FLAG_BATTLEMASTER,       LANG_NPCINFO_BATTLEMASTER },
    { UNIT_NPC_FLAG_FLIGHTMASTER,       LANG_NPCINFO_FLIGHTMASTER },
    { UNIT_NPC_FLAG_GOSSIP,             LANG_NPCINFO_GOSSIP },
    { UNIT_NPC_FLAG_GUILD_BANKER,       LANG_NPCINFO_GUILD_BANKER },
    { UNIT_NPC_FLAG_INNKEEPER,          LANG_NPCINFO_INNKEEPER },
    { UNIT_NPC_FLAG_PETITIONER,         LANG_NPCINFO_PETITIONER },
    { UNIT_NPC_FLAG_PLAYER_VEHICLE,     LANG_NPCINFO_PLAYER_VEHICLE },
    { UNIT_NPC_FLAG_QUESTGIVER,         LANG_NPCINFO_QUESTGIVER },
    { UNIT_NPC_FLAG_REPAIR,             LANG_NPCINFO_REPAIR },
    { UNIT_NPC_FLAG_SPELLCLICK,         LANG_NPCINFO_SPELLCLICK },
    { UNIT_NPC_FLAG_SPIRITGUIDE,        LANG_NPCINFO_SPIRITGUIDE },
    { UNIT_NPC_FLAG_SPIRITHEALER,       LANG_NPCINFO_SPIRITHEALER },
    { UNIT_NPC_FLAG_STABLEMASTER,       LANG_NPCINFO_STABLEMASTER },
    { UNIT_NPC_FLAG_TABARDDESIGNER,     LANG_NPCINFO_TABARDDESIGNER },
    { UNIT_NPC_FLAG_TRAINER,            LANG_NPCINFO_TRAINER },
    { UNIT_NPC_FLAG_TRAINER_CLASS,      LANG_NPCINFO_TRAINER_CLASS },
    { UNIT_NPC_FLAG_TRAINER_PROFESSION, LANG_NPCINFO_TRAINER_PROFESSION },
    { UNIT_NPC_FLAG_VENDOR,             LANG_NPCINFO_VENDOR },
    { UNIT_NPC_FLAG_VENDOR_AMMO,        LANG_NPCINFO_VENDOR_AMMO },
    { UNIT_NPC_FLAG_VENDOR_FOOD,        LANG_NPCINFO_VENDOR_FOOD },
    { UNIT_NPC_FLAG_VENDOR_POISON,      LANG_NPCINFO_VENDOR_POISON },
    { UNIT_NPC_FLAG_VENDOR_REAGENT,     LANG_NPCINFO_VENDOR_REAGENT }
};

EnumName<Mechanics> const mechanicImmunes[MAX_MECHANIC] =
{
    CREATE_NAMED_ENUM(MECHANIC_NONE),
    CREATE_NAMED_ENUM(MECHANIC_CHARM),
    CREATE_NAMED_ENUM(MECHANIC_DISORIENTED),
    CREATE_NAMED_ENUM(MECHANIC_DISARM),
    CREATE_NAMED_ENUM(MECHANIC_DISTRACT),
    CREATE_NAMED_ENUM(MECHANIC_FEAR),
    CREATE_NAMED_ENUM(MECHANIC_GRIP),
    CREATE_NAMED_ENUM(MECHANIC_ROOT),
    CREATE_NAMED_ENUM(MECHANIC_SLOW_ATTACK),
    CREATE_NAMED_ENUM(MECHANIC_SILENCE),
    CREATE_NAMED_ENUM(MECHANIC_SLEEP),
    CREATE_NAMED_ENUM(MECHANIC_SNARE),
    CREATE_NAMED_ENUM(MECHANIC_STUN),
    CREATE_NAMED_ENUM(MECHANIC_FREEZE),
    CREATE_NAMED_ENUM(MECHANIC_KNOCKOUT),
    CREATE_NAMED_ENUM(MECHANIC_BLEED),
    CREATE_NAMED_ENUM(MECHANIC_BANDAGE),
    CREATE_NAMED_ENUM(MECHANIC_POLYMORPH),
    CREATE_NAMED_ENUM(MECHANIC_BANISH),
    CREATE_NAMED_ENUM(MECHANIC_SHIELD),
    CREATE_NAMED_ENUM(MECHANIC_SHACKLE),
    CREATE_NAMED_ENUM(MECHANIC_MOUNT),
    CREATE_NAMED_ENUM(MECHANIC_INFECTED),
    CREATE_NAMED_ENUM(MECHANIC_TURN),
    CREATE_NAMED_ENUM(MECHANIC_HORROR),
    CREATE_NAMED_ENUM(MECHANIC_INVULNERABILITY),
    CREATE_NAMED_ENUM(MECHANIC_INTERRUPT),
    CREATE_NAMED_ENUM(MECHANIC_DAZE),
    CREATE_NAMED_ENUM(MECHANIC_DISCOVERY),
    CREATE_NAMED_ENUM(MECHANIC_IMMUNE_SHIELD),
    CREATE_NAMED_ENUM(MECHANIC_SAPPED),
    CREATE_NAMED_ENUM(MECHANIC_ENRAGED),
    CREATE_NAMED_ENUM(MECHANIC_WOUNDED)
};

EnumName<UnitFlags> const unitFlags[MAX_UNIT_FLAGS] =
{
    CREATE_NAMED_ENUM(UNIT_FLAG_SERVER_CONTROLLED),
    CREATE_NAMED_ENUM(UNIT_FLAG_NON_ATTACKABLE),
    CREATE_NAMED_ENUM(UNIT_FLAG_DISABLE_MOVE),
    CREATE_NAMED_ENUM(UNIT_FLAG_PVP_ATTACKABLE),
    CREATE_NAMED_ENUM(UNIT_FLAG_RENAME),
    CREATE_NAMED_ENUM(UNIT_FLAG_PREPARATION),
    CREATE_NAMED_ENUM(UNIT_FLAG_UNK_6),
    CREATE_NAMED_ENUM(UNIT_FLAG_NOT_ATTACKABLE_1),
    CREATE_NAMED_ENUM(UNIT_FLAG_IMMUNE_TO_PC),
    CREATE_NAMED_ENUM(UNIT_FLAG_IMMUNE_TO_NPC),
    CREATE_NAMED_ENUM(UNIT_FLAG_LOOTING),
    CREATE_NAMED_ENUM(UNIT_FLAG_PET_IN_COMBAT),
    CREATE_NAMED_ENUM(UNIT_FLAG_PVP),
    CREATE_NAMED_ENUM(UNIT_FLAG_SILENCED),
    CREATE_NAMED_ENUM(UNIT_FLAG_UNK_14),
    CREATE_NAMED_ENUM(UNIT_FLAG_UNK_15),
    CREATE_NAMED_ENUM(UNIT_FLAG_UNK_16),
    CREATE_NAMED_ENUM(UNIT_FLAG_PACIFIED),
    CREATE_NAMED_ENUM(UNIT_FLAG_STUNNED),
    CREATE_NAMED_ENUM(UNIT_FLAG_IN_COMBAT),
    CREATE_NAMED_ENUM(UNIT_FLAG_TAXI_FLIGHT),
    CREATE_NAMED_ENUM(UNIT_FLAG_DISARMED),
    CREATE_NAMED_ENUM(UNIT_FLAG_CONFUSED),
    CREATE_NAMED_ENUM(UNIT_FLAG_FLEEING),
    CREATE_NAMED_ENUM(UNIT_FLAG_PLAYER_CONTROLLED),
    CREATE_NAMED_ENUM(UNIT_FLAG_NOT_SELECTABLE),
    CREATE_NAMED_ENUM(UNIT_FLAG_SKINNABLE),
    CREATE_NAMED_ENUM(UNIT_FLAG_MOUNT),
    CREATE_NAMED_ENUM(UNIT_FLAG_UNK_28),
    CREATE_NAMED_ENUM(UNIT_FLAG_UNK_29),
    CREATE_NAMED_ENUM(UNIT_FLAG_SHEATHE),
    CREATE_NAMED_ENUM(UNIT_FLAG_UNK_31)
};

EnumName<CreatureFlagsExtra> const flagsExtra[FLAGS_EXTRA_COUNT] =
{
    CREATE_NAMED_ENUM(CREATURE_FLAG_EXTRA_INSTANCE_BIND),
    CREATE_NAMED_ENUM(CREATURE_FLAG_EXTRA_CIVILIAN),
    CREATE_NAMED_ENUM(CREATURE_FLAG_EXTRA_NO_PARRY),
    CREATE_NAMED_ENUM(CREATURE_FLAG_EXTRA_NO_PARRY_HASTEN),
    CREATE_NAMED_ENUM(CREATURE_FLAG_EXTRA_NO_BLOCK),
    CREATE_NAMED_ENUM(CREATURE_FLAG_EXTRA_NO_CRUSH),
    CREATE_NAMED_ENUM(CREATURE_FLAG_EXTRA_NO_XP_AT_KILL),
    CREATE_NAMED_ENUM(CREATURE_FLAG_EXTRA_TRIGGER),
    CREATE_NAMED_ENUM(CREATURE_FLAG_EXTRA_NO_TAUNT),
    CREATE_NAMED_ENUM(CREATURE_FLAG_EXTRA_WORLDEVENT),
    CREATE_NAMED_ENUM(CREATURE_FLAG_EXTRA_GUARD),
    CREATE_NAMED_ENUM(CREATURE_FLAG_EXTRA_NO_CRIT),
    CREATE_NAMED_ENUM(CREATURE_FLAG_EXTRA_NO_SKILLGAIN),
    CREATE_NAMED_ENUM(CREATURE_FLAG_EXTRA_TAUNT_DIMINISH),
    CREATE_NAMED_ENUM(CREATURE_FLAG_EXTRA_ALL_DIMINISH),
    CREATE_NAMED_ENUM(CREATURE_FLAG_EXTRA_DUNGEON_BOSS)
};

class fnpc_commandscript : public CommandScript
{
public:
    fnpc_commandscript() : CommandScript("fnpc_commandscript") { }

    std::vector<ChatCommand> GetCommands() const override
    {
        static std::vector<ChatCommand> npcAddCommandTable =
        {
            { "formation", rbac::RBAC_PERM_COMMAND_NPC_ADD_FORMATION, false, &HandleNpcAddFormationCommand,      "" },
            { "item",      rbac::RBAC_PERM_COMMAND_NPC_ADD_ITEM,      false, &HandleNpcAddVendorItemCommand,     "" },
            { "move",      rbac::RBAC_PERM_COMMAND_NPC_ADD_MOVE,      false, &HandleNpcAddMoveCommand,           "" },
            { "temp",      rbac::RBAC_PERM_COMMAND_NPC_ADD_TEMP,      false, &HandleNpcAddTempSpawnCommand,      "" },
            //{ "weapon",    rbac::RBAC_PERM_COMMAND_NPC_ADD_WEAPON,    false, &HandleNpcAddWeaponCommand,         "" },
            { "",          rbac::RBAC_PERM_COMMAND_NPC_ADD,           false, &HandleNpcAddCommand,               "" },
        };
        static std::vector<ChatCommand> npcDeleteCommandTable =
        {
            { "item", rbac::RBAC_PERM_COMMAND_NPC_DELETE_ITEM, false, &HandleNpcDeleteVendorItemCommand,  "" },
            { "",     rbac::RBAC_PERM_COMMAND_NPC_DELETE,      false, &HandleNpcDeleteCommand,            "" },
        };
        static std::vector<ChatCommand> npcFollowCommandTable =
        {
            { "stop", rbac::RBAC_PERM_COMMAND_NPC_FOLLOW_STOP, false, &HandleNpcUnFollowCommand,          "" },
            { "",     rbac::RBAC_PERM_COMMAND_NPC_FOLLOW,      false, &HandleNpcFollowCommand,            "" },
        };
        static std::vector<ChatCommand> npcSetCommandTable =
        {
            { "allowmove",  rbac::RBAC_PERM_COMMAND_NPC_SET_ALLOWMOVE, false, &HandleNpcSetAllowMovementCommand, "" },
            { "entry",      rbac::RBAC_PERM_COMMAND_NPC_SET_ENTRY,     false, &HandleNpcSetEntryCommand,         "" },
            { "factionid",  rbac::RBAC_PERM_COMMAND_NPC_SET_FACTIONID, false, &HandleNpcSetFactionIdCommand,     "" },
            { "flag",       rbac::RBAC_PERM_COMMAND_NPC_SET_FLAG,      false, &HandleNpcSetFlagCommand,          "" },
            { "level",      rbac::RBAC_PERM_COMMAND_NPC_SET_LEVEL,     false, &HandleNpcSetLevelCommand,         "" },
            { "link",       rbac::RBAC_PERM_COMMAND_NPC_SET_LINK,      false, &HandleNpcSetLinkCommand,          "" },
            { "model",      rbac::RBAC_PERM_COMMAND_NPC_SET_MODEL,     false, &HandleNpcSetModelCommand,         "" },
            { "movetype",   rbac::RBAC_PERM_COMMAND_NPC_SET_MOVETYPE,  false, &HandleNpcSetMoveTypeCommand,      "" },
            { "phase",      rbac::RBAC_PERM_COMMAND_NPC_SET_PHASE,     false, &HandleNpcSetPhaseCommand,         "" },
            { "phasegroup", rbac::RBAC_PERM_COMMAND_NPC_SET_PHASE,     false, &HandleNpcSetPhaseGroup,           "" },
            { "spawndist",  rbac::RBAC_PERM_COMMAND_NPC_SET_SPAWNDIST, false, &HandleNpcSetSpawnDistCommand,     "" },
            { "spawntime",  rbac::RBAC_PERM_COMMAND_NPC_SET_SPAWNTIME, false, &HandleNpcSetSpawnTimeCommand,     "" },
            { "data",       rbac::RBAC_PERM_COMMAND_NPC_SET_DATA,      false, &HandleNpcSetDataCommand,          "" },
        };
        static std::vector<ChatCommand> npcCommandTable =
        {
            { "info",      rbac::RBAC_PERM_COMMAND_NPC_INFO,      false, &HandleNpcInfoCommand,              "" },
            { "near",      rbac::RBAC_PERM_COMMAND_NPC_NEAR,      false, &HandleNpcNearCommand,              "" },
            { "move",      rbac::RBAC_PERM_COMMAND_NPC_MOVE,      false, &HandleNpcMoveCommand,              "" },
            { "playemote", rbac::RBAC_PERM_COMMAND_NPC_PLAYEMOTE, false, &HandleNpcPlayEmoteCommand,         "" },
            { "say",       rbac::RBAC_PERM_COMMAND_NPC_SAY,       false, &HandleNpcSayCommand,               "" },
            { "textemote", rbac::RBAC_PERM_COMMAND_NPC_TEXTEMOTE, false, &HandleNpcTextEmoteCommand,         "" },
            { "whisper",   rbac::RBAC_PERM_COMMAND_NPC_WHISPER,   false, &HandleNpcWhisperCommand,           "" },
            { "yell",      rbac::RBAC_PERM_COMMAND_NPC_YELL,      false, &HandleNpcYellCommand,              "" },
            { "tame",      rbac::RBAC_PERM_COMMAND_NPC_TAME,      false, &HandleNpcTameCommand,              "" },
            { "add",       rbac::RBAC_PERM_COMMAND_NPC_ADD,       false, NULL,                 "", npcAddCommandTable },
            { "delete",    rbac::RBAC_PERM_COMMAND_NPC_DELETE,    false, NULL,              "", npcDeleteCommandTable },
            { "follow",    rbac::RBAC_PERM_COMMAND_NPC_FOLLOW,    false, NULL,              "", npcFollowCommandTable },
            { "set",       rbac::RBAC_PERM_COMMAND_NPC_SET,       false, NULL,                 "", npcSetCommandTable },
        };
        static std::vector<ChatCommand> commandTable =
        {
            { "npc", rbac::RBAC_PERM_COMMAND_NPC, false, NULL, "", npcCommandTable },
        };
        return commandTable;
    }

    //add spawn of creature
    static bool HandleNpcAddCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        char* charID = handler->extractKeyFromLink((char*)args, "Hcreature_entry");
        if (!charID)
            return false;

        uint32 id = atoi(charID);
        if (!sObjectMgr->GetCreatureTemplate(id))
            return false;

        Player* chr = handler->GetSession()->GetPlayer();
        float x = chr->GetPositionX();
        float y = chr->GetPositionY();
        float z = chr->GetPositionZ();
        float o = chr->GetOrientation();
        Map* map = chr->GetMap();

        if (Transport* trans = chr->GetTransport())
        {
            ObjectGuid::LowType guid = map->GenerateLowGuid<HighGuid::Creature>();
            CreatureData& data = sObjectMgr->NewOrExistCreatureData(guid);
            data.id = id;
            data.phaseMask = chr->GetPhaseMask();
            data.posX = chr->GetTransOffsetX();
            data.posY = chr->GetTransOffsetY();
            data.posZ = chr->GetTransOffsetZ();
            data.orientation = chr->GetTransOffsetO();

            Creature* creature = trans->CreateNPCPassenger(guid, &data);

            creature->SaveToDB(trans->GetGOInfo()->moTransport.mapID, 1 << map->GetSpawnMode(), chr->GetPhaseMask());

            sObjectMgr->AddCreatureToGrid(guid, &data);
            return true;
        }

        Creature* creature = new Creature();
        if (!creature->Create(map->GenerateLowGuid<HighGuid::Creature>(), map, chr->GetPhaseMask(), id, x, y, z, o))
        {
            delete creature;
            return false;
        }

        creature->SaveToDB(map->GetId(), (1 << map->GetSpawnMode()), chr->GetPhaseMask());

        ObjectGuid::LowType db_guid = creature->GetSpawnId();

        // To call _LoadGoods(); _LoadQuests(); CreateTrainerSpells()
        // current "creature" variable is deleted and created fresh new, otherwise old values might trigger asserts or cause undefined behavior
        creature->CleanupsBeforeDelete();
        delete creature;
        creature = new Creature();
        if (!creature->LoadCreatureFromDB(db_guid, map))
        {
            delete creature;
            return false;
        }

        sObjectMgr->AddCreatureToGrid(db_guid, sObjectMgr->GetCreatureData(db_guid));
        return true;
    }

    //add item in vendorlist
    static bool HandleNpcAddVendorItemCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        const uint8 type = 1; // FIXME: make type (1 item, 2 currency) an argument

        char* pitem = handler->extractKeyFromLink((char*)args, "Hitem");
        if (!pitem)
        {
            handler->SendSysMessage(LANG_COMMAND_NEEDITEMSEND);
            handler->SetSentErrorMessage(true);
            return false;
        }

        int32 item_int = atol(pitem);
        if (item_int <= 0)
            return false;

        uint32 itemId = item_int;

        char* fmaxcount = strtok(NULL, " ");                    //add maxcount, default: 0
        uint32 maxcount = 0;
        if (fmaxcount)
            maxcount = atoul(fmaxcount);

        char* fincrtime = strtok(NULL, " ");                    //add incrtime, default: 0
        uint32 incrtime = 0;
        if (fincrtime)
            incrtime = atoul(fincrtime);

        char* fextendedcost = strtok(NULL, " ");                //add ExtendedCost, default: 0
        uint32 extendedcost = fextendedcost ? atoul(fextendedcost) : 0;
        Creature* vendor = handler->getSelectedCreature();
        if (!vendor)
        {
            handler->SendSysMessage(LANG_SELECT_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        uint32 vendor_entry = vendor->GetEntry();

        if (!sObjectMgr->IsVendorItemValid(vendor_entry, itemId, maxcount, incrtime, extendedcost, type, handler->GetSession()->GetPlayer()))
        {
            handler->SetSentErrorMessage(true);
            return false;
        }

        sObjectMgr->AddVendorItem(vendor_entry, itemId, maxcount, incrtime, extendedcost, type);

        ItemTemplate const* itemTemplate = sObjectMgr->GetItemTemplate(itemId);

        handler->PSendSysMessage(LANG_ITEM_ADDED_TO_LIST, itemId, itemTemplate->GetDefaultLocaleName(), maxcount, incrtime, extendedcost);
        return true;
    }

    //add move for creature
    static bool HandleNpcAddMoveCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        char* guidStr = strtok((char*)args, " ");
        char* waitStr = strtok((char*)NULL, " ");

        ObjectGuid::LowType lowGuid = strtoull(guidStr, nullptr, 10);

        // attempt check creature existence by DB data
        CreatureData const* data = sObjectMgr->GetCreatureData(lowGuid);
        if (!data)
        {
            handler->PSendSysMessage(LANG_COMMAND_CREATGUIDNOTFOUND, lowGuid);
            handler->SetSentErrorMessage(true);
            return false;
        }

        int wait = waitStr ? atoi(waitStr) : 0;

        if (wait < 0)
            wait = 0;

        // Update movement type
        PreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_UPD_CREATURE_MOVEMENT_TYPE);

        stmt->setUInt8(0, uint8(WAYPOINT_MOTION_TYPE));
        stmt->setUInt64(1, lowGuid);

        WorldDatabase.Execute(stmt);

        handler->SendSysMessage(LANG_WAYPOINT_ADDED);

        return true;
    }

    static bool HandleNpcSetAllowMovementCommand(ChatHandler* handler, char const* /*args*/)
    {
        if (sWorld->getAllowMovement())
        {
            sWorld->SetAllowMovement(false);
            handler->SendSysMessage(LANG_CREATURE_MOVE_DISABLED);
        }
        else
        {
            sWorld->SetAllowMovement(true);
            handler->SendSysMessage(LANG_CREATURE_MOVE_ENABLED);
        }
        return true;
    }

    static bool HandleNpcSetEntryCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        uint32 newEntryNum = atoi(args);
        if (!newEntryNum)
            return false;

        Unit* unit = handler->getSelectedUnit();
        if (!unit || unit->GetTypeId() != TYPEID_UNIT)
        {
            handler->SendSysMessage(LANG_SELECT_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }
        Creature* creature = unit->ToCreature();
        if (creature->UpdateEntry(newEntryNum))
            handler->SendSysMessage(LANG_DONE);
        else
            handler->SendSysMessage(LANG_ERROR);
        return true;
    }

    //change level of creature or pet
    static bool HandleNpcSetLevelCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        uint8 lvl = (uint8)atoi((char*)args);
        if (lvl < 1 || lvl > sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL) + 3)
        {
            handler->SendSysMessage(LANG_BAD_VALUE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        Creature* creature = handler->getSelectedCreature();
        if (!creature)
        {
            handler->SendSysMessage(LANG_SELECT_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (creature->IsPet())
        {
            if (((Pet*)creature)->getPetType() == HUNTER_PET)
            {
                creature->SetUInt32Value(UNIT_FIELD_PETNEXTLEVELEXP, sObjectMgr->GetXPForLevel(lvl) / 4);
                creature->SetUInt32Value(UNIT_FIELD_PETEXPERIENCE, 0);
            }
            ((Pet*)creature)->GivePetLevel(lvl);
        }
        else
        {
            creature->SetMaxHealth(100 + 30 * lvl);
            creature->SetHealth(100 + 30 * lvl);
            creature->SetLevel(lvl);
            creature->SaveToDB();
        }

        return true;
    }

    static bool HandleNpcDeleteCommand(ChatHandler* handler, char const* args)
    {
        Creature* unit = NULL;

        if (*args)
        {
            // number or [name] Shift-click form |color|Hcreature:creature_guid|h[name]|h|r
            char* cId = handler->extractKeyFromLink((char*)args, "Hcreature");
            if (!cId)
                return false;

            ObjectGuid::LowType lowguid = strtoull(cId, nullptr, 10);
            if (!lowguid)
                return false;

            if (CreatureData const* cr_data = sObjectMgr->GetCreatureData(lowguid))
                unit = handler->GetSession()->GetPlayer()->GetMap()->GetCreature(ObjectGuid::Create<HighGuid::Creature>(cr_data->mapid, cr_data->id, lowguid));
        }
        else
            unit = handler->getSelectedCreature();

        if (!unit || unit->IsPet() || unit->IsTotem())
        {
            handler->SendSysMessage(LANG_SELECT_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        // Delete the creature
        unit->CombatStop();
        unit->DeleteFromDB();
        unit->AddObjectToRemoveList();

        handler->SendSysMessage(LANG_COMMAND_DELCREATMESSAGE);

        return true;
    }

    //del item from vendor list
    static bool HandleNpcDeleteVendorItemCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        Creature* vendor = handler->getSelectedCreature();
        if (!vendor || !vendor->IsVendor())
        {
            handler->SendSysMessage(LANG_COMMAND_VENDORSELECTION);
            handler->SetSentErrorMessage(true);
            return false;
        }

        char* pitem = handler->extractKeyFromLink((char*)args, "Hitem");
        if (!pitem)
        {
            handler->SendSysMessage(LANG_COMMAND_NEEDITEMSEND);
            handler->SetSentErrorMessage(true);
            return false;
        }
        uint32 itemId = atoul(pitem);

        const uint8 type = 1; // FIXME: make type (1 item, 2 currency) an argument

        if (!sObjectMgr->RemoveVendorItem(vendor->GetEntry(), itemId, type))
        {
            handler->PSendSysMessage(LANG_ITEM_NOT_IN_LIST, itemId);
            handler->SetSentErrorMessage(true);
            return false;
        }

        ItemTemplate const* itemTemplate = sObjectMgr->GetItemTemplate(itemId);

        handler->PSendSysMessage(LANG_ITEM_DELETED_FROM_LIST, itemId, itemTemplate->GetDefaultLocaleName());
        return true;
    }

    //set faction of creature
    static bool HandleNpcSetFactionIdCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        uint32 factionId = (uint32)atoi((char*)args);

        if (!sFactionTemplateStore.LookupEntry(factionId))
        {
            handler->PSendSysMessage(LANG_WRONG_FACTION, factionId);
            handler->SetSentErrorMessage(true);
            return false;
        }

        Creature* creature = handler->getSelectedCreature();

        if (!creature)
        {
            handler->SendSysMessage(LANG_SELECT_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        creature->setFaction(factionId);

        // Faction is set in creature_template - not inside creature

        // Update in memory..
        if (CreatureTemplate const* cinfo = creature->GetCreatureTemplate())
            const_cast<CreatureTemplate*>(cinfo)->faction = factionId;

        // ..and DB
        PreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_UPD_CREATURE_FACTION);

        stmt->setUInt16(0, uint16(factionId));
        stmt->setUInt32(1, creature->GetEntry());

        WorldDatabase.Execute(stmt);

        return true;
    }

    //set npcflag of creature
    static bool HandleNpcSetFlagCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        uint64 npcFlags = std::strtoull(args, nullptr, 10);

        Creature* creature = handler->getSelectedCreature();

        if (!creature)
        {
            handler->SendSysMessage(LANG_SELECT_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        creature->SetUInt64Value(UNIT_NPC_FLAGS, npcFlags);

        PreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_UPD_CREATURE_NPCFLAG);

        stmt->setUInt64(0, npcFlags);
        stmt->setUInt32(1, creature->GetEntry());

        WorldDatabase.Execute(stmt);

        handler->SendSysMessage(LANG_VALUE_SAVED_REJOIN);

        return true;
    }

    //set data of creature for testing scripting
    static bool HandleNpcSetDataCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        char* arg1 = strtok((char*)args, " ");
        char* arg2 = strtok((char*)NULL, "");

        if (!arg1 || !arg2)
            return false;

        uint32 data_1 = (uint32)atoi(arg1);
        uint32 data_2 = (uint32)atoi(arg2);

        if (!data_1 || !data_2)
            return false;

        Creature* creature = handler->getSelectedCreature();

        if (!creature)
        {
            handler->SendSysMessage(LANG_SELECT_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        creature->AI()->SetData(data_1, data_2);
        std::string AIorScript = !creature->GetAIName().empty() ? "AI type: " + creature->GetAIName() : (!creature->GetScriptName().empty() ? "Script Name: " + creature->GetScriptName() : "No AI or Script Name Set");
        handler->PSendSysMessage(LANG_NPC_SETDATA, creature->GetGUID().ToString().c_str(), creature->GetName().c_str(), data_1, data_2, AIorScript.c_str());
        return true;
    }

    //npc follow handling
    static bool HandleNpcFollowCommand(ChatHandler* handler, char const* /*args*/)
    {
        Player* player = handler->GetSession()->GetPlayer();
        Creature* creature = handler->getSelectedCreature();

        if (!creature)
        {
            handler->PSendSysMessage(LANG_SELECT_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        // Follow player - Using pet's default dist and angle
        creature->GetMotionMaster()->MoveFollow(player, PET_FOLLOW_DIST, creature->GetFollowAngle());

        handler->PSendSysMessage(LANG_CREATURE_FOLLOW_YOU_NOW, creature->GetName().c_str());
        return true;
    }

    static bool HandleNpcInfoCommand(ChatHandler* handler, char const* /*args*/)
    {
        Creature* target = handler->getSelectedCreature();

        if (!target)
        {
            handler->SendSysMessage(LANG_SELECT_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        CreatureTemplate const* cInfo = target->GetCreatureTemplate();

        uint32 faction = target->getFaction();
        uint64 npcflags = target->GetUInt64Value(UNIT_NPC_FLAGS);
        uint32 mechanicImmuneMask = cInfo->MechanicImmuneMask;
        uint32 displayid = target->GetDisplayId();
        uint32 nativeid = target->GetNativeDisplayId();
        uint32 Entry = target->GetEntry();

        int64 curRespawnDelay = target->GetRespawnTimeEx() - time(NULL);
        if (curRespawnDelay < 0)
            curRespawnDelay = 0;
        std::string curRespawnDelayStr = secsToTimeString(uint64(curRespawnDelay), true);
        std::string defRespawnDelayStr = secsToTimeString(target->GetRespawnDelay(), true);

        handler->PSendSysMessage(LANG_NPCINFO_CHAR, target->GetSpawnId(), target->GetGUID().ToString().c_str(), faction, npcflags, Entry, displayid, nativeid);
        handler->PSendSysMessage(LANG_NPCINFO_LEVEL, target->getLevel());
        handler->PSendSysMessage(LANG_NPCINFO_EQUIPMENT, target->GetCurrentEquipmentId(), target->GetOriginalEquipmentId());
        handler->PSendSysMessage(LANG_NPCINFO_HEALTH, target->GetCreateHealth(), target->GetMaxHealth(), target->GetHealth());
        handler->PSendSysMessage(LANG_NPCINFO_INHABIT_TYPE, cInfo->InhabitType);

        handler->PSendSysMessage(LANG_NPCINFO_UNIT_FIELD_FLAGS, target->GetUInt32Value(UNIT_FIELD_FLAGS));
        for (uint8 i = 0; i < MAX_UNIT_FLAGS; ++i)
            if (target->GetUInt32Value(UNIT_FIELD_FLAGS) & unitFlags[i].Value)
                handler->PSendSysMessage("%s (0x%X)", unitFlags[i].Name, unitFlags[i].Value);

        handler->PSendSysMessage(LANG_NPCINFO_FLAGS, target->GetUInt32Value(UNIT_FIELD_FLAGS_2), target->GetUInt32Value(OBJECT_DYNAMIC_FLAGS), target->getFaction());
        handler->PSendSysMessage(LANG_COMMAND_RAWPAWNTIMES, defRespawnDelayStr.c_str(), curRespawnDelayStr.c_str());
        handler->PSendSysMessage(LANG_NPCINFO_LOOT, cInfo->lootid, cInfo->pickpocketLootId, cInfo->SkinLootId);
        handler->PSendSysMessage(LANG_NPCINFO_DUNGEON_ID, target->GetInstanceId());

        if (CreatureData const* data = sObjectMgr->GetCreatureData(target->GetSpawnId()))
        {
            handler->PSendSysMessage(LANG_NPCINFO_PHASES, data->phaseid, data->phaseGroup);
            if (data->phaseGroup)
            {
                std::set<uint32> _phases = target->GetPhases();

                if (!_phases.empty())
                {
                    handler->PSendSysMessage(LANG_NPCINFO_PHASE_IDS);
                    for (uint32 phaseId : _phases)
                        handler->PSendSysMessage("%u", phaseId);
                }
            }
        }

        handler->PSendSysMessage(LANG_NPCINFO_ARMOR, target->GetArmor());
        handler->PSendSysMessage(LANG_NPCINFO_POSITION, target->GetPositionX(), target->GetPositionY(), target->GetPositionZ());
        handler->PSendSysMessage(LANG_NPCINFO_AIINFO, target->GetAIName().c_str(), target->GetScriptName().c_str());
        handler->PSendSysMessage(LANG_NPCINFO_FLAGS_EXTRA, cInfo->flags_extra);
        for (uint8 i = 0; i < FLAGS_EXTRA_COUNT; ++i)
            if (cInfo->flags_extra & flagsExtra[i].Value)
                handler->PSendSysMessage("%s (0x%X)", flagsExtra[i].Name, flagsExtra[i].Value);

        for (uint8 i = 0; i < NPCFLAG_COUNT; i++)
            if (npcflags & npcFlagTexts[i].Value)
                handler->PSendSysMessage(npcFlagTexts[i].Name, npcFlagTexts[i].Value);

        handler->PSendSysMessage(LANG_NPCINFO_MECHANIC_IMMUNE, mechanicImmuneMask);
        for (uint8 i = 1; i < MAX_MECHANIC; ++i)
            if (mechanicImmuneMask & (1 << (mechanicImmunes[i].Value - 1)))
                handler->PSendSysMessage("%s (0x%X)", mechanicImmunes[i].Name, mechanicImmunes[i].Value);

        return true;
    }

    static bool HandleNpcNearCommand(ChatHandler* handler, char const* args)
    {
        float distance = (!*args) ? 10.0f : float((atof(args)));
        uint32 count = 0;

        Player* player = handler->GetSession()->GetPlayer();

        PreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_SEL_CREATURE_NEAREST);
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
                ObjectGuid::LowType guid = fields[0].GetUInt64();
                uint32 entry = fields[1].GetUInt32();
                float x = fields[2].GetFloat();
                float y = fields[3].GetFloat();
                float z = fields[4].GetFloat();
                uint16 mapId = fields[5].GetUInt16();

                CreatureTemplate const* creatureTemplate = sObjectMgr->GetCreatureTemplate(entry);
                if (!creatureTemplate)
                    continue;

                handler->PSendSysMessage(LANG_CREATURE_LIST_CHAT, guid, guid, creatureTemplate->Name.c_str(), x, y, z, mapId);

                ++count;
            } while (result->NextRow());
        }

        handler->PSendSysMessage(LANG_COMMAND_NEAR_NPC_MESSAGE, distance, count);

        return true;
    }

    //move selected creature
    static bool HandleNpcMoveCommand(ChatHandler* handler, char const* args)
    {
        ObjectGuid::LowType lowguid = UI64LIT(0);

        Creature* creature = handler->getSelectedCreature();

        if (!creature)
        {
            // number or [name] Shift-click form |color|Hcreature:creature_guid|h[name]|h|r
            char* cId = handler->extractKeyFromLink((char*)args, "Hcreature");
            if (!cId)
                return false;

            lowguid = strtoull(cId, nullptr, 10);

            // Attempting creature load from DB data
            CreatureData const* data = sObjectMgr->GetCreatureData(lowguid);
            if (!data)
            {
                handler->PSendSysMessage(LANG_COMMAND_CREATGUIDNOTFOUND, lowguid);
                handler->SetSentErrorMessage(true);
                return false;
            }

            uint32 map_id = data->mapid;

            if (handler->GetSession()->GetPlayer()->GetMapId() != map_id)
            {
                handler->PSendSysMessage(LANG_COMMAND_CREATUREATSAMEMAP, lowguid);
                handler->SetSentErrorMessage(true);
                return false;
            }
        }
        else
        {
            lowguid = creature->GetSpawnId();
        }

        float x = handler->GetSession()->GetPlayer()->GetPositionX();
        float y = handler->GetSession()->GetPlayer()->GetPositionY();
        float z = handler->GetSession()->GetPlayer()->GetPositionZ();
        float o = handler->GetSession()->GetPlayer()->GetOrientation();

        if (creature)
        {
            if (CreatureData const* data = sObjectMgr->GetCreatureData(creature->GetSpawnId()))
            {
                const_cast<CreatureData*>(data)->posX = x;
                const_cast<CreatureData*>(data)->posY = y;
                const_cast<CreatureData*>(data)->posZ = z;
                const_cast<CreatureData*>(data)->orientation = o;
            }
            creature->SetPosition(x, y, z, o);
            creature->GetMotionMaster()->Initialize();
            if (creature->IsAlive())                            // dead creature will reset movement generator at respawn
            {
                creature->setDeathState(JUST_DIED);
                creature->Respawn();
            }
        }

        PreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_UPD_CREATURE_POSITION);

        stmt->setFloat(0, x);
        stmt->setFloat(1, y);
        stmt->setFloat(2, z);
        stmt->setFloat(3, o);
        stmt->setUInt64(4, lowguid);

        WorldDatabase.Execute(stmt);

        handler->PSendSysMessage(LANG_COMMAND_CREATUREMOVED);
        return true;
    }

    //play npc emote
    static bool HandleNpcPlayEmoteCommand(ChatHandler* handler, char const* args)
    {
        uint32 emote = atoi((char*)args);

        Creature* target = handler->getSelectedCreature();
        if (!target)
        {
            handler->SendSysMessage(LANG_SELECT_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        target->SetUInt32Value(UNIT_NPC_EMOTESTATE, emote);

        return true;
    }

    //set model of creature
    static bool HandleNpcSetModelCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        uint32 displayId = (uint32)atoi((char*)args);

        Creature* creature = handler->getSelectedCreature();

        if (!creature || creature->IsPet())
        {
            handler->SendSysMessage(LANG_SELECT_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        creature->SetDisplayId(displayId);
        creature->SetNativeDisplayId(displayId);

        creature->SaveToDB();

        return true;
    }

    /**HandleNpcSetMoveTypeCommand
    * Set the movement type for an NPC.<br/>
    * <br/>
    * Valid movement types are:
    * <ul>
    * <li> stay - NPC wont move </li>
    * <li> random - NPC will move randomly according to the spawndist </li>
    * <li> way - NPC will move with given waypoints set </li>
    * </ul>
    * additional parameter: NODEL - so no waypoints are deleted, if you
    *                       change the movement type
    */
    static bool HandleNpcSetMoveTypeCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        // 3 arguments:
        // GUID (optional - you can also select the creature)
        // stay|random|way (determines the kind of movement)
        // NODEL (optional - tells the system NOT to delete any waypoints)
        //        this is very handy if you want to do waypoints, that are
        //        later switched on/off according to special events (like escort
        //        quests, etc)
        char* guid_str = strtok((char*)args, " ");
        char* type_str = strtok((char*)NULL, " ");
        char* dontdel_str = strtok((char*)NULL, " ");

        bool doNotDelete = false;

        if (!guid_str)
            return false;

        ObjectGuid::LowType lowguid = UI64LIT(0);
        Creature* creature = NULL;

        if (dontdel_str)
        {
            //TC_LOG_ERROR("misc", "DEBUG: All 3 params are set");

            // All 3 params are set
            // GUID
            // type
            // doNotDEL
            if (stricmp(dontdel_str, "NODEL") == 0)
            {
                //TC_LOG_ERROR("misc", "DEBUG: doNotDelete = true;");
                doNotDelete = true;
            }
        }
        else
        {
            // Only 2 params - but maybe NODEL is set
            if (type_str)
            {
                TC_LOG_ERROR("misc", "DEBUG: Only 2 params ");
                if (stricmp(type_str, "NODEL") == 0)
                {
                    //TC_LOG_ERROR("misc", "DEBUG: type_str, NODEL ");
                    doNotDelete = true;
                    type_str = NULL;
                }
            }
        }

        if (!type_str)                                           // case .setmovetype $move_type (with selected creature)
        {
            type_str = guid_str;
            creature = handler->getSelectedCreature();
            if (!creature || creature->IsPet())
                return false;
            lowguid = creature->GetSpawnId();
        }
        else                                                    // case .setmovetype #creature_guid $move_type (with selected creature)
        {
            lowguid = strtoull(guid_str, nullptr, 10);

            /* impossible without entry
            if (lowguid)
            creature = ObjectAccessor::GetCreature(*handler->GetSession()->GetPlayer(), MAKE_GUID(lowguid, HIGHGUID_UNIT));
            */

            // attempt check creature existence by DB data
            if (!creature)
            {
                CreatureData const* data = sObjectMgr->GetCreatureData(lowguid);
                if (!data)
                {
                    handler->PSendSysMessage(LANG_COMMAND_CREATGUIDNOTFOUND, lowguid);
                    handler->SetSentErrorMessage(true);
                    return false;
                }
            }
            else
            {
                lowguid = creature->GetSpawnId();
            }
        }

        // now lowguid is low guid really existed creature
        // and creature point (maybe) to this creature or NULL

        MovementGeneratorType move_type;

        std::string type = type_str;

        if (type == "stay")
            move_type = IDLE_MOTION_TYPE;
        else if (type == "random")
            move_type = RANDOM_MOTION_TYPE;
        else if (type == "way")
            move_type = WAYPOINT_MOTION_TYPE;
        else
            return false;

        // update movement type
        //if (doNotDelete == false)
        //    WaypointMgr.DeletePath(lowguid);

        if (creature)
        {
            // update movement type
            if (doNotDelete == false)
                creature->LoadPath(0);

            creature->SetDefaultMovementType(move_type);
            creature->GetMotionMaster()->Initialize();
            if (creature->IsAlive())                            // dead creature will reset movement generator at respawn
            {
                creature->setDeathState(JUST_DIED);
                creature->Respawn();
            }
            creature->SaveToDB();
        }
        if (doNotDelete == false)
        {
            handler->PSendSysMessage(LANG_MOVE_TYPE_SET, type_str);
        }
        else
        {
            handler->PSendSysMessage(LANG_MOVE_TYPE_SET_NODEL, type_str);
        }

        return true;
    }

    //npc phase handling
    //change phase of creature
    static bool HandleNpcSetPhaseGroup(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        uint32 phaseGroupId = (uint32)atoi((char*)args);

        Creature* creature = handler->getSelectedCreature();
        if (!creature || creature->IsPet())
        {
            handler->SendSysMessage(LANG_SELECT_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        creature->ClearPhases();

        for (uint32 id : sDB2Manager.GetPhasesForGroup(phaseGroupId))
            creature->SetInPhase(id, false, true); // don't send update here for multiple phases, only send it once after adding all phases

        creature->UpdateObjectVisibility();
        creature->SetDBPhase(-int(phaseGroupId));

        creature->SaveToDB();

        return true;
    }

    //npc phase handling
    //change phase of creature
    static bool HandleNpcSetPhaseCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        uint32 phase = (uint32)atoi((char*)args);

        Creature* creature = handler->getSelectedCreature();
        if (!creature || creature->IsPet())
        {
            handler->SendSysMessage(LANG_SELECT_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        creature->ClearPhases();
        creature->SetInPhase(phase, true, true);
        creature->SetDBPhase(phase);

        creature->SaveToDB();

        return true;
    }

    //set spawn dist of creature
    static bool HandleNpcSetSpawnDistCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        float option = (float)(atof((char*)args));
        if (option < 0.0f)
        {
            handler->SendSysMessage(LANG_BAD_VALUE);
            return false;
        }

        MovementGeneratorType mtype = IDLE_MOTION_TYPE;
        if (option >0.0f)
            mtype = RANDOM_MOTION_TYPE;

        Creature* creature = handler->getSelectedCreature();
        ObjectGuid::LowType guidLow = UI64LIT(0);

        if (creature)
            guidLow = creature->GetSpawnId();
        else
            return false;

        creature->SetRespawnRadius(option);
        creature->SetDefaultMovementType(mtype);
        creature->GetMotionMaster()->Initialize();
        if (creature->IsAlive())                                // dead creature will reset movement generator at respawn
        {
            creature->setDeathState(JUST_DIED);
            creature->Respawn();
        }

        PreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_UPD_CREATURE_SPAWN_DISTANCE);

        stmt->setFloat(0, option);
        stmt->setUInt8(1, uint8(mtype));
        stmt->setUInt64(2, guidLow);

        WorldDatabase.Execute(stmt);

        handler->PSendSysMessage(LANG_COMMAND_SPAWNDIST, option);
        return true;
    }

    //spawn time handling
    static bool HandleNpcSetSpawnTimeCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        char* stime = strtok((char*)args, " ");

        if (!stime)
            return false;

        int spawnTime = atoi((char*)stime);

        if (spawnTime < 0)
        {
            handler->SendSysMessage(LANG_BAD_VALUE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        Creature* creature = handler->getSelectedCreature();
        ObjectGuid::LowType guidLow = UI64LIT(0);

        if (creature)
            guidLow = creature->GetSpawnId();
        else
            return false;

        PreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_UPD_CREATURE_SPAWN_TIME_SECS);

        stmt->setUInt32(0, uint32(spawnTime));
        stmt->setUInt64(1, guidLow);

        WorldDatabase.Execute(stmt);

        creature->SetRespawnDelay((uint32)spawnTime);
        handler->PSendSysMessage(LANG_COMMAND_SPAWNTIME, spawnTime);

        return true;
    }

    static bool HandleNpcSayCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        Creature* creature = handler->getSelectedCreature();
        if (!creature)
        {
            handler->SendSysMessage(LANG_SELECT_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        creature->Say(args, LANG_UNIVERSAL);

        // make some emotes
        char lastchar = args[strlen(args) - 1];
        switch (lastchar)
        {
        case '?':   creature->HandleEmoteCommand(EMOTE_ONESHOT_QUESTION);      break;
        case '!':   creature->HandleEmoteCommand(EMOTE_ONESHOT_EXCLAMATION);   break;
        default:    creature->HandleEmoteCommand(EMOTE_ONESHOT_TALK);          break;
        }

        return true;
    }

    //show text emote by creature in chat
    static bool HandleNpcTextEmoteCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        Creature* creature = handler->getSelectedCreature();

        if (!creature)
        {
            handler->SendSysMessage(LANG_SELECT_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        creature->TextEmote(args);

        return true;
    }

    //npc unfollow handling
    static bool HandleNpcUnFollowCommand(ChatHandler* handler, char const* /*args*/)
    {
        Player* player = handler->GetSession()->GetPlayer();
        Creature* creature = handler->getSelectedCreature();

        if (!creature)
        {
            handler->PSendSysMessage(LANG_SELECT_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (/*creature->GetMotionMaster()->empty() ||*/
            creature->GetMotionMaster()->GetCurrentMovementGeneratorType() != FOLLOW_MOTION_TYPE)
        {
            handler->PSendSysMessage(LANG_CREATURE_NOT_FOLLOW_YOU, creature->GetName().c_str());
            handler->SetSentErrorMessage(true);
            return false;
        }

        FollowMovementGenerator<Creature> const* mgen = static_cast<FollowMovementGenerator<Creature> const*>((creature->GetMotionMaster()->top()));

        if (mgen->GetTarget() != player)
        {
            handler->PSendSysMessage(LANG_CREATURE_NOT_FOLLOW_YOU, creature->GetName().c_str());
            handler->SetSentErrorMessage(true);
            return false;
        }

        // reset movement
        creature->GetMotionMaster()->MovementExpired(true);

        handler->PSendSysMessage(LANG_CREATURE_NOT_FOLLOW_YOU_NOW, creature->GetName().c_str());
        return true;
    }

    // make npc whisper to player
    static bool HandleNpcWhisperCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
        {
            handler->SendSysMessage(LANG_CMD_SYNTAX);
            handler->SetSentErrorMessage(true);
            return false;
        }

        char* receiver_str = strtok((char*)args, " ");
        char* text = strtok(NULL, "");

        if (!receiver_str || !text)
        {
            handler->SendSysMessage(LANG_CMD_SYNTAX);
            handler->SetSentErrorMessage(true);
            return false;
        }

        Creature* creature = handler->getSelectedCreature();
        if (!creature)
        {
            handler->SendSysMessage(LANG_SELECT_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        ObjectGuid receiver_guid = ObjectGuid::Create<HighGuid::Player>(strtoull(receiver_str, nullptr, 10));

        // check online security
        Player* receiver = ObjectAccessor::FindPlayer(receiver_guid);
        if (handler->HasLowerSecurity(receiver, ObjectGuid::Empty))
            return false;

        creature->Whisper(text, LANG_UNIVERSAL, receiver);
        return true;
    }

    static bool HandleNpcYellCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
        {
            handler->SendSysMessage(LANG_CMD_SYNTAX);
            handler->SetSentErrorMessage(true);
            return false;
        }

        Creature* creature = handler->getSelectedCreature();
        if (!creature)
        {
            handler->SendSysMessage(LANG_SELECT_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        creature->Yell(args, LANG_UNIVERSAL);

        // make an emote
        creature->HandleEmoteCommand(EMOTE_ONESHOT_SHOUT);

        return true;
    }

    // add creature, temp only
    static bool HandleNpcAddTempSpawnCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        char* charID = handler->extractKeyFromLink((char*)args, "Hcreature_entry");
        if (!charID)
            return false;

        Player* chr = handler->GetSession()->GetPlayer();

        uint32 id = atoi(charID);
        if (!id)
            return false;

        if (!sObjectMgr->GetCreatureTemplate(id))
            return false;

        chr->SummonCreature(id, *chr, TEMPSUMMON_CORPSE_DESPAWN, 120);

        return true;
    }

    //npc tame handling
    static bool HandleNpcTameCommand(ChatHandler* handler, char const* /*args*/)
    {
        Creature* creatureTarget = handler->getSelectedCreature();
        if (!creatureTarget || creatureTarget->IsPet())
        {
            handler->PSendSysMessage(LANG_SELECT_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        Player* player = handler->GetSession()->GetPlayer();

        if (!player->GetPetGUID().IsEmpty())
        {
            handler->SendSysMessage(LANG_YOU_ALREADY_HAVE_PET);
            handler->SetSentErrorMessage(true);
            return false;
        }

        CreatureTemplate const* cInfo = creatureTarget->GetCreatureTemplate();

        if (!cInfo->IsTameable(player->CanTameExoticPets()))
        {
            handler->PSendSysMessage(LANG_CREATURE_NON_TAMEABLE, cInfo->Entry);
            handler->SetSentErrorMessage(true);
            return false;
        }

        // Everything looks OK, create new pet
        Pet* pet = player->CreateTamedPetFrom(creatureTarget);
        if (!pet)
        {
            handler->PSendSysMessage(LANG_CREATURE_NON_TAMEABLE, cInfo->Entry);
            handler->SetSentErrorMessage(true);
            return false;
        }

        // place pet before player
        float x, y, z;
        player->GetClosePoint(x, y, z, creatureTarget->GetObjectSize(), CONTACT_DISTANCE);
        pet->Relocate(x, y, z, float(M_PI) - player->GetOrientation());

        // set pet to defensive mode by default (some classes can't control controlled pets in fact).
        pet->SetReactState(REACT_DEFENSIVE);

        // calculate proper level
        uint8 level = (creatureTarget->getLevel() < (player->getLevel() - 5)) ? (player->getLevel() - 5) : creatureTarget->getLevel();

        // prepare visual effect for levelup
        pet->SetUInt32Value(UNIT_FIELD_LEVEL, level - 1);

        // add to world
        pet->GetMap()->AddToMap(pet->ToCreature());

        // visual effect for levelup
        pet->SetUInt32Value(UNIT_FIELD_LEVEL, level);

        // caster have pet now
        player->SetMinion(pet, true);

        pet->SavePetToDB(PET_SAVE_AS_CURRENT);
        player->PetSpellInitialize();

        return true;
    }

    static bool HandleNpcAddFormationCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        ObjectGuid::LowType leaderGUID = strtoull(args, nullptr, 10);
        Creature* creature = handler->getSelectedCreature();

        if (!creature || !creature->GetSpawnId())
        {
            handler->SendSysMessage(LANG_SELECT_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        ObjectGuid::LowType lowguid = creature->GetSpawnId();
        if (creature->GetFormation())
        {
            handler->PSendSysMessage("Selected creature is already member of group " UI64FMTD, creature->GetFormation()->GetId());
            return false;
        }

        if (!lowguid)
            return false;

        Player* chr = handler->GetSession()->GetPlayer();
        FormationInfo* group_member;

        group_member = new FormationInfo;
        group_member->follow_angle = (creature->GetAngle(chr) - chr->GetOrientation()) * 180 / float(M_PI);
        group_member->follow_dist = std::sqrt(std::pow(chr->GetPositionX() - creature->GetPositionX(), 2.f) + std::pow(chr->GetPositionY() - creature->GetPositionY(), 2.f));
        group_member->leaderGUID = leaderGUID;
        group_member->groupAI = 0;

        sFormationMgr->CreatureGroupMap[lowguid] = group_member;
        creature->SearchFormation();

        PreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_INS_CREATURE_FORMATION);

        stmt->setUInt64(0, leaderGUID);
        stmt->setUInt64(1, lowguid);
        stmt->setFloat(2, group_member->follow_dist);
        stmt->setFloat(3, group_member->follow_angle);
        stmt->setUInt32(4, uint32(group_member->groupAI));

        WorldDatabase.Execute(stmt);

        handler->PSendSysMessage("Creature " UI64FMTD " added to formation with leader " UI64FMTD, lowguid, leaderGUID);

        return true;
    }

    static bool HandleNpcSetLinkCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        ObjectGuid::LowType linkguid = strtoull(args, nullptr, 10);

        Creature* creature = handler->getSelectedCreature();

        if (!creature)
        {
            handler->SendSysMessage(LANG_SELECT_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (!creature->GetSpawnId())
        {
            handler->PSendSysMessage("Selected %s isn't in creature table", creature->GetGUID().ToString().c_str());
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (!sObjectMgr->SetCreatureLinkedRespawn(creature->GetSpawnId(), linkguid))
        {
            handler->PSendSysMessage("Selected creature can't link with guid '" UI64FMTD "'", linkguid);
            handler->SetSentErrorMessage(true);
            return false;
        }

        handler->PSendSysMessage("LinkGUID '" UI64FMTD "' added to creature with DBTableGUID: '" UI64FMTD "'", linkguid, creature->GetSpawnId());
        return true;
    }

    /// @todo NpcCommands that need to be fixed :
    static bool HandleNpcAddWeaponCommand(ChatHandler* /*handler*/, char const* /*args*/)
    {
        /*if (!*args)
        return false;

        uint64 guid = handler->GetSession()->GetPlayer()->GetSelection();
        if (guid == 0)
        {
        handler->SendSysMessage(LANG_NO_SELECTION);
        return true;
        }

        Creature* creature = ObjectAccessor::GetCreature(*handler->GetSession()->GetPlayer(), guid);

        if (!creature)
        {
        handler->SendSysMessage(LANG_SELECT_CREATURE);
        return true;
        }

        char* pSlotID = strtok((char*)args, " ");
        if (!pSlotID)
        return false;

        char* pItemID = strtok(NULL, " ");
        if (!pItemID)
        return false;

        uint32 ItemID = atoi(pItemID);
        uint32 SlotID = atoi(pSlotID);

        ItemTemplate* tmpItem = sObjectMgr->GetItemTemplate(ItemID);

        bool added = false;
        if (tmpItem)
        {
        switch (SlotID)
        {
        case 1:
        creature->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_DISPLAY, ItemID);
        added = true;
        break;
        case 2:
        creature->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_DISPLAY_01, ItemID);
        added = true;
        break;
        case 3:
        creature->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_DISPLAY_02, ItemID);
        added = true;
        break;
        default:
        handler->PSendSysMessage(LANG_ITEM_SLOT_NOT_EXIST, SlotID);
        added = false;
        break;
        }

        if (added)
        handler->PSendSysMessage(LANG_ITEM_ADDED_TO_SLOT, ItemID, tmpItem->Name1, SlotID);
        }
        else
        {
        handler->PSendSysMessage(LANG_ITEM_NOT_FOUND, ItemID);
        return true;
        }
        */
        return true;
    }
};

void AddSC_fnpc_commandscript()
{
    new fnpc_commandscript();
}

#ifdef FREEDOM_MOP_548_CODE

template<typename E, typename T = char const*>
struct EnumName
{
    E Value;
    T Name;
};

#define CREATE_NAMED_ENUM(VALUE) { VALUE, STRINGIZE(VALUE) }

#define NPCFLAG_COUNT   27

EnumName<NPCFlags, int32> const npcFlagTexts[NPCFLAG_COUNT] =
{
    { UNIT_NPC_FLAG_AUCTIONEER, LANG_NPCINFO_AUCTIONEER },
    { UNIT_NPC_FLAG_BANKER, LANG_NPCINFO_BANKER },
    { UNIT_NPC_FLAG_BATTLEMASTER, LANG_NPCINFO_BATTLEMASTER },
    { UNIT_NPC_FLAG_FLIGHTMASTER, LANG_NPCINFO_FLIGHTMASTER },
    { UNIT_NPC_FLAG_GOSSIP, LANG_NPCINFO_GOSSIP },
    { UNIT_NPC_FLAG_GUILD_BANKER, LANG_NPCINFO_GUILD_BANKER },
    { UNIT_NPC_FLAG_INNKEEPER, LANG_NPCINFO_INNKEEPER },
    { UNIT_NPC_FLAG_PETITIONER, LANG_NPCINFO_PETITIONER },
    { UNIT_NPC_FLAG_PLAYER_VEHICLE, LANG_NPCINFO_PLAYER_VEHICLE },
    { UNIT_NPC_FLAG_QUESTGIVER, LANG_NPCINFO_QUESTGIVER },
    { UNIT_NPC_FLAG_REFORGER, LANG_NPCINFO_REFORGER },
    { UNIT_NPC_FLAG_REPAIR, LANG_NPCINFO_REPAIR },
    { UNIT_NPC_FLAG_SPELLCLICK, LANG_NPCINFO_SPELLCLICK },
    { UNIT_NPC_FLAG_SPIRITGUIDE, LANG_NPCINFO_SPIRITGUIDE },
    { UNIT_NPC_FLAG_SPIRITHEALER, LANG_NPCINFO_SPIRITHEALER },
    { UNIT_NPC_FLAG_STABLEMASTER, LANG_NPCINFO_STABLEMASTER },
    { UNIT_NPC_FLAG_TABARDDESIGNER, LANG_NPCINFO_TABARDDESIGNER },
    { UNIT_NPC_FLAG_TRAINER, LANG_NPCINFO_TRAINER },
    { UNIT_NPC_FLAG_TRAINER_CLASS, LANG_NPCINFO_TRAINER_CLASS },
    { UNIT_NPC_FLAG_TRAINER_PROFESSION, LANG_NPCINFO_TRAINER_PROFESSION },
    { UNIT_NPC_FLAG_TRANSMOGRIFIER, LANG_NPCINFO_TRANSMOGRIFIER },
    { UNIT_NPC_FLAG_VAULTKEEPER, LANG_NPCINFO_VAULTKEEPER },
    { UNIT_NPC_FLAG_VENDOR, LANG_NPCINFO_VENDOR },
    { UNIT_NPC_FLAG_VENDOR_AMMO, LANG_NPCINFO_VENDOR_AMMO },
    { UNIT_NPC_FLAG_VENDOR_FOOD, LANG_NPCINFO_VENDOR_FOOD },
    { UNIT_NPC_FLAG_VENDOR_POISON, LANG_NPCINFO_VENDOR_POISON },
    { UNIT_NPC_FLAG_VENDOR_REAGENT, LANG_NPCINFO_VENDOR_REAGENT }
};

EnumName<Mechanics> const mechanicImmunes[MAX_MECHANIC] =
{
    CREATE_NAMED_ENUM(MECHANIC_NONE),
    CREATE_NAMED_ENUM(MECHANIC_CHARM),
    CREATE_NAMED_ENUM(MECHANIC_DISORIENTED),
    CREATE_NAMED_ENUM(MECHANIC_DISARM),
    CREATE_NAMED_ENUM(MECHANIC_DISTRACT),
    CREATE_NAMED_ENUM(MECHANIC_FEAR),
    CREATE_NAMED_ENUM(MECHANIC_GRIP),
    CREATE_NAMED_ENUM(MECHANIC_ROOT),
    CREATE_NAMED_ENUM(MECHANIC_SLOW_ATTACK),
    CREATE_NAMED_ENUM(MECHANIC_SILENCE),
    CREATE_NAMED_ENUM(MECHANIC_SLEEP),
    CREATE_NAMED_ENUM(MECHANIC_SNARE),
    CREATE_NAMED_ENUM(MECHANIC_STUN),
    CREATE_NAMED_ENUM(MECHANIC_FREEZE),
    CREATE_NAMED_ENUM(MECHANIC_KNOCKOUT),
    CREATE_NAMED_ENUM(MECHANIC_BLEED),
    CREATE_NAMED_ENUM(MECHANIC_BANDAGE),
    CREATE_NAMED_ENUM(MECHANIC_POLYMORPH),
    CREATE_NAMED_ENUM(MECHANIC_BANISH),
    CREATE_NAMED_ENUM(MECHANIC_SHIELD),
    CREATE_NAMED_ENUM(MECHANIC_SHACKLE),
    CREATE_NAMED_ENUM(MECHANIC_MOUNT),
    CREATE_NAMED_ENUM(MECHANIC_INFECTED),
    CREATE_NAMED_ENUM(MECHANIC_TURN),
    CREATE_NAMED_ENUM(MECHANIC_HORROR),
    CREATE_NAMED_ENUM(MECHANIC_INVULNERABILITY),
    CREATE_NAMED_ENUM(MECHANIC_INTERRUPT),
    CREATE_NAMED_ENUM(MECHANIC_DAZE),
    CREATE_NAMED_ENUM(MECHANIC_DISCOVERY),
    CREATE_NAMED_ENUM(MECHANIC_IMMUNE_SHIELD),
    CREATE_NAMED_ENUM(MECHANIC_SAPPED),
    CREATE_NAMED_ENUM(MECHANIC_ENRAGED),
    CREATE_NAMED_ENUM(MECHANIC_WOUNDED)
};

EnumName<UnitFlags> const unitFlags[MAX_UNIT_FLAGS] =
{
    CREATE_NAMED_ENUM(UNIT_FLAG_SERVER_CONTROLLED),
    CREATE_NAMED_ENUM(UNIT_FLAG_NON_ATTACKABLE),
    CREATE_NAMED_ENUM(UNIT_FLAG_DISABLE_MOVE),
    CREATE_NAMED_ENUM(UNIT_FLAG_PVP_ATTACKABLE),
    CREATE_NAMED_ENUM(UNIT_FLAG_RENAME),
    CREATE_NAMED_ENUM(UNIT_FLAG_PREPARATION),
    CREATE_NAMED_ENUM(UNIT_FLAG_UNK_6),
    CREATE_NAMED_ENUM(UNIT_FLAG_NOT_ATTACKABLE_1),
    CREATE_NAMED_ENUM(UNIT_FLAG_IMMUNE_TO_PC),
    CREATE_NAMED_ENUM(UNIT_FLAG_IMMUNE_TO_NPC),
    CREATE_NAMED_ENUM(UNIT_FLAG_LOOTING),
    CREATE_NAMED_ENUM(UNIT_FLAG_PET_IN_COMBAT),
    CREATE_NAMED_ENUM(UNIT_FLAG_PVP),
    CREATE_NAMED_ENUM(UNIT_FLAG_SILENCED),
    CREATE_NAMED_ENUM(UNIT_FLAG_UNK_14),
    CREATE_NAMED_ENUM(UNIT_FLAG_UNK_15),
    CREATE_NAMED_ENUM(UNIT_FLAG_UNK_16),
    CREATE_NAMED_ENUM(UNIT_FLAG_PACIFIED),
    CREATE_NAMED_ENUM(UNIT_FLAG_STUNNED),
    CREATE_NAMED_ENUM(UNIT_FLAG_IN_COMBAT),
    CREATE_NAMED_ENUM(UNIT_FLAG_TAXI_FLIGHT),
    CREATE_NAMED_ENUM(UNIT_FLAG_DISARMED),
    CREATE_NAMED_ENUM(UNIT_FLAG_CONFUSED),
    CREATE_NAMED_ENUM(UNIT_FLAG_FLEEING),
    CREATE_NAMED_ENUM(UNIT_FLAG_PLAYER_CONTROLLED),
    CREATE_NAMED_ENUM(UNIT_FLAG_NOT_SELECTABLE),
    CREATE_NAMED_ENUM(UNIT_FLAG_SKINNABLE),
    CREATE_NAMED_ENUM(UNIT_FLAG_MOUNT),
    CREATE_NAMED_ENUM(UNIT_FLAG_UNK_28),
    CREATE_NAMED_ENUM(UNIT_FLAG_UNK_29),
    CREATE_NAMED_ENUM(UNIT_FLAG_SHEATHE),
    CREATE_NAMED_ENUM(UNIT_FLAG_UNK_31)
};

enum AuraSpells
{
    SPELL_PERMANENT_FEIGN_DEATH = 114371,
    SPELL_PERMANENT_SLEEP_VISUAL = 107674,
    SPELL_PERMANENT_HOVER = 138092
};

class fnpc_commandscript : public CommandScript
{
public:
    fnpc_commandscript() : CommandScript("fnpc_commandscript") { }

    ChatCommand* GetCommands() const OVERRIDE
    {
        static ChatCommand npcAddCommandTable[] =
        {
            { "item",       rbac::RBAC_PERM_COMMAND_NPC_ADD_ITEM,           false, &HandleNpcAddVendorItemCommand,                  "", NULL },
            //{ "temp",     rbac::RBAC_PERM_COMMAND_NPC_ADD_TEMP,           false, &HandleNpcAddTempSpawnCommand,                   "", NULL },
            //{ "weapon",   rbac::RBAC_PERM_COMMAND_NPC_ADD_WEAPON,         false, &HandleNpcAddWeaponCommand,                      "", NULL },
            { "",           rbac::RBAC_PERM_COMMAND_NPC_ADD,                false, &HandleNpcAddCommand,                            "", NULL },
            { NULL, 0, false, NULL, "", NULL }
        };

        static ChatCommand npcDeleteCommandTable[] =
        {
            { "item",       rbac::RBAC_PERM_COMMAND_NPC_DELETE_ITEM,        false, &HandleNpcDeleteVendorItemCommand,               "", NULL },
            { "",           rbac::RBAC_PERM_COMMAND_NPC_DELETE,             false, &HandleNpcDeleteCommand,                         "", NULL },
            { NULL, 0, false, NULL, "", NULL }
        };

        static ChatCommand npcFollowCommandTable[] =
        {
            { "stop",       rbac::RBAC_PERM_COMMAND_NPC_FOLLOW_STOP,        false, &HandleNpcUnFollowCommand,                       "", NULL },
            { "",           rbac::RBAC_PERM_COMMAND_NPC_FOLLOW,             false, &HandleNpcFollowCommand,                         "", NULL },
            { NULL, 0, false, NULL, "", NULL }
        };

        static ChatCommand npcSetStateCommandTable[] =
        {
            { "death",      rbac::RBAC_PERM_COMMAND_NPC_SET_STATE,          false, &HandleNpcSetStateDeathCommand,                  "", NULL },
            { "sleep",      rbac::RBAC_PERM_COMMAND_NPC_SET_STATE,          false, &HandleNpcSetStateSleepCommand,                  "", NULL },
            { "hover",      rbac::RBAC_PERM_COMMAND_NPC_SET_STATE,          false, &HandleNpcSetStateHoverCommand,                  "", NULL },
            { NULL, 0, false, NULL, "", NULL }
        };

        static ChatCommand npcSetCommandTable[] =
        {
            { "entry",      rbac::RBAC_PERM_COMMAND_NPC_SET_ENTRY,          false, &HandleNpcSetEntryCommand,                       "", NULL },
            { "factionid",  rbac::RBAC_PERM_COMMAND_NPC_SET_FACTIONID,      false, &HandleNpcSetFactionIdCommand,                   "", NULL },
            { "flag",       rbac::RBAC_PERM_COMMAND_NPC_SET_FLAG,           false, &HandleNpcSetFlagCommand,                        "", NULL },
            { "level",      rbac::RBAC_PERM_COMMAND_NPC_SET_LEVEL,          false, &HandleNpcSetLevelCommand,                       "", NULL },
            { "model",      rbac::RBAC_PERM_COMMAND_NPC_SET_MODEL,          false, &HandleNpcSetModelCommand,                       "", NULL },
            { "phase",      rbac::RBAC_PERM_COMMAND_NPC_SET_PHASE,          false, &HandleNpcSetPhaseCommand,                       "", NULL },
            { "spawndist",  rbac::RBAC_PERM_COMMAND_NPC_SET_SPAWNDIST,      false, &HandleNpcSetSpawnDistCommand,                   "", NULL },
            { "spawntime",  rbac::RBAC_PERM_COMMAND_NPC_SET_SPAWNTIME,      false, &HandleNpcSetSpawnTimeCommand,                   "", NULL },
            { "data",       rbac::RBAC_PERM_COMMAND_NPC_SET_DATA,           false, &HandleNpcSetDataCommand,                        "", NULL },
            { "scale",      rbac::RBAC_PERM_COMMAND_NPC_SCALE,              false, &HandleNpcSetScaleCommand,                       "", NULL },
            { "emote",      rbac::RBAC_PERM_COMMAND_NPC_SET_EMOTE,          false, &HandleNpcSetEmoteCommand,                       "", NULL },
            { "state",      rbac::RBAC_PERM_COMMAND_NPC_SET_STATE,          false, NULL,                                            "", npcSetStateCommandTable },
            //{ "name",     rbac::RBAC_PERM_COMMAND_NPC_SET_NAME,           false, &HandleNpcSetNameCommand,                        "", NULL },
            //{ "subname",  rbac::RBAC_PERM_COMMAND_NPC_SET_SUBNAME,        false, &HandleNpcSetSubNameCommand,                     "", NULL },
            { NULL, 0, false, NULL, "", NULL }
        };

        static ChatCommand npcCommandTable[] =
        {
            { "info",       rbac::RBAC_PERM_COMMAND_NPC_INFO,               false, &HandleNpcInfoCommand,                           "", NULL },
            { "near",       rbac::RBAC_PERM_COMMAND_NPC_NEAR,               false, &HandleNpcNearCommand,                           "", NULL },
            { "move",       rbac::RBAC_PERM_COMMAND_NPC_MOVE,               false, &HandleNpcMoveCommand,                           "", NULL },
            { "playemote",  rbac::RBAC_PERM_COMMAND_NPC_PLAYEMOTE,          false, &HandleNpcPlayEmoteCommand,                      "", NULL },
            { "say",        rbac::RBAC_PERM_COMMAND_NPC_SAY,                false, &HandleNpcSayCommand,                            "", NULL },
            { "textemote",  rbac::RBAC_PERM_COMMAND_NPC_TEXTEMOTE,          false, &HandleNpcTextEmoteCommand,                      "", NULL },
            { "whisper",    rbac::RBAC_PERM_COMMAND_NPC_WHISPER,            false, &HandleNpcWhisperCommand,                        "", NULL },
            { "yell",       rbac::RBAC_PERM_COMMAND_NPC_YELL,               false, &HandleNpcYellCommand,                           "", NULL },
            { "tame",       rbac::RBAC_PERM_COMMAND_NPC_TAME,               false, &HandleNpcTameCommand,                           "", NULL },
            { "disable",    rbac::RBAC_PERM_COMMAND_NPC_DISABLE,            false, &HandleCreatureDisableCommand,                   "", NULL },
            { "enable",     rbac::RBAC_PERM_COMMAND_NPC_ENABLE,             false, &HandleCreatureEnableCommand,                    "", NULL },
            { "select",     rbac::RBAC_PERM_COMMAND_NPC_SELECT,             false, &HandleNpcSelectCommand,                         "", NULL },
            { "unselect",   rbac::RBAC_PERM_COMMAND_NPC_SELECT,             false, &HandleNpcUnSelectCommand,                       "", NULL },
            { "scale",      rbac::RBAC_PERM_COMMAND_NPC_SCALE,              false, &HandleNpcSetScaleCommand,                       "", NULL },
            { "add",        rbac::RBAC_PERM_COMMAND_NPC_ADD,                false, NULL,                                            "", npcAddCommandTable },
            { "delete",     rbac::RBAC_PERM_COMMAND_NPC_DELETE,             false, NULL,                                            "", npcDeleteCommandTable },
            { "follow",     rbac::RBAC_PERM_COMMAND_NPC_FOLLOW,             false, NULL,                                            "", npcFollowCommandTable },
            { "set",        rbac::RBAC_PERM_COMMAND_NPC_SET,                false, NULL,                                            "", npcSetCommandTable },
            { NULL, 0, false, NULL, "", NULL }
        };

        static ChatCommand commandTable[] =
        {
            { "npc",        rbac::RBAC_PERM_COMMAND_NPC,                    false, NULL,                                            "", npcCommandTable },
            { NULL, 0, false, NULL, "", NULL }
        };

        return commandTable;
    }

    #pragma region CUSTOM_COMMANDS

    static bool HandleNpcSetStateDeathCommand(ChatHandler* handler, char const* args)
    {
        Creature* creature = NULL;
        char* params[2];
        params[0] = strtok((char*)args, " ");
        params[1] = strtok(NULL, " ");

        if (!params[0])
        {
            handler->PSendSysMessage("Not enough arguments. Syntax: .npc set state death on/off [$guid]");
            return true;
        }

        if (strcmp(params[0], "on") != 0 && strcmp(params[0], "off") != 0)
        {
            handler->PSendSysMessage("First argument must be 'on' OR 'off' (without quotes).");
            return true;
        }

        bool toggle = strcmp(params[0], "on") == 0;

        if (params[1])
        {
            // number or [name] Shift-click form |color|Hcreature:creature_guid|h[name]|h|r
            char* cId = handler->extractKeyFromLink(params[1], "Hcreature");
            if (!cId)
                return false;

            uint32 lowguid = atoi(cId);
            if (!lowguid)
                return false;

            if (CreatureData const* cr_data = sObjectMgr->GetCreatureData(lowguid))
            {
                creature = sObjectAccessor->GetObjectInWorld(MAKE_NEW_GUID(lowguid, cr_data->id, HIGHGUID_UNIT), (Creature*)NULL);
                if (!creature)
                    creature = sObjectAccessor->GetObjectInWorld(MAKE_NEW_GUID(lowguid, cr_data->id, HIGHGUID_PET), (Creature*)NULL);
                if (!creature)
                    creature = sObjectAccessor->GetObjectInWorld(MAKE_NEW_GUID(lowguid, cr_data->id, HIGHGUID_VEHICLE), (Creature*)NULL);
            }
        }
        else
        {
            creature = handler->getSelectedCreature();
        }

        if (!creature)
        {
            handler->SendSysMessage(LANG_SELECT_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        CreatureAddon& creature_addon = sObjectMgr->GetCreatureAddonContainer()[creature->GetGUIDLow()];

        // remove aura
        creature_addon.auras.erase(std::remove(creature_addon.auras.begin(), creature_addon.auras.end(), SPELL_PERMANENT_FEIGN_DEATH), creature_addon.auras.end());
        creature->SetUInt32Value(UNIT_FIELD_NPC_EMOTESTATE, 0x0);
        creature->RemoveAura(SPELL_PERMANENT_FEIGN_DEATH);

        // apply aura if executor turned it on
        if (toggle)
        {
            // make sure that previous auras are removed, since death state blocks all of them
            creature_addon.auras.erase(creature_addon.auras.begin(), creature_addon.auras.end());
            creature->RemoveAllAuras();

            creature_addon.auras.push_back(SPELL_PERMANENT_FEIGN_DEATH);
            creature->AddAura(SPELL_PERMANENT_FEIGN_DEATH, creature);
        }

        creature->SetCreatureAddonDB(creature_addon);

        handler->PSendSysMessage("Creature's death animation state successfully toggled (previously active auras are removed).");
        return true;
    }

    static bool HandleNpcSetStateSleepCommand(ChatHandler* handler, char const* args)
    {
        Creature* creature = NULL;
        char* params[2];
        params[0] = strtok((char*)args, " ");
        params[1] = strtok(NULL, " ");

        if (!params[0])
        {
            handler->PSendSysMessage("Not enough arguments. Syntax: .npc set state sleep on/off [$guid]");
            return true;
        }

        if (strcmp(params[0], "on") != 0 && strcmp(params[0], "off") != 0)
        {
            handler->PSendSysMessage("First argument must be 'on' OR 'off' (without quotes).");
            return true;
        }

        bool toggle = strcmp(params[0], "on") == 0;

        if (params[1])
        {
            // number or [name] Shift-click form |color|Hcreature:creature_guid|h[name]|h|r
            char* cId = handler->extractKeyFromLink(params[1], "Hcreature");
            if (!cId)
                return false;

            uint32 lowguid = atoi(cId);
            if (!lowguid)
                return false;

            if (CreatureData const* cr_data = sObjectMgr->GetCreatureData(lowguid))
            {
                creature = sObjectAccessor->GetObjectInWorld(MAKE_NEW_GUID(lowguid, cr_data->id, HIGHGUID_UNIT), (Creature*)NULL);
                if (!creature)
                    creature = sObjectAccessor->GetObjectInWorld(MAKE_NEW_GUID(lowguid, cr_data->id, HIGHGUID_PET), (Creature*)NULL);
                if (!creature)
                    creature = sObjectAccessor->GetObjectInWorld(MAKE_NEW_GUID(lowguid, cr_data->id, HIGHGUID_VEHICLE), (Creature*)NULL);
            }
        }
        else
        {
            creature = handler->getSelectedCreature();
        }

        if (!creature)
        {
            handler->SendSysMessage(LANG_SELECT_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        CreatureAddon& creature_addon = sObjectMgr->GetCreatureAddonContainer()[creature->GetGUIDLow()];

        // remove aura
        creature_addon.auras.erase(std::remove(creature_addon.auras.begin(), creature_addon.auras.end(), SPELL_PERMANENT_SLEEP_VISUAL), creature_addon.auras.end());
        creature->RemoveAura(SPELL_PERMANENT_SLEEP_VISUAL);

        // apply/remove permanent sleep visual
        if (toggle)
        {
            // make sure that deathstate is removed, else sleep visual is blocked
            creature_addon.auras.erase(std::remove(creature_addon.auras.begin(), creature_addon.auras.end(), SPELL_PERMANENT_FEIGN_DEATH), creature_addon.auras.end());
            creature->RemoveAura(SPELL_PERMANENT_FEIGN_DEATH);

            creature_addon.auras.push_back(SPELL_PERMANENT_SLEEP_VISUAL);
            creature->AddAura(SPELL_PERMANENT_SLEEP_VISUAL, creature);
        }

        creature->SetCreatureAddonDB(creature_addon);

        handler->PSendSysMessage("Creature's sleep animation state successfully toggled (previously active death state will be removed).");
        return true;
    }

    static bool HandleNpcSetStateHoverCommand(ChatHandler* handler, char const* args)
    {
        Creature* creature = NULL;
        char* params[2];
        params[0] = strtok((char*)args, " ");
        params[1] = strtok(NULL, " ");

        if (!params[0])
        {
            handler->PSendSysMessage("Not enough arguments. Syntax: .npc set state hover on/off [$guid]");
            return true;
        }

        if (strcmp(params[0], "on") != 0 && strcmp(params[0], "off") != 0)
        {
            handler->PSendSysMessage("First argument must be 'on' OR 'off' (without quotes).");
            return true;
        }

        bool toggle = strcmp(params[0], "on") == 0;

        if (params[1])
        {
            // number or [name] Shift-click form |color|Hcreature:creature_guid|h[name]|h|r
            char* cId = handler->extractKeyFromLink(params[1], "Hcreature");
            if (!cId)
                return false;

            uint32 lowguid = atoi(cId);
            if (!lowguid)
                return false;

            if (CreatureData const* cr_data = sObjectMgr->GetCreatureData(lowguid))
            {
                creature = sObjectAccessor->GetObjectInWorld(MAKE_NEW_GUID(lowguid, cr_data->id, HIGHGUID_UNIT), (Creature*)NULL);
                if (!creature)
                    creature = sObjectAccessor->GetObjectInWorld(MAKE_NEW_GUID(lowguid, cr_data->id, HIGHGUID_PET), (Creature*)NULL);
                if (!creature)
                    creature = sObjectAccessor->GetObjectInWorld(MAKE_NEW_GUID(lowguid, cr_data->id, HIGHGUID_VEHICLE), (Creature*)NULL);
            }
        }
        else
        {
            creature = handler->getSelectedCreature();
        }

        if (!creature)
        {
            handler->SendSysMessage(LANG_SELECT_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        CreatureAddon& creature_addon = sObjectMgr->GetCreatureAddonContainer()[creature->GetGUIDLow()];

        // remove aura
        creature_addon.auras.erase(std::remove(creature_addon.auras.begin(), creature_addon.auras.end(), SPELL_PERMANENT_HOVER), creature_addon.auras.end());
        creature->RemoveAura(SPELL_PERMANENT_HOVER);

        // apply/remove permanent sleep visual
        if (toggle)
        {
            // make sure that deathstate is removed, else this visual is blocked
            creature_addon.auras.erase(std::remove(creature_addon.auras.begin(), creature_addon.auras.end(), SPELL_PERMANENT_FEIGN_DEATH), creature_addon.auras.end());
            creature->RemoveAura(SPELL_PERMANENT_FEIGN_DEATH);

            creature_addon.auras.push_back(SPELL_PERMANENT_HOVER);
            creature->AddAura(SPELL_PERMANENT_HOVER, creature);
        }

        creature->SetCreatureAddonDB(creature_addon);

        handler->PSendSysMessage("Creature's hover animation state successfully toggled (previously active death state will be removed).");
        return true;
    }

    static bool HandleNpcSetEmoteCommand(ChatHandler* handler, char const* args)
    {
        Creature* creature = NULL;
        char* params[2];
        params[0] = strtok((char*)args, " ");
        params[1] = strtok(NULL, " ");

        if (!params[0])
        {
            handler->PSendSysMessage("Not enough arguments. Syntax: .npc set emote $emoteId [$guid]");
            return true;
        }

        if (params[1])
        {
            // number or [name] Shift-click form |color|Hcreature:creature_guid|h[name]|h|r
            char* cId = handler->extractKeyFromLink(params[1], "Hcreature");
            if (!cId)
                return false;

            uint32 lowguid = atoi(cId);
            if (!lowguid)
                return false;

            if (CreatureData const* cr_data = sObjectMgr->GetCreatureData(lowguid))
            {
                creature = sObjectAccessor->GetObjectInWorld(MAKE_NEW_GUID(lowguid, cr_data->id, HIGHGUID_UNIT), (Creature*)NULL);
                if (!creature)
                    creature = sObjectAccessor->GetObjectInWorld(MAKE_NEW_GUID(lowguid, cr_data->id, HIGHGUID_PET), (Creature*)NULL);
                if (!creature)
                    creature = sObjectAccessor->GetObjectInWorld(MAKE_NEW_GUID(lowguid, cr_data->id, HIGHGUID_VEHICLE), (Creature*)NULL);
            }
        }
        else
        {
            creature = handler->getSelectedCreature();
        }

        if (!creature)
        {
            handler->SendSysMessage(LANG_SELECT_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        uint32 emote_id = atol(params[0]);
        creature->SetUInt32Value(UNIT_FIELD_NPC_EMOTESTATE, emote_id);

        // update singleton and database
        CreatureAddon& creature_addon = sObjectMgr->GetCreatureAddonContainer()[creature->GetGUIDLow()];
        creature_addon.emote = emote_id;
        creature->SetCreatureAddonDB(creature_addon);
        
        handler->PSendSysMessage("Creature's permanent emote state successfully applied.");
        return true;
    }

    static bool HandleNpcSetScaleCommand(ChatHandler* handler, char const* args)
    {
        if (!*args) {
            handler->PSendSysMessage("Please specify new scale parameter.");
            return true;
        }

        char * params[2];
        params[0] = strtok((char *)args, " ");
        params[1] = strtok(NULL, " ");

        uint32 guid_low = 0;
        float scale;
        Player* source = handler->GetSession()->GetPlayer();
        Creature* creature = NULL;

        if (!params[1])
        {
            creature = handler->getSelectedCreature();
            guid_low = creature->GetGUIDLow();
            scale = atof(params[0]);
        }
        else
        {
            guid_low = atoi(handler->extractKeyFromLink(params[0], "Hcreature"));
            scale = atof(params[1]);

            if (CreatureData const* cr_data = sObjectMgr->GetCreatureData(guid_low))
            {
                creature = sObjectAccessor->GetObjectInWorld(MAKE_NEW_GUID(guid_low, cr_data->id, HIGHGUID_UNIT), (Creature*)NULL);
                if (!creature)
                    creature = sObjectAccessor->GetObjectInWorld(MAKE_NEW_GUID(guid_low, cr_data->id, HIGHGUID_PET), (Creature*)NULL);
                if (!creature)
                    creature = sObjectAccessor->GetObjectInWorld(MAKE_NEW_GUID(guid_low, cr_data->id, HIGHGUID_VEHICLE), (Creature*)NULL);
            }
        }

        if (!creature || creature->IsPet())
        {
            handler->PSendSysMessage("NPC (GUID: %u) not found", guid_low);
            return true;
        }

        if (scale > 10.0f || scale < 0.0f)
        {
            handler->PSendSysMessage("Scale must be between 0.0f and 10.0f.");
            return true;
        }
        
        // set scale
        float old_scale = creature->GetObjectScale();
        creature->SetObjectScale(scale);

        // Update editor
        creature->SetEditor(handler->GetSession()->GetAccountId());

        // Update modified datetime
        creature->SetModifiedTimestamp(time(NULL));

        creature->SaveToDB();

        handler->PSendSysMessage(">> NPC successfully scaled from %.3f to %.3f!", old_scale, scale);

        return true;
    }

    static bool HandleNpcSelectCommand(ChatHandler* handler, char const* args)
    {
        Player* source = handler->GetSession()->GetPlayer();
        char * entry_id_string = handler->extractKeyFromLink((char*)args, "Hcreature_entry");
        uint32 entry_id = entry_id_string ? atol(entry_id_string) : 0;
        PreparedQueryResult result;

        // get first nearest creature relative to player's position with or without given entry ID for narrowing results
        if (entry_id)
        {
            PreparedStatement * stmt = WorldDatabase.GetPreparedStatement(WORLD_SEL_CREATURE_NEAREST_SINGLE_EID);
            stmt->setFloat(0, source->GetPositionX());
            stmt->setFloat(1, source->GetPositionY());
            stmt->setFloat(2, source->GetPositionZ());
            stmt->setUInt16(3, uint16(source->GetMapId()));
            stmt->setUInt32(4, entry_id);
            result = WorldDatabase.Query(stmt);
        }
        else
        {
            PreparedStatement * stmt = WorldDatabase.GetPreparedStatement(WORLD_SEL_CREATURE_NEAREST_SINGLE);
            stmt->setFloat(0, source->GetPositionX());
            stmt->setFloat(1, source->GetPositionY());
            stmt->setFloat(2, source->GetPositionZ());
            stmt->setUInt16(3, uint16(source->GetMapId()));
            result = WorldDatabase.Query(stmt);
        }

        if (!result) {
            handler->PSendSysMessage("No creatures found!");
            return true;
        }

        // 0:guid, 1:id, 2:position_x, 3:position_y, 4:position_z, 5:map, 6:distance
        Field * fields = result->Fetch();
        uint32 guid_low = fields[0].GetUInt32();
        uint32 template_id = fields[1].GetUInt32();

        CreatureTemplate const* creature_info = sObjectMgr->GetCreatureTemplate(template_id);
        if (!creature_info)
        {
            handler->PSendSysMessage("Selected creature (EID: %u) (GUID: %u) does not exist in gameobject_template table! Contact devs about this.", template_id, guid_low);
            return true;
        }

        // set game object to player's selection
        source->SetSelectedCreature(guid_low);
    
        HandleNpcInfoCommand(handler, args);
        return true;
    }

    static bool HandleNpcUnSelectCommand(ChatHandler* handler, char const* /*args*/)
    {
        Player* source = handler->GetSession()->GetPlayer();
        uint32 guid_low = source->GetSelectedCreature();
        source->SetSelectedCreature(0);
        handler->PSendSysMessage(">> NPC selection (GUID: %u) has been reset.", guid_low);
        return true;
    }

    static bool HandleCreatureDisableCommand(ChatHandler* handler, char const* args)
    {
        if (!*args) {
            handler->PSendSysMessage("Please specify Entry ID of the NPC to disable.");
            return true;
        }

        // number or [name] Shift-click form |color|Hcreature_entry:go_id|h[name]|h|r
        char* id = handler->extractKeyFromLink((char*)args, "Hcreature_entry");
        uint32 entry_id = atol(id);

        if (!entry_id)
        {
            handler->PSendSysMessage("NPC (Entry ID: %u) not found", entry_id);
            return true;
        }

        PreparedStatement * stmt = WorldDatabase.GetPreparedStatement(WORLD_UPD_CREATURE_DISABLED);
        stmt->setUInt8(0, 1);
        stmt->setUInt32(1, entry_id);
        WorldDatabase.Execute(stmt);

        handler->PSendSysMessage(">> NPC (Entry ID: %u) has been disabled from spawning and listing.", entry_id);

        return true;
    }

    static bool HandleCreatureEnableCommand(ChatHandler* handler, char const* args)
    {
        if (!*args) {
            handler->PSendSysMessage("Please specify Entry ID of the NPC to enable.");
            return true;
        }

        // number or [name] Shift-click form |color|Hcreature_entry:go_id|h[name]|h|r
        char* id = handler->extractKeyFromLink((char*)args, "Hcreature_entry");
        uint32 entry_id = atol(id);

        if (!entry_id)
        {
            handler->PSendSysMessage("NPC (Entry ID: %u) not found", entry_id);
            return true;
        }

        PreparedStatement * stmt = WorldDatabase.GetPreparedStatement(WORLD_UPD_CREATURE_DISABLED);
        stmt->setUInt8(0, 0);
        stmt->setUInt32(1, entry_id);
        WorldDatabase.Execute(stmt);

        handler->PSendSysMessage(">> NPC (Entry ID: %u) has been enabled for spawning and listing.", entry_id);

        return true;
    }

    #pragma endregion

    #pragma region DEFAULT_COMMANDS

    //add spawn of creature
    static bool HandleNpcAddCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        char* charID = handler->extractKeyFromLink((char*)args, "Hcreature_entry");
        if (!charID)
            return false;

        char* team = strtok(NULL, " ");
        int32 teamval = 0;
        if (team)
            teamval = atoi(team);

        if (teamval < 0)
            teamval = 0;

        uint32 entry_id = atoi(charID);
        if (!sObjectMgr->GetCreatureTemplate(entry_id))
        {
            handler->PSendSysMessage("NPC (Entry ID: %u) not found", entry_id);
            return true;
        }

        // check if the game object entry ID is disabled
        PreparedStatement * disable_check_stmt = WorldDatabase.GetPreparedStatement(WORLD_SEL_CREATURE_IS_DISABLED);
        disable_check_stmt->setUInt32(0, entry_id);
        PreparedQueryResult disable_check_result = WorldDatabase.Query(disable_check_stmt);

        if (disable_check_result)
        {
            handler->PSendSysMessage("NPC (Entry ID: %u) is disabled. Contact the staff about its re-enabling.", entry_id);
            return true;
        }

        Player* source = handler->GetSession()->GetPlayer();
        float pos_x = source->GetPositionX();
        float pos_y = source->GetPositionY();
        float pos_z = source->GetPositionZ();
        float pos_o = source->GetOrientation();
        Map* map = source->GetMap();

        if (Transport* trans = source->GetTransport())
        {
            uint32 guid = sObjectMgr->GenerateLowGuid(HIGHGUID_UNIT);
            source->SetSelectedCreature(guid);
            CreatureData& data = sObjectMgr->NewOrExistCreatureData(guid);
            data.id = entry_id;
            data.phaseMask = source->GetPhaseMgr().GetPhaseMaskForSpawn();
            data.posX = source->GetTransOffsetX();
            data.posY = source->GetTransOffsetY();
            data.posZ = source->GetTransOffsetZ();
            data.orientation = source->GetTransOffsetO();

            Creature* creature = trans->CreateNPCPassenger(guid, &data);

            // Update creator
            creature->SetCreator(handler->GetSession()->GetAccountId());

            // Update editor
            creature->SetEditor(handler->GetSession()->GetAccountId());

            // Update created datetime
            creature->SetCreatedTimestamp(time(NULL));

            // Update modified datetime
            creature->SetModifiedTimestamp(time(NULL));

            creature->SaveToDB(trans->GetGOInfo()->moTransport.mapID, 1 << map->GetSpawnMode(), source->GetPhaseMgr().GetPhaseMaskForSpawn());

            sObjectMgr->AddCreatureToGrid(guid, &data);
            return true;
        }

        Creature* creature = new Creature();
        if (!creature->Create(sObjectMgr->GenerateLowGuid(HIGHGUID_UNIT), map, source->GetPhaseMgr().GetPhaseMaskForSpawn(), entry_id, 0, (uint32)teamval, pos_x, pos_y, pos_z, pos_o))
        {
            delete creature;
            handler->PSendSysMessage("NPC (Entry ID: %u) failed to spawn. Some of its data likely isn't correct or is missing.", entry_id);
            return true;
        }

        // Update creator
        creature->SetCreator(handler->GetSession()->GetAccountId());

        // Update editor
        creature->SetEditor(handler->GetSession()->GetAccountId());

        // Update created datetime
        creature->SetCreatedTimestamp(time(NULL));

        // Update modified datetime
        creature->SetModifiedTimestamp(time(NULL));

        creature->SaveToDB(map->GetId(), (1 << map->GetSpawnMode()), source->GetPhaseMgr().GetPhaseMaskForSpawn());

        uint32 db_guid = creature->GetDBTableGUIDLow();

        // To call _LoadGoods(); _LoadQuests(); CreateTrainerSpells();
        if (!creature->LoadCreatureFromDB(db_guid, map))
        {
            delete creature;
            handler->PSendSysMessage("NPC (DB_GUID: %u) failed to load the creature from the DB.", db_guid);
            return true;
        }

        source->SetSelectedCreature(db_guid);
        sObjectMgr->AddCreatureToGrid(db_guid, sObjectMgr->GetCreatureData(db_guid));
        return true;
    }

    //add item in vendorlist
    static bool HandleNpcAddVendorItemCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        const uint8 type = 1; // FIXME: make type (1 item, 2 currency) an argument

        char* pitem = handler->extractKeyFromLink((char*)args, "Hitem");
        if (!pitem)
        {
            handler->SendSysMessage(LANG_COMMAND_NEEDITEMSEND);
            handler->SetSentErrorMessage(true);
            return false;
        }

        int32 item_int = atol(pitem);
        if (item_int <= 0)
            return false;

        uint32 itemId = item_int;

        char* fmaxcount = strtok(NULL, " ");                    //add maxcount, default: 0
        uint32 maxcount = 0;
        if (fmaxcount)
            maxcount = atol(fmaxcount);

        char* fincrtime = strtok(NULL, " ");                    //add incrtime, default: 0
        uint32 incrtime = 0;
        if (fincrtime)
            incrtime = atol(fincrtime);

        char* fextendedcost = strtok(NULL, " ");                //add ExtendedCost, default: 0
        uint32 extendedcost = fextendedcost ? atol(fextendedcost) : 0;
        Creature* vendor = handler->getSelectedCreature();
        if (!vendor)
        {
            handler->SendSysMessage(LANG_SELECT_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        uint32 vendor_entry = vendor->GetEntry();

        if (!sObjectMgr->IsVendorItemValid(vendor_entry, itemId, maxcount, incrtime, extendedcost, type, handler->GetSession()->GetPlayer()))
        {
            handler->SetSentErrorMessage(true);
            return false;
        }

        sObjectMgr->AddVendorItem(vendor_entry, itemId, maxcount, incrtime, extendedcost, type);

        ItemTemplate const* itemTemplate = sObjectMgr->GetItemTemplate(itemId);

        handler->PSendSysMessage(LANG_ITEM_ADDED_TO_LIST, itemId, itemTemplate->Name1.c_str(), maxcount, incrtime, extendedcost);
        return true;
    }

    static bool HandleNpcSetEntryCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        uint32 newEntryNum = atoi(args);
        if (!newEntryNum)
            return false;

        Unit* unit = handler->getSelectedUnit();
        if (!unit || unit->GetTypeId() != TYPEID_UNIT)
        {
            handler->SendSysMessage(LANG_SELECT_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }
        Creature* creature = unit->ToCreature();
        if (creature->UpdateEntry(newEntryNum))
            handler->SendSysMessage(LANG_DONE);
        else
            handler->SendSysMessage(LANG_ERROR);
        return true;
    }

    //change level of creature or pet
    static bool HandleNpcSetLevelCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        uint8 lvl = (uint8)atoi((char*)args);
        if (lvl < 1 || lvl > sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL) + 3)
        {
            handler->SendSysMessage(LANG_BAD_VALUE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        Creature* creature = handler->getSelectedCreature();
        if (!creature)
        {
            handler->SendSysMessage(LANG_SELECT_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (creature->IsPet())
        {
            if (((Pet*)creature)->getPetType() == HUNTER_PET)
            {
                creature->SetUInt32Value(UNIT_FIELD_PET_NEXT_LEVEL_EXPERIENCE, sObjectMgr->GetXPForLevel(lvl) / 4);
                creature->SetUInt32Value(UNIT_FIELD_PET_EXPERIENCE, 0);
            }
            ((Pet*)creature)->GivePetLevel(lvl);
        }
        else
        {
            creature->SetMaxHealth(100 + 30 * lvl);
            creature->SetHealth(100 + 30 * lvl);
            creature->SetLevel(lvl);

            // Update editor
            creature->SetEditor(handler->GetSession()->GetAccountId());

            // Update modified datetime
            creature->SetModifiedTimestamp(time(NULL));

            creature->SaveToDB();
        }

        return true;
    }

    static bool HandleNpcDeleteCommand(ChatHandler* handler, char const* args)
    {
        Creature* unit = NULL;

        if (*args)
        {
            // number or [name] Shift-click form |color|Hcreature:creature_guid|h[name]|h|r
            char* cId = handler->extractKeyFromLink((char*)args, "Hcreature");
            if (!cId)
                return false;

            uint32 lowguid = atoi(cId);
            if (!lowguid)
                return false;

            if (CreatureData const* cr_data = sObjectMgr->GetCreatureData(lowguid))
            {
                unit = sObjectAccessor->GetObjectInWorld(MAKE_NEW_GUID(lowguid, cr_data->id, HIGHGUID_UNIT), (Creature*)NULL);
                if (!unit)
                    unit = sObjectAccessor->GetObjectInWorld(MAKE_NEW_GUID(lowguid, cr_data->id, HIGHGUID_PET), (Creature*)NULL);
                if (!unit)
                    unit = sObjectAccessor->GetObjectInWorld(MAKE_NEW_GUID(lowguid, cr_data->id, HIGHGUID_VEHICLE), (Creature*)NULL);
            }
        }
        else
        {
            unit = handler->getSelectedCreature();
        }

        if (!unit || unit->IsPet() || unit->IsTotem())
        {
            handler->SendSysMessage(LANG_SELECT_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        // Delete the creature
        unit->CombatStop();
        unit->DeleteFromDB();
        unit->AddObjectToRemoveList();

        handler->SendSysMessage(LANG_COMMAND_DELCREATMESSAGE);

        return true;
    }

    //del item from vendor list
    static bool HandleNpcDeleteVendorItemCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        Creature* vendor = handler->getSelectedCreature();
        if (!vendor || !vendor->IsVendor())
        {
            handler->SendSysMessage(LANG_COMMAND_VENDORSELECTION);
            handler->SetSentErrorMessage(true);
            return false;
        }

        char* pitem = handler->extractKeyFromLink((char*)args, "Hitem");
        if (!pitem)
        {
            handler->SendSysMessage(LANG_COMMAND_NEEDITEMSEND);
            handler->SetSentErrorMessage(true);
            return false;
        }
        uint32 itemId = atol(pitem);

        const uint8 type = 1; // FIXME: make type (1 item, 2 currency) an argument

        if (!sObjectMgr->RemoveVendorItem(vendor->GetEntry(), itemId, type))
        {
            handler->PSendSysMessage(LANG_ITEM_NOT_IN_LIST, itemId);
            handler->SetSentErrorMessage(true);
            return false;
        }

        ItemTemplate const* itemTemplate = sObjectMgr->GetItemTemplate(itemId);

        handler->PSendSysMessage(LANG_ITEM_DELETED_FROM_LIST, itemId, itemTemplate->Name1.c_str());
        return true;
    }

    //set faction of creature
    static bool HandleNpcSetFactionIdCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        uint32 factionId = (uint32)atoi((char*)args);

        if (!sFactionTemplateStore.LookupEntry(factionId))
        {
            handler->PSendSysMessage(LANG_WRONG_FACTION, factionId);
            handler->SetSentErrorMessage(true);
            return false;
        }

        Creature* creature = handler->getSelectedCreature();

        if (!creature)
        {
            handler->SendSysMessage(LANG_SELECT_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        creature->setFaction(factionId);

        // Faction is set in creature_template - not inside creature

        // Update in memory..
        if (CreatureTemplate const* cinfo = creature->GetCreatureTemplate())
        {
            const_cast<CreatureTemplate*>(cinfo)->faction_A = factionId;
            const_cast<CreatureTemplate*>(cinfo)->faction_H = factionId;
        }

        // ..and DB
        PreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_UPD_CREATURE_FACTION);

        stmt->setUInt16(0, uint16(factionId));
        stmt->setUInt16(1, uint16(factionId));
        stmt->setUInt32(2, creature->GetEntry());

        WorldDatabase.Execute(stmt);

        return true;
    }

    //set npcflag of creature
    static bool HandleNpcSetFlagCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        uint32 npcFlags = (uint32)atoi((char*)args);

        Creature* creature = handler->getSelectedCreature();

        if (!creature)
        {
            handler->SendSysMessage(LANG_SELECT_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        creature->SetUInt32Value(UNIT_FIELD_NPC_FLAGS, npcFlags);

        PreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_UPD_CREATURE_NPCFLAG);

        stmt->setUInt32(0, npcFlags);
        stmt->setUInt32(1, creature->GetEntry());

        WorldDatabase.Execute(stmt);

        handler->SendSysMessage(LANG_VALUE_SAVED_REJOIN);

        return true;
    }

    //set data of creature for testing scripting
    static bool HandleNpcSetDataCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        char* arg1 = strtok((char*)args, " ");
        char* arg2 = strtok((char*)NULL, "");

        if (!arg1 || !arg2)
            return false;

        uint32 data_1 = (uint32)atoi(arg1);
        uint32 data_2 = (uint32)atoi(arg2);

        if (!data_1 || !data_2)
            return false;

        Creature* creature = handler->getSelectedCreature();

        if (!creature)
        {
            handler->SendSysMessage(LANG_SELECT_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        creature->AI()->SetData(data_1, data_2);
        std::string AIorScript = creature->GetAIName() != "" ? "AI type: " + creature->GetAIName() : (creature->GetScriptName() != "" ? "Script Name: " + creature->GetScriptName() : "No AI or Script Name Set");
        handler->PSendSysMessage(LANG_NPC_SETDATA, creature->GetGUID(), creature->GetEntry(), creature->GetName().c_str(), data_1, data_2, AIorScript.c_str());
        return true;
    }

    //npc follow handling
    static bool HandleNpcFollowCommand(ChatHandler* handler, char const* /*args*/)
    {
        Player* player = handler->GetSession()->GetPlayer();
        Creature* creature = handler->getSelectedCreature();

        if (!creature)
        {
            handler->PSendSysMessage(LANG_SELECT_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        // Follow player - Using pet's default dist and angle
        creature->GetMotionMaster()->MoveFollow(player, PET_FOLLOW_DIST, creature->GetFollowAngle());

        handler->PSendSysMessage(LANG_CREATURE_FOLLOW_YOU_NOW, creature->GetName().c_str());
        return true;
    }

    static bool HandleNpcInfoCommand(ChatHandler* handler, char const* /*args*/)
    {
        Creature* creature = handler->getSelectedCreature();

        if (!creature)
        {
            handler->SendSysMessage(LANG_SELECT_CREATURE);
            return true;
        }

        // get creature's information
        Player * source = handler->GetSession()->GetPlayer();
        uint32 faction = creature->getFaction();
        uint32 display_id = creature->GetDisplayId();
        uint32 native_id = creature->GetNativeDisplayId();
        uint32 entry_id = creature->GetEntry();
        float pos_x = creature->GetPositionX();
        float pos_y = creature->GetPositionY();
        float pos_z = creature->GetPositionZ();
        // cant use distance value from query, as for some reason the float value from it is drastically incorrect (extremely huge value), therefore, recalculate the distance in server-code
        float distance = sqrt(pow(pos_x - source->GetPositionX(), 2) + pow(pos_y - source->GetPositionY(), 2) + pow(pos_z - source->GetPositionZ(), 2));

        // get creature's history
        uint32 creator_id = creature->GetCreator();
        uint32 editor_id = creature->GetEditor();
        uint32 account_id = handler->GetSession()->GetAccountId();
        std::string created = TimeToTimestampStr(creature->GetCreatedTimestamp());
        std::string modified = TimeToTimestampStr(creature->GetModifiedTimestamp());

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
        handler->PSendSysMessage("SELECTED NPC: |cFFFFFFFF|Hcreature_entry:%u|h[%s]|h|r", entry_id, creature->GetName().c_str());
        handler->PSendSysMessage("> GUID: %u", creature->GetGUIDLow());
        handler->PSendSysMessage("> Entry: %u", entry_id);
        handler->PSendSysMessage("> PhaseMask: %u", creature->GetPhaseMask());
        handler->PSendSysMessage("> Faction: %u", faction);
        handler->PSendSysMessage("> DisplayID: %u (NativeID: %u)", display_id, native_id);
        handler->PSendSysMessage("> Scale: %.2f", creature->GetFloatValue(OBJECT_FIELD_SCALE));
        handler->PSendSysMessage("> Created by: %s (ID: %u, TIMESTAMP: %s)", creator_username.c_str(), creator_id, created.c_str());
        handler->PSendSysMessage("> Last modified by: %s (ID: %u, TIMESTAMP: %s)", editor_username.c_str(), editor_id, modified.c_str());
        handler->PSendSysMessage("> Position: X: %f Y: %f Z: %f", pos_x, pos_y, pos_z);
        handler->PSendSysMessage("> Distance: %.2f yards", distance);

        return true;
    }

    static bool HandleNpcNearCommand(ChatHandler* handler, char const* args)
    {
        float distance = (!*args) ? 10.0f : float((atof(args)));
        uint32 count = 0;

        Player* player = handler->GetSession()->GetPlayer();

        PreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_SEL_CREATURE_NEAREST);
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

                CreatureTemplate const* creatureTemplate = sObjectMgr->GetCreatureTemplate(entry);
                if (!creatureTemplate)
                    continue;

                handler->PSendSysMessage(LANG_CREATURE_LIST_CHAT, guid, guid, creatureTemplate->Name.c_str(), x, y, z, mapId);

                ++count;
            } while (result->NextRow());
        }

        handler->PSendSysMessage(LANG_COMMAND_NEAR_NPC_MESSAGE, distance, count);

        return true;
    }

    //move selected creature
    static bool HandleNpcMoveCommand(ChatHandler* handler, char const* args)
    {
        uint32 guid_low;

        Creature* creature;
        Player* source = handler->GetSession()->GetPlayer();

        if (*args) 
        {
            // number or [name] Shift-click form |color|Hcreature:creature_guid|h[name]|h|r
            char* cId = handler->extractKeyFromLink((char*)args, "Hcreature");
            if (!cId)
                return false;

            guid_low = atoi(cId);
            // Attempting creature load from DB data
            CreatureData const* data = sObjectMgr->GetCreatureData(guid_low);
            if (data)
            {
                creature = sObjectAccessor->GetObjectInWorld(MAKE_NEW_GUID(guid_low, data->id, HIGHGUID_UNIT), (Creature*)NULL);
                if (!creature)
                    creature = sObjectAccessor->GetObjectInWorld(MAKE_NEW_GUID(guid_low, data->id, HIGHGUID_PET), (Creature*)NULL);
                if (!creature)
                    creature = sObjectAccessor->GetObjectInWorld(MAKE_NEW_GUID(guid_low, data->id, HIGHGUID_VEHICLE), (Creature*)NULL);

                if (!creature)
                {
                    handler->PSendSysMessage(LANG_COMMAND_CREATGUIDNOTFOUND, guid_low);
                    return true;
                }
            }
            else
            {
                handler->PSendSysMessage(LANG_COMMAND_CREATGUIDNOTFOUND, guid_low);
                return true;
            }
        } 
        else
        {
            creature = handler->getSelectedCreature();
            if (!creature)
            {
                handler->PSendSysMessage("NPC (GUID: %u) not found", source->GetSelectedCreature());
                return true;
            }
            guid_low = creature->GetGUIDLow();
        }

        if (handler->GetSession()->GetPlayer()->GetMapId() != creature->GetMapId())
        {
            handler->PSendSysMessage(LANG_COMMAND_CREATUREATSAMEMAP, guid_low);
            return true;
        }

        float pos_x = source->GetPositionX();
        float pos_y = source->GetPositionY();
        float pos_z = source->GetPositionZ();
        float pos_o = source->GetOrientation();
        Position pos = {pos_x, pos_y, pos_z, pos_o};

        // Update editor
        creature->SetEditor(handler->GetSession()->GetAccountId());

        // Update modified datetime
        creature->SetModifiedTimestamp(time(NULL));

        creature->Relocate(pos);

        creature->SaveToDB();

        // TODO: BUG: Creature's new position is updated server-side but not client-side
        // TEMP. WORKAROUND: Similar to gameobject phasing/moving/turning, destroying sight of NPC for nearby players
        // moving too early (or for players who are in motion) will not show the NPC in new position, it will stay in the old one
        creature->DestroyForNearbyPlayers();

        handler->PSendSysMessage(LANG_COMMAND_CREATUREMOVED);
        return true;
    }

    //play npc emote
    static bool HandleNpcPlayEmoteCommand(ChatHandler* handler, char const* args)
    {
        uint32 emote = atoi((char*)args);

        Creature* target = handler->getSelectedCreature();
        if (!target)
        {
            handler->SendSysMessage(LANG_SELECT_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        target->SetUInt32Value(UNIT_FIELD_NPC_EMOTESTATE, emote);

        return true;
    }

    //set model of creature
    static bool HandleNpcSetModelCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        uint32 displayId = (uint32)atoi((char*)args);

        Creature* creature = handler->getSelectedCreature();

        if (!creature || creature->IsPet())
        {
            handler->SendSysMessage(LANG_SELECT_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        creature->SetDisplayId(displayId);
        creature->SetNativeDisplayId(displayId);

        // Update editor
        creature->SetEditor(handler->GetSession()->GetAccountId());

        // Update modified datetime
        creature->SetModifiedTimestamp(time(NULL));

        creature->SaveToDB();

        return true;
    }

    //npc phasemask handling
    //change phasemask of creature or pet
    static bool HandleNpcSetPhaseCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        uint32 phasemask = (uint32)atoi((char*)args);
        if (phasemask == 0)
        {
            handler->SendSysMessage(LANG_BAD_VALUE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        Creature* creature = handler->getSelectedCreature();
        if (!creature)
        {
            handler->SendSysMessage(LANG_SELECT_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        creature->SetPhaseMask(phasemask, true);

        // Update editor
        creature->SetEditor(handler->GetSession()->GetAccountId());

        // Update modified datetime
        creature->SetModifiedTimestamp(time(NULL));

        if (!creature->IsPet())
            creature->SaveToDB();

        return true;
    }

    //set spawn dist of creature
    static bool HandleNpcSetSpawnDistCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        float option = (float)(atof((char*)args));
        if (option < 0.0f)
        {
            handler->SendSysMessage(LANG_BAD_VALUE);
            return false;
        }

        MovementGeneratorType mtype = IDLE_MOTION_TYPE;
        if (option >0.0f)
            mtype = RANDOM_MOTION_TYPE;

        Creature* creature = handler->getSelectedCreature();
        uint32 guidLow = 0;

        if (creature)
            guidLow = creature->GetDBTableGUIDLow();
        else
            return false;

        creature->SetRespawnRadius((float)option);
        creature->SetDefaultMovementType(mtype);
        creature->GetMotionMaster()->Initialize();
        if (creature->IsAlive())                                // dead creature will reset movement generator at respawn
        {
            creature->setDeathState(JUST_DIED);
            creature->Respawn();
        }

        PreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_UPD_CREATURE_SPAWN_DISTANCE);

        stmt->setFloat(0, option);
        stmt->setUInt8(1, uint8(mtype));
        stmt->setUInt32(2, guidLow);

        WorldDatabase.Execute(stmt);

        handler->PSendSysMessage(LANG_COMMAND_SPAWNDIST, option);
        return true;
    }

    //spawn time handling
    static bool HandleNpcSetSpawnTimeCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        char* stime = strtok((char*)args, " ");

        if (!stime)
            return false;

        int spawnTime = atoi((char*)stime);

        if (spawnTime < 0)
        {
            handler->SendSysMessage(LANG_BAD_VALUE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        Creature* creature = handler->getSelectedCreature();
        uint32 guidLow = 0;

        if (creature)
            guidLow = creature->GetDBTableGUIDLow();
        else
            return false;

        PreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_UPD_CREATURE_SPAWN_TIME_SECS);

        stmt->setUInt32(0, uint32(spawnTime));
        stmt->setUInt32(1, guidLow);

        WorldDatabase.Execute(stmt);

        creature->SetRespawnDelay((uint32)spawnTime);
        handler->PSendSysMessage(LANG_COMMAND_SPAWNTIME, spawnTime);

        return true;
    }

    static bool HandleNpcSayCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        Creature* creature = handler->getSelectedCreature();
        if (!creature)
        {
            handler->SendSysMessage(LANG_SELECT_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        creature->MonsterSay(args, LANG_UNIVERSAL, NULL);

        // make some emotes
        char lastchar = args[strlen(args) - 1];
        switch (lastchar)
        {
        case '?':   creature->HandleEmoteCommand(EMOTE_ONESHOT_QUESTION);      break;
        case '!':   creature->HandleEmoteCommand(EMOTE_ONESHOT_EXCLAMATION);   break;
        default:    creature->HandleEmoteCommand(EMOTE_ONESHOT_TALK);          break;
        }

        return true;
    }

    //show text emote by creature in chat
    static bool HandleNpcTextEmoteCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        Creature* creature = handler->getSelectedCreature();

        if (!creature)
        {
            handler->SendSysMessage(LANG_SELECT_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        creature->MonsterTextEmote(args, 0);

        return true;
    }

    //npc unfollow handling
    static bool HandleNpcUnFollowCommand(ChatHandler* handler, char const* /*args*/)
    {
        Player* player = handler->GetSession()->GetPlayer();
        Creature* creature = handler->getSelectedCreature();

        if (!creature)
        {
            handler->PSendSysMessage(LANG_SELECT_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (/*creature->GetMotionMaster()->empty() ||*/
            creature->GetMotionMaster()->GetCurrentMovementGeneratorType() != FOLLOW_MOTION_TYPE)
        {
            handler->PSendSysMessage(LANG_CREATURE_NOT_FOLLOW_YOU, creature->GetName().c_str());
            handler->SetSentErrorMessage(true);
            return false;
        }

        FollowMovementGenerator<Creature> const* mgen = static_cast<FollowMovementGenerator<Creature> const*>((creature->GetMotionMaster()->top()));

        if (mgen->GetTarget() != player)
        {
            handler->PSendSysMessage(LANG_CREATURE_NOT_FOLLOW_YOU, creature->GetName().c_str());
            handler->SetSentErrorMessage(true);
            return false;
        }

        // reset movement
        creature->GetMotionMaster()->MovementExpired(true);

        handler->PSendSysMessage(LANG_CREATURE_NOT_FOLLOW_YOU_NOW, creature->GetName().c_str());
        return true;
    }

    // make npc whisper to player
    static bool HandleNpcWhisperCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        char* receiver_str = strtok((char*)args, " ");
        char* text = strtok(NULL, "");

        Creature* creature = handler->getSelectedCreature();
        if (!creature || !receiver_str || !text)
            return false;

        uint64 receiver_guid = atol(receiver_str);

        // check online security
        Player* receiver = ObjectAccessor::FindPlayer(receiver_guid);
        if (handler->HasLowerSecurity(receiver, 0))
            return false;

        creature->MonsterWhisper(text, receiver);
        return true;
    }

    static bool HandleNpcYellCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        Creature* creature = handler->getSelectedCreature();
        if (!creature)
        {
            handler->SendSysMessage(LANG_SELECT_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        creature->MonsterYell(args, LANG_UNIVERSAL, NULL);

        // make an emote
        creature->HandleEmoteCommand(EMOTE_ONESHOT_SHOUT);

        return true;
    }

    //npc tame handling
    static bool HandleNpcTameCommand(ChatHandler* handler, char const* /*args*/)
    {
        Creature* creatureTarget = handler->getSelectedCreature();
        if (!creatureTarget || creatureTarget->IsPet())
        {
            handler->PSendSysMessage(LANG_SELECT_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        Player* player = handler->GetSession()->GetPlayer();

        if (player->GetPetGUID())
        {
            handler->SendSysMessage(LANG_YOU_ALREADY_HAVE_PET);
            handler->SetSentErrorMessage(true);
            return false;
        }

        CreatureTemplate const* cInfo = creatureTarget->GetCreatureTemplate();

        if (!cInfo->IsTameable(player->CanTameExoticPets()))
        {
            handler->PSendSysMessage(LANG_CREATURE_NON_TAMEABLE, cInfo->Entry);
            handler->SetSentErrorMessage(true);
            return false;
        }

        // Everything looks OK, create new pet
        Pet* pet = player->CreateTamedPetFrom(creatureTarget);
        if (!pet)
        {
            handler->PSendSysMessage(LANG_CREATURE_NON_TAMEABLE, cInfo->Entry);
            handler->SetSentErrorMessage(true);
            return false;
        }

        // place pet before player
        float x, y, z;
        player->GetClosePoint(x, y, z, creatureTarget->GetObjectSize(), CONTACT_DISTANCE);
        pet->Relocate(x, y, z, M_PI - player->GetOrientation());

        // set pet to defensive mode by default (some classes can't control controlled pets in fact).
        pet->SetReactState(REACT_DEFENSIVE);

        // calculate proper level
        uint8 level = (creatureTarget->getLevel() < (player->getLevel() - 5)) ? (player->getLevel() - 5) : creatureTarget->getLevel();

        // prepare visual effect for levelup
        pet->SetUInt32Value(UNIT_FIELD_LEVEL, level - 1);

        // add to world
        pet->GetMap()->AddToMap(pet->ToCreature());

        // visual effect for levelup
        pet->SetUInt32Value(UNIT_FIELD_LEVEL, level);

        // caster have pet now
        player->SetMinion(pet, true);

        pet->SavePetToDB(PET_SAVE_AS_CURRENT);
        player->PetSpellInitialize();

        return true;
    }

    #pragma endregion

};

#endif