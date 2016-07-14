#include "ScriptMgr.h"
#include "ObjectMgr.h"
#include "AccountMgr.h"
#include "Config.h"
#include "SocialMgr.h"
#include "Chat.h"
#include "Language.h"
#include "World.h"
#include "Player.h"
#include "Opcodes.h"
#include "MovementPackets.h"
#include "MoveSpline.h"
#include "Pet.h"
#include <boost/algorithm/string/predicate.hpp>

enum FreedomCmdAuraSpells
{
    SPELL_PERMANENT_FEIGN_DEATH = 114371,
    SPELL_PERMANENT_SLEEP_VISUAL = 107674,
    SPELL_PERMANENT_HOVER = 138092
};

class freedom_commandscript : public CommandScript
{
public:
    freedom_commandscript() : CommandScript("freedom_commandscript") { }

    std::vector<ChatCommand> GetCommands() const override
    {
        static std::vector<ChatCommand> freedomMorphCommandTable = 
        {
            { "list",           rbac::RBAC_FPERM_COMMAND_FREEDOM_MORPH,             false, &HandleFreedomMorphListCommand,          "" },
            { "add",            rbac::RBAC_FPERM_COMMAND_FREEDOM_MORPH_MODIFY,      false, &HandleFreedomMorphAddCommand,           "" },
            { "delete",         rbac::RBAC_FPERM_COMMAND_FREEDOM_MORPH_MODIFY,      false, &HandleFreedomMorphDelCommand,           "" },
            { "",               rbac::RBAC_FPERM_COMMAND_FREEDOM_MORPH,             false, &HandleFreedomMorphCommand,              "" },
        };

        static std::vector<ChatCommand> freedomTeleportCommandTable = 
        {
            { "list",           rbac::RBAC_FPERM_COMMAND_FREEDOM_TELE,              false, &HandleFreedomTeleListCommand,           "" },
            { "add",            rbac::RBAC_FPERM_COMMAND_FREEDOM_TELE_MODIFY,       false, &HandleFreedomTeleAddCommand,            "" },
            { "delete",         rbac::RBAC_FPERM_COMMAND_FREEDOM_TELE_MODIFY,       false, &HandleFreedomTeleDelCommand,            "" },
            { "",               rbac::RBAC_FPERM_COMMAND_FREEDOM_TELE,              false, &HandleFreedomTeleCommand,               "" },
        };

        static std::vector<ChatCommand> freedomPrivateTeleportCommandTable = 
        {
            { "list",           rbac::RBAC_FPERM_COMMAND_FREEDOM_PTELE,             false, &HandleFreedomPrivateTeleListCommand,    "" },
            { "add",            rbac::RBAC_FPERM_COMMAND_FREEDOM_PTELE,             false, &HandleFreedomPrivateTeleAddCommand,     "" },
            { "delete",         rbac::RBAC_FPERM_COMMAND_FREEDOM_PTELE,             false, &HandleFreedomPrivateTeleDelCommand,     "" },
            { "",               rbac::RBAC_FPERM_COMMAND_FREEDOM_PTELE,             false, &HandleFreedomPrivateTeleCommand,        "" },
        };

        static std::vector<ChatCommand> freedomSpellCommandTable = 
        {
            { "list",           rbac::RBAC_FPERM_COMMAND_FREEDOM_SPELL,             false, &HandleFreedomSpellListCommand,          "" },
            { "add",            rbac::RBAC_FPERM_COMMAND_FREEDOM_SPELL_MODIFY,      false, &HandleFreedomSpellAddCommand,           "" },
            { "delete",         rbac::RBAC_FPERM_COMMAND_FREEDOM_SPELL_MODIFY,      false, &HandleFreedomSpellDelCommand,           "" },
            { "",               rbac::RBAC_FPERM_COMMAND_FREEDOM_SPELL,             false, &HandleFreedomSpellCommand,              "" },
        };

        static std::vector<ChatCommand> freedomReloadCommandTable = 
        {
            { "gameobject",     rbac::RBAC_FPERM_ADMINISTRATION,                    false, &HandleFreedomReloadGameobjectCommand,   "" },
            { "public_tele",    rbac::RBAC_FPERM_ADMINISTRATION,                    false, &HandleFreedomReloadPublicTeleCommand,   "" },
            { "private_tele",   rbac::RBAC_FPERM_ADMINISTRATION,                    false, &HandleFreedomReloadPrivateTeleCommand,  "" },
            { "public_spell",   rbac::RBAC_FPERM_ADMINISTRATION,                    false, &HandleFreedomReloadPublicSpellCommand,  "" },
            { "morphs",         rbac::RBAC_FPERM_ADMINISTRATION,                    false, &HandleFreedomReloadMorphsCommand,       "" },
            { "item",           rbac::RBAC_FPERM_ADMINISTRATION,                    false, &HandleFreedomReloadItemCommand,         "" },
            { "all",            rbac::RBAC_FPERM_ADMINISTRATION,                    false, &HandleFreedomReloadAllCommand,          "" },
        };

        static std::vector<ChatCommand> freedomCommandTable =
        {
            { "hover",          rbac::RBAC_FPERM_COMMAND_FREEDOM_UTILITIES,         false, &HandleFreedomHoverCommand,              "" },
            { "cast",           rbac::RBAC_FPERM_COMMAND_FREEDOM_SPELL,             false, &HandleFreedomSpellCommand,              "" },
            { "summon",         rbac::RBAC_FPERM_COMMAND_FREEDOM_UTILITIES,         false, &HandleFreedomSummonCommand,             "" },
            { "demorph",        rbac::RBAC_FPERM_COMMAND_FREEDOM_UTILITIES,         false, &HandleFreedomDemorphCommand,            "" },
            { "fly",            rbac::RBAC_FPERM_COMMAND_FREEDOM_UTILITIES,         false, &HandleFreedomFlyCommand,                "" },
            { "revive",         rbac::RBAC_FPERM_COMMAND_FREEDOM_UTILITIES,         false, &HandleFreedomReviveCommand,             "" },
            { "unaura",         rbac::RBAC_FPERM_COMMAND_FREEDOM_UTILITIES,         false, &HandleFreedomUnAuraCommand,             "" },
            { "walk",           rbac::RBAC_FPERM_COMMAND_FREEDOM_UTILITIES,         false, &HandleFreedomWalkCommand,               "" },
            { "run",            rbac::RBAC_FPERM_COMMAND_FREEDOM_UTILITIES,         false, &HandleFreedomRunCommand,                "" },
            { "swim",           rbac::RBAC_FPERM_COMMAND_FREEDOM_UTILITIES,         false, &HandleFreedomSwimCommand,               "" },
            { "scale",          rbac::RBAC_FPERM_COMMAND_FREEDOM_UTILITIES,         false, &HandleFreedomScaleCommand,              "" },
            { "drunk",          rbac::RBAC_FPERM_COMMAND_FREEDOM_UTILITIES,         false, &HandleFreedomDrunkCommand,              "" },
            { "waterwalk",      rbac::RBAC_FPERM_COMMAND_FREEDOM_UTILITIES,         false, &HandleFreedomWaterwalkCommand,          "" },
            { "fix",            rbac::RBAC_FPERM_COMMAND_FREEDOM_UTILITIES,         false, &HandleFreedomFixCommand,                "" },
            { "morph",          rbac::RBAC_FPERM_COMMAND_FREEDOM_MORPH,             false, NULL,                                    "", freedomMorphCommandTable },
            { "mailbox",        rbac::RBAC_FPERM_COMMAND_FREEDOM_UTILITIES,         false, &HandleFreedomMailboxCommand,            "" },
            { "money",          rbac::RBAC_FPERM_COMMAND_FREEDOM_UTILITIES,         false, &HandleFreedomMoneyCommand,              "" },
            { "bank",           rbac::RBAC_FPERM_COMMAND_FREEDOM_UTILITIES,         false, &HandleFreedomBankCommand,               "" },
            { "customize",      rbac::RBAC_FPERM_COMMAND_FREEDOM_UTILITIES,         false, &HandleFreedomCustomizeCommand,          "" },
            { "racechange",     rbac::RBAC_FPERM_COMMAND_FREEDOM_UTILITIES,         false, &HandleFreedomRaceChangeCommand,         "" },
            { "factionchange",  rbac::RBAC_FPERM_COMMAND_FREEDOM_UTILITIES,         false, &HandleFreedomFactionChangeCommand,      "" },
            { "teleport",       rbac::RBAC_FPERM_COMMAND_FREEDOM_TELE,              false, NULL,                                    "", freedomTeleportCommandTable },
            { "pteleport",      rbac::RBAC_FPERM_COMMAND_FREEDOM_PTELE,             false, NULL,                                    "", freedomPrivateTeleportCommandTable },
            { "spell",          rbac::RBAC_FPERM_COMMAND_FREEDOM_SPELL,             false, NULL,                                    "", freedomSpellCommandTable },
            { "speed",          rbac::RBAC_FPERM_COMMAND_FREEDOM_UTILITIES,         false, &HandleFreedomSpeedCommand,              "" },
            { "reload",         rbac::RBAC_FPERM_ADMINISTRATION,                    false, NULL,                                    "", freedomReloadCommandTable },
            { "tabard",         rbac::RBAC_FPERM_COMMAND_FREEDOM_UTILITIES,         false, &HandleFreedomTabardCommand,               "" },
            { "tame",           rbac::RBAC_FPERM_COMMAND_FREEDOM_UTILITIES,         false, &HandleFreedomTameCommand,               "" },
        };

        static std::vector<ChatCommand> commandTable =
        {
            { "freedom",            rbac::RBAC_FPERM_COMMAND_FREEDOM,                    false, NULL,                                "", freedomCommandTable },
        };
        return commandTable;
    }

