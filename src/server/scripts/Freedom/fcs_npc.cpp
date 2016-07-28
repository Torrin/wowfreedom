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
#include "Config.h"
#include "BattlenetAccountMgr.h"

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

    std::vector<ChatCommand> GetCommands() const override
    {
        static std::vector<ChatCommand> npcAddCommandTable =
        {
            { "formation",  rbac::RBAC_PERM_COMMAND_NPC_ADD_FORMATION,          false, &HandleNpcAddFormationCommand,       "" },
            { "item",       rbac::RBAC_PERM_COMMAND_NPC_ADD_ITEM,               false, &HandleNpcAddVendorItemCommand,      "" },
            { "",           rbac::RBAC_PERM_COMMAND_NPC_ADD,                    false, &HandleNpcAddCommand,                "" },
        };
        static std::vector<ChatCommand> npcDeleteCommandTable =
        {
            { "item",       rbac::RBAC_PERM_COMMAND_NPC_DELETE_ITEM,            false, &HandleNpcDeleteVendorItemCommand,   "" },
            { "",           rbac::RBAC_PERM_COMMAND_NPC_DELETE,                 false, &HandleNpcDeleteCommand,             "" },
        };
        static std::vector<ChatCommand> npcFollowCommandTable =
        {
            { "stop",       rbac::RBAC_PERM_COMMAND_NPC_FOLLOW_STOP,            false, &HandleNpcUnFollowCommand,           "" },
            { "",           rbac::RBAC_PERM_COMMAND_NPC_FOLLOW,                 false, &HandleNpcFollowCommand,             "" },
        };
        static std::vector<ChatCommand> npcSetCommandTable =
        {
            { "emote",          rbac::RBAC_PERM_COMMAND_NPC,                        false, &HandleNpcSetEmoteCommand,           "" },
            { "mount",          rbac::RBAC_PERM_COMMAND_NPC,                        false, &HandleNpcSetMountCommand,           "" },
            { "aura",           rbac::RBAC_PERM_COMMAND_NPC,                        false, &HandleNpcSetAuraCommand,            "" },
            { "deathstate",     rbac::RBAC_PERM_COMMAND_NPC,                        false, &HandleNpcSetDeathStateCommand,      "" },
            { "sleepstate",     rbac::RBAC_PERM_COMMAND_NPC,                        false, &HandleNpcSetSleepStateCommand,      "" },
            { "hoverstate",     rbac::RBAC_PERM_COMMAND_NPC,                        false, &HandleNpcSetHoverStateCommand,      "" },
            { "allowmove",      rbac::RBAC_FPERM_ADMINISTRATION,                    false, &HandleNpcSetAllowMovementCommand,   "" },
            { "entry",          rbac::RBAC_FPERM_ADMINISTRATION,                    false, &HandleNpcSetEntryCommand,           "" },
            { "factionid",      rbac::RBAC_PERM_COMMAND_NPC_SET_FACTIONID,          false, &HandleNpcSetFactionIdCommand,       "" },
            { "flag",           rbac::RBAC_FPERM_ADMINISTRATION,                    false, &HandleNpcSetFlagCommand,            "" },
            { "level",          rbac::RBAC_FPERM_ADMINISTRATION,                    false, &HandleNpcSetLevelCommand,           "" },
            { "link",           rbac::RBAC_FPERM_ADMINISTRATION,                    false, &HandleNpcSetLinkCommand,            "" },
            { "model",          rbac::RBAC_PERM_COMMAND_NPC_SET_MODEL,              false, &HandleNpcSetModelCommand,           "" },
            { "movetype",       rbac::RBAC_PERM_COMMAND_NPC_SET_MOVETYPE,           false, &HandleNpcSetMoveTypeCommand,        "" },
            { "phase",          rbac::RBAC_PERM_COMMAND_NPC_SET_PHASE,              false, &HandleNpcSetPhaseCommand,           "" },
            { "scale",          rbac::RBAC_PERM_COMMAND_NPC,                        false, &HandleNpcSetScaleCommand,           "" },
            { "spawndist",      rbac::RBAC_FPERM_ADMINISTRATION,                    false, &HandleNpcSetSpawnDistCommand,       "" },
            { "spawntime",      rbac::RBAC_FPERM_ADMINISTRATION,                    false, &HandleNpcSetSpawnTimeCommand,       "" },
            { "data",           rbac::RBAC_FPERM_ADMINISTRATION,                    false, &HandleNpcSetDataCommand,            "" },
            { "bytes1",         rbac::RBAC_PERM_COMMAND_NPC,                        false, &HandleNpcSetBytes1Command,          "" },
            { "bytes2",         rbac::RBAC_PERM_COMMAND_NPC,                        false, &HandleNpcSetBytes2Command,          "" },
            { "sitgroundstate", rbac::RBAC_PERM_COMMAND_NPC,                        false, &HandleNpcSetSitGroundStateCommand,  "" },
            { "sitlowstate",    rbac::RBAC_PERM_COMMAND_NPC,                        false, &HandleNpcSetSitLowStateCommand,     "" },
            { "sitmediumstate", rbac::RBAC_PERM_COMMAND_NPC,                        false, &HandleNpcSetSitMediumStateCommand,  "" },
            { "sithighstate",   rbac::RBAC_PERM_COMMAND_NPC,                        false, &HandleNpcSetSitHighStateCommand,    "" },
            { "kneelstate",     rbac::RBAC_PERM_COMMAND_NPC,                        false, &HandleNpcSetKneelStateCommand,      "" },
        };
        static std::vector<ChatCommand> npcCommandTable =
        {
            { "info",       rbac::RBAC_PERM_COMMAND_NPC_INFO,                   false, &HandleNpcInfoCommand,               "" },
            { "near",       rbac::RBAC_PERM_COMMAND_NPC_NEAR,                   false, &HandleNpcNearCommand,               "" },
            { "move",       rbac::RBAC_PERM_COMMAND_NPC_MOVE,                   false, &HandleNpcMoveCommand,               "" },
            { "turn",       rbac::RBAC_PERM_COMMAND_NPC,                        false, &HandleNpcTurnCommand,               "" },
            { "playemote",  rbac::RBAC_PERM_COMMAND_NPC_PLAYEMOTE,              false, &HandleNpcPlayEmoteCommand,          "" },
            { "say",        rbac::RBAC_PERM_COMMAND_NPC_SAY,                    false, &HandleNpcSayCommand,                "" },
            { "textemote",  rbac::RBAC_PERM_COMMAND_NPC_TEXTEMOTE,              false, &HandleNpcTextEmoteCommand,          "" },
            { "whisper",    rbac::RBAC_PERM_COMMAND_NPC_WHISPER,                false, &HandleNpcWhisperCommand,            "" },
            { "yell",       rbac::RBAC_PERM_COMMAND_NPC_YELL,                   false, &HandleNpcYellCommand,               "" },
            { "select",     rbac::RBAC_PERM_COMMAND_NPC,                        false, &HandleNpcSelectCommand,             "" },
            { "spawn",      rbac::RBAC_PERM_COMMAND_NPC_ADD,                    false, &HandleNpcAddCommand,                "" },
            { "add",        rbac::RBAC_PERM_COMMAND_NPC_ADD,                    false, NULL,                                "", npcAddCommandTable },
            { "delete",     rbac::RBAC_PERM_COMMAND_NPC_DELETE,                 false, NULL,                                "", npcDeleteCommandTable },
            { "follow",     rbac::RBAC_PERM_COMMAND_NPC_FOLLOW,                 false, NULL,                                "", npcFollowCommandTable },
            { "set",        rbac::RBAC_PERM_COMMAND_NPC_SET,                    false, NULL,                                "", npcSetCommandTable },
        };
        static std::vector<ChatCommand> commandTable =
        {
            { "npc", rbac::RBAC_PERM_COMMAND_NPC, false, NULL, "", npcCommandTable },
        };
        return commandTable;
    }

    static bool HandleNpcSetKneelStateCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
        {
            handler->PSendSysMessage(FREEDOM_CMDH_NPC_SET_KNEELSTATE);
            return true;
        }

        std::string toggleToken = "";
        Player* source = handler->GetSession()->GetPlayer();
        Creature* target = handler->getSelectedCreature();
        uint64 guidLow = target ? target->GetSpawnId() : sFreedomMgr->GetSelectedCreatureGuidFromPlayer(source->GetGUID().GetCounter());

        AdvancedArgumentTokenizer tokenizer(args);
        toggleToken = tokenizer.TryGetParam(0);

        if (tokenizer.size() > 1)
        {
            guidLow = tokenizer.TryGetParam<uint64>(1, "Hcreature");
        }

        if (toggleToken != "on" && toggleToken != "off")
        {
            handler->PSendSysMessage(FREEDOM_CMDH_NPC_SET_KNEELSTATE);
            return true;
        }

        target = sFreedomMgr->GetAnyCreature(guidLow);

        if (!target)
        {
            handler->PSendSysMessage(FREEDOM_CMDE_CREATURE_NOT_FOUND);
            return true;
        }

        bool toggle = (toggleToken == "on");

        sFreedomMgr->CreatureSetAuraToggle(target, uint32(SPELL_PERMANENT_HOVER), false);
        sFreedomMgr->CreatureSetBytes1(target, toggle ? UNIT_STAND_STATE_KNEEL : 0);
        sFreedomMgr->CreatureSetModifyHistory(target, source);
        sFreedomMgr->SaveCreature(target);

        handler->PSendSysMessage(FREEDOM_CMDI_NPC_SET_KNEELSTATE);
        return true;
    }

    static bool HandleNpcSetSitHighStateCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
        {
            handler->PSendSysMessage(FREEDOM_CMDH_NPC_SET_SITHIGHSTATE);
            return true;
        }

        std::string toggleToken = "";
        Player* source = handler->GetSession()->GetPlayer();
        Creature* target = handler->getSelectedCreature();
        uint64 guidLow = target ? target->GetSpawnId() : sFreedomMgr->GetSelectedCreatureGuidFromPlayer(source->GetGUID().GetCounter());

        AdvancedArgumentTokenizer tokenizer(args);
        toggleToken = tokenizer.TryGetParam(0);

        if (tokenizer.size() > 1)
        {
            guidLow = tokenizer.TryGetParam<uint64>(1, "Hcreature");
        }

        if (toggleToken != "on" && toggleToken != "off")
        {
            handler->PSendSysMessage(FREEDOM_CMDH_NPC_SET_SITHIGHSTATE);
            return true;
        }

        target = sFreedomMgr->GetAnyCreature(guidLow);

        if (!target)
        {
            handler->PSendSysMessage(FREEDOM_CMDE_CREATURE_NOT_FOUND);
            return true;
        }

        bool toggle = (toggleToken == "on");

        sFreedomMgr->CreatureSetAuraToggle(target, uint32(SPELL_PERMANENT_HOVER), false);
        sFreedomMgr->CreatureSetBytes1(target, toggle ? UNIT_STAND_STATE_SIT_HIGH_CHAIR : 0);
        sFreedomMgr->CreatureSetModifyHistory(target, source);
        sFreedomMgr->SaveCreature(target);

        handler->PSendSysMessage(FREEDOM_CMDI_NPC_SET_SITHIGHSTATE);
        return true;
    }

    static bool HandleNpcSetSitMediumStateCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
        {
            handler->PSendSysMessage(FREEDOM_CMDH_NPC_SET_SITMEDIUMSTATE);
            return true;
        }

        std::string toggleToken = "";
        Player* source = handler->GetSession()->GetPlayer();
        Creature* target = handler->getSelectedCreature();
        uint64 guidLow = target ? target->GetSpawnId() : sFreedomMgr->GetSelectedCreatureGuidFromPlayer(source->GetGUID().GetCounter());

        AdvancedArgumentTokenizer tokenizer(args);
        toggleToken = tokenizer.TryGetParam(0);

        if (tokenizer.size() > 1)
        {
            guidLow = tokenizer.TryGetParam<uint64>(1, "Hcreature");
        }

        if (toggleToken != "on" && toggleToken != "off")
        {
            handler->PSendSysMessage(FREEDOM_CMDH_NPC_SET_SITMEDIUMSTATE);
            return true;
        }

        target = sFreedomMgr->GetAnyCreature(guidLow);

        if (!target)
        {
            handler->PSendSysMessage(FREEDOM_CMDE_CREATURE_NOT_FOUND);
            return true;
        }

        bool toggle = (toggleToken == "on");

        sFreedomMgr->CreatureSetAuraToggle(target, uint32(SPELL_PERMANENT_HOVER), false);
        sFreedomMgr->CreatureSetBytes1(target, toggle ? UNIT_STAND_STATE_SIT_MEDIUM_CHAIR : 0);
        sFreedomMgr->CreatureSetModifyHistory(target, source);
        sFreedomMgr->SaveCreature(target);

        handler->PSendSysMessage(FREEDOM_CMDI_NPC_SET_SITMEDIUMSTATE);
        return true;
    }

    static bool HandleNpcSetSitLowStateCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
        {
            handler->PSendSysMessage(FREEDOM_CMDH_NPC_SET_SITLOWSTATE);
            return true;
        }

        std::string toggleToken = "";
        Player* source = handler->GetSession()->GetPlayer();
        Creature* target = handler->getSelectedCreature();
        uint64 guidLow = target ? target->GetSpawnId() : sFreedomMgr->GetSelectedCreatureGuidFromPlayer(source->GetGUID().GetCounter());

        AdvancedArgumentTokenizer tokenizer(args);
        toggleToken = tokenizer.TryGetParam(0);

        if (tokenizer.size() > 1)
        {
            guidLow = tokenizer.TryGetParam<uint64>(1, "Hcreature");
        }

        if (toggleToken != "on" && toggleToken != "off")
        {
            handler->PSendSysMessage(FREEDOM_CMDH_NPC_SET_SITLOWSTATE);
            return true;
        }

        target = sFreedomMgr->GetAnyCreature(guidLow);

        if (!target)
        {
            handler->PSendSysMessage(FREEDOM_CMDE_CREATURE_NOT_FOUND);
            return true;
        }

        bool toggle = (toggleToken == "on");

        sFreedomMgr->CreatureSetAuraToggle(target, uint32(SPELL_PERMANENT_HOVER), false);
        sFreedomMgr->CreatureSetBytes1(target, toggle ? UNIT_STAND_STATE_SIT_LOW_CHAIR : 0);
        sFreedomMgr->CreatureSetModifyHistory(target, source);
        sFreedomMgr->SaveCreature(target);

        handler->PSendSysMessage(FREEDOM_CMDI_NPC_SET_SITLOWSTATE);
        return true;
    }

    static bool HandleNpcSetSitGroundStateCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
        {
            handler->PSendSysMessage(FREEDOM_CMDH_NPC_SET_SITGROUNDSTATE);
            return true;
        }

        std::string toggleToken = "";
        Player* source = handler->GetSession()->GetPlayer();
        Creature* target = handler->getSelectedCreature();
        uint64 guidLow = target ? target->GetSpawnId() : sFreedomMgr->GetSelectedCreatureGuidFromPlayer(source->GetGUID().GetCounter());

        AdvancedArgumentTokenizer tokenizer(args);
        toggleToken = tokenizer.TryGetParam(0);

        if (tokenizer.size() > 1)
        {
            guidLow = tokenizer.TryGetParam<uint64>(1, "Hcreature");
        }

        if (toggleToken != "on" && toggleToken != "off")
        {
            handler->PSendSysMessage(FREEDOM_CMDH_NPC_SET_SITGROUNDSTATE);
            return true;
        }

        target = sFreedomMgr->GetAnyCreature(guidLow);

        if (!target)
        {
            handler->PSendSysMessage(FREEDOM_CMDE_CREATURE_NOT_FOUND);
            return true;
        }

        bool toggle = (toggleToken == "on");

        sFreedomMgr->CreatureSetAuraToggle(target, uint32(SPELL_PERMANENT_HOVER), false);
        sFreedomMgr->CreatureSetBytes1(target, toggle ? UNIT_STAND_STATE_SIT : 0);
        sFreedomMgr->CreatureSetModifyHistory(target, source);
        sFreedomMgr->SaveCreature(target);

        handler->PSendSysMessage(FREEDOM_CMDI_NPC_SET_SITGROUNDSTATE);
        return true;
    }

    static bool HandleNpcSetBytes1Command(ChatHandler* handler, char const* args)
    {
        if (!*args)
        {
            handler->PSendSysMessage(FREEDOM_CMDH_NPC_SET_BYTES1);
            return true;
        }

        uint32 bytes1 = 0;
        Player* source = handler->GetSession()->GetPlayer();
        Creature* target = handler->getSelectedCreature();
        uint64 guidLow = target ? target->GetSpawnId() : sFreedomMgr->GetSelectedCreatureGuidFromPlayer(source->GetGUID().GetCounter());

        AdvancedArgumentTokenizer tokenizer(args);
        bytes1 = tokenizer.TryGetParam<uint32>(0);

        if (tokenizer.size() > 1)
        {
            guidLow = tokenizer.TryGetParam<uint64>(1, "Hcreature");
        }

        target = sFreedomMgr->GetAnyCreature(guidLow);

        if (!target)
        {
            handler->PSendSysMessage(FREEDOM_CMDE_CREATURE_NOT_FOUND);
            return true;
        }

        sFreedomMgr->CreatureSetBytes1(target, bytes1);
        sFreedomMgr->CreatureSetModifyHistory(target, source);
        sFreedomMgr->SaveCreature(target);

        handler->PSendSysMessage(FREEDOM_CMDI_NPC_SET_BYTES1);
        return true;
    }

    static bool HandleNpcSetBytes2Command(ChatHandler* handler, char const* args)
    {
        if (!*args)
        {
            handler->PSendSysMessage(FREEDOM_CMDH_NPC_SET_BYTES2);
            return true;
        }

        uint32 bytes2 = 0;
        Player* source = handler->GetSession()->GetPlayer();
        Creature* target = handler->getSelectedCreature();
        uint64 guidLow = target ? target->GetSpawnId() : sFreedomMgr->GetSelectedCreatureGuidFromPlayer(source->GetGUID().GetCounter());

        AdvancedArgumentTokenizer tokenizer(args);
        bytes2 = tokenizer.TryGetParam<uint32>(0);

        if (tokenizer.size() > 1)
        {
            guidLow = tokenizer.TryGetParam<uint64>(1, "Hcreature");
        }

        target = sFreedomMgr->GetAnyCreature(guidLow);

        if (!target)
        {
            handler->PSendSysMessage(FREEDOM_CMDE_CREATURE_NOT_FOUND);
            return true;
        }

        sFreedomMgr->CreatureSetBytes2(target, bytes2);
        sFreedomMgr->CreatureSetModifyHistory(target, source);
        sFreedomMgr->SaveCreature(target);

        handler->PSendSysMessage(FREEDOM_CMDI_NPC_SET_BYTES2);
        return true;
    }

    static bool HandleNpcSetHoverStateCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
        {
            handler->PSendSysMessage(FREEDOM_CMDH_NPC_SET_HOVERSTATE);
            return true;
        }

        std::string toggleToken = "";
        Player* source = handler->GetSession()->GetPlayer();
        Creature* target = handler->getSelectedCreature();
        uint64 guidLow = target ? target->GetSpawnId() : sFreedomMgr->GetSelectedCreatureGuidFromPlayer(source->GetGUID().GetCounter());

        AdvancedArgumentTokenizer tokenizer(args);
        toggleToken = tokenizer.TryGetParam(0);

        if (tokenizer.size() > 1)
        {
            guidLow = tokenizer.TryGetParam<uint64>(1, "Hcreature");
        }

        if (toggleToken != "on" && toggleToken != "off")
        {
            handler->PSendSysMessage(FREEDOM_CMDH_NPC_SET_HOVERSTATE);
            return true;
        }

        target = sFreedomMgr->GetAnyCreature(guidLow);

        if (!target)
        {
            handler->PSendSysMessage(FREEDOM_CMDE_CREATURE_NOT_FOUND);
            return true;
        }

        bool toggle = (toggleToken == "on");
        
        sFreedomMgr->CreatureSetBytes1(target, 0);
        sFreedomMgr->CreatureSetAuraToggle(target, uint32(SPELL_PERMANENT_HOVER), toggle);
        sFreedomMgr->CreatureSetModifyHistory(target, source);
        sFreedomMgr->SaveCreature(target);

        handler->PSendSysMessage(FREEDOM_CMDI_NPC_SET_HOVERSTATE);
        return true;
    }

    static bool HandleNpcSetSleepStateCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
        {
            handler->PSendSysMessage(FREEDOM_CMDH_NPC_SET_SLEEPSTATE);
            return true;
        }

        std::string toggleToken = "";
        Player* source = handler->GetSession()->GetPlayer();
        Creature* target = handler->getSelectedCreature();
        uint64 guidLow = target ? target->GetSpawnId() : sFreedomMgr->GetSelectedCreatureGuidFromPlayer(source->GetGUID().GetCounter());

        AdvancedArgumentTokenizer tokenizer(args);
        toggleToken = tokenizer.TryGetParam(0);

        if (tokenizer.size() > 1)
        {
            guidLow = tokenizer.TryGetParam<uint64>(1, "Hcreature");
        }

        if (toggleToken != "on" && toggleToken != "off")
        {
            handler->PSendSysMessage(FREEDOM_CMDH_NPC_SET_SLEEPSTATE);
            return true;
        }

        target = sFreedomMgr->GetAnyCreature(guidLow);

        if (!target)
        {
            handler->PSendSysMessage(FREEDOM_CMDE_CREATURE_NOT_FOUND);
            return true;
        }

        bool toggle = (toggleToken == "on");

        sFreedomMgr->CreatureSetAuraToggle(target, uint32(SPELL_PERMANENT_HOVER), false);
        sFreedomMgr->CreatureSetBytes1(target, toggle ? UNIT_STAND_STATE_SLEEP : 0);
        sFreedomMgr->CreatureSetModifyHistory(target, source);
        sFreedomMgr->SaveCreature(target);

        handler->PSendSysMessage(FREEDOM_CMDI_NPC_SET_SLEEPTSTATE);
        return true;
    }

    static bool HandleNpcSetDeathStateCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
        {
            handler->PSendSysMessage(FREEDOM_CMDH_NPC_SET_DEATHSTATE);
            return true;
        }

        std::string toggleToken = "";
        Player* source = handler->GetSession()->GetPlayer();
        Creature* target = handler->getSelectedCreature();
        uint64 guidLow = target ? target->GetSpawnId() : sFreedomMgr->GetSelectedCreatureGuidFromPlayer(source->GetGUID().GetCounter());

        AdvancedArgumentTokenizer tokenizer(args);
        toggleToken = tokenizer.TryGetParam(0);

        if (tokenizer.size() > 1)
        {
            guidLow = tokenizer.TryGetParam<uint64>(1, "Hcreature");
        }

        if (toggleToken != "on" && toggleToken != "off")
        {
            handler->PSendSysMessage(FREEDOM_CMDH_NPC_SET_DEATHSTATE);
            return true;
        }

        target = sFreedomMgr->GetAnyCreature(guidLow);

        if (!target)
        {
            handler->PSendSysMessage(FREEDOM_CMDE_CREATURE_NOT_FOUND);
            return true;
        }

        bool toggle = (toggleToken == "on");

        sFreedomMgr->CreatureSetAuraToggle(target, uint32(SPELL_PERMANENT_HOVER), false);
        sFreedomMgr->CreatureSetBytes1(target, toggle ? UNIT_STAND_STATE_DEAD : 0);
        sFreedomMgr->CreatureSetModifyHistory(target, source);
        sFreedomMgr->SaveCreature(target);

        handler->PSendSysMessage(FREEDOM_CMDI_NPC_SET_DEATHSTATE);
        return true;
    }

    static bool HandleNpcSetAuraCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
        {
            handler->PSendSysMessage(FREEDOM_CMDH_NPC_SET_AURA);
            return true;
        }

        uint32 auraId = 0;
        std::string toggleToken = "";
        Player* source = handler->GetSession()->GetPlayer();
        Creature* target = handler->getSelectedCreature();
        uint64 guidLow = target ? target->GetSpawnId() : sFreedomMgr->GetSelectedCreatureGuidFromPlayer(source->GetGUID().GetCounter());

        AdvancedArgumentTokenizer tokenizer(args);
        auraId = tokenizer.TryGetParam<uint32>(0);
        toggleToken = tokenizer.TryGetParam(1);

        if (tokenizer.size() > 2)
        {
            guidLow = tokenizer.TryGetParam<uint64>(2, "Hcreature");
        }

        if (toggleToken != "on" && toggleToken != "off")
        {
            handler->PSendSysMessage(FREEDOM_CMDH_NPC_SET_AURA);
            return true;
        }

        target = sFreedomMgr->GetAnyCreature(guidLow);

        if (!target)
        {
            handler->PSendSysMessage(FREEDOM_CMDE_CREATURE_NOT_FOUND);
            return true;
        }

        sFreedomMgr->CreatureSetAuraToggle(target, auraId, toggleToken == "on");
        sFreedomMgr->CreatureSetModifyHistory(target, source);
        sFreedomMgr->SaveCreature(target);

        handler->PSendSysMessage(FREEDOM_CMDI_NPC_SET_AURA);
        return true;
    }

    static bool HandleNpcSetMountCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
        {
            handler->PSendSysMessage(FREEDOM_CMDH_NPC_SET_MOUNT);
            return true;
        }

        uint32 mountId = 0;
        Player* source = handler->GetSession()->GetPlayer();
        Creature* target = handler->getSelectedCreature();
        uint64 guidLow = target ? target->GetSpawnId() : sFreedomMgr->GetSelectedCreatureGuidFromPlayer(source->GetGUID().GetCounter());

        AdvancedArgumentTokenizer tokenizer(args);
        mountId = tokenizer.TryGetParam<uint32>(0);

        if (tokenizer.size() > 1)
        {
            guidLow = tokenizer.TryGetParam<uint64>(1, "Hcreature");
        }

        target = sFreedomMgr->GetAnyCreature(guidLow);

        if (!target)
        {
            handler->PSendSysMessage(FREEDOM_CMDE_CREATURE_NOT_FOUND);
            return true;
        }

        sFreedomMgr->CreatureSetMount(target, mountId);
        sFreedomMgr->CreatureSetModifyHistory(target, source);
        sFreedomMgr->SaveCreature(target);

        handler->PSendSysMessage(FREEDOM_CMDI_NPC_SET_MOUNT);
        return true;
    }

    static bool HandleNpcSetEmoteCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
        {
            handler->PSendSysMessage(FREEDOM_CMDH_NPC_SET_EMOTE);
            return true;
        }

        uint32 emoteId = 0;
        Player* source = handler->GetSession()->GetPlayer();
        Creature* target = handler->getSelectedCreature();
        uint64 guidLow = target ? target->GetSpawnId() : sFreedomMgr->GetSelectedCreatureGuidFromPlayer(source->GetGUID().GetCounter());

        AdvancedArgumentTokenizer tokenizer(args);
        emoteId = tokenizer.TryGetParam<uint32>(0);

        if (tokenizer.size() > 1)
        {
            guidLow = tokenizer.TryGetParam<uint64>(1, "Hcreature");
        }

        target = sFreedomMgr->GetAnyCreature(guidLow);

        if (!target)
        {
            handler->PSendSysMessage(FREEDOM_CMDE_CREATURE_NOT_FOUND);
            return true;
        }

        sFreedomMgr->CreatureSetEmote(target, emoteId);
        sFreedomMgr->CreatureSetModifyHistory(target, source);
        sFreedomMgr->SaveCreature(target);

        handler->PSendSysMessage(FREEDOM_CMDI_NPC_SET_EMOTE);
        return true;
    }

    static bool HandleNpcSelectCommand(ChatHandler* handler, char const* args)
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
            AdvancedArgumentTokenizer tokenizer(args);
            entryId = tokenizer.TryGetParam<uint32>(0, "Hcreature_entry");
        }

        if (entryId)
        {
            PreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_SEL_NEAREST_CREATURE_BY_EID);
            stmt->setFloat(index++, x);
            stmt->setFloat(index++, y);
            stmt->setFloat(index++, z);
            stmt->setUInt16(index++, mapId);
            stmt->setUInt32(index++, entryId);

            result = WorldDatabase.Query(stmt);
        }
        else
        {
            PreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_SEL_NEAREST_CREATURE);
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
            entryId = fields[1].GetUInt32();
            double creatureDist = fields[2].GetDouble();

            CreatureTemplate const* creatureTemplate = sObjectMgr->GetCreatureTemplate(entryId);

            if (!creatureTemplate)
            {
                handler->PSendSysMessage(FREEDOM_CMDE_CREATURE_SELECT_ENTRY_ID, entryId, guidLow);
                return true;
            }

            Creature* creature = sFreedomMgr->GetAnyCreature(source->GetMap(), guidLow, entryId);

            if (!creature)
            {
                handler->PSendSysMessage(FREEDOM_CMDE_CREATURE_SELECT_NOT_IN_WORLD, guidLow, entryId);
                return true;
            }

            sFreedomMgr->SetCreatureSelectionForPlayer(source->GetGUID().GetCounter(), guidLow);
            handler->PSendSysMessage(FREEDOM_CMDI_CREATURE_SELECT,
                sFreedomMgr->ToChatLink("Hcreature", guidLow, creatureTemplate->Name),
                guidLow,
                entryId,
                creatureDist);

            return true;
        }
        else
        {
            handler->PSendSysMessage(FREEDOM_CMDE_CREATURE_NOT_FOUND);
            return true;
        }
    }

    //add spawn of creature
    static bool HandleNpcAddCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
        {
            handler->PSendSysMessage(FREEDOM_CMDH_NPC_ADD);
            return true;
        }

        Player* source = handler->GetSession()->GetPlayer();

        AdvancedArgumentTokenizer tokenizer(args);
        uint32 entryId = tokenizer.TryGetParam<uint32>(0, "Hcreature_entry");
        auto creatureTemplate = sObjectMgr->GetCreatureTemplate(entryId);

        if (!creatureTemplate)
        {
            handler->PSendSysMessage(FREEDOM_CMDE_CREATURE_NOT_FOUND);
            return true;
        }
        
        auto creature = sFreedomMgr->CreatureCreate(source, creatureTemplate);

        if (!creature)
        {
            handler->PSendSysMessage(FREEDOM_CMDE_NPC_ADD_FAIL, entryId);
            return true;
        }

        handler->PSendSysMessage(FREEDOM_CMDI_NPC_ADD,
            sFreedomMgr->ToChatLink("Hcreature", creature->GetSpawnId(), creatureTemplate->Name),
            creature->GetSpawnId(),
            entryId);
        sFreedomMgr->SetCreatureSelectionForPlayer(source->GetGUID().GetCounter(), creature->GetSpawnId());
        return true;
    }

    //add item in vendorlist
    static bool HandleNpcAddVendorItemCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
        {
            handler->PSendSysMessage(FREEDOM_CMDH_NPC_ADD_ITEM);
            return true;
        }

        Player* source = handler->GetSession()->GetPlayer();

        AdvancedArgumentTokenizer tokenizer(args);
        uint32 itemId = tokenizer.TryGetParam<uint32>(0, "Hitem");
        uint32 maxCount = tokenizer.TryGetParam<uint32>(1);
        uint32 incrTime = tokenizer.TryGetParam<uint32>(2);
        uint32 extendedCost = tokenizer.TryGetParam<uint32>(3);
        const uint8 type = 1; // FIXME: make type (1 item, 2 currency) an argument

        if (!itemId)
        {
            handler->SendSysMessage(FREEDOM_CMDE_INVALID_ITEM_ID);
            return true;
        }

        Creature* vendor = handler->getSelectedCreature();
        if (!vendor || !vendor->IsVendor())
        {
            handler->SendSysMessage(FREEDOM_CMDE_CREATURE_SELECT_VENDOR);
            return true;
        }

        uint32 vendor_entry = vendor->GetEntry();

        if (!sObjectMgr->IsVendorItemValid(vendor_entry, itemId, maxCount, incrTime, extendedCost, type, handler->GetSession()->GetPlayer()))
        {
            handler->PSendSysMessage(FREEDOM_CMDE_NPC_ADD_ITEM_CANNOT_ADD);
            return true;
        }

        sObjectMgr->AddVendorItem(vendor_entry, itemId, maxCount, incrTime, extendedCost, type);
        sFreedomMgr->CreatureSetModifyHistory(vendor, source);
        sFreedomMgr->SaveCreature(vendor);

        ItemTemplate const* itemTemplate = sObjectMgr->GetItemTemplate(itemId);

        handler->PSendSysMessage(FREEDOM_CMDI_NPC_ADD_ITEM, 
            sFreedomMgr->ToChatLink("Hitem", itemId, itemTemplate->GetDefaultLocaleName()),
            sFreedomMgr->ToChatLink("Hcreature", vendor->GetSpawnId(), vendor->GetName()),
            maxCount,
            incrTime,
            extendedCost);
        return true;
    }

    static bool HandleNpcSetScaleCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
        {
            handler->PSendSysMessage(FREEDOM_CMDH_NPC_SET_SCALE);
            return true;
        }

        float scale = 0;
        Player* source = handler->GetSession()->GetPlayer();
        Creature* target = handler->getSelectedCreature();
        uint64 guidLow = target ? target->GetSpawnId() : sFreedomMgr->GetSelectedCreatureGuidFromPlayer(source->GetGUID().GetCounter());

        AdvancedArgumentTokenizer tokenizer(args);
        scale = tokenizer.TryGetParam<float>(0);
        
        if (tokenizer.size() > 1)
        {
            guidLow = tokenizer.TryGetParam<uint64>(1, "Hcreature");
        }

        target = sFreedomMgr->GetAnyCreature(guidLow);

        if (!target)
        {
            handler->PSendSysMessage(FREEDOM_CMDE_CREATURE_NOT_FOUND);
            return true;
        }

        float maxScale = sConfigMgr->GetFloatDefault("Freedom.Creature.MaxScale", 15.0f);
        float minScale = sConfigMgr->GetFloatDefault("Freedom.Creature.MinScale", 0.0f);

        if (scale < minScale)
            scale = minScale;
        if (scale > maxScale)
            scale = maxScale;

        sFreedomMgr->CreatureScale(target, scale);
        sFreedomMgr->CreatureSetModifyHistory(target, source);
        sFreedomMgr->SaveCreature(target);

        handler->PSendSysMessage(FREEDOM_CMDI_NPC_SET_SCALE, scale);
        return true;
    }

    static bool HandleNpcSetAllowMovementCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
        {
            handler->PSendSysMessage(FREEDOM_CMDH_NPC_SET_ALLOWMOVEMENT);
            return true;
        }

        AdvancedArgumentTokenizer tokenizer(args);
        std::string toggleToken = tokenizer.TryGetParam(0);

        if (toggleToken != "on" && toggleToken != "off")
        {
            handler->PSendSysMessage(FREEDOM_CMDH_NPC_SET_ALLOWMOVEMENT);
            return true;
        }

        if (toggleToken == "off")
        {
            sWorld->SetAllowMovement(false);
            handler->SendSysMessage(FREEDOM_CMDI_NPC_SET_ALLOWMOVEMENT_OFF);
        }
        else
        {
            sWorld->SetAllowMovement(true);
            handler->SendSysMessage(FREEDOM_CMDI_NPC_SET_ALLOWMOVEMENT_ON);
        }
        return true;
    }

    static bool HandleNpcSetEntryCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
        {
            handler->PSendSysMessage(FREEDOM_CMDH_NPC_SET_ENTRY);
            return true;
        }

        Player* source = handler->GetSession()->GetPlayer();
        Creature* target = handler->getSelectedCreature();
        uint64 guidLow = target ? target->GetSpawnId() : sFreedomMgr->GetSelectedCreatureGuidFromPlayer(source->GetGUID().GetCounter());

        AdvancedArgumentTokenizer tokenizer(args);
        uint32 newEntryNum = tokenizer.TryGetParam<uint32>(0, "Hcreature_entry");

        if (!newEntryNum)
        {
            handler->PSendSysMessage(FREEDOM_CMDH_NPC_SET_ENTRY);
            return true;
        }

        if (tokenizer.size() > 1)
        {
            guidLow = tokenizer.TryGetParam<uint64>(1, "Hcreature");
        }       

        target = sFreedomMgr->GetAnyCreature(guidLow);

        if (!target || target->GetTypeId() != TYPEID_UNIT)
        {
            handler->PSendSysMessage(FREEDOM_CMDE_CREATURE_NOT_FOUND);
            return true;
        }

        if (target->UpdateEntry(newEntryNum))
            handler->SendSysMessage(FREEDOM_CMDI_NPC_SET_ENTRY);
        else
            handler->SendSysMessage(FREEDOM_CMDE_NPC_SET_ENTRY_FAIL);

        sFreedomMgr->CreatureSetModifyHistory(target, source);
        sFreedomMgr->SaveCreature(target);
        return true;
    }

    //change level of creature or pet
    static bool HandleNpcSetLevelCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
        {
            handler->PSendSysMessage(FREEDOM_CMDH_NPC_SET_LEVEL);
            return true;
        }

        Player* source = handler->GetSession()->GetPlayer();
        Creature* target = handler->getSelectedCreature();
        uint64 guidLow = target ? target->GetSpawnId() : sFreedomMgr->GetSelectedCreatureGuidFromPlayer(source->GetGUID().GetCounter());

        AdvancedArgumentTokenizer tokenizer(args);
        uint8 lvl = (uint8)tokenizer.TryGetParam<uint32>(0);

        if (tokenizer.size() > 1)
        {
            guidLow = tokenizer.TryGetParam<uint64>(1, "Hcreature");
        }

        target = sFreedomMgr->GetAnyCreature(guidLow);

        if (!target)
        {
            handler->PSendSysMessage(FREEDOM_CMDE_CREATURE_NOT_FOUND);
            return true;
        }

        uint8 maxLvl = sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL) + 3;
        if (lvl < 1 || lvl > maxLvl)
        {
            handler->PSendSysMessage(FREEDOM_CMDE_NPC_SET_LEVEL_BAD_VALUE, 1, maxLvl);
            return true;
        }

        if (target->IsPet())
        {
            if (((Pet*)target)->getPetType() == HUNTER_PET)
            {
                target->SetUInt32Value(UNIT_FIELD_PETNEXTLEVELEXP, sObjectMgr->GetXPForLevel(lvl) / 4);
                target->SetUInt32Value(UNIT_FIELD_PETEXPERIENCE, 0);
            }
            ((Pet*)target)->GivePetLevel(lvl);
        }
        else
        {
            target->SetMaxHealth(100 + 30 * lvl);
            target->SetHealth(100 + 30 * lvl);
            target->SetLevel(lvl);

            sFreedomMgr->CreatureSetModifyHistory(target, source);
            sFreedomMgr->SaveCreature(target);
        }

        handler->PSendSysMessage(FREEDOM_CMDI_NPC_SET_LEVEL);
        return true;
    }

    static bool HandleNpcDeleteCommand(ChatHandler* handler, char const* args)
    {
        Player* source = handler->GetSession()->GetPlayer();
        Creature* target = handler->getSelectedCreature();
        uint64 guidLow = target ? target->GetSpawnId() : sFreedomMgr->GetSelectedCreatureGuidFromPlayer(source->GetGUID().GetCounter());
       
        if (*args)
        {
            AdvancedArgumentTokenizer tokenizer(args);
            guidLow = tokenizer.TryGetParam<uint64>(0, "Hcreature");
            target = nullptr; // remove selected target
        }
        
        if (!target && guidLow)
            target = sFreedomMgr->GetAnyCreature(guidLow);

        if (!target)
        {
            handler->PSendSysMessage(FREEDOM_CMDE_CREATURE_NOT_FOUND);
            return true;
        }

        if (target->IsPet() || target->IsTotem())
        {
            handler->SendSysMessage(FREEDOM_CMDE_CREATURE_CANNOT_BE_PET_OR_TOTEM);
            return true;
        }

        sFreedomMgr->CreatureDelete(target);

        handler->SendSysMessage(FREEDOM_CMDI_NPC_DELETE);
        return true;
    }

    //del item from vendor list
    static bool HandleNpcDeleteVendorItemCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
        {
            handler->SendSysMessage(FREEDOM_CMDH_NPC_DELETE_ITEM);
            return true;
        }

        Creature* vendor = handler->getSelectedCreature();
        if (!vendor || !vendor->IsVendor())
        {
            handler->SendSysMessage(FREEDOM_CMDE_CREATURE_SELECT_VENDOR);
            return true;
        }

        AdvancedArgumentTokenizer tokenizer(args);
        uint32 itemId = tokenizer.TryGetParam<uint32>(0, "Hitem");

        if (!itemId)
        {
            handler->SendSysMessage(FREEDOM_CMDE_INVALID_ITEM_ID);
            return true;
        }

        const uint8 type = 1; // FIXME: make type (1 item, 2 currency) an argument

        if (!sObjectMgr->RemoveVendorItem(vendor->GetEntry(), itemId, type))
        {
            handler->PSendSysMessage(FREEDOM_CMDE_NPC_DELETE_ITEM_NOT_IN_LIST, itemId);
            return true;
        }

        ItemTemplate const* itemTemplate = sObjectMgr->GetItemTemplate(itemId);

        handler->PSendSysMessage(FREEDOM_CMDI_NPC_DELETE_ITEM, 
            sFreedomMgr->ToChatLink("Hitem", itemId, itemTemplate->GetDefaultLocaleName()),
            itemId);
        return true;
    }

    //set faction of creature
    static bool HandleNpcSetFactionIdCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
        {
            handler->PSendSysMessage(FREEDOM_CMDH_NPC_SET_FACTION_ID);
            return true;
        }

        uint32 factionId = 0;
        Player* source = handler->GetSession()->GetPlayer();
        Creature* target = handler->getSelectedCreature();
        uint64 guidLow = target ? target->GetSpawnId() : sFreedomMgr->GetSelectedCreatureGuidFromPlayer(source->GetGUID().GetCounter());

        AdvancedArgumentTokenizer tokenizer(args);
        factionId = tokenizer.TryGetParam<uint32>(0);

        if (tokenizer.size() > 1)
        {
            guidLow = tokenizer.TryGetParam<uint64>(1, "Hcreature");
            target = nullptr; // remove selected target
        }

        // Only fetch new target if selected target does not exist but we still managed to get guid
        if (!target && guidLow)
            target = sFreedomMgr->GetAnyCreature(guidLow);

        if (!target)
        {
            handler->PSendSysMessage(FREEDOM_CMDE_CREATURE_NOT_FOUND);
            return true;
        }

        if (!sFactionTemplateStore.LookupEntry(factionId))
        {
            handler->PSendSysMessage(FREEDOM_CMDE_NPC_SET_FACTION_INVALID_ID, factionId);
            return true;
        }

        target->setFaction(factionId);

        // Faction is set in creature_template - not inside creature

        // Update in memory..
        if (CreatureTemplate const* cinfo = target->GetCreatureTemplate())
            const_cast<CreatureTemplate*>(cinfo)->faction = factionId;

        // ..and DB
        PreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_UPD_CREATURE_FACTION);

        stmt->setUInt16(0, uint16(factionId));
        stmt->setUInt32(1, target->GetEntry());

        WorldDatabase.Execute(stmt);

        handler->PSendSysMessage(FREEDOM_CMDI_NPC_SET_FACTION_ID);
        return true;
    }

    //set npcflag of creature
    static bool HandleNpcSetFlagCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
        {
            handler->PSendSysMessage(FREEDOM_CMDH_NPC_SET_FLAG);
            return true;
        }

        uint64 npcFlags = 0;
        Player* source = handler->GetSession()->GetPlayer();
        Creature* target = handler->getSelectedCreature();
        uint64 guidLow = target ? target->GetSpawnId() : sFreedomMgr->GetSelectedCreatureGuidFromPlayer(source->GetGUID().GetCounter());

        AdvancedArgumentTokenizer tokenizer(args);
        npcFlags = tokenizer.TryGetParam<uint64>(0);

        if (tokenizer.size() > 1)
        {
            guidLow = tokenizer.TryGetParam<uint64>(1, "Hcreature");
        }

        target = sFreedomMgr->GetAnyCreature(guidLow);

        if (!target)
        {
            handler->PSendSysMessage(FREEDOM_CMDE_CREATURE_NOT_FOUND);
            return true;
        }

        target->SetUInt64Value(UNIT_NPC_FLAGS, npcFlags);

        PreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_UPD_CREATURE_NPCFLAG);

        stmt->setUInt64(0, npcFlags);
        stmt->setUInt32(1, target->GetEntry());

        WorldDatabase.Execute(stmt);

        handler->SendSysMessage(FREEDOM_CMDI_NPC_SET_FLAG);

        return true;
    }

    //set data of creature for testing scripting
    static bool HandleNpcSetDataCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
        {
            handler->PSendSysMessage(FREEDOM_CMDH_NPC_SET_DATA);
            return true;
        }

        uint32 field = 0;
        uint32 data = 0;
        Player* source = handler->GetSession()->GetPlayer();
        Creature* target = handler->getSelectedCreature();
        uint64 guidLow = target ? target->GetSpawnId() : sFreedomMgr->GetSelectedCreatureGuidFromPlayer(source->GetGUID().GetCounter());

        AdvancedArgumentTokenizer tokenizer(args);
        field = tokenizer.TryGetParam<uint32>(0);
        data = tokenizer.TryGetParam<uint32>(1);

        if (!field || !data)
        {
            handler->PSendSysMessage(FREEDOM_CMDH_NPC_SET_DATA);
            return true;
        }

        if (tokenizer.size() > 2)
        {
            guidLow = tokenizer.TryGetParam<uint64>(2, "Hcreature");
        }

        target = sFreedomMgr->GetAnyCreature(guidLow);

        if (!target)
        {
            handler->PSendSysMessage(FREEDOM_CMDE_CREATURE_NOT_FOUND);
            return true;
        }

        target->AI()->SetData(field, data);
        std::string AIorScript = !target->GetAIName().empty() ? "AI type: " + target->GetAIName() : 
            (!target->GetScriptName().empty() ? "Script Name: " + target->GetScriptName() : "No AI or Script Name Set");
        handler->PSendSysMessage(FREEDOM_CMDI_NPC_SET_DATA, 
            sFreedomMgr->ToChatLink("Hcreature", guidLow, target->GetName()),
            field,
            data, 
            AIorScript.c_str());
        return true;
    }

    //npc follow handling
    static bool HandleNpcFollowCommand(ChatHandler* handler, char const* /*args*/)
    {
        Player* player = handler->GetSession()->GetPlayer();
        Creature* creature = handler->getSelectedCreature();

        if (!creature)
        {
            handler->PSendSysMessage(FREEDOM_CMDE_MANUAL_SELECT_CREATURE);
            return true;
        }

        // Follow player - Using pet's default dist and angle
        creature->GetMotionMaster()->MoveFollow(player, PET_FOLLOW_DIST, creature->GetFollowAngle());

        handler->PSendSysMessage(FREEDOM_CMDI_NPC_FOLLOW, sFreedomMgr->ToChatLink("Hcreature", creature->GetSpawnId(), creature->GetName()));
        return true;
    }

    static bool HandleNpcInfoCommand(ChatHandler* handler, char const* args)
    {
        Player* source = handler->GetSession()->GetPlayer();
        Creature* target = handler->getSelectedCreature();
        uint64 guidLow = target ? target->GetSpawnId() : sFreedomMgr->GetSelectedCreatureGuidFromPlayer(source->GetGUID().GetCounter());
        bool historyInfo = false;
        bool positionInfo = false;
        bool advancedInfo = false;

        AdvancedArgumentTokenizer tokenizer(*args ? args : "");
        tokenizer.LoadModifier("-full", 0);
        tokenizer.LoadModifier("-pos", 0);
        tokenizer.LoadModifier("-history", 0);
        tokenizer.LoadModifier("-advanced", 0);

        if (tokenizer.size() > 1)
        {
            guidLow = tokenizer.TryGetParam<uint64>(1, "Hcreature");
        }

        target = sFreedomMgr->GetAnyCreature(guidLow);

        if (!target)
        {
            handler->PSendSysMessage(FREEDOM_CMDE_CREATURE_NOT_FOUND);
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

        CreatureTemplate const* cInfo = target->GetCreatureTemplate();
        CreatureExtraData const* extraData = sFreedomMgr->GetCreatureExtraData(guidLow);

        float px, py, pz, po;
        source->GetPosition(px, py, pz, po);
        float cx, cy, cz, co;
        target->GetPosition(cx, cy, cz, co);
        float distance = std::sqrt(std::pow(cx - px, 2) + std::pow(cy - py, 2) + std::pow(cz - pz, 2));

        std::string name = target->GetName();
        uint32 faction = target->getFaction();
        uint64 npcflags = target->GetUInt64Value(UNIT_NPC_FLAGS);
        uint32 mechanicImmuneMask = cInfo->MechanicImmuneMask;
        uint32 displayId = target->GetDisplayId();
        uint32 nativeId = target->GetNativeDisplayId();
        uint32 entryId = cInfo->Entry;
        auto phaseList = target->GetPhases();

        int64 curRespawnDelay = target->GetRespawnTimeEx() - time(NULL);
        if (curRespawnDelay < 0)
            curRespawnDelay = 0;
        std::string curRespawnDelayStr = secsToTimeString(uint64(curRespawnDelay), true);
        std::string defRespawnDelayStr = secsToTimeString(target->GetRespawnDelay(), true);

        handler->PSendSysMessage(FREEDOM_CMDI_CREATURE_INFO, sFreedomMgr->ToChatLink("Hcreature", guidLow, name));
        if (advancedInfo)
            handler->PSendSysMessage(FREEDOM_CMDI_CREATURE_INFO_LI_GUID_FULL, target->GetGUID().ToString());
        handler->PSendSysMessage(FREEDOM_CMDI_CREATURE_INFO_LI_GUID, guidLow);
        handler->PSendSysMessage(FREEDOM_CMDI_CREATURE_INFO_LI_ENTRY, entryId, sFreedomMgr->ToChatLink("Hcreature_entry", entryId, "Spawn link"));
        handler->PSendSysMessage(FREEDOM_CMDI_CREATURE_INFO_LI_DISPLAY_ID, displayId, nativeId);

        if (!phaseList.empty())
        {
            handler->PSendSysMessage(FREEDOM_CMDI_GAMEOBJECT_INFO_LI_PHASELIST);

            for (uint32 phaseId : phaseList)
            {
                handler->PSendSysMessage(FREEDOM_CMDI_GAMEOBJECT_INFO_LI_PHASE_LI, phaseId, sFreedomMgr->GetPhaseMask(phaseId));
            }
        }

        if (advancedInfo)
        {
            handler->PSendSysMessage(FREEDOM_CMDI_CREATURE_INFO_LI_PHASEMASK, extraData ? extraData->phaseMask : target->GetPhaseMask());
            handler->PSendSysMessage(FREEDOM_CMDI_CREATURE_INFO_LI_FACTION_ID, faction);
            handler->PSendSysMessage(FREEDOM_CMDI_CREATURE_INFO_LI_NAME, name);
            handler->PSendSysMessage(FREEDOM_CMDI_CREATURE_INFO_LI_AI_INFO, target->GetAIName());
            handler->PSendSysMessage(FREEDOM_CMDI_CREATURE_INFO_LI_SCRIPT_INFO, target->GetScriptName());            

            handler->PSendSysMessage(FREEDOM_CMDI_CREATURE_INFO_LI_FLAGSE_LIST, cInfo->flags_extra);
            for (uint8 i = 0; i < FLAGS_EXTRA_COUNT; ++i) 
            {
                if (cInfo->flags_extra & flagsExtra[i].Value)
                    handler->PSendSysMessage(FREEDOM_CMDI_CREATURE_INFO_LI_FLAGSE_LI, flagsExtra[i].Name, flagsExtra[i].Value);
            }

            handler->PSendSysMessage(FREEDOM_CMDI_CREATURE_INFO_LI_NPCFLAGS_LIST, cInfo->npcflag);
            for (uint8 i = 0; i < NPCFLAG_COUNT; i++)
            {
                if (npcflags & npcFlagTexts[i].Value)
                {
                    std::string npcFlagTextsName = handler->GetTrinityString(npcFlagTexts[i].Name);
                    if (!npcFlagTextsName.empty())
                    {
                        npcFlagTextsName = "  " + npcFlagTextsName;
                        handler->PSendSysMessage(npcFlagTextsName.c_str(), npcFlagTexts[i].Value);
                    }
                }                    
            }

            handler->PSendSysMessage(FREEDOM_CMDI_CREATURE_INFO_LI_MECHANICIMM_LIST, mechanicImmuneMask);
            for (uint8 i = 1; i < MAX_MECHANIC; ++i)
            {
                if (mechanicImmuneMask & (1 << (mechanicImmunes[i].Value - 1)))
                    handler->PSendSysMessage(FREEDOM_CMDI_CREATURE_INFO_LI_MECHANICIMM_LI, mechanicImmunes[i].Name, mechanicImmunes[i].Value);
            }
                    
            handler->PSendSysMessage(FREEDOM_CMDI_CREATURE_INFO_LI_LEVEL, target->getLevel());
            handler->PSendSysMessage(FREEDOM_CMDI_CREATURE_INFO_LI_EQUIPMENT, target->GetCurrentEquipmentId(), target->GetOriginalEquipmentId());
            handler->PSendSysMessage(FREEDOM_CMDI_CREATURE_INFO_LI_HEALTH, target->GetHealth(), target->GetMaxHealth(), target->GetCreateHealth());
            handler->PSendSysMessage(FREEDOM_CMDI_CREATURE_INFO_LI_INHABIT_TYPE, cInfo->InhabitType);

            handler->PSendSysMessage(FREEDOM_CMDI_CREATURE_INFO_LI_UNITFLAGS_LIST, target->GetUInt32Value(UNIT_FIELD_FLAGS));
            for (uint8 i = 0; i < MAX_UNIT_FLAGS; ++i)
            {
                if (target->GetUInt32Value(UNIT_FIELD_FLAGS) & unitFlags[i].Value)
                    handler->PSendSysMessage(FREEDOM_CMDI_CREATURE_INFO_LI_UNITFLAGS_LI, unitFlags[i].Name, unitFlags[i].Value);
            }
                
            handler->PSendSysMessage(FREEDOM_CMDI_CREATURE_INFO_LI_UNITFLAGS2, target->GetUInt32Value(UNIT_FIELD_FLAGS_2));
            handler->PSendSysMessage(FREEDOM_CMDI_CREATURE_INFO_LI_DYNFLAGS, target->GetUInt32Value(OBJECT_DYNAMIC_FLAGS));
            handler->PSendSysMessage(FREEDOM_CMDI_CREATURE_INFO_LI_FACTION_TEMPLATE, target->getFaction());            
            handler->PSendSysMessage(FREEDOM_CMDI_CREATURE_INFO_LI_RESPAWN_TIMES, defRespawnDelayStr.c_str(), curRespawnDelayStr.c_str());
            handler->PSendSysMessage(FREEDOM_CMDI_CREATURE_INFO_LI_LOOT, cInfo->lootid, cInfo->pickpocketLootId, cInfo->SkinLootId);
            handler->PSendSysMessage(FREEDOM_CMDI_CREATURE_INFO_LI_DUNGEON_ID, target->GetInstanceId());
            handler->PSendSysMessage(FREEDOM_CMDI_CREATURE_INFO_LI_ARMOR, target->GetArmor());
        }

        if (positionInfo)
        {
            handler->PSendSysMessage(FREEDOM_CMDI_CREATURE_INFO_LI_POS_X, cx);
            handler->PSendSysMessage(FREEDOM_CMDI_CREATURE_INFO_LI_POS_Y, cy);
            handler->PSendSysMessage(FREEDOM_CMDI_CREATURE_INFO_LI_POS_Z, cz);
            handler->PSendSysMessage(FREEDOM_CMDI_CREATURE_INFO_LI_ORIENTATION, co);
        }

        if (historyInfo)
        {
            if (!extraData)
            {
                handler->PSendSysMessage(FREEDOM_CMDI_CREATURE_INFO_LI_NO_HISTORY);
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

                handler->PSendSysMessage(FREEDOM_CMDI_CREATURE_INFO_LI_CREATOR_PLAYER, creatorPlayerName, creatorPlayerId);
                if (handler->HasPermission(rbac::RBAC_FPERM_ADMINISTRATION))
                    handler->PSendSysMessage(FREEDOM_CMDI_CREATURE_INFO_LI_CREATOR_ACC, creatorAccountName, creatorBnetId);
                handler->PSendSysMessage(FREEDOM_CMDI_CREATURE_INFO_LI_MODIFIER_PLAYER, modifierPlayerName, modifierPlayerId);
                if (handler->HasPermission(rbac::RBAC_FPERM_ADMINISTRATION))
                    handler->PSendSysMessage(FREEDOM_CMDI_CREATURE_INFO_LI_MODIFIER_ACC, modifierAccountName, modifierBnetId);
                handler->PSendSysMessage(FREEDOM_CMDI_CREATURE_INFO_LI_CREATED, createdTimestamp);
                handler->PSendSysMessage(FREEDOM_CMDI_CREATURE_INFO_LI_MODIFIED, modifiedTimestamp);
            }
        }     

        return true;
    }

    static bool HandleNpcNearCommand(ChatHandler* handler, char const* args)
    {
        float distance = (!*args) ? 10.0f : (float)(atof(args));
        uint32 count = 0;

        Player* player = handler->GetSession()->GetPlayer();
        uint32 maxResults = sWorld->getIntConfig(CONFIG_MAX_RESULTS_LOOKUP_COMMANDS);

        PreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_SEL_NEAREST_CREATURES);
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
                double creatureDist = fields[2].GetDouble();

                CreatureTemplate const* creatureInfo = sObjectMgr->GetCreatureTemplate(entry);

                if (!creatureInfo)
                    continue;

                handler->PSendSysMessage(FREEDOM_CMDI_NPC_NEAR_LIST_ITEM,
                    creatureDist,
                    entry,
                    sFreedomMgr->ToChatLink("Hcreature", guid, creatureInfo->Name));

                ++count;
            } while (result->NextRow());
        }

        if (count == maxResults)
            handler->PSendSysMessage(FREEDOM_CMDI_NPC_NEAR_MAX_RESULT_COUNT, count, distance);
        else
            handler->PSendSysMessage(FREEDOM_CMDI_NPC_NEAR, count, distance);
        return true;
    }

    //move selected creature
    static bool HandleNpcMoveCommand(ChatHandler* handler, char const* args)
    {
        Player* source = handler->GetSession()->GetPlayer();
        Creature* target = handler->getSelectedCreature();
        uint64 guidLow = target ? target->GetSpawnId() : sFreedomMgr->GetSelectedCreatureGuidFromPlayer(source->GetGUID().GetCounter());

        AdvancedArgumentTokenizer tokenizer(*args ? args : "");
        tokenizer.LoadModifier("-guid", 1);
        tokenizer.LoadModifier("-adeg", 1);
        tokenizer.LoadModifier("-sdeg", 1);
        tokenizer.LoadModifier("-co", 0);

        if (tokenizer.ModifierExists("-guid"))
        {
            std::string guidValue = tokenizer.GetModifierValue("-guid", 0);
            std::string guidKey = sFreedomMgr->GetChatLinkKey(guidValue, "Hcreature");
            guidLow = atoul(guidKey.c_str());
        }

        target = sFreedomMgr->GetAnyCreature(guidLow);

        if (!target)
        {
            handler->PSendSysMessage(FREEDOM_CMDE_CREATURE_NOT_FOUND);
            return true;
        }

        float o = source->GetOrientation();
        float oldO = o;

        if (tokenizer.ModifierExists("-adeg"))
        {
            std::string addDeg = tokenizer.GetModifierValue("-adeg", 0);
            o = ((float)atof(addDeg.c_str())) * M_PI / 180.0f + target->GetOrientation();
        }

        if (tokenizer.ModifierExists("-sdeg"))
        {
            std::string setDeg = tokenizer.GetModifierValue("-sdeg", 0);
            o = ((float)atof(setDeg.c_str())) * M_PI / 180.0f;
        }

        if (tokenizer.ModifierExists("-co"))
        {
            o = target->GetOrientation();
        }

        float x, y, z;
        if (tokenizer.empty())
        {
            Player* player = handler->GetSession()->GetPlayer();
            player->GetPosition(x, y, z);
        }
        else
        {
            float add_x = tokenizer.TryGetParam<float>(0);
            float add_y = tokenizer.TryGetParam<float>(1);
            float add_z = tokenizer.TryGetParam<float>(2);
            // rotation matrix
            x = add_x*cos(oldO) - add_y*sin(oldO) + target->GetPositionX();
            y = add_x*sin(oldO) + add_y*cos(oldO) + target->GetPositionY();
            z = add_z + target->GetPositionZ();
        }

        sFreedomMgr->CreatureMove(target, x, y, z, o);
        sFreedomMgr->CreatureSetModifyHistory(target, source);
        sFreedomMgr->SaveCreature(target);
        target = sFreedomMgr->CreatureRefresh(target);

        handler->PSendSysMessage(FREEDOM_CMDI_CREATURE_MOVE,
            sFreedomMgr->ToChatLink("Hcreature", guidLow, target->GetName()),
            guidLow);
        return true;
    }

    //turn npc
    static bool HandleNpcTurnCommand(ChatHandler* handler, char const* args)
    {
        Player* source = handler->GetSession()->GetPlayer();
        Creature* target = handler->getSelectedCreature();
        uint64 guidLow = target ? target->GetSpawnId() : sFreedomMgr->GetSelectedCreatureGuidFromPlayer(source->GetGUID().GetCounter());

        AdvancedArgumentTokenizer tokenizer(*args ? args : "");
        tokenizer.LoadModifier("-guid", 1);
        tokenizer.LoadModifier("-adeg", 1);
        tokenizer.LoadModifier("-sdeg", 1);

        if (tokenizer.ModifierExists("-guid"))
        {
            std::string guidValue = tokenizer.GetModifierValue("-guid", 0);
            std::string guidKey = sFreedomMgr->GetChatLinkKey(guidValue, "Hcreature");
            guidLow = atoul(guidKey.c_str());
        }

        target = sFreedomMgr->GetAnyCreature(guidLow);

        if (!target)
        {
            handler->PSendSysMessage(FREEDOM_CMDE_CREATURE_NOT_FOUND);
            return true;
        }

        float o = source->GetOrientation();

        if (tokenizer.ModifierExists("-adeg"))
        {
            std::string addDeg = tokenizer.GetModifierValue("-adeg", 0);
            o = ((float)atof(addDeg.c_str())) * M_PI / 180.0f + target->GetOrientation();
        }

        if (tokenizer.ModifierExists("-sdeg"))
        {
            std::string setDeg = tokenizer.GetModifierValue("-sdeg", 0);
            o = ((float)atof(setDeg.c_str())) * M_PI / 180.0f;
        }

        sFreedomMgr->CreatureTurn(target, o);
        sFreedomMgr->CreatureSetModifyHistory(target, source);
        sFreedomMgr->SaveCreature(target);
        target = sFreedomMgr->CreatureRefresh(target);

        handler->PSendSysMessage(FREEDOM_CMDI_CREATURE_TURNED,
            sFreedomMgr->ToChatLink("Hcreature", guidLow, target->GetName()),
            guidLow);

        return true;
    }

    //play npc emote
    static bool HandleNpcPlayEmoteCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
        {
            handler->PSendSysMessage(FREEDOM_CMDH_NPC_PLAYEMOTE);
            return true;
        }

        uint32 emoteId = 0;
        Player* source = handler->GetSession()->GetPlayer();
        Creature* target = handler->getSelectedCreature();
        uint64 guidLow = target ? target->GetSpawnId() : sFreedomMgr->GetSelectedCreatureGuidFromPlayer(source->GetGUID().GetCounter());

        AdvancedArgumentTokenizer tokenizer(args);
        emoteId = tokenizer.TryGetParam<uint32>(0);

        if (tokenizer.size() > 1)
        {
            guidLow = tokenizer.TryGetParam<uint64>(1, "Hcreature");
        }

        target = sFreedomMgr->GetAnyCreature(guidLow);

        if (!target)
        {
            handler->PSendSysMessage(FREEDOM_CMDE_CREATURE_NOT_FOUND);
            return true;
        }

        target->SetUInt32Value(UNIT_NPC_EMOTESTATE, emoteId);

        handler->PSendSysMessage(FREEDOM_CMDI_NPC_PLAYEMOTE);
        return true;
    }

    //set model of creature
    static bool HandleNpcSetModelCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
        {
            handler->PSendSysMessage(FREEDOM_CMDH_NPC_SET_MODEL);
            return true;
        }

        uint32 displayId = 0;
        Player* source = handler->GetSession()->GetPlayer();
        Creature* target = handler->getSelectedCreature();
        uint64 guidLow = target ? target->GetSpawnId() : sFreedomMgr->GetSelectedCreatureGuidFromPlayer(source->GetGUID().GetCounter());

        AdvancedArgumentTokenizer tokenizer(args);
        displayId = tokenizer.TryGetParam<uint32>(0);

        if (tokenizer.size() > 1)
        {
            guidLow = tokenizer.TryGetParam<uint64>(1, "Hcreature");
        }

        target = sFreedomMgr->GetAnyCreature(guidLow);

        if (!target)
        {
            handler->PSendSysMessage(FREEDOM_CMDE_CREATURE_NOT_FOUND);
            return true;
        }

        target->SetDisplayId(displayId);
        target->SetNativeDisplayId(displayId);

        target->SaveToDB();

        handler->PSendSysMessage(FREEDOM_CMDI_NPC_SET_MODEL);
        return true;
    }

    //npc phase handling
    //change phase of creature
    static bool HandleNpcSetPhaseCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
        {
            handler->PSendSysMessage(FREEDOM_CMDH_CREATURE_PHASE);
            return true;
        }

        Creature* creature = handler->getSelectedCreature();
        Player* source = handler->GetSession()->GetPlayer();
        ObjectGuid::LowType guidLow = creature ? creature->GetSpawnId() : sFreedomMgr->GetSelectedCreatureGuidFromPlayer(source->GetGUID().GetCounter());
        bool parseAsPhaseMasks = true;

        AdvancedArgumentTokenizer tokenizer(args);
        tokenizer.LoadModifier("-ids", 0);
        tokenizer.LoadModifier("-guid", 1);

        if (tokenizer.ModifierExists("-guid"))
        {
            std::string guidValue = tokenizer.GetModifierValue("-guid", 0);
            std::string guidKey = sFreedomMgr->GetChatLinkKey(guidValue, "Hcreature");
            guidLow = atoul(guidKey.c_str());
        }

        if (tokenizer.ModifierExists("-ids"))
        {
            parseAsPhaseMasks = false;
        }

        if (!guidLow)
        {
            handler->PSendSysMessage(FREEDOM_CMDE_CREATURE_ENTRY_NOT_EXISTS);
            return true;
        }

        CreatureData const* creatureData = NULL;

        // by DB guid
        if (creatureData = sObjectMgr->GetCreatureData(guidLow))
            creature = sFreedomMgr->GetAnyCreature(source->GetMap(), guidLow, creatureData->id);

        if (!creature)
        {
            handler->PSendSysMessage(FREEDOM_CMDE_CREATURE_NOT_FOUND);
            return true;
        }

        if (creature->IsPet())
        {
            handler->PSendSysMessage(FREEDOM_CMDE_CREATURE_CANNOT_EXECUTE_ON_PETS);
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
            sFreedomMgr->CreaturePhase(creature, phaseMask);
            sFreedomMgr->CreatureSetModifyHistory(creature, source);
            sFreedomMgr->SaveCreature(creature);
            creature = sFreedomMgr->CreatureRefresh(creature);
        }

        if (phases.empty())
            handler->PSendSysMessage(FREEDOM_CMDE_CREATURE_PHASE_NOT_SET);
        else if (parseAsPhaseMasks)
            handler->PSendSysMessage(FREEDOM_CMDI_CREATURE_PHASE_BITFIELDS, phases);
        else
            handler->PSendSysMessage(FREEDOM_CMDI_CREATURE_PHASE_PHASEIDS, phases);

        return true;
    }

    static bool HandleNpcSayCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
        {
            handler->PSendSysMessage(FREEDOM_CMDH_NPC_SAY);
            return true;
        }

        Creature* target = handler->getSelectedCreature();
        if (!target)
        {
            handler->PSendSysMessage(FREEDOM_CMDE_MANUAL_SELECT_CREATURE);
            return true;
        }

        target->Say(args, LANG_UNIVERSAL);

        // make some emotes
        char lastchar = args[strlen(args) - 1];
        switch (lastchar)
        {
        case '?':   target->HandleEmoteCommand(EMOTE_ONESHOT_QUESTION);      break;
        case '!':   target->HandleEmoteCommand(EMOTE_ONESHOT_EXCLAMATION);   break;
        default:    target->HandleEmoteCommand(EMOTE_ONESHOT_TALK);          break;
        }

        return true;
    }

    //show text emote by creature in chat
    static bool HandleNpcTextEmoteCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
        {
            handler->PSendSysMessage(FREEDOM_CMDH_NPC_TEXTEMOTE);
            return true;
        }

        Creature* target = handler->getSelectedCreature();

        if (!target)
        {
            handler->PSendSysMessage(FREEDOM_CMDE_MANUAL_SELECT_CREATURE);
            return true;
        }

        target->TextEmote(args);

        return true;
    }

    // make npc whisper to player
    static bool HandleNpcWhisperCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
        {
            handler->PSendSysMessage(FREEDOM_CMDH_NPC_WHISPER);
            return true;
        }

        char* receiverToken = strtok((char*)args, " ");
        char* text = strtok(NULL, "");

        if (!receiverToken || !text)
        {
            handler->PSendSysMessage(FREEDOM_CMDH_NPC_WHISPER);
            return true;
        }

        Creature* target = handler->getSelectedCreature();

        if (!target)
        {
            handler->PSendSysMessage(FREEDOM_CMDE_MANUAL_SELECT_CREATURE);
            return true;
        }

        Player* receiver = ObjectAccessor::FindPlayerByName(receiverToken);

        target->Whisper(text, LANG_UNIVERSAL, receiver);
        return true;
    }

    static bool HandleNpcYellCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
        {
            handler->SendSysMessage(FREEDOM_CMDH_NPC_YELL);
            return true;
        }

        Creature* target = handler->getSelectedCreature();
        if (!target)
        {
            handler->PSendSysMessage(FREEDOM_CMDE_MANUAL_SELECT_CREATURE);
            return true;
        }

        target->Yell(args, LANG_UNIVERSAL);

        // make an emote
        target->HandleEmoteCommand(EMOTE_ONESHOT_SHOUT);

        return true;
    }

    //npc unfollow handling
    static bool HandleNpcUnFollowCommand(ChatHandler* handler, char const* /*args*/)
    {
        Player* source = handler->GetSession()->GetPlayer();
        Creature* target = handler->getSelectedCreature();

        if (!target)
        {
            handler->PSendSysMessage(FREEDOM_CMDE_MANUAL_SELECT_CREATURE);
            return true;
        }

        if (/*creature->GetMotionMaster()->empty() ||*/
            target->GetMotionMaster()->GetCurrentMovementGeneratorType() != FOLLOW_MOTION_TYPE)
        {
            handler->PSendSysMessage(FREEDOM_CMDE_NPC_UNFOLLOW_NOT_A_FOLLOWER, 
                sFreedomMgr->ToChatLink("Hcreature", target->GetSpawnId(), target->GetName()));
            return true;
        }

        FollowMovementGenerator<Creature> const* mgen = static_cast<FollowMovementGenerator<Creature> const*>((target->GetMotionMaster()->top()));

        if (mgen->GetTarget() != source)
        {
            handler->PSendSysMessage(FREEDOM_CMDE_NPC_UNFOLLOW_NOT_A_FOLLOWER,
                sFreedomMgr->ToChatLink("Hcreature", target->GetSpawnId(), target->GetName()));
            return true;
        }

        // reset movement
        target->GetMotionMaster()->MovementExpired(true);

        handler->PSendSysMessage(FREEDOM_CMDI_NPC_UNFOLLOW,
            sFreedomMgr->ToChatLink("Hcreature", target->GetSpawnId(), target->GetName()));
        return true;
    }

    static bool HandleNpcAddFormationCommand(ChatHandler* handler, char const* args)
    {
        Player* source = handler->GetSession()->GetPlayer();
        ObjectGuid::LowType leaderGUID = sFreedomMgr->GetSelectedCreatureGuidFromPlayer(source->GetGUID().GetCounter());

        if (*args)
        {
            AdvancedArgumentTokenizer tokenizer(args);
            leaderGUID = tokenizer.TryGetParam<uint64>(0, "Hcreature");
        }

        Creature* creature = handler->getSelectedCreature();
        Creature* leader = nullptr;

        if (auto creatureData = sObjectMgr->GetCreatureData(leaderGUID))
            leader = sFreedomMgr->GetAnyCreature(source->GetMap(), leaderGUID, creatureData->id);

        if (!leader)
        {
            handler->PSendSysMessage(FREEDOM_CMDE_NPC_ADD_FORMATION_LEADER_NOT_FOUND);
            return true;
        }

        if (!creature)
        {
            handler->PSendSysMessage(FREEDOM_CMDE_MANUAL_SELECT_CREATURE);
            return true;
        }

        ObjectGuid::LowType guidLow = creature->GetSpawnId();

        if (creature->GetFormation())
        {
            handler->PSendSysMessage(FREEDOM_CMDE_NPC_ADD_FORMATION_ALREADY_IN, 
                sFreedomMgr->ToChatLink("Hcreature", guidLow, creature->GetName()),
                creature->GetFormation()->GetId());
            return false;
        }

        FormationInfo* group_member;

        group_member = new FormationInfo;
        group_member->follow_angle = (creature->GetAngle(source) - source->GetOrientation()) * 180 / float(M_PI);
        group_member->follow_dist = std::sqrt(std::pow(source->GetPositionX() - creature->GetPositionX(), 2.f) + std::pow(source->GetPositionY() - creature->GetPositionY(), 2.f));
        group_member->leaderGUID = leaderGUID;
        group_member->groupAI = 0;

        sFormationMgr->CreatureGroupMap[guidLow] = group_member;
        creature->SearchFormation();

        PreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_INS_CREATURE_FORMATION);

        stmt->setUInt64(0, leaderGUID);
        stmt->setUInt64(1, guidLow);
        stmt->setFloat(2, group_member->follow_dist);
        stmt->setFloat(3, group_member->follow_angle);
        stmt->setUInt32(4, uint32(group_member->groupAI));

        WorldDatabase.Execute(stmt);

        handler->PSendSysMessage(FREEDOM_CMDI_NPC_ADD_FORMATION, 
            sFreedomMgr->ToChatLink("Hcreature", guidLow, creature->GetName()), 
            leaderGUID);

        return true;
    }

#pragma region UNREFINED_DEFAULT_COMMANDS
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
#pragma endregion
};

void AddSC_fnpc_commandscript()
{
    new fnpc_commandscript();
}