    static void ModifyMovementSpeed(ChatHandler* handler, UnitMoveType type, float value)
    {
        float max = sConfigMgr->GetFloatDefault("Freedom.Modify.MaxSpeed", 10.0f);
        float min = sConfigMgr->GetFloatDefault("Freedom.Modify.MinSpeed", 0.01f);
        std::string speedName;

        if (value < min || value > max)
        {
            handler->PSendSysMessage(FREEDOM_CMDE_VALUE_OUT_OF_RANGE, max, min);
            return;
        }

        switch (type)
        {
        case UnitMoveType::MOVE_FLIGHT:
        case UnitMoveType::MOVE_FLIGHT_BACK:
            speedName = "fly";
            handler->GetSession()->GetPlayer()->SetSpeed(MOVE_FLIGHT, value);
            handler->GetSession()->GetPlayer()->SetSpeed(MOVE_FLIGHT_BACK, value);
            break;
        case UnitMoveType::MOVE_RUN:
            speedName = "run";
            handler->GetSession()->GetPlayer()->SetSpeed(MOVE_RUN, value);
            break;
        case UnitMoveType::MOVE_SWIM:
        case UnitMoveType::MOVE_SWIM_BACK:
            speedName = "swim";
            handler->GetSession()->GetPlayer()->SetSpeed(MOVE_SWIM, value);
            handler->GetSession()->GetPlayer()->SetSpeed(MOVE_SWIM_BACK, value);
            break;
        case UnitMoveType::MOVE_WALK:
        case UnitMoveType::MOVE_RUN_BACK:
            speedName = "walk";
            handler->GetSession()->GetPlayer()->SetSpeed(MOVE_WALK, value);
            handler->GetSession()->GetPlayer()->SetSpeed(MOVE_RUN_BACK, value);
            break;
        default:
            return;
        }

        handler->PSendSysMessage(FREEDOM_CMDI_MOD_SPEED, speedName.c_str(), value);
    }

#pragma region COMMAND TABLE : .freedom -> morph -> *
    static bool HandleFreedomMorphListCommand(ChatHandler* handler, char const* args)
    {
        const MorphDataContainer morphList = sFreedomMgr->GetMorphContainer(handler->GetSession()->GetPlayer()->GetGUID().GetCounter());
        uint64 count = 0;

        if (!*args)
        {
            for (auto morphData : morphList)
            {
                handler->PSendSysMessage(FREEDOM_CMDI_MORPH_LIST_ITEM, morphData.displayId, morphData.name);
                count++;
            }
        }
        else
        {
            Tokenizer tokens = Tokenizer(args, ' ');
            std::string name = tokens[0];

            for (auto morphData : morphList)
            {
                if (boost::istarts_with(morphData.name, name))
                {
                    handler->PSendSysMessage(FREEDOM_CMDI_MORPH_LIST_ITEM, morphData.displayId, morphData.name);
                    count++;
                }
            }
        }

        if (count == 0)
            handler->PSendSysMessage(FREEDOM_CMDI_X_NOT_FOUND, "Morphs");
        else
            handler->PSendSysMessage(FREEDOM_CMDI_SEARCH_QUERY_RESULT, count);

        return true;
    }

    static bool HandleFreedomMorphAddCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
        {
            handler->PSendSysMessage(FREEDOM_CMDH_MORPH_ADD);
            return true;
        }

        Tokenizer tokens = Tokenizer(args, ' ');

        if (tokens.size() < 2)
        {
            handler->PSendSysMessage(FREEDOM_CMDE_NOT_ENOUGH_PARAMS);
            handler->PSendSysMessage(FREEDOM_CMDH_MORPH_ADD);
            return true;
        }

        std::string morphName = tokens[0];
        uint32 displayId = atoul(tokens[1]);
        Player* source = handler->GetSession()->GetPlayer();
        std::string targetNameArg = tokens.size() > 2 ? tokens[2] : "";
        std::string targetName;
        ObjectGuid targetGuid;

        if (!displayId)
        {
            handler->PSendSysMessage(FREEDOM_CMDE_INVALID_ARGUMENT_X, "$displayId");
            return true;
        }

        if (!handler->extractPlayerTarget(&targetNameArg[0], nullptr, &targetGuid, &targetName))
            return true;

        // Check if morph already exists
        const MorphData* morphDataByName = sFreedomMgr->GetMorphByName(targetGuid.GetCounter(), morphName);
        const MorphData* morphDataByDisplayId = sFreedomMgr->GetMorphByDisplayId(targetGuid.GetCounter(), displayId);

        if (morphDataByName)
        {
            handler->PSendSysMessage(FREEDOM_CMDE_X_WITH_NAME_ALREADY_EXISTS, "Morph", morphName);
            return true;
        }

        if (morphDataByDisplayId)
        {
            handler->PSendSysMessage(FREEDOM_CMDE_X_WITH_ID_ALREADY_EXISTS, "Morph", displayId);
            return true;
        }

        // Create teleport
        MorphData newMorphData;
        newMorphData.name = morphName;
        newMorphData.displayId = displayId;
        newMorphData.gmBnetAccId = source->GetSession()->GetBattlenetAccountId();

        sFreedomMgr->AddMorph(targetGuid.GetCounter(), newMorphData);

        handler->PSendSysMessage(FREEDOM_CMDI_MORPH_ADD, morphName, displayId, targetName);
        return true;
    }

    static bool HandleFreedomMorphDelCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
        {
            handler->PSendSysMessage(FREEDOM_CMDH_MORPH_DEL);
            return true;
        }

        Tokenizer tokens = Tokenizer(args, ' ');
        std::string morphName = tokens[0];
        uint32 displayId = atoul(tokens[0]);
        Player* source = handler->GetSession()->GetPlayer();
        std::string targetNameArg = tokens.size() > 1 ? tokens[1] : "";
        std::string targetName;
        ObjectGuid targetGuid;

        if (!handler->extractPlayerTarget(&targetNameArg[0], nullptr, &targetGuid, &targetName))
            return true;

        // Check if morph actually exists
        const MorphData* morphData = sFreedomMgr->GetMorphByName(targetGuid.GetCounter(), morphName);

        if (displayId && !morphData) // get by displayId only if name search doesn't turn up anything
        {
            morphData = sFreedomMgr->GetMorphByDisplayId(targetGuid.GetCounter(), displayId);

            if (!morphData)
            {
                handler->PSendSysMessage(FREEDOM_CMDE_X_WITH_ID_NOT_FOUND, "Morph", displayId);
                return true;
            }
        }
        else if (!morphData)
        {
            handler->PSendSysMessage(FREEDOM_CMDE_X_WITH_NAME_NOT_FOUND, "Morph", morphName);
            return true;
        }

        morphName = morphData->name;
        displayId = morphData->displayId;

        sFreedomMgr->DeleteMorphByName(targetGuid.GetCounter(), morphData->name);

        handler->PSendSysMessage(FREEDOM_CMDI_MORPH_DEL, morphName, displayId, targetName);
        return true;
    }

    static bool HandleFreedomMorphCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
        {
            handler->PSendSysMessage(FREEDOM_CMDH_MORPH);
            return true;
        }

        Tokenizer tokens = Tokenizer(args, ' ');
        std::string morphName = tokens[0];
        uint32 displayId = atoul(tokens[0]);
        Player* source = handler->GetSession()->GetPlayer();

        // Check if morph actually exists
        const MorphData* morphData = sFreedomMgr->GetMorphByName(source->GetGUID().GetCounter(), morphName);

        if (displayId && !morphData) // get by displayId only if name search doesn't turn up anything
        {
            morphData = sFreedomMgr->GetMorphByDisplayId(source->GetGUID().GetCounter(), displayId);

            if (!morphData)
            {
                handler->PSendSysMessage(FREEDOM_CMDE_X_WITH_ID_NOT_FOUND, "Morph", displayId);
                return true;
            }
        }
        else if (!morphData)
        {
            handler->PSendSysMessage(FREEDOM_CMDE_X_WITH_NAME_NOT_FOUND, "Morph", morphName);
            return true;
        }

        source->SetDisplayId(morphData->displayId);
        handler->PSendSysMessage(FREEDOM_CMDI_MORPH, morphData->name, morphData->displayId);
        return true;
    }
#pragma endregion

#pragma region COMMAND TABLE : .freedom -> teleport -> *
    static bool HandleFreedomTeleListCommand(ChatHandler* handler, char const* args)
    {    
        const PublicTeleContainer teleList = sFreedomMgr->GetPublicTeleportContainer();
        uint64 count = 0;

        if (!*args)
        {
            for (auto teleData : teleList)
            {
                handler->PSendSysMessage(FREEDOM_CMDI_PUBLIC_TELE_LIST_ITEM, teleData.name, sFreedomMgr->GetMapName(teleData.map));
                count++;
            }
        }
        else
        {
            Tokenizer tokens = Tokenizer(args, ' ');
            std::string name = tokens[0];

            for (auto teleData : teleList)
            {
                if (boost::istarts_with(teleData.name, name))
                {
                    handler->PSendSysMessage(FREEDOM_CMDI_PUBLIC_TELE_LIST_ITEM, teleData.name, sFreedomMgr->GetMapName(teleData.map));
                    count++;
                }                    
            }
        }

        if (count == 0)
            handler->PSendSysMessage(FREEDOM_CMDI_X_NOT_FOUND, "Public teleports");
        else
            handler->PSendSysMessage(FREEDOM_CMDI_SEARCH_QUERY_RESULT, count);

        return true;
    }

    static bool HandleFreedomTeleAddCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
        {
            handler->PSendSysMessage(FREEDOM_CMDH_PUBLIC_TELE_ADD);
            return true;
        }

        Tokenizer tokens = Tokenizer(args, ' ');
        std::string name = tokens[0];
        Player* source = handler->GetSession()->GetPlayer();

        // Check if teleport already exists
        const PublicTeleData* teleData = sFreedomMgr->GetPublicTeleport(name);

        if (teleData)
        {
            handler->PSendSysMessage(FREEDOM_CMDE_X_WITH_NAME_ALREADY_EXISTS, "Public teleport", name);
            return true;
        }

        // Create teleport
        PublicTeleData newTeleData;
        source->GetPosition(newTeleData.x, newTeleData.y, newTeleData.z, newTeleData.o);
        newTeleData.map = source->GetMapId();
        newTeleData.name = name;
        newTeleData.gmBnetAccId = source->GetSession()->GetBattlenetAccountId();

        sFreedomMgr->AddPublicTeleport(newTeleData);
        
        handler->PSendSysMessage(FREEDOM_CMDI_X_ADDED_WITH_NAME, "Public teleport", name);
        return true;
    }

    static bool HandleFreedomTeleDelCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
        {
            handler->PSendSysMessage(FREEDOM_CMDH_PUBLIC_TELE_DEL);
            return true;
        }

        Tokenizer tokens = Tokenizer(args, ' ');
        std::string name = tokens[0];
        Player* source = handler->GetSession()->GetPlayer();

        // Check if teleport actually exists
        const PublicTeleData* teleData = sFreedomMgr->GetPublicTeleport(name);

        if (!teleData)
        {
            handler->PSendSysMessage(FREEDOM_CMDE_X_WITH_NAME_NOT_FOUND, "Public teleport", name);
            return true;
        }

        sFreedomMgr->DeletePublicTeleport(name);

        handler->PSendSysMessage(FREEDOM_CMDI_X_WITH_NAME_REMOVED, "Public teleport", name);
        return true;
    }

    static bool HandleFreedomTeleCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
        {
            handler->PSendSysMessage(FREEDOM_CMDH_PUBLIC_TELE);
            return true;
        }

        Tokenizer tokens = Tokenizer(args, ' ');
        std::string name = tokens[0];
        Player* source = handler->GetSession()->GetPlayer();

        // Stop combat before teleporting
        if (source->IsInCombat())
        {
            source->CombatStop();
        }

        // Get first match of public teleport
        const PublicTeleData* teleData = sFreedomMgr->GetFirstClosestPublicTeleport(name);

        if (!teleData)
        {
            handler->PSendSysMessage(FREEDOM_CMDI_X_NONE_FOUND_WITH_NAME, "public teleports", name);
            return true;
        }

        // Stop flight if needed
        if (source->IsInFlight())
        {
            source->GetMotionMaster()->MovementExpired();
            source->CleanupAfterTaxiFlight();
        }
        // Save only in non-flight case
        else
        {
            source->SaveRecallPosition();
        }            
        
        source->TeleportTo(teleData->map, teleData->x, teleData->y, teleData->z, teleData->o);
        return true;
    }
#pragma endregion

#pragma region COMMAND TABLE : .freedom -> pteleport -> *
    static bool HandleFreedomPrivateTeleListCommand(ChatHandler* handler, char const* args)
    {
        const PrivateTeleVector teleList = sFreedomMgr->GetPrivateTeleportContainer(handler->GetSession()->GetBattlenetAccountId());
        uint64 count = 0;

        if (!*args)
        {
            for (auto teleData : teleList)
            {
                handler->PSendSysMessage(FREEDOM_CMDI_PRIVATE_TELE_LIST_ITEM, teleData.name, sFreedomMgr->GetMapName(teleData.map));
                count++;
            }
        }
        else
        {
            Tokenizer tokens = Tokenizer(args, ' ');
            std::string name = tokens[0];

            for (auto teleData : teleList)
            {
                if (boost::istarts_with(teleData.name, name))
                {
                    handler->PSendSysMessage(FREEDOM_CMDI_PRIVATE_TELE_LIST_ITEM, teleData.name, sFreedomMgr->GetMapName(teleData.map));
                    count++;
                }
            }
        }

        if (count == 0)
            handler->PSendSysMessage(FREEDOM_CMDI_X_NOT_FOUND, "Private teleports");
        else
            handler->PSendSysMessage(FREEDOM_CMDI_SEARCH_QUERY_RESULT, count);

        return true;
    }

    static bool HandleFreedomPrivateTeleAddCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
        {
            handler->PSendSysMessage(FREEDOM_CMDH_PRIVATE_TELE_ADD);
            return true;
        }

        Tokenizer tokens = Tokenizer(args, ' ');
        std::string name = tokens[0];
        Player* source = handler->GetSession()->GetPlayer();

        // Check if teleport already exists
        const PrivateTeleData* teleData = sFreedomMgr->GetPrivateTeleport(source->GetSession()->GetBattlenetAccountId(), name);

        if (teleData)
        {
            handler->PSendSysMessage(FREEDOM_CMDE_X_WITH_NAME_ALREADY_EXISTS, "Private teleport", name);
            return true;
        }

        // Create teleport
        PrivateTeleData newTeleData;
        source->GetPosition(newTeleData.x, newTeleData.y, newTeleData.z, newTeleData.o);
        newTeleData.map = source->GetMapId();
        newTeleData.name = name;

        sFreedomMgr->AddPrivateTeleport(source->GetSession()->GetBattlenetAccountId(), newTeleData);

        handler->PSendSysMessage(FREEDOM_CMDI_X_ADDED_WITH_NAME, "Private teleport", name);
        return true;
    }

    static bool HandleFreedomPrivateTeleDelCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
        {
            handler->PSendSysMessage(FREEDOM_CMDH_PRIVATE_TELE_DEL);
            return true;
        }

        Tokenizer tokens = Tokenizer(args, ' ');
        std::string name = tokens[0];
        Player* source = handler->GetSession()->GetPlayer();

        // Check if teleport actually exists
        const PrivateTeleData* teleData = sFreedomMgr->GetPrivateTeleport(source->GetSession()->GetBattlenetAccountId(), name);

        if (!teleData)
        {
            handler->PSendSysMessage(FREEDOM_CMDE_X_WITH_NAME_NOT_FOUND, "Private teleport", name);
            return true;
        }

        sFreedomMgr->DeletePrivateTeleport(source->GetSession()->GetBattlenetAccountId(), name);

        handler->PSendSysMessage(FREEDOM_CMDI_X_WITH_NAME_REMOVED, "Private teleport", name);
        return true;
    }

    static bool HandleFreedomPrivateTeleCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
        {
            handler->PSendSysMessage(FREEDOM_CMDH_PRIVATE_TELE);
            return true;
        }

        Tokenizer tokens = Tokenizer(args, ' ');
        std::string name = tokens[0];
        Player* source = handler->GetSession()->GetPlayer();

        // Stop combat before teleporting
        if (source->IsInCombat())
        {
            source->CombatStop();
        }

        // Get first match of public teleport
        const PrivateTeleData* teleData = sFreedomMgr->GetFirstClosestPrivateTeleport(source->GetSession()->GetBattlenetAccountId(), name);

        if (!teleData)
        {
            handler->PSendSysMessage(FREEDOM_CMDI_X_NONE_FOUND_WITH_NAME, "private teleports", name);
            return true;
        }

        // Stop flight if needed
        if (source->IsInFlight())
        {
            source->GetMotionMaster()->MovementExpired();
            source->CleanupAfterTaxiFlight();
        }
        // Save only in non-flight case
        else
        {
            source->SaveRecallPosition();
        }

        source->TeleportTo(teleData->map, teleData->x, teleData->y, teleData->z, teleData->o);
        return true;
    }
#pragma endregion

#pragma region COMMAND TABLE : .freedom -> spell -> *
    static bool HandleFreedomSpellListCommand(ChatHandler* handler, char const* args)
    {
        const PublicSpellContainer spellList = sFreedomMgr->GetPublicSpellContainer();
        uint64 count = 0;

        if (!*args)
        {
            for (auto spellData : spellList)
            {
                handler->PSendSysMessage(FREEDOM_CMDI_PUBLIC_SPELL_LIST_ITEM, spellData.first, sFreedomMgr->ToChatLink("Hspell", spellData.first, spellData.second.name));
                count++;
            }
        }
        else
        {
            Tokenizer tokens = Tokenizer(args, ' ');
            std::string name = tokens[0];

            for (auto spellData : spellList)
            {
                if (boost::istarts_with(spellData.second.name, name))
                {
                    handler->PSendSysMessage(FREEDOM_CMDI_PUBLIC_SPELL_LIST_ITEM, spellData.first, sFreedomMgr->ToChatLink("Hspell", spellData.first, spellData.second.name));
                    count++;
                }
            }
        }

        if (count == 0)
            handler->PSendSysMessage(FREEDOM_CMDI_X_NOT_FOUND, "Public spells");
        else
            handler->PSendSysMessage(FREEDOM_CMDI_SEARCH_QUERY_RESULT, count);

        return true;
    }

    static bool HandleFreedomSpellAddCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
        {
            handler->PSendSysMessage(FREEDOM_CMDH_PUBLIC_SPELL_ADD);
            return true;
        }

        uint32 spellId = handler->extractSpellIdFromLink((char*)args);
        Player* source = handler->GetSession()->GetPlayer();
        uint32 bnetAccId = source->GetSession()->GetBattlenetAccountId();
        uint8 targetOthers = 0;
        char* targetOthersArg = strtok(NULL, args);

        if (targetOthersArg && boost::iequals("true", targetOthersArg))
            targetOthers = 1;

        // Check if public spell already exists
        const PublicSpellData* spellData = sFreedomMgr->GetPublicSpell(spellId);

        if (spellData)
        {
            handler->PSendSysMessage(FREEDOM_CMDE_X_WITH_ID_ALREADY_EXISTS, "Public spell", spellId);
            return true;
        }

        const SpellEntry* spellEntry = sSpellStore.LookupEntry(spellId);

        if (!spellEntry)
        {
            handler->PSendSysMessage(FREEDOM_CMDE_X_WITH_ID_NOT_FOUND, "Spell entry", spellId);
            return true;
        }

        // Create teleport
        PublicSpellData newSpellData;
        newSpellData.name = spellEntry->Name_lang;
        newSpellData.targetOthers = targetOthers;
        newSpellData.gmBnetAccId = source->GetSession()->GetBattlenetAccountId();

        sFreedomMgr->AddPublicSpell(spellId, newSpellData);

        handler->PSendSysMessage(FREEDOM_CMDI_PUBLIC_SPELL_ADD, sFreedomMgr->ToChatLink("Hspell", spellId, spellEntry->Name_lang), spellId);
        return true;
    }

    static bool HandleFreedomSpellDelCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
        {
            handler->PSendSysMessage(FREEDOM_CMDH_PUBLIC_SPELL_DEL);
            return true;
        }

        uint32 spellId = handler->extractSpellIdFromLink((char*)args);
        std::string spellName;

        // Check if public spell already exists
        const PublicSpellData* spellData = sFreedomMgr->GetPublicSpell(spellId);

        if (!spellData)
        {
            handler->PSendSysMessage(FREEDOM_CMDE_X_WITH_ID_NOT_FOUND, "Public spell", spellId);
            return true;
        }

        spellName = spellData->name;

        sFreedomMgr->DeletePublicSpell(spellId);

        handler->PSendSysMessage(FREEDOM_CMDI_PUBLIC_SPELL_DEL, sFreedomMgr->ToChatLink("Hspell", spellId, spellName), spellId);
        return true;
    }

    static bool HandleFreedomSpellCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
        {
            handler->PSendSysMessage(FREEDOM_CMDH_PUBLIC_SPELL);
            return true;
        }

        uint32 spellId = handler->extractSpellIdFromLink((char*)args);
        Player* source = handler->GetSession()->GetPlayer();
        Player* target = handler->getSelectedPlayerOrSelf();
        std::string spellName;

        // Check if public spell already exists
        const PublicSpellData* spellData = sFreedomMgr->GetPublicSpell(spellId);

        if (!spellData)
        {
            handler->PSendSysMessage(FREEDOM_CMDE_X_WITH_ID_NOT_FOUND, "Public spell", spellId);
            return true;
        }

        const SpellEntry* spellEntry = sSpellStore.LookupEntry(spellId);

        if (!spellEntry)
        {
            handler->PSendSysMessage(FREEDOM_CMDE_X_WITH_ID_NOT_FOUND, "Spell entry", spellId);
            return true;
        }

        if (spellData->targetOthers)
        {
            source->CastSpell(target, spellId);
        }
        else
        {
            source->CastSpell(source, spellId);
        }

        spellName = spellData->name;
        return true;
    }
#pragma endregion

#pragma region COMMAND TABLE : .freedom -> reload -> *
    static bool HandleFreedomReloadAllCommand(ChatHandler* handler, char const* args)
    {
        sFreedomMgr->LoadAllTables();
        handler->SendGlobalGMSysMessage(handler->PGetParseString(FREEDOM_CMDI_RELOAD_ALL).c_str());
        return true;
    }

    static bool HandleFreedomReloadGameobjectCommand(ChatHandler* handler, char const* args)
    {
        sFreedomMgr->LoadGameObjectTemplateExtras();
        handler->SendGlobalGMSysMessage(handler->PGetParseString(FREEDOM_CMDI_RELOAD, "gameobject_template_extra").c_str());
        sFreedomMgr->LoadGameObjectExtras();
        handler->SendGlobalGMSysMessage(handler->PGetParseString(FREEDOM_CMDI_RELOAD, "gameobject_extra").c_str());
        return true;
    }

    static bool HandleFreedomReloadPublicTeleCommand(ChatHandler* handler, char const* args)
    {
        sFreedomMgr->LoadPublicTeleports();        
        handler->SendGlobalGMSysMessage(handler->PGetParseString(FREEDOM_CMDI_RELOAD, "public_tele").c_str());
        return true;
    }

    static bool HandleFreedomReloadPrivateTeleCommand(ChatHandler* handler, char const* args)
    {
        sFreedomMgr->LoadPrivateTeleports();
        handler->SendGlobalGMSysMessage(handler->PGetParseString(FREEDOM_CMDI_RELOAD, "private_tele").c_str());
        return true;
    }

    static bool HandleFreedomReloadPublicSpellCommand(ChatHandler* handler, char const* args)
    {
        sFreedomMgr->LoadPublicSpells();
        handler->SendGlobalGMSysMessage(handler->PGetParseString(FREEDOM_CMDI_RELOAD, "public_spell").c_str());
        return true;
    }

    static bool HandleFreedomReloadMorphsCommand(ChatHandler* handler, char const* args)
    {
        sFreedomMgr->LoadMorphs();
        handler->SendGlobalGMSysMessage(handler->PGetParseString(FREEDOM_CMDI_RELOAD, "morphs").c_str());
        return true;
    }

    static bool HandleFreedomReloadItemCommand(ChatHandler* handler, char const* args)
    {
        sFreedomMgr->LoadItemTemplateExtras();
        handler->SendGlobalGMSysMessage(handler->PGetParseString(FREEDOM_CMDI_RELOAD, "item_template_extra").c_str());
        return true;
    }
#pragma endregion

#pragma region COMMAND TABLE : .freedom -> *
    static bool HandleFreedomTabardCommand(ChatHandler* handler, char const* args)
    {
        Player* source = handler->GetSession()->GetPlayer();
        handler->GetSession()->SendTabardVendorActivate(source->GetGUID());
        return true;
    }

    static bool HandleFreedomTameCommand(ChatHandler* handler, char const* args)
    {
        Creature* target = handler->getSelectedCreature();

        if (!target)
        {
            handler->PSendSysMessage(FREEDOM_CMDE_MANUAL_SELECT_CREATURE);
            return true;
        }

        if (target->IsPet())
        {
            handler->PSendSysMessage(FREEDOM_CMDE_FREEDOM_TAME_ALREADY_A_PET);
            return true;
        }

        Player* source = handler->GetSession()->GetPlayer();

        if (!source->GetPetGUID().IsEmpty())
        {
            handler->SendSysMessage(FREEDOM_CMDE_FREEDOM_TAME_ALREADY_HAVE_PET);
            return true;
        }

        CreatureTemplate const* cInfo = target->GetCreatureTemplate();

        if (!cInfo->IsTameable(source->CanTameExoticPets()))
        {
            handler->PSendSysMessage(FREEDOM_CMDE_FREEDOM_TAME_NOT_TAMEABLE);
            return true;
        }

        // Everything looks OK, create new pet
        Pet* pet = source->CreateTamedPetFrom(target);
        if (!pet)
        {
            handler->PSendSysMessage(FREEDOM_CMDE_FREEDOM_TAME_NOT_TAMEABLE);
            return true;
        }

        // place pet before player
        float x, y, z;
        source->GetClosePoint(x, y, z, target->GetObjectSize(), CONTACT_DISTANCE);
        pet->Relocate(x, y, z, float(M_PI) - source->GetOrientation());

        // set pet to defensive mode by default (some classes can't control controlled pets in fact).
        pet->SetReactState(REACT_DEFENSIVE);

        // calculate proper level
        uint8 level = (target->getLevel() < (source->getLevel() - 5)) ? (source->getLevel() - 5) : target->getLevel();

        // prepare visual effect for levelup
        pet->SetUInt32Value(UNIT_FIELD_LEVEL, level - 1);

        // add to world
        pet->GetMap()->AddToMap(pet->ToCreature());

        // visual effect for levelup
        pet->SetUInt32Value(UNIT_FIELD_LEVEL, level);

        // caster have pet now
        source->SetMinion(pet, true);

        pet->SavePetToDB(PET_SAVE_AS_CURRENT);
        source->PetSpellInitialize();

        handler->PSendSysMessage(FREEDOM_CMDI_FREEDOM_TAME,
            sFreedomMgr->ToChatLink("Hcreature_entry", target->GetEntry(), target->GetName()));
        return true;
    }

    static bool HandleFreedomHoverCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
        {
            // did not specify ON or OFF value
            handler->PSendSysMessage(FREEDOM_CMDH_HOVER);
            return true;
        }

        Player* source = handler->GetSession()->GetPlayer();

        // on & off
        std::string arg = strtok((char*)args, " ");
        if (boost::iequals(arg, "on"))
        {
            handler->PSendSysMessage(FREEDOM_CMDI_HOVER, "on");
            source->AddAura(SPELL_PERMANENT_HOVER, source);
            source->SetHover(true);
            return true;
        }
        else if (boost::iequals(arg, "off"))
        {
            sFreedomMgr->RemoveHoverFromPlayer(source);
            source->RemoveAura(SPELL_PERMANENT_HOVER);
            handler->PSendSysMessage(FREEDOM_CMDI_HOVER, "off");
            return true;
        }

        // did not specify ON or OFF value
        handler->PSendSysMessage(FREEDOM_CMDH_HOVER);

        return true;
    }

    static bool HandleFreedomSummonCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
        {
            handler->PSendSysMessage(FREEDOM_CMDH_SUMMON);
            return true;
        }

        Player* source = handler->GetSession()->GetPlayer();
        ObjectGuid targetGuid = sObjectMgr->GetPlayerGUIDByName(args);
        Player* target = sObjectMgr->GetPlayerByLowGUID(targetGuid.GetCounter());

        if (!target || target->IsLoading() || target->IsBeingTeleported())
        {
            handler->PSendSysMessage(FREEDOM_CMDE_SUMMON_NOT_FOUND, args);
            return true;
        }

        if (target->GetSocial()->HasIgnore(source->GetGUID()))
        {
            handler->PSendSysMessage(FREEDOM_CMDE_SUMMON_IGNORE, target->GetName().c_str());
            return true;
        }

        if (target->IsGameMaster())
        {
            handler->PSendSysMessage(FREEDOM_CMDE_SUMMON_GM_ON, target->GetName().c_str());
            return true;
        }
       
        if (target->HasSummonPending())
        {
            handler->PSendSysMessage(FREEDOM_CMDE_SUMMON_PENDING, target->GetName().c_str());
            return true;
        }

        // Evil Twin (ignore player summon, but hide this for summoner)
        if (target->HasAura(23445))
        {
            handler->PSendSysMessage(FREEDOM_CMDE_SUMMON_EVIL_TWIN, target->GetName().c_str());
            return true;
        }

        target->SendSummonRequestFrom(source);

        handler->PSendSysMessage(FREEDOM_CMDI_SUMMON, target->GetName().c_str());
        return true;
    }

    static bool HandleFreedomDemorphCommand(ChatHandler* handler, char const* args)
    {
        Player* source = handler->GetSession()->GetPlayer();
        source->DeMorph();
        handler->PSendSysMessage(FREEDOM_CMDI_DEMORPH);
        return true;
    }

    static bool HandleFreedomFlyCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
        {
            handler->PSendSysMessage(FREEDOM_CMDH_FLY);
            handler->PSendSysMessage(FREEDOM_CMDH_MOD_SPEED);
            return true;
        }

        Player* source = handler->GetSession()->GetPlayer();
        std::string arg = strtok((char*)args, " ");

        if (boost::iequals(arg, "on"))
        {
            source->SetCanFly(true);
            handler->PSendSysMessage(FREEDOM_CMDI_FLY, "on");
        }
        else if (boost::iequals(arg, "off"))
        {
            sFreedomMgr->RemoveFlyFromPlayer(source);
            handler->PSendSysMessage(FREEDOM_CMDI_FLY, "off");
        }
        else
        {
            ModifyMovementSpeed(handler, MOVE_FLIGHT, (float)atof(arg.c_str()));
        }

        return true;
    }

    static bool HandleFreedomReviveCommand(ChatHandler* handler, char const* args)
    {
        Player* source = handler->GetSession()->GetPlayer();

        if (source->IsAlive()) {
            handler->PSendSysMessage(FREEDOM_CMDE_REVIVE);
            return true;
        }
        
        source->ResurrectPlayer(1.0);
        source->SaveToDB();
        handler->PSendSysMessage(FREEDOM_CMDI_REVIVE);
        return true;
    }

    static bool HandleFreedomUnAuraCommand(ChatHandler* handler, char const* args)
    {
        Player* source = handler->GetSession()->GetPlayer();   
        sFreedomMgr->RemoveHoverFromPlayer(source); // unaura removes hover animation, so proceed to remove entire hover mechanic
        source->RemoveAllAuras();        
        handler->PSendSysMessage(FREEDOM_CMDI_UNAURA);
        return true;
    }

    static bool HandleFreedomSpeedCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
        {
            handler->PSendSysMessage(FREEDOM_CMDH_MOD_SPEED);
            return true;
        }

        float speed = (float)atof((char*)args);
        float max = sConfigMgr->GetFloatDefault("Freedom.Modify.MaxSpeed", 10.0f);
        float min = sConfigMgr->GetFloatDefault("Freedom.Modify.MinSpeed", 0.01f);
        std::string speedName = "all";

        if (speed < min || speed > max)
        {
            handler->PSendSysMessage(FREEDOM_CMDE_VALUE_OUT_OF_RANGE, max, min);
            return true;
        }

        handler->GetSession()->GetPlayer()->SetSpeed(MOVE_FLIGHT, speed);
        handler->GetSession()->GetPlayer()->SetSpeed(MOVE_FLIGHT_BACK, speed);
        handler->GetSession()->GetPlayer()->SetSpeed(MOVE_RUN, speed);
        handler->GetSession()->GetPlayer()->SetSpeed(MOVE_SWIM, speed);
        handler->GetSession()->GetPlayer()->SetSpeed(MOVE_SWIM_BACK, speed);
        handler->GetSession()->GetPlayer()->SetSpeed(MOVE_WALK, speed);
        handler->GetSession()->GetPlayer()->SetSpeed(MOVE_RUN_BACK, speed);
        handler->PSendSysMessage(FREEDOM_CMDI_MOD_SPEED, speedName.c_str(), speed);
        return true;
    }

    static bool HandleFreedomWalkCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
        {
            handler->PSendSysMessage(FREEDOM_CMDH_MOD_SPEED);
            return true;
        }

        ModifyMovementSpeed(handler, MOVE_WALK, (float)atof((char*)args));
        return true;
    }

    static bool HandleFreedomRunCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
        {
            handler->PSendSysMessage(FREEDOM_CMDH_MOD_SPEED);
            return true;
        }

        ModifyMovementSpeed(handler, MOVE_RUN, (float)atof((char*)args));
        return true;
    }

    static bool HandleFreedomSwimCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
        {
            handler->PSendSysMessage(FREEDOM_CMDH_MOD_SPEED);
            return true;
        }

        ModifyMovementSpeed(handler, MOVE_SWIM, (float)atof((char*)args));
        return true;
    }

    static bool HandleFreedomScaleCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
        {
            handler->PSendSysMessage(FREEDOM_CMDH_SCALE);
            return true;
        }

        Player* source = handler->GetSession()->GetPlayer();
        float scale = (float)atof((char*)args);
        float max = sConfigMgr->GetFloatDefault("Freedom.Modify.MaxScale", 10.0f);
        float min = sConfigMgr->GetFloatDefault("Freedom.Modify.MinScale", 0.01f);

        if (scale < min || scale > max)
        {
            handler->PSendSysMessage(FREEDOM_CMDE_VALUE_OUT_OF_RANGE, max, min);
            return true;
        }

        source->SetObjectScale(scale);
        handler->PSendSysMessage(FREEDOM_CMDI_SCALE, scale);
        return true;
    }

    static bool HandleFreedomDrunkCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
        {
            handler->PSendSysMessage(FREEDOM_CMDH_DRUNK);
            return true;
        }

        Player* source = handler->GetSession()->GetPlayer();
        uint8 drunklevel = (uint8)atoul(args);

        if (drunklevel > 100)
            drunklevel = 100;

        source->SetDrunkValue(drunklevel);
        handler->PSendSysMessage(FREEDOM_CMDI_DRUNK, drunklevel);
        return true;
    }

    static bool HandleFreedomWaterwalkCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
        {
            // did not specify ON or OFF value
            handler->PSendSysMessage(FREEDOM_CMDH_WATERWALK);
            return true;
        }

        Player* source = handler->GetSession()->GetPlayer();

        // on & off
        std::string arg = strtok((char*)args, " ");
        if (boost::iequals(arg, "on"))
        {
            handler->PSendSysMessage(FREEDOM_CMDI_WATERWALK, "on");
            source->SetWaterWalking(true);
            return true;
        }
        else if (boost::iequals(arg, "off"))
        {
            sFreedomMgr->RemoveWaterwalkFromPlayer(source);
            handler->PSendSysMessage(FREEDOM_CMDI_WATERWALK, "off");
            return true;
        }

        // did not specify ON or OFF value
        handler->PSendSysMessage(FREEDOM_CMDH_WATERWALK);

        return true;
    }

    static bool HandleFreedomFixCommand(ChatHandler* handler, char const* args)
    {
        Player* source = handler->GetSession()->GetPlayer();
        source->DurabilityRepairAll(false, 0, false);
        handler->PSendSysMessage(FREEDOM_CMDI_FIX_ITEMS);
        return true;
    }

    static bool HandleFreedomMailboxCommand(ChatHandler* handler, char const* args)
    {
        Player* source = handler->GetSession()->GetPlayer();
        handler->GetSession()->SendShowMailBox(source->GetGUID());
        return true;
    }

    static bool HandleFreedomMoneyCommand(ChatHandler* handler, char const* args)
    {
        Player* source = handler->GetSession()->GetPlayer();
        source->SetMoney(uint64(100000000000Ui64));
        handler->PSendSysMessage(FREEDOM_CMDI_MONEY_RESET);
        return true;
    }

    static bool HandleFreedomBankCommand(ChatHandler* handler, char const* args)
    {
        Player* source = handler->GetSession()->GetPlayer();
        handler->GetSession()->SendShowBank(source->GetGUID());
        return true;
    }

    static bool HandleFreedomCustomizeCommand(ChatHandler* handler, char const* args)
    {
        Player* source = handler->GetSession()->GetPlayer();

        PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_ADD_AT_LOGIN_FLAG);
        stmt->setUInt16(0, uint16(AT_LOGIN_CUSTOMIZE));
        stmt->setUInt32(1, source->GetGUID().GetCounter());
        CharacterDatabase.Execute(stmt);

        source->SetAtLoginFlag(AT_LOGIN_CUSTOMIZE);
        handler->PSendSysMessage(FREEDOM_CMDI_FLAG_FOR_CUSTOMIZATION);

        return true;
    }

    static bool HandleFreedomRaceChangeCommand(ChatHandler* handler, char const* args)
    {
        Player* source = handler->GetSession()->GetPlayer();

        PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_ADD_AT_LOGIN_FLAG);
        stmt->setUInt16(0, uint16(AT_LOGIN_CHANGE_RACE));
        stmt->setUInt32(1, source->GetGUID().GetCounter());
        CharacterDatabase.Execute(stmt);

        source->SetAtLoginFlag(AT_LOGIN_CHANGE_RACE);
        handler->PSendSysMessage(FREEDOM_CMDI_FLAG_FOR_RACECHANGE);
        return true;
    }

    static bool HandleFreedomFactionChangeCommand(ChatHandler* handler, char const* args)
    {
        Player* source = handler->GetSession()->GetPlayer();

        PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_ADD_AT_LOGIN_FLAG);
        stmt->setUInt16(0, uint16(AT_LOGIN_CHANGE_FACTION));
        stmt->setUInt32(1, source->GetGUID().GetCounter());
        CharacterDatabase.Execute(stmt);

        source->SetAtLoginFlag(AT_LOGIN_CHANGE_FACTION);
        handler->PSendSysMessage(FREEDOM_CMDI_FLAG_FOR_FACTIONCHANGE);
        return true;
    }
#pragma endregion
};

void AddSC_freedom_commandscript()
{
    new freedom_commandscript();
}
