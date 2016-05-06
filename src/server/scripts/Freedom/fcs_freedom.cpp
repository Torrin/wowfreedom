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
            { "public_tele",    rbac::RBAC_FPERM_COMMAND_FREEDOM_ADMINISTRATION,    false, &HandleFreedomReloadPublicTeleCommand,   "" },
            { "private_tele",   rbac::RBAC_FPERM_COMMAND_FREEDOM_ADMINISTRATION,    false, &HandleFreedomReloadPrivateTeleCommand,  "" },
            { "public_spell",   rbac::RBAC_FPERM_COMMAND_FREEDOM_ADMINISTRATION,    false, &HandleFreedomReloadPublicSpellCommand,  "" },
            { "morphs",         rbac::RBAC_FPERM_COMMAND_FREEDOM_ADMINISTRATION,    false, &HandleFreedomReloadMorphsCommand,       "" },
            { "all",            rbac::RBAC_FPERM_COMMAND_FREEDOM_ADMINISTRATION,    false, &HandleFreedomReloadAllCommand,          "" },
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
            { "reload",         rbac::RBAC_FPERM_COMMAND_FREEDOM_ADMINISTRATION,    false, NULL,                                    "", freedomReloadCommandTable },
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
            handler->PSendSysMessage(FREEDOM_CMDI_X_NONE_FOUND, "morphs");
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
            handler->PSendSysMessage(FREEDOM_CMDI_X_NONE_FOUND, "public teleports");
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
            handler->PSendSysMessage(FREEDOM_CMDI_X_NONE_FOUND, "private teleports");
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
            handler->PSendSysMessage(FREEDOM_CMDI_X_NONE_FOUND, "public spells");
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
#pragma endregion

#pragma region COMMAND TABLE : .freedom -> *
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

#ifdef FREEDOM_MOP_548_CODE

class freedom_commandscript : public CommandScript
{
public:
    freedom_commandscript() : CommandScript("freedom_commandscript") { }

    ChatCommand* GetCommands() const OVERRIDE
    {
        static ChatCommand freedomMorphCommandTable[] = {
                { "list",           rbac::RBAC_PERM_COMMAND_FREEDOM_MORPH_LIST,         false, &HandleFreedomMorphListCommand,      "", NULL },
                { "add",            rbac::RBAC_PERM_COMMAND_FREEDOM_MORPH_MODIFY,       false, &HandleFreedomMorphAddCommand,       "", NULL },
                { "delete",         rbac::RBAC_PERM_COMMAND_FREEDOM_MORPH_MODIFY,       false, &HandleFreedomMorphDelCommand,       "", NULL },
                { "",               rbac::RBAC_PERM_COMMAND_FREEDOM_MORPH,              false, &HandleFreedomMorphCommand,          "", NULL },
                { NULL, 0, false, NULL, "", NULL }
        };

        static ChatCommand freedomTeleportCommandTable[] = {
                { "list",           rbac::RBAC_PERM_COMMAND_FREEDOM_TELE_LIST,          false, &HandleFreedomTeleListCommand,       "", NULL },
                { "add",            rbac::RBAC_PERM_COMMAND_FREEDOM_TELE_MODIFY,        false, &HandleFreedomTeleAddCommand,        "", NULL },
                { "delete",         rbac::RBAC_PERM_COMMAND_FREEDOM_TELE_MODIFY,        false, &HandleFreedomTeleDelCommand,        "", NULL },
                { "",               rbac::RBAC_PERM_COMMAND_FREEDOM_TELE,               false, &HandleFreedomTeleCommand,           "", NULL },
                { NULL, 0, false, NULL, "", NULL }
        };

        static ChatCommand freedomPrivateTeleportCommandTable[] = {
                { "list",           rbac::RBAC_PERM_COMMAND_FREEDOM_PTELE,              false, &HandleFreedomPrivateTeleListCommand,"", NULL },
                { "add",            rbac::RBAC_PERM_COMMAND_FREEDOM_PTELE,              false, &HandleFreedomPrivateTeleAddCommand, "", NULL },
                { "delete",         rbac::RBAC_PERM_COMMAND_FREEDOM_PTELE,              false, &HandleFreedomPrivateTeleDelCommand, "", NULL },
                { "",               rbac::RBAC_PERM_COMMAND_FREEDOM_PTELE,              false, &HandleFreedomPrivateTeleCommand,    "", NULL },
                { NULL, 0, false, NULL, "", NULL }
        };

        static ChatCommand freedomSpellCommandTable[] = {
                { "list",           rbac::RBAC_PERM_COMMAND_FREEDOM_SPELL,              false, &HandleFreedomSpellListCommand,      "", NULL },
                { "add",            rbac::RBAC_PERM_COMMAND_FREEDOM_SPELL_MODIFY,       false, &HandleFreedomSpellAddCommand,       "", NULL },
                { "delete",         rbac::RBAC_PERM_COMMAND_FREEDOM_SPELL_MODIFY,       false, &HandleFreedomSpellDelCommand,       "", NULL },
                { "",               rbac::RBAC_PERM_COMMAND_FREEDOM_SPELL,              false, &HandleFreedomSpellCommand,          "", NULL },
                { NULL, 0, false, NULL, "", NULL }
        };

        static ChatCommand freedomCommandTable[] = {
                { "cast",           rbac::RBAC_PERM_COMMAND_FREEDOM_SPELL,              false, &HandleFreedomSpellCommand,          "", NULL },
                { "summon",         rbac::RBAC_PERM_COMMAND_FREEDOM_SUMMON,             false, &HandleFreedomSummonCommand,         "", NULL },
                { "demorph",        rbac::RBAC_PERM_COMMAND_FREEDOM_DEMORPH,            false, &HandleFreedomDemorphCommand,        "", NULL },
                { "fly",            rbac::RBAC_PERM_COMMAND_FREEDOM_FLY,                false, &HandleFreedomFlyCommand,            "", NULL },
                { "revive",         rbac::RBAC_PERM_COMMAND_FREEDOM_REVIVE,             false, &HandleFreedomReviveCommand,         "", NULL },
                { "unaura",         rbac::RBAC_PERM_COMMAND_FREEDOM_UNAURA,             false, &HandleFreedomUnAuraCommand,         "", NULL },
                { "speed",          rbac::RBAC_PERM_COMMAND_FREEDOM_SPEEDS,             false, &HandleFreedomSpeedCommand,          "", NULL },
                { "walk",           rbac::RBAC_PERM_COMMAND_FREEDOM_WALK,               false, &HandleFreedomWalkCommand,           "", NULL },
                { "run",            rbac::RBAC_PERM_COMMAND_FREEDOM_RUN,                false, &HandleFreedomRunCommand,            "", NULL },
                { "swim",           rbac::RBAC_PERM_COMMAND_FREEDOM_SWIM,               false, &HandleFreedomSwimCommand,           "", NULL },
                { "scale",          rbac::RBAC_PERM_COMMAND_FREEDOM_SCALE,              false, &HandleFreedomScaleCommand,          "", NULL },
                { "drunk",          rbac::RBAC_PERM_COMMAND_FREEDOM_DRUNK,              false, &HandleFreedomDrunkCommand,          "", NULL },
                { "waterwalk",      rbac::RBAC_PERM_COMMAND_FREEDOM_WATERWALK,          false, &HandleFreedomWaterwalkCommand,      "", NULL },
                { "fix",            rbac::RBAC_PERM_COMMAND_FREEDOM_FIX,                false, &HandleFreedomFixCommand,            "", NULL },
                //{ "mailbox",        rbac::RBAC_PERM_COMMAND_FREEDOM_MAILBOX,            false, &HandleFreedomMailboxCommand,        "", NULL }, TODO: implement mailbox command when SMSG_SHOW_MAILBOX or similar opcode is implemented
                { "morph",          rbac::RBAC_PERM_COMMAND_FREEDOM_MORPH,              false, NULL,                                "", freedomMorphCommandTable },
                { "money",          rbac::RBAC_PERM_COMMAND_FREEDOM_MONEY,              false, &HandleFreedomMoneyCommand,          "", NULL },
                { "bank",           rbac::RBAC_PERM_COMMAND_FREEDOM_BANK,               false, &HandleFreedomBankCommand,           "", NULL },
                { "customize",      rbac::RBAC_PERM_COMMAND_FREEDOM_CUSTOMIZE,          false, &HandleFreedomCustomizeCommand,      "", NULL },
                //{ "racechange",     rbac::RBAC_PERM_COMMAND_FREEDOM_RACE_CHANGE,        false, &HandleFreedomRaceChangeCommand,     "", NULL }, race/faction change opcode is not handled yet
                //{ "factionchange",  rbac::RBAC_PERM_COMMAND_FREEDOM_FACTION_CHANGE,     false, &HandleFreedomFactionChangeCommand,  "", NULL }, race/faction change opcode is not handled yet
                { "teleport",       rbac::RBAC_PERM_COMMAND_FREEDOM_TELE,               false, NULL,                                "", freedomTeleportCommandTable },
                { "pteleport",      rbac::RBAC_PERM_COMMAND_FREEDOM_PTELE,              false, NULL,                                "", freedomPrivateTeleportCommandTable },
                { "spell",          rbac::RBAC_PERM_COMMAND_FREEDOM_SPELL,              false, NULL,                                "", freedomSpellCommandTable },
                { NULL, 0, false, NULL, "", NULL }
        };

        static ChatCommand commandTable[] =
        {
            { "freedom",            rbac::RBAC_PERM_COMMAND_FREEDOM,                    false, NULL,                                "", freedomCommandTable },
            { NULL, 0, false, NULL, "", NULL }
        };
        return commandTable;
    }

    /*===========================
    * ----- COMMAND HANDLES -----
    * =========================== 
    * Color guide:
    * |cFFD2691E MSG_COLOR_CHOCOLATE - message title/name/source.
    * |cFFFF0000 MSG_COLOR_RED       - error message.
    * |cFFBBBBBB MSG_COLOR_SUBWHITE  - description, normal text.
    * |cFFFF4500 MSG_COLOR_ORANGEY   - tag name, link name, target/source name, for exclamation.
    */

    #pragma region SPELLS

    static bool HandleFreedomSpellCommand(ChatHandler* handler, const char* args)
    {
        Unit* target = handler->GetSession()->GetPlayer();
        Player* source = handler->GetSession()->GetPlayer();

        if (!*args)
        {
            handler->PSendSysMessage(
                "%s.freedom spell: %sPlease specify spell id to cast.\n"
                "%sSyntax: %s.freedom spell/cast $spellId",
                MSG_COLOR_CHOCOLATE, MSG_COLOR_SUBWHITE, MSG_COLOR_CHOCOLATE, MSG_COLOR_ORANGEY);
            return true;
        }

        uint32 spell_id = handler->extractSpellIdFromLink((char*)args);

        PreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_SEL_FREEDOM_SPELL_SINGLE);
        stmt->setUInt32(0, spell_id);
        PreparedQueryResult result_exists = WorldDatabase.Query(stmt);

        if (!result_exists)
        {
            handler->PSendSysMessage(
                "%s.freedom spell: %sSpell not found on public use table.",
                MSG_COLOR_CHOCOLATE, MSG_COLOR_SUBWHITE);
            return true;
        }

        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spell_id);
        if (!spellInfo)
        {
            handler->PSendSysMessage(
                "%s.freedom spell: %sInvalid spell id. Contact the staff to remove this faulty spell from the list.",
                MSG_COLOR_CHOCOLATE, MSG_COLOR_SUBWHITE);
            return true;
        }

        if (result_exists->Fetch()[1].GetUInt8())
        {
            target = handler->getSelectedUnit();
        }

        if (!target)
        {
            handler->PSendSysMessage(
                "%s.freedom spell: %sInvalid target to cast the spell on.",
                MSG_COLOR_CHOCOLATE, MSG_COLOR_SUBWHITE);
            return true;
        }

        if (!SpellMgr::IsSpellValid(spellInfo, handler->GetSession()->GetPlayer()))
        {
            handler->PSendSysMessage(LANG_COMMAND_SPELL_BROKEN, spell_id);
            handler->SetSentErrorMessage(true);
            return false;
        }

        source->CastSpell(target, spell_id);
        return true;
    }

    static bool HandleFreedomSpellListCommand(ChatHandler* handler, const char* args)
    {
        std::string spell_name_fragment = "";

        if (*args)
        {
            spell_name_fragment += (char*)args;
        }

        // refine query parameter for LIKE statement
        replaceAll(spell_name_fragment, "%", "\\%");
        replaceAll(spell_name_fragment, "_", "\\_");
        spell_name_fragment = "%" + spell_name_fragment + "%";

        // get exact match
        PreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_SEL_FREEDOM_SPELL);
        stmt->setString(0, spell_name_fragment);
        PreparedQueryResult result = WorldDatabase.Query(stmt);

        if (!result)
        {
            handler->PSendSysMessage(
                "%s.freedom spell list: %sNo spells found.\n"
                "%sReason: %sNo spells using that string (or blank) was found in freedom spell list.",
                MSG_COLOR_CHOCOLATE, MSG_COLOR_RED, MSG_COLOR_CHOCOLATE, MSG_COLOR_SUBWHITE);
            return true;
        }

        do
        {
            Field * fields = result->Fetch();

            handler->PSendSysMessage(
                "%s> %s%u%s - |cffffffff|Hspell:%u|h[%s]|h|r%s (Usable on: %s%s%s)",
                MSG_COLOR_CHOCOLATE, MSG_COLOR_ORANGEY, fields[0].GetUInt32(), MSG_COLOR_SUBWHITE, 
                fields[0].GetUInt32(), fields[2].GetCString(), MSG_COLOR_SUBWHITE, MSG_COLOR_ORANGEY, fields[1].GetUInt8() ? "EVERYONE" : "SELF", MSG_COLOR_SUBWHITE);
        } while (result->NextRow());

        handler->PSendSysMessage(
            "%s>>%s Retrieved %s%u%s spell result(s).",
            MSG_COLOR_CHOCOLATE, MSG_COLOR_SUBWHITE, MSG_COLOR_ORANGEY, uint32(result->GetRowCount()), MSG_COLOR_SUBWHITE);

        return true;
    }

    static bool HandleFreedomSpellAddCommand(ChatHandler* handler, const char* args)
    {
        char* params[2];
        params[0] = strtok((char*)args, " ");
        params[1] = strtok(NULL, " ");

        if (!params[0] || !params[1])
        {
            handler->PSendSysMessage(
                "%s.freedom spell: %sNot enough parameters to add public use spell.\n"
                "%sSyntax: %s.freedom spell add $spellId y/n\n"
                "%sNotes: %sSecond parameter sets whether the spell can be cast on other targets/players (y) or just on user/self (n).",
                MSG_COLOR_CHOCOLATE, MSG_COLOR_SUBWHITE, MSG_COLOR_CHOCOLATE, MSG_COLOR_ORANGEY, MSG_COLOR_CHOCOLATE, MSG_COLOR_ORANGEY);
            return true;
        }

        uint32 spell_id = handler->extractSpellIdFromLink(params[0]);

        PreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_SEL_FREEDOM_SPELL_SINGLE);
        stmt->setUInt32(0, spell_id);
        PreparedQueryResult result_exists = WorldDatabase.Query(stmt);

        if (result_exists)
        {
            handler->PSendSysMessage(
                "%s.freedom spell: %sSpell with such id already exists on the public use table.",
                MSG_COLOR_CHOCOLATE, MSG_COLOR_SUBWHITE);
            return true;
        }

        // check if spell is valid
        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spell_id);
        if (!spellInfo)
        {
            handler->PSendSysMessage(
                "%s.freedom spell: %sInvalid spell id.",
                MSG_COLOR_CHOCOLATE, MSG_COLOR_SUBWHITE);
            return true;
        }

        if (strcmp(params[1], "y") != 0 && strcmp(params[1], "n") != 0)
        {
            handler->PSendSysMessage(
                "%s.freedom spell add: %sSecond parameter must be 'y' or 'n'.\n"
                "%sSyntax: %s.freedom spell add $spellId y/n",
                MSG_COLOR_CHOCOLATE, MSG_COLOR_SUBWHITE, MSG_COLOR_CHOCOLATE, MSG_COLOR_ORANGEY);
            return true;
        }

        uint8 allow_targeting = (strcmp(params[1], "y") == 0 ? 1 : 0);
        std::string spell_name = spellInfo->SpellName;
        uint32 added_by = handler->GetSession()->GetAccountId();

        stmt = WorldDatabase.GetPreparedStatement(WORLD_INS_FREEDOM_SPELL);
        stmt->setUInt32(0, spell_id);
        stmt->setUInt8(1, allow_targeting);
        stmt->setString(2, spell_name);
        stmt->setUInt32(3, added_by);
        WorldDatabase.Execute(stmt);

        handler->PSendSysMessage("%s>>%sSpell was successfully added to public use table.", MSG_COLOR_CHOCOLATE, MSG_COLOR_SUBWHITE);
        return true;
    }

    static bool HandleFreedomSpellDelCommand(ChatHandler* handler, const char* args)
    {
        if (!*args)
        {
            handler->PSendSysMessage(
                "%s.freedom spell delete: %sPlease specify spell id to delete.\n"
                "%sSyntax: %s.freedom spell delete $spellId",
                MSG_COLOR_CHOCOLATE, MSG_COLOR_SUBWHITE, MSG_COLOR_CHOCOLATE, MSG_COLOR_ORANGEY);
            return true;
        }

        uint32 spell_id = handler->extractSpellIdFromLink((char*)args);

        PreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_SEL_FREEDOM_SPELL_SINGLE);
        stmt->setUInt32(0, spell_id);
        PreparedQueryResult result_exists = WorldDatabase.Query(stmt);

        if (!result_exists)
        {
            handler->PSendSysMessage(
                "%s.freedom spell: %sSpell with such id does not exist on the public use table.",
                MSG_COLOR_CHOCOLATE, MSG_COLOR_SUBWHITE);
            return true;
        }

        stmt = WorldDatabase.GetPreparedStatement(WORLD_DEL_FREEDOM_SPELL);
        stmt->setUInt32(0, spell_id);
        WorldDatabase.Execute(stmt);

        handler->PSendSysMessage("%s>>%sSpell was successfully removed from public use.", MSG_COLOR_CHOCOLATE, MSG_COLOR_SUBWHITE);
        return true;
    }

    static bool HandleFreedomUnAuraCommand(ChatHandler* handler, const char* /*args*/) {
        Player* source = handler->GetSession()->GetPlayer();
        source->RemoveAllAuras();
        handler->PSendSysMessage("%s>>%sAll auras/passive/spell effects are removed from your character.", MSG_COLOR_CHOCOLATE, MSG_COLOR_SUBWHITE);
        return true;
    }

    #pragma endregion

    #pragma region UTILITIES
    static bool HandleFreedomCustomizeCommand(ChatHandler* handler, const char* /*args*/) {
        Player* source = handler->GetSession()->GetPlayer();

        PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_ADD_AT_LOGIN_FLAG);
        stmt->setUInt16(0, uint16(AT_LOGIN_CUSTOMIZE));
        stmt->setUInt32(1, source->GetGUIDLow());
        CharacterDatabase.Execute(stmt);

        source->SetAtLoginFlag(AT_LOGIN_CUSTOMIZE);
        handler->PSendSysMessage("%s>>%s Your character is now flagged for a re-customization.", MSG_COLOR_CHOCOLATE, MSG_COLOR_SUBWHITE);

        return true;
    }

    static bool HandleFreedomRaceChangeCommand(ChatHandler* handler, const char* /*args*/) {
        Player* source = handler->GetSession()->GetPlayer();

        PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_ADD_AT_LOGIN_FLAG);
        stmt->setUInt16(0, uint16(AT_LOGIN_CHANGE_RACE));
        stmt->setUInt32(1, source->GetGUIDLow());
        CharacterDatabase.Execute(stmt);

        source->SetAtLoginFlag(AT_LOGIN_CHANGE_RACE);
        handler->PSendSysMessage("%s>>%s Your character is now flagged for a race change. Previously flagged customization must be finished before this one is displayed at login screen.", MSG_COLOR_CHOCOLATE, MSG_COLOR_SUBWHITE);

        return true;
    }

    static bool HandleFreedomFactionChangeCommand(ChatHandler* handler, const char* /*args*/) {
        Player* source = handler->GetSession()->GetPlayer();

        PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_ADD_AT_LOGIN_FLAG);
        stmt->setUInt16(0, uint16(AT_LOGIN_CHANGE_FACTION));
        stmt->setUInt32(1, source->GetGUIDLow());
        CharacterDatabase.Execute(stmt);

        source->SetAtLoginFlag(AT_LOGIN_CHANGE_FACTION);
        handler->PSendSysMessage("%s>>%s Your character is now flagged for a faction change. Previously flagged customization must be finished before this one is displayed at login screen.", MSG_COLOR_CHOCOLATE, MSG_COLOR_SUBWHITE);

        return true;
    }

    static bool HandleFreedomMoneyCommand(ChatHandler* handler, const char* /*args*/) {
        Player* source = handler->GetSession()->GetPlayer();
        source->SetMoney(int64(100000000000));
        handler->PSendSysMessage("%s>>%s Your money is reset.", MSG_COLOR_CHOCOLATE, MSG_COLOR_SUBWHITE);
        return true;
    }

    static bool HandleFreedomFixCommand(ChatHandler* handler, const char* /*args*/) {
        Player* source = handler->GetSession()->GetPlayer();
        source->DurabilityRepairAll(false, 0, false);
        handler->PSendSysMessage("%s>>%s All your character's items are repaired.", MSG_COLOR_CHOCOLATE, MSG_COLOR_SUBWHITE);
        return true;
    }

    static bool HandleFreedomReviveCommand(ChatHandler* handler, const char* /*args*/) {
        Player* source = handler->GetSession()->GetPlayer();
        if (source->IsAlive()) {
            handler->PSendSysMessage("%s.freedom revive: %s You are already alive.", MSG_COLOR_CHOCOLATE, MSG_COLOR_SUBWHITE);
            return true;
        }

        source->ResurrectPlayer(1.0f);
        source->SaveToDB();
        handler->PSendSysMessage("%s>>%s Resurrected with full health.", MSG_COLOR_CHOCOLATE, MSG_COLOR_SUBWHITE);
        return true;
    }

    static bool HandleFreedomBankCommand(ChatHandler* handler, const char* /*args*/)
    {
        Player* source = handler->GetSession()->GetPlayer();
        handler->GetSession()->SendShowBank(source->GetGUID());
        return true;
    }

    static bool HandleFreedomMailboxCommand(ChatHandler* handler, const char* /*args*/) {
        return true;
    }
    #pragma endregion UTILITIES

    #pragma region MODIFICATION
    // MODIFICATION -> CS_MODIFY-CMDs
    static bool HandleFreedomScaleCommand(ChatHandler* handler, const char* args)
    {
        if (!*args)
        {
            handler->PSendSysMessage(
                    "%s.freedom scale: %sPlease specify a decimal number.\n"
                    "%sSyntax: %s.freedom scale $newScale",
                    MSG_COLOR_CHOCOLATE, MSG_COLOR_SUBWHITE, MSG_COLOR_CHOCOLATE, MSG_COLOR_ORANGEY);
            return true;
        }

        Player* source = handler->GetSession()->GetPlayer();
        float scale_new = (float)atof((char*)args);
        float max = sConfigMgr->GetFloatDefault("Freedom.Modify.MaxScale", 3.0f);
        float min = sConfigMgr->GetFloatDefault("Freedom.Modify.MinScale", 0.1f);

        if (scale_new > max || scale_new < min)
        {
            handler->PSendSysMessage(
                    "%s.freedom scale: %sIncorrect value.\n"
                    "%sReason: %sSpecified value must be between %s%.2f%s and %s%.2f%s.",
                    MSG_COLOR_CHOCOLATE, MSG_COLOR_RED, MSG_COLOR_CHOCOLATE, MSG_COLOR_SUBWHITE, MSG_COLOR_ORANGEY, min, MSG_COLOR_SUBWHITE, MSG_COLOR_ORANGEY, max, MSG_COLOR_SUBWHITE);
            return true;
        }

        source->SetObjectScale(scale_new);
        handler->PSendSysMessage(
                "%s>>%s Your character's scale is changed to %s%d%%%s.",
                MSG_COLOR_CHOCOLATE, MSG_COLOR_SUBWHITE, MSG_COLOR_ORANGEY, (int) (scale_new*100), MSG_COLOR_SUBWHITE);

        return true;
    }

    static bool HandleFreedomSwimCommand(ChatHandler* handler, const char* args)
    {
        if (!*args)
        {
            handler->PSendSysMessage(
                    "%s.freedom swim: %sPlease specify a decimal number.\n"
                    "%sSyntax: %s.freedom swim $newSpeed",
                    MSG_COLOR_CHOCOLATE, MSG_COLOR_SUBWHITE, MSG_COLOR_CHOCOLATE, MSG_COLOR_ORANGEY);
            return true;
        }

        Player* source = handler->GetSession()->GetPlayer();
        float speed_new = (float)atof((char*)args);
        float max = sConfigMgr->GetFloatDefault("Freedom.Modify.MaxSwim", 10.0f);
        float min = sConfigMgr->GetFloatDefault("Freedom.Modify.MinSwim", 0.1f);

        if (speed_new > max || speed_new < min)
        {
            handler->PSendSysMessage(
                    "%s.freedom swim: %sIncorrect value.\n"
                    "%sReason: %sSpecified value must be between %s%.2f%s and %s%.2f%s.",
                    MSG_COLOR_CHOCOLATE, MSG_COLOR_RED, MSG_COLOR_CHOCOLATE, MSG_COLOR_SUBWHITE, MSG_COLOR_ORANGEY, min, MSG_COLOR_SUBWHITE, MSG_COLOR_ORANGEY, max, MSG_COLOR_SUBWHITE);
            return true;
        }

        source->SetSpeed(MOVE_SWIM, speed_new, true);
        source->SetSpeed(MOVE_SWIM_BACK, speed_new, true);
        handler->PSendSysMessage(
                "%s>>%s Your character's swim speed is changed to %s%d%%%s.",
                MSG_COLOR_CHOCOLATE, MSG_COLOR_SUBWHITE, MSG_COLOR_ORANGEY, (int) (speed_new*100), MSG_COLOR_SUBWHITE);

        return true;
    }

    static bool HandleFreedomWalkCommand(ChatHandler* handler, const char* args)
    {
        if (!*args)
        {
            handler->PSendSysMessage(
                    "%s.freedom walk: %sPlease specify a decimal number.\n"
                    "%sSyntax: %s.freedom walk $newSpeed",
                    MSG_COLOR_CHOCOLATE, MSG_COLOR_SUBWHITE, MSG_COLOR_CHOCOLATE, MSG_COLOR_ORANGEY);
            return true;
        }

        Player* source = handler->GetSession()->GetPlayer();
        float speed_new = (float)atof((char*)args);
        float max = sConfigMgr->GetFloatDefault("Freedom.Modify.MaxWalk", 10.0f);
        float min = sConfigMgr->GetFloatDefault("Freedom.Modify.MinWalk", 0.1f);

        if (speed_new > max || speed_new < min)
        {
            handler->PSendSysMessage(
                    "%s.freedom walk: %sIncorrect value.\n"
                    "%sReason: %sSpecified value must be between %s%.2f%s and %s%.2f%s.",
                    MSG_COLOR_CHOCOLATE, MSG_COLOR_RED, MSG_COLOR_CHOCOLATE, MSG_COLOR_SUBWHITE, MSG_COLOR_ORANGEY, min, MSG_COLOR_SUBWHITE, MSG_COLOR_ORANGEY, max, MSG_COLOR_SUBWHITE);
            return true;
        }

        source->SetSpeed(MOVE_WALK, speed_new, true);
        handler->PSendSysMessage(
                "%s>>%s Your character's walk speed is changed to %s%d%%%s.",
                MSG_COLOR_CHOCOLATE, MSG_COLOR_SUBWHITE, MSG_COLOR_ORANGEY, (int) (speed_new * 100), MSG_COLOR_SUBWHITE);

        return true;
    }

    static bool HandleFreedomRunCommand(ChatHandler* handler, const char* args)
    {
        if (!*args)
        {
            handler->PSendSysMessage(
                    "%s.freedom run: %sPlease specify a decimal number.\n"
                    "%sSyntax: %s.freedom run $newSpeed",
                    MSG_COLOR_CHOCOLATE, MSG_COLOR_SUBWHITE, MSG_COLOR_CHOCOLATE, MSG_COLOR_ORANGEY);
            return true;
        }

        Player* source = handler->GetSession()->GetPlayer();
        float speed_new = (float)atof((char*)args);
        float max = sConfigMgr->GetFloatDefault("Freedom.Modify.MaxRun", 10.0f);
        float min = sConfigMgr->GetFloatDefault("Freedom.Modify.MinRun", 0.1f);

        if (speed_new > max || speed_new < min)
        {
            handler->PSendSysMessage(
                    "%s.freedom run: %sIncorrect value.\n"
                    "%sReason: %sSpecified value must be between %s%.2f%s and %s%.2f%s.",
                    MSG_COLOR_CHOCOLATE, MSG_COLOR_RED, MSG_COLOR_CHOCOLATE, MSG_COLOR_SUBWHITE, MSG_COLOR_ORANGEY, min, MSG_COLOR_SUBWHITE, MSG_COLOR_ORANGEY, max, MSG_COLOR_SUBWHITE);
            return true;
        }

        source->SetSpeed(MOVE_RUN, speed_new, true);
        source->SetSpeed(MOVE_RUN_BACK, speed_new, true);
        handler->PSendSysMessage(
                "%s>>%s Your character's run speed is changed to %s%d%%%s.",
                MSG_COLOR_CHOCOLATE, MSG_COLOR_SUBWHITE, MSG_COLOR_ORANGEY, (int) (speed_new * 100), MSG_COLOR_SUBWHITE);

        return true;
    }

    static bool HandleFreedomSpeedCommand(ChatHandler* handler, const char* args)
    {
        if (!*args)
        {
            handler->PSendSysMessage(
                    "%s.freedom speed: %sPlease specify a decimal number.\n"
                    "%sSyntax: %s.freedom speed $newSpeed",
                    MSG_COLOR_CHOCOLATE, MSG_COLOR_SUBWHITE, MSG_COLOR_CHOCOLATE, MSG_COLOR_ORANGEY);
            return true;
        }

        Player* source = handler->GetSession()->GetPlayer();
        float speed_new = (float)atof((char*)args);
        float max = sConfigMgr->GetFloatDefault("Freedom.Modify.MaxSpeed", 10.0f);
        float min = sConfigMgr->GetFloatDefault("Freedom.Modify.MinSpeed", 0.1f);

        if (speed_new > max || speed_new < min)
        {
            handler->PSendSysMessage(
                    "%s.freedom speed: %sIncorrect value.\n"
                    "%sReason: %sSpecified value must be between %s%.2f%s and %s%.2f%s.",
                    MSG_COLOR_CHOCOLATE, MSG_COLOR_RED, MSG_COLOR_CHOCOLATE, MSG_COLOR_SUBWHITE, MSG_COLOR_ORANGEY, min, MSG_COLOR_SUBWHITE, MSG_COLOR_ORANGEY, max, MSG_COLOR_SUBWHITE);
            return true;
        }
        
        source->SetSpeed(MOVE_FLIGHT, speed_new, true);
        source->SetSpeed(MOVE_FLIGHT_BACK, speed_new, true);
        source->SetSpeed(MOVE_SWIM, speed_new, true);
        source->SetSpeed(MOVE_SWIM_BACK, speed_new, true);
        source->SetSpeed(MOVE_RUN, speed_new, true);
        source->SetSpeed(MOVE_RUN_BACK, speed_new, true);
        source->SetSpeed(MOVE_WALK, speed_new, true);
        source->SetSpeed(MOVE_WALK, speed_new, true);
        handler->PSendSysMessage(
                "%s>>%s Your character's all movement types speed is changed to %s%d%%%s.",
                MSG_COLOR_CHOCOLATE, MSG_COLOR_SUBWHITE, MSG_COLOR_ORANGEY, (int) (speed_new * 100), MSG_COLOR_SUBWHITE);

        return true;
    }

    static bool HandleFreedomDrunkCommand(ChatHandler* handler, const char* args)
    {
        if (!*args)
        {
            handler->PSendSysMessage(
                    "%s.freedom drunk: %sPlease specify a number between 0 and 100.\n"
                    "%sSyntax: %s.freedom speed $newPercentage",
                    MSG_COLOR_CHOCOLATE, MSG_COLOR_SUBWHITE, MSG_COLOR_CHOCOLATE, MSG_COLOR_ORANGEY);
            return true;
        }

        Player* source = handler->GetSession()->GetPlayer();
        uint8 drunklevel = (uint8)atoi(args);

        // if input really was not a number
        // TODO: replace with something better when low-priority improvements can be made
        std::string input = args;
        if (drunklevel == 0 && input[0] != '0')
        {
            handler->PSendSysMessage(
                    "%s.freedom drunk: %sPlease specify a number between 0 and 100.\n"
                    "%sSyntax: %s.freedom speed $newPercentage",
                    MSG_COLOR_CHOCOLATE, MSG_COLOR_SUBWHITE, MSG_COLOR_CHOCOLATE, MSG_COLOR_ORANGEY);
            return true;
        }

        if (drunklevel > 100)
            drunklevel = 100;

        source->SetDrunkValue(drunklevel);
        handler->PSendSysMessage(
                "%s>>%s Your character's drunk level is changed to %s%d%%%s.",
                MSG_COLOR_CHOCOLATE, MSG_COLOR_SUBWHITE, MSG_COLOR_ORANGEY, drunklevel, MSG_COLOR_SUBWHITE);

        return true;
    }

    // MODIFICATION -> FLY & WATERWALK
    static bool HandleFreedomWaterwalkCommand(ChatHandler* handler, const char* args)
    {
        if (!*args)
        {
            handler->PSendSysMessage(
                    "%s.freedom waterwalk: %sPlease specify %son%s or %soff%s.\n"
                    "%sSyntax: %s.freedom waterwalk $onOrOff",
                    MSG_COLOR_CHOCOLATE, MSG_COLOR_SUBWHITE, MSG_COLOR_ORANGEY, MSG_COLOR_SUBWHITE, MSG_COLOR_ORANGEY, MSG_COLOR_SUBWHITE, MSG_COLOR_CHOCOLATE, MSG_COLOR_ORANGEY);
            return true;
        }

        Player* source = handler->GetSession()->GetPlayer();

        // on & off
        std::string subcommand = args;
        std::transform(subcommand.begin(), subcommand.end(), subcommand.begin(), ::tolower);
        if (subcommand == "on")
        {
            handler->PSendSysMessage(
                    "%s>>%s Your character's waterwalk is toggled %son%s.",
                    MSG_COLOR_CHOCOLATE, MSG_COLOR_SUBWHITE, MSG_COLOR_ORANGEY, MSG_COLOR_SUBWHITE);
            source->SetWaterWalking(true);
            return true;
        }
        else if (subcommand == "off")
        {
            //SetWaterWalking does not turn off properly, WORKAROUND: sending packets directly
            Unit* source_unit = handler->GetSession()->GetPlayer()->ToUnit();
            source_unit->RemoveUnitMovementFlag(MOVEMENTFLAG_WATERWALKING);
            Movement::PacketSender(source_unit, SMSG_SPLINE_MOVE_SET_LAND_WALK, SMSG_MOVE_LAND_WALK).Send();

            handler->PSendSysMessage(
                    "%s>>%s Your character's waterwalk is toggled %soff%s.",
                    MSG_COLOR_CHOCOLATE, MSG_COLOR_SUBWHITE, MSG_COLOR_ORANGEY, MSG_COLOR_SUBWHITE);
            return true;
        }

        // did not specify ON or OFF value
        handler->PSendSysMessage(
                "%s.freedom waterwalk: %sPlease specify %son%s or %soff%s.\n"
                "%sSyntax: %s.freedom waterwalk $onOrOff",
                MSG_COLOR_CHOCOLATE, MSG_COLOR_SUBWHITE, MSG_COLOR_ORANGEY, MSG_COLOR_SUBWHITE, MSG_COLOR_ORANGEY, MSG_COLOR_SUBWHITE, MSG_COLOR_CHOCOLATE, MSG_COLOR_ORANGEY);

        return true;
    }

    static bool HandleFreedomFlyCommand(ChatHandler* handler, const char* args)
    {
        if (!*args)
        {
            handler->PSendSysMessage(
                    "%s.freedom fly: %sPlease specify a decimal number or toggle sub-command %son%s / %soff%s.\n"
                    "%sSyntax: %s.freedom fly $onOrOffOrNewSpeed",
                    MSG_COLOR_CHOCOLATE, MSG_COLOR_SUBWHITE, MSG_COLOR_ORANGEY, MSG_COLOR_SUBWHITE, MSG_COLOR_ORANGEY, MSG_COLOR_SUBWHITE, MSG_COLOR_CHOCOLATE, MSG_COLOR_ORANGEY);
            return true;
        }
        
        Player* source = handler->GetSession()->GetPlayer();
        
        // on & off
        std::string subcommand = args;
        std::transform(subcommand.begin(), subcommand.end(), subcommand.begin(), ::tolower);
        if (subcommand == "on")
        {
            handler->PSendSysMessage(
                    "%s>>%s Your character's fly is toggled %son%s.",
                    MSG_COLOR_CHOCOLATE, MSG_COLOR_SUBWHITE, MSG_COLOR_ORANGEY, MSG_COLOR_SUBWHITE);
            source->SetCanFly(true);
            return true;
        }
        else if (subcommand == "off")
        {
            //SetCanFly not turning off properly, WORKAROUND: sending packets directly
            Unit* source_unit = handler->GetSession()->GetPlayer()->ToUnit();
            source_unit->RemoveUnitMovementFlag(MOVEMENTFLAG_CAN_FLY | MOVEMENTFLAG_MASK_MOVING_FLY);
            if (!source_unit->IsLevitating())
                source_unit->SetFall(true);
            Movement::PacketSender(source_unit, SMSG_SPLINE_MOVE_UNSET_FLYING, SMSG_MOVE_UNSET_CAN_FLY).Send();

            handler->PSendSysMessage(
                    "%s>>%s Your character's fly is toggled %soff%s.",
                    MSG_COLOR_CHOCOLATE, MSG_COLOR_SUBWHITE, MSG_COLOR_ORANGEY, MSG_COLOR_SUBWHITE);
            return true;
        }

        float speed_new = (float)atof((char*)args);
        float max = sConfigMgr->GetFloatDefault("Freedom.Modify.MaxFly", 10.0f);
        float min = sConfigMgr->GetFloatDefault("Freedom.Modify.MinFly", 0.1f);

        if (speed_new > max || speed_new < min)
        {
            handler->PSendSysMessage(
                    "%s.freedom fly: %sIncorrect value.\n"
                    "%sReason: %sSpecified value must be between %s%.2f%s and %s%.2f%s if changing speed.\n"
                    "%sHint: %sIf you wish to toggle your speed on or off, simply use either %s.f fly on %sor %s.f fly off%s command.",
                    MSG_COLOR_CHOCOLATE, MSG_COLOR_RED, MSG_COLOR_CHOCOLATE, MSG_COLOR_SUBWHITE, MSG_COLOR_ORANGEY, min, MSG_COLOR_SUBWHITE, MSG_COLOR_ORANGEY, max, MSG_COLOR_SUBWHITE, MSG_COLOR_CHOCOLATE, MSG_COLOR_SUBWHITE, MSG_COLOR_ORANGEY, MSG_COLOR_SUBWHITE, MSG_COLOR_ORANGEY, MSG_COLOR_SUBWHITE);
            return true;
        }

        source->SetSpeed(MOVE_FLIGHT, speed_new, true);
        source->SetSpeed(MOVE_FLIGHT_BACK, speed_new, true);

        handler->PSendSysMessage(
                "%s>>%s Your character's fly speed is changed to %s%d%%%s.",
                MSG_COLOR_CHOCOLATE, MSG_COLOR_SUBWHITE, MSG_COLOR_ORANGEY, (int) (speed_new*100), MSG_COLOR_SUBWHITE);

        return true;
    }

    // MODIFICATION -> MORPHING
    static bool HandleFreedomMorphCommand(ChatHandler* handler, const char* args) 
    {
        if (!*args)
        {
            handler->PSendSysMessage(
                    "%s.freedom morph: %sPlease specify a name of one of this character's morphs.\n"
                    "%sSyntax: %s.freedom morph $morphName\n"
                    "%sHint: %sUse %s.freedom morph list %scommand to search for available morphs for this character.",
                    MSG_COLOR_CHOCOLATE, MSG_COLOR_SUBWHITE, MSG_COLOR_CHOCOLATE, MSG_COLOR_ORANGEY, MSG_COLOR_CHOCOLATE, MSG_COLOR_SUBWHITE, MSG_COLOR_ORANGEY, MSG_COLOR_SUBWHITE);
            return true;
        }

        Player* source = handler->GetSession()->GetPlayer();
        std::string morph_name_exact;
        morph_name_exact = strtok((char *) args, " ");

        PreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_SEL_FREEDOM_MORPH_EXACT);
        stmt->setString(0, morph_name_exact);
        stmt->setUInt32(1, source->GetGUIDLow());
        PreparedQueryResult result = WorldDatabase.Query(stmt);

        if (!result)
        {
            handler->PSendSysMessage(
                    "%s.freedom morph: %sMorph not found.\n"
                    "%sReason: %sNo such morph with that exact name for this character was found.\n"
                    "%sHint: %sUse %s.freedom morph list %scommand to search for available morphs for this character.",
                    MSG_COLOR_CHOCOLATE, MSG_COLOR_RED, MSG_COLOR_CHOCOLATE, MSG_COLOR_SUBWHITE, MSG_COLOR_CHOCOLATE, MSG_COLOR_SUBWHITE, MSG_COLOR_ORANGEY, MSG_COLOR_SUBWHITE);
            return true;
        }

        Field * fields = result->Fetch();
        source->SetDisplayId(fields[2].GetUInt32());

        handler->PSendSysMessage(
                "%s>>%s Successfully morphed into %s%s%s.",
                MSG_COLOR_CHOCOLATE, MSG_COLOR_SUBWHITE, MSG_COLOR_ORANGEY, morph_name_exact.c_str(), MSG_COLOR_SUBWHITE);

        return true;
    }

    static bool HandleFreedomDemorphCommand(ChatHandler* handler, const char* args)
    {
        Player* source = handler->GetSession()->GetPlayer();

        source->DeMorph();

        handler->PSendSysMessage(
                "%s>>%s Successfully demorphed.",
                MSG_COLOR_CHOCOLATE, MSG_COLOR_SUBWHITE);

        return true;
    }

    static bool HandleFreedomMorphAddCommand(ChatHandler* handler, const char* args)
    {
        if (!*args)
        {
            handler->PSendSysMessage(
                    "%s.freedom morph add: %sPlease specify a name and a display ID of the new morph for the target character (can specify player name as third parameter to find the target).\n"
                    "%sSyntax: %s.freedom morph add $morphName $displayId [$playerName]",
                    MSG_COLOR_CHOCOLATE, MSG_COLOR_SUBWHITE, MSG_COLOR_CHOCOLATE, MSG_COLOR_ORANGEY);
            return true;
        }
        
        // extract parameters
        char * params[3];
        params[0] = strtok((char *)args, " ");
        params[1] = strtok(NULL, " ");
        params[2] = strtok(NULL, " ");

        if (!params[1])
        {
            handler->PSendSysMessage(
                    "%s.freedom morph add: %sPlease specify a name and a display ID of the new morph for the target character (can specify player name as third parameter to find the target).\n"
                    "%sSyntax: %s.freedom morph add $morphName $displayId [$playerName]",
                    MSG_COLOR_CHOCOLATE, MSG_COLOR_SUBWHITE, MSG_COLOR_CHOCOLATE, MSG_COLOR_ORANGEY);
            return true;
        }

        // attempt to get valid target, priority to third parameter instead of in-game targeting
        Player* source = handler->GetSession()->GetPlayer();
        Player * target = NULL;
        std::string player_name = "Target";
        std::string morph_name_exact = params[0];
        uint32 morph_display_id = atol(params[1]);
        uint32 char_guid = NULL;
        PreparedStatement * stmt;

        if (!params[2]) // via DB
        {
            target = handler->getSelectedPlayer();
            if (target)
            {
                char_guid = target->GetGUIDLow();
                player_name = target->GetName();
            }
        }
        else // via in-game target
        {
            player_name = params[2];
            stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHAR_GUID_BY_NAME);
            stmt->setString(0, params[2]);
            PreparedQueryResult result = CharacterDatabase.Query(stmt);
            if (result)
            {
                Field * fields = result->Fetch();
                char_guid = fields[0].GetUInt32();
            }
        }

        if (!char_guid)
        {
            handler->PSendSysMessage(
                    "%s.freedom morph add: %sPlayer not found.\n"
                    "%sReason: %s%s%s is not a player or no character with such name exists.",
                    MSG_COLOR_CHOCOLATE, MSG_COLOR_RED, MSG_COLOR_CHOCOLATE, MSG_COLOR_ORANGEY, player_name.c_str(), MSG_COLOR_SUBWHITE);
            return true;
        }

        if (!morph_display_id)
        {
            handler->PSendSysMessage(
                "%s.freedom morph add: %sInvalid display ID.\n"
                "%sReason: %sDisplay ID (second parameter) you supplied was invalid.",
                MSG_COLOR_CHOCOLATE, MSG_COLOR_RED, MSG_COLOR_CHOCOLATE, MSG_COLOR_SUBWHITE);
            return true;
        }

        // check for duplicate morph name
        stmt = WorldDatabase.GetPreparedStatement(WORLD_SEL_FREEDOM_MORPH_EXACT);
        stmt->setString(0, morph_name_exact);
        stmt->setUInt32(1, char_guid);
        PreparedQueryResult result = WorldDatabase.Query(stmt);
        
        if (result)
        {
            handler->PSendSysMessage(
                    "%s.freedom morph add: %sAlready exists.\n"
                    "%sReason: %s%s%s already has a morph under that name.",
                    MSG_COLOR_CHOCOLATE, MSG_COLOR_RED, MSG_COLOR_CHOCOLATE, MSG_COLOR_ORANGEY, player_name.c_str(), MSG_COLOR_SUBWHITE);
            return true;
        }

        // save to DB and notify target of success
        stmt = WorldDatabase.GetPreparedStatement(WORLD_INS_FREEDOM_MORPH);
        stmt->setUInt32(0, char_guid);
        stmt->setString(1, morph_name_exact);
        stmt->setUInt32(2, morph_display_id);
        stmt->setUInt32(3, source->GetSession()->GetAccountId());
        WorldDatabase.Execute(stmt);

        if (handler->needReportToTarget(target))
            ChatHandler(target->GetSession()).PSendSysMessage(
                    "%s>>%s Player %s added a morph under the name of %s%s%s and display id of %s%u%s to you.", 
                    MSG_COLOR_CHOCOLATE, MSG_COLOR_SUBWHITE, handler->GetNameLink().c_str(), MSG_COLOR_ORANGEY, morph_name_exact.c_str(), MSG_COLOR_SUBWHITE, MSG_COLOR_ORANGEY, morph_display_id, MSG_COLOR_SUBWHITE);

        handler->PSendSysMessage(
                "%s>>%s Successfully added a morph under the name of %s%s%s and display id of %s%u%s to %s%s%s.",
                MSG_COLOR_CHOCOLATE, MSG_COLOR_SUBWHITE, MSG_COLOR_ORANGEY, morph_name_exact.c_str(), MSG_COLOR_SUBWHITE, MSG_COLOR_ORANGEY, morph_display_id, MSG_COLOR_SUBWHITE, MSG_COLOR_ORANGEY, player_name.c_str(), MSG_COLOR_SUBWHITE);

        return true;
    }

    static bool HandleFreedomMorphDelCommand(ChatHandler* handler, const char* args)
    {
        if (!*args)
        {
            handler->PSendSysMessage(
                    "%s.freedom morph delete: %sPlease specify a name of the morph of the target character (can specify player name as second parameter to find the target).\n"
                    "%sSyntax: %s.freedom morph delete $morphName [$playerName]",
                    MSG_COLOR_CHOCOLATE, MSG_COLOR_SUBWHITE, MSG_COLOR_CHOCOLATE, MSG_COLOR_ORANGEY);
            return true;
        }

        // extract parameters
        char * params[2];
        params[0] = strtok((char *)args, " ");
        params[1] = strtok(NULL, " ");

        // attempt to get valid target, priority to third parameter instead of in-game targeting
        Player* source = handler->GetSession()->GetPlayer();
        Player * target = NULL;
        uint32 char_guid = NULL;
        std::string player_name = "Target";
        std::string morph_name_exact = params[0];
        PreparedStatement * stmt;

        if (!params[1]) // via in-game target
        {
            target = handler->getSelectedPlayer();
            if (target)
            {
                char_guid = target->GetGUIDLow();
                player_name = target->GetName();
            }
        }
        else // via DB
        {
            player_name = params[1];
            stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHAR_GUID_BY_NAME);
            stmt->setString(0, params[1]);
            PreparedQueryResult result = CharacterDatabase.Query(stmt);
            if (result)
            {
                Field * fields = result->Fetch();
                char_guid = fields[0].GetUInt32();
            }
        }

        if (!char_guid)
        {
            handler->PSendSysMessage(
                    "%s.freedom morph delete: %sPlayer not found.\n"
                    "%sReason: %s%s%s is not a player or no character with such name exists.",
                    MSG_COLOR_CHOCOLATE, MSG_COLOR_RED, MSG_COLOR_CHOCOLATE, MSG_COLOR_ORANGEY, player_name.c_str(), MSG_COLOR_SUBWHITE);
            return true;
        }

        // check for duplicate morph name
        stmt = WorldDatabase.GetPreparedStatement(WORLD_SEL_FREEDOM_MORPH_EXACT);
        stmt->setString(0, morph_name_exact);
        stmt->setUInt32(1, char_guid);
        PreparedQueryResult result = WorldDatabase.Query(stmt);

        if (!result)
        {
            handler->PSendSysMessage(
                    "%s.freedom morph delete: %sNot found.\n"
                    "%sReason: %s%s%s does not have a morph under that name.\n"
                    "%sHint: %sUse %s.freedom morph list --p [$playerName] %scommand to search for assigned morphs of the target or $playerName character.",
                    MSG_COLOR_CHOCOLATE, MSG_COLOR_RED, MSG_COLOR_CHOCOLATE, MSG_COLOR_ORANGEY, player_name.c_str(), MSG_COLOR_SUBWHITE, MSG_COLOR_CHOCOLATE, MSG_COLOR_SUBWHITE, MSG_COLOR_ORANGEY, MSG_COLOR_SUBWHITE);
            return true;
        }

        // save to DB and notify target of success
        stmt = WorldDatabase.GetPreparedStatement(WORLD_DEL_FREEDOM_MORPH);
        stmt->setString(0, morph_name_exact);
        stmt->setUInt32(1, char_guid);
        WorldDatabase.Execute(stmt);

        if (handler->needReportToTarget(target))
            ChatHandler(target->GetSession()).PSendSysMessage(
                    "%s>>%s Player %s removed a morph under the name of %s%s%s from you.",
                    MSG_COLOR_CHOCOLATE, MSG_COLOR_SUBWHITE, handler->GetNameLink().c_str(), MSG_COLOR_ORANGEY, morph_name_exact.c_str(), MSG_COLOR_SUBWHITE);

        handler->PSendSysMessage(
                "%s>>%s Successfully removed a morph under the name of %s%s%s from %s%s%s.",
                MSG_COLOR_CHOCOLATE, MSG_COLOR_SUBWHITE, MSG_COLOR_ORANGEY, morph_name_exact.c_str(), MSG_COLOR_SUBWHITE, MSG_COLOR_ORANGEY, player_name.c_str(), MSG_COLOR_SUBWHITE);

        return true;
    }

    static bool HandleFreedomMorphListCommand(ChatHandler* handler, const char* args)
    {
        //if (!*args)
        //{
        //    handler->PSendSysMessage(
        //            "%s.freedom morph list: %sList all your character's morphs.\n"
        //            "%sSyntax: %s.freedom morph list",
        //            MSG_COLOR_CHOCOLATE, MSG_COLOR_SUBWHITE, MSG_COLOR_CHOCOLATE, MSG_COLOR_ORANGEY);
        //    handler->PSendSysMessage(
        //            "%sSpecial Parameters: %s--f $morphNamePart%s, %s--filter $morphNamePart%s : Makes the command to filter listed morphs, showing ones who start with $morphNamePart (case-insensitive).",
        //            MSG_COLOR_CHOCOLATE, MSG_COLOR_ORANGEY, MSG_COLOR_SUBWHITE, MSG_COLOR_ORANGEY, MSG_COLOR_SUBWHITE);
        //    if (handler->GetSession()->GetSecurity() >= SEC_GAMEMASTER) 
        //    {
        //        handler->PSendSysMessage(
        //                "%sSpecial Parameters: %s--p [$playerName]%s, %s--player [$playerName]%s : Makes the command to list targeted or $playerName morphs instead.",
        //                MSG_COLOR_CHOCOLATE, MSG_COLOR_ORANGEY, MSG_COLOR_SUBWHITE, MSG_COLOR_ORANGEY, MSG_COLOR_SUBWHITE);
        //    }
        //    return true;
        //}

        // extract parameters
        char * params[4];
        params[0] = strtok((char *)args, " ");
        params[1] = strtok(NULL, " ");
        params[2] = strtok(NULL, " ");
        params[3] = strtok(NULL, " ");
        Player * target = handler->GetSession()->GetPlayer();
        uint32 char_guid = target->GetGUIDLow();
        PreparedStatement * stmt;
        std::string morph_name_part = "";
        std::string player_name = target->GetName();
        int param_index;

        // special parameter switches, true: not used yet, false: used
        bool sp_player = true;
        bool sp_filter = true;

        // check & set special parameters
        for (param_index = 0; param_index < 4;)
        {
            if (!params[param_index])
                break;
            std::string temp = params[param_index];
            if (!isSpecialParam(temp))
                break;

            if ((temp == "--p") && handler->GetSession()->GetSecurity() >= SEC_GAMEMASTER && sp_player)
            {
                param_index++;
                if (!params[param_index] || isSpecialParam(params[param_index]))
                {
                    handler->PSendSysMessage(
                        "%s.freedom morph list [--p]: %sMissing argument.\n"
                        "%sReason: %sSpecial parameter %s--p%s requires an argument followed after it.",
                        MSG_COLOR_CHOCOLATE, MSG_COLOR_RED, MSG_COLOR_CHOCOLATE, MSG_COLOR_SUBWHITE, MSG_COLOR_ORANGEY, MSG_COLOR_SUBWHITE);
                    return true;
                }

                char_guid = NULL;
                player_name = params[param_index];

                stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHAR_GUID_BY_NAME);
                stmt->setString(0, params[param_index]);
                PreparedQueryResult result = CharacterDatabase.Query(stmt);
                if (result)
                {
                    Field * fields = result->Fetch();
                    char_guid = fields[0].GetUInt32();
                }
                else
                {
                    break;
                }

                sp_player = false;
            }
            else if ((temp == "--f") && sp_filter)
            {
                param_index++;
                if (!params[param_index] || isSpecialParam(params[param_index]))
                {
                    handler->PSendSysMessage(
                            "%s.freedom morph list [--f]: %sMissing argument.\n"
                            "%sReason: %sSpecial parameter %s--f%s requires an argument followed after it.",
                            MSG_COLOR_CHOCOLATE, MSG_COLOR_RED, MSG_COLOR_CHOCOLATE, MSG_COLOR_SUBWHITE, MSG_COLOR_ORANGEY, MSG_COLOR_SUBWHITE);
                    return true;
                }

                morph_name_part = params[param_index];
                replaceAll(morph_name_part, "%", "\\%"); //escape inputted % wildcards
                replaceAll(morph_name_part, "_", "\\_"); //escape inputted _ wildcards

                sp_filter = false;
            }
            else
            {
                handler->PSendSysMessage(
                        "%s.freedom morph list: %sInvalid special parameter.\n"
                        "%sReason: %sSpecial parameter (parameter which starts with %s--%s) you used was incorrect (already used or no such special parameter is available).\n"
                        "%sHint: %sRemember that special parameters with their arguments have to be entered before command's normal parameters and you can't enter same special parameter twice!",
                        MSG_COLOR_CHOCOLATE, MSG_COLOR_RED, MSG_COLOR_CHOCOLATE, MSG_COLOR_SUBWHITE, MSG_COLOR_ORANGEY, MSG_COLOR_SUBWHITE, MSG_COLOR_CHOCOLATE, MSG_COLOR_SUBWHITE);
                return true;
            }

            param_index++;
        }
        
        if (!char_guid)
        {
            handler->PSendSysMessage(
                    "%s.freedom morph list [--player]: %sPlayer not found.\n"
                    "%sReason: %sTarget is not a player or no character with such name exists.",
                    MSG_COLOR_CHOCOLATE, MSG_COLOR_RED, MSG_COLOR_CHOCOLATE, MSG_COLOR_SUBWHITE);
            return true;
        }

        morph_name_part += "%";
        stmt = WorldDatabase.GetPreparedStatement(WORLD_SEL_FREEDOM_MORPH);
        stmt->setString(0, morph_name_part);
        stmt->setUInt32(1, char_guid);
        PreparedQueryResult result = WorldDatabase.Query(stmt);

        if (!result)
        {
            handler->PSendSysMessage(
                    "%s>>%s No morphs found.",
                    MSG_COLOR_CHOCOLATE, MSG_COLOR_SUBWHITE);
            return true;
        }

        handler->PSendSysMessage(
                "%s>>%s Listing %s%u%s morph(s) for character %s%s%s...",
                MSG_COLOR_CHOCOLATE, MSG_COLOR_SUBWHITE, MSG_COLOR_ORANGEY, uint32(result->GetRowCount()), MSG_COLOR_SUBWHITE, MSG_COLOR_ORANGEY, player_name.c_str(), MSG_COLOR_SUBWHITE);
        
        do 
        {
            Field * fields = result->Fetch();
            handler->PSendSysMessage(
            "%s%u - %s%s",
            MSG_COLOR_SUBWHITE, fields[2].GetUInt32(), MSG_COLOR_ORANGEY, fields[1].GetCString());
        } while (result->NextRow());

        return true;
    }
    #pragma endregion MODIFICATION

    #pragma region LOCATIONAL
    // LOCATIONAL -> SUMMON
    static bool HandleFreedomSummonCommand(ChatHandler* handler, const char* args)
    {
        if (!*args)
        {
            handler->PSendSysMessage(
                    "%s.freedom summon: %sPlease specify the name of the player you wish to summon to your location.\n"
                    "%sSyntax: %s.freedom summon $playerName",
                    MSG_COLOR_CHOCOLATE, MSG_COLOR_SUBWHITE, MSG_COLOR_CHOCOLATE, MSG_COLOR_ORANGEY);
            return true;
        }

        Player* source = handler->GetSession()->GetPlayer();
        Player* target = sObjectAccessor->FindPlayerByName(args);

        if (!target || target->isBeingLoaded())
        {
            handler->PSendSysMessage(
                    "%s.freedom summon: %sSummon failed.\n"
                    "%sReason: %sPlayer %s%s%s is either offline, currently loading or does not exist.",
                    MSG_COLOR_CHOCOLATE, MSG_COLOR_RED, MSG_COLOR_CHOCOLATE, MSG_COLOR_SUBWHITE, MSG_COLOR_ORANGEY, args, MSG_COLOR_SUBWHITE);
            return true;
        }

        if (target->GetSocial()->HasIgnore(source->GetGUIDLow()))
        {
            handler->PSendSysMessage(
                    "%s.freedom summon: %sSummon failed.\n"
                    "%sReason: %sPlayer %s[%s]%s is ignoring you.",
                    MSG_COLOR_CHOCOLATE, MSG_COLOR_RED, MSG_COLOR_CHOCOLATE, MSG_COLOR_SUBWHITE, MSG_COLOR_ORANGEY, target->GetName().c_str(), MSG_COLOR_SUBWHITE);
            return true;
        }

        if (target->IsGameMaster())
        {
            handler->PSendSysMessage(
                    "%s.freedom summon: %sSummon failed.\n"
                    "%sReason: %sYou cannot summon a staff member with GM mode on.", 
                    MSG_COLOR_CHOCOLATE, MSG_COLOR_RED, MSG_COLOR_CHOCOLATE, MSG_COLOR_SUBWHITE);
            return true;
        }

        target->SetSummonPoint(source->GetMapId(), source->GetPositionX(), source->GetPositionY(), source->GetPositionZ());

        WorldPacket data(SMSG_SUMMON_REQUEST, 8 + 4 + 4);
        ObjectGuid SummonerGUID = source->GetGUID();

        data.WriteBit(SummonerGUID[0]);
        data.WriteBit(SummonerGUID[6]);
        data.WriteBit(SummonerGUID[3]);
        data.WriteBit(SummonerGUID[2]);
        data.WriteBit(SummonerGUID[1]);
        data.WriteBit(SummonerGUID[4]);
        data.WriteBit(SummonerGUID[7]);
        data.WriteBit(SummonerGUID[5]);

        data.WriteByteSeq(SummonerGUID[4]);
        data << uint32(source->GetZoneId());                  // summoner zone
        data.WriteByteSeq(SummonerGUID[7]);
        data.WriteByteSeq(SummonerGUID[3]);
        data.WriteByteSeq(SummonerGUID[1]);
        data << uint32(MAX_PLAYER_SUMMON_DELAY*IN_MILLISECONDS); // auto decline after msecs
        data.WriteByteSeq(SummonerGUID[2]);
        data.WriteByteSeq(SummonerGUID[6]);
        data.WriteByteSeq(SummonerGUID[5]);
        data.WriteByteSeq(SummonerGUID[0]);
        target->GetSession()->SendPacket(&data);

        handler->PSendSysMessage(
                "%s>>%s Successfully sent a summon request to player %s%s.", 
                MSG_COLOR_CHOCOLATE, MSG_COLOR_SUBWHITE, MSG_COLOR_ORANGEY, target->GetName().c_str());

        return true;
    }

    // LOCATIONAL -> TELEPORT
    static bool HandleFreedomTeleCommand(ChatHandler* handler, const char* args)
    {
        if (!*args)
        {
            handler->PSendSysMessage(
                    "%s.freedom teleport: %sPlease specify a name for the location to teleport to.\n"
                    "%sSyntax: %s.freedom teleport $teleNamePart\n"
                    "%sHint: %sUse %s.f tele list $namePart %scommand to search for available destinations.", 
                    MSG_COLOR_CHOCOLATE, MSG_COLOR_SUBWHITE, MSG_COLOR_CHOCOLATE, MSG_COLOR_ORANGEY, MSG_COLOR_CHOCOLATE, MSG_COLOR_SUBWHITE, MSG_COLOR_ORANGEY, MSG_COLOR_SUBWHITE);
            return true;
        }

        std::string tele_name_exact;
        tele_name_exact = strtok((char *)args, " ");;
        std::string tele_name_start = tele_name_exact;
        Player* source = handler->GetSession()->GetPlayer();

        if (source->IsInCombat())
        {
            handler->PSendSysMessage(
                "%s.freedom teleport: %sCan't teleport currently.\n"
                "%sReason: %sYou cannot teleport while in combat.",
                MSG_COLOR_CHOCOLATE, MSG_COLOR_RED, MSG_COLOR_CHOCOLATE, MSG_COLOR_SUBWHITE);
            return true;
        }

        // refine query parameter for LIKE statement
        replaceAll(tele_name_start, "%", "\\%"); //disable inputted % wildcards
        replaceAll(tele_name_start, "_", "\\_"); //disable inputted _ wildcards
        tele_name_start += "%"; // ensure that tele_name_start can end with anything

        // get exact match
        PreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_SEL_FREEDOM_TELE_EXACT);
        stmt->setString(0, tele_name_exact);
        PreparedQueryResult result = WorldDatabase.Query(stmt);

        // if exact match failed, get first teleport, which name starts with given parameter
        if (!result) {
            stmt = WorldDatabase.GetPreparedStatement(WORLD_SEL_FREEDOM_TELE);
            stmt->setString(0, tele_name_start);
            result = WorldDatabase.Query(stmt);
        }

        if (!result)
        {
            handler->PSendSysMessage(
                    "%s.freedom teleport: %sTeleport not found.\n"
                    "%sReason: %sNo teleport using that string was found.\n"
                    "%sHint: %sUse %s.f tele list [$namePart] %scommand to search for available destinations.",
                    MSG_COLOR_CHOCOLATE, MSG_COLOR_RED, MSG_COLOR_CHOCOLATE, MSG_COLOR_SUBWHITE, MSG_COLOR_CHOCOLATE, MSG_COLOR_SUBWHITE, MSG_COLOR_ORANGEY, MSG_COLOR_SUBWHITE);
            return true;
        }

        Field *fields = result->Fetch();

        if (source->IsInFlight())
        {
            source->GetMotionMaster()->MovementExpired();
            source->CleanupAfterTaxiFlight();
        }

        source->SaveRecallPosition();
        //             0            1           2           3        4     5      6
        // "SELECT position_x, position_y, position_z, orientation, map, name, gm_uid FROM freedom_tele WHERE name LIKE/= ?",
        source->TeleportTo(uint32(fields[4].GetUInt16()), fields[0].GetFloat(), fields[1].GetFloat(), fields[2].GetFloat(), fields[3].GetFloat());

        handler->PSendSysMessage(
            "%s>>%s You have been successfully teleported to %s%s %slocation.",
            MSG_COLOR_CHOCOLATE, MSG_COLOR_SUBWHITE, MSG_COLOR_ORANGEY, fields[5].GetCString(), MSG_COLOR_SUBWHITE);

        return true;
    }

    static bool HandleFreedomTeleAddCommand(ChatHandler* handler, const char* args)
    {
        if (!*args)
        {
            handler->PSendSysMessage(
                "%s.freedom teleport add: %sPlease specify a name for the freedom teleport you want to add.\n"
                "%sSyntax: %s.freedom teleport add $newTeleName",
                MSG_COLOR_CHOCOLATE, MSG_COLOR_SUBWHITE, MSG_COLOR_CHOCOLATE, MSG_COLOR_ORANGEY);
            return true;
        }

        std::string tele_name_new = strtok((char*)args, " ");
        Player* source = handler->GetSession()->GetPlayer();

        PreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_SEL_FREEDOM_TELE_EXACT);
        stmt->setString(0, tele_name_new);
        PreparedQueryResult result = WorldDatabase.Query(stmt);

        if (result)
        {
            handler->PSendSysMessage(
                "%s.freedom teleport add: %sAlready exists.\n"
                "%sReason: %sThe name of the freedom teleport you want to create already exists.\n"
                "%sHint: %sUse a different name or delete the existing one using %s.f tele del $name %scommand.",
                MSG_COLOR_CHOCOLATE, MSG_COLOR_RED, MSG_COLOR_CHOCOLATE, MSG_COLOR_SUBWHITE, MSG_COLOR_CHOCOLATE, MSG_COLOR_SUBWHITE, MSG_COLOR_ORANGEY, MSG_COLOR_SUBWHITE);
            return true;
        }

        stmt = WorldDatabase.GetPreparedStatement(WORLD_INS_FREEDOM_TELE);
        stmt->setFloat(0, source->GetPositionX());
        stmt->setFloat(1, source->GetPositionY());
        stmt->setFloat(2, source->GetPositionZ());
        stmt->setFloat(3, source->GetOrientation());
        stmt->setUInt32(4, source->GetMapId());
        stmt->setString(5, tele_name_new);
        stmt->setUInt32(6, source->GetSession()->GetAccountId());
        WorldDatabase.Execute(stmt);

        handler->PSendSysMessage(
            "%s>>%s Teleport (Freedom-type) with the name %s%s%s successfully created.",
            MSG_COLOR_CHOCOLATE, MSG_COLOR_SUBWHITE, MSG_COLOR_ORANGEY, tele_name_new.c_str(), MSG_COLOR_SUBWHITE);

        return true;
    }

    static bool HandleFreedomTeleDelCommand(ChatHandler* handler, const char* args)
    {
        if (!*args)
        {
            handler->PSendSysMessage(
                "%s.freedom teleport delete: %sPlease specify a name of the freedom teleport you want to delete.\n"
                "%sSyntax: %s.freedom teleport delete $teleName\n"
                "%sHint: %sUse %s.f tele list $namePart %scommand to search for freedom teleports.",
                MSG_COLOR_CHOCOLATE, MSG_COLOR_SUBWHITE, MSG_COLOR_CHOCOLATE, MSG_COLOR_ORANGEY, MSG_COLOR_CHOCOLATE, MSG_COLOR_SUBWHITE, MSG_COLOR_ORANGEY, MSG_COLOR_SUBWHITE);
            return true;
        }

        std::string tele_name_delete;
        tele_name_delete = strtok((char*)args, " ");

        PreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_SEL_FREEDOM_TELE_EXACT);
        stmt->setString(0, tele_name_delete);
        PreparedQueryResult result = WorldDatabase.Query(stmt);

        if (!result)
        {
            handler->PSendSysMessage(
                "%s.freedom teleport delete: %sNot found.\n"
                "%sReason: %sThe name of the freedom teleport you want to delete is not found in the freedom teleport list.\n"
                "%sHint: %sRemember to specify full teleport's name, just part of the name will not work, however, case-sensitive name is not required.",
                MSG_COLOR_CHOCOLATE, MSG_COLOR_RED, MSG_COLOR_CHOCOLATE, MSG_COLOR_SUBWHITE, MSG_COLOR_CHOCOLATE, MSG_COLOR_SUBWHITE);
            return true;
        }

        stmt = WorldDatabase.GetPreparedStatement(WORLD_DEL_FREEDOM_TELE);
        stmt->setString(0, tele_name_delete);
        WorldDatabase.Execute(stmt);

        handler->PSendSysMessage(
            "%s>>%s Teleport (Freedom-type) with the name %s%s%s successfully deleted.",
            MSG_COLOR_CHOCOLATE, MSG_COLOR_SUBWHITE, MSG_COLOR_ORANGEY, tele_name_delete.c_str(), MSG_COLOR_SUBWHITE);

        return true;
    }

    static bool HandleFreedomTeleListCommand(ChatHandler* handler, const char* args)
    {
        std::string tele_name_start = "";
        
        if (*args) 
        {
            tele_name_start += strtok((char*)args, " ");
        }

        // refine query parameter for LIKE statement
        replaceAll(tele_name_start, "%", "\\%"); //disable inputted % wildcards
        replaceAll(tele_name_start, "_", "\\_"); //disable inputted _ wildcards
        tele_name_start += "%"; // ensure that tele_name_start can end with anything

        // get exact match
        PreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_SEL_FREEDOM_TELE);
        stmt->setString(0, tele_name_start);
        PreparedQueryResult result = WorldDatabase.Query(stmt);

        if (!result)
        {
            handler->PSendSysMessage(
                "%s.freedom teleport list: %sNo teleports found.\n"
                "%sReason: %sNo teleports using that string (or blank) was found in freedom teleport list.",
                MSG_COLOR_CHOCOLATE, MSG_COLOR_RED, MSG_COLOR_CHOCOLATE, MSG_COLOR_SUBWHITE, MSG_COLOR_CHOCOLATE, MSG_COLOR_SUBWHITE, MSG_COLOR_ORANGEY, MSG_COLOR_SUBWHITE);
            return true;
        }

        handler->PSendSysMessage(
            "%s>>%s Retrieving %s%u%s teleports...",
            MSG_COLOR_CHOCOLATE, MSG_COLOR_SUBWHITE, MSG_COLOR_ORANGEY, uint32(result->GetRowCount()), MSG_COLOR_SUBWHITE);

        do
        {
            Field * fields = result->Fetch();
            std::string map_name = "<unknown>";
            
            MapEntry const* map = sMapStore.LookupEntry(uint32(fields[4].GetUInt16()));
            if (map)
                map_name = map->name;

            handler->PSendSysMessage(
                "%s> %s%s%s (Map: %s%s%s)",
                MSG_COLOR_CHOCOLATE, MSG_COLOR_ORANGEY, fields[5].GetCString(), MSG_COLOR_SUBWHITE, MSG_COLOR_ORANGEY, map_name.c_str(), MSG_COLOR_SUBWHITE);
        } while (result->NextRow());

        return true;
    }

    // LOCATIONAL -> PRIVATE TELEPORT
    static bool HandleFreedomPrivateTeleCommand(ChatHandler* handler, const char* args) 
    {
        if (!*args)
        {
            handler->PSendSysMessage(
                "%s.freedom pteleport: %sPlease specify a name for the location to teleport to.\n"
                "%sSyntax: %s.freedom pteleport $teleNamePart\n"
                "%sHint: %sUse %s.f ptele list [$namePart] %scommand to search for available destinations.",
                MSG_COLOR_CHOCOLATE, MSG_COLOR_SUBWHITE, MSG_COLOR_CHOCOLATE, MSG_COLOR_ORANGEY, MSG_COLOR_CHOCOLATE, MSG_COLOR_SUBWHITE, MSG_COLOR_ORANGEY, MSG_COLOR_SUBWHITE);
            return true;
        }

        std::string tele_name_exact;
        tele_name_exact = strtok((char *)args, " ");
        std::string tele_name_start = tele_name_exact;
        Player* source = handler->GetSession()->GetPlayer();

        if (source->IsInCombat())
        {
            handler->PSendSysMessage(
                "%s.freedom pteleport: %sCan't teleport currently.\n"
                "%sReason: %sYou cannot teleport while in combat.",
                MSG_COLOR_CHOCOLATE, MSG_COLOR_RED, MSG_COLOR_CHOCOLATE, MSG_COLOR_SUBWHITE);
            return true;
        }

        // refine query parameter for LIKE statement
        replaceAll(tele_name_start, "%", "\\%"); //disable inputted % wildcards
        replaceAll(tele_name_start, "_", "\\_"); //disable inputted _ wildcards
        tele_name_start += "%"; // ensure that tele_name_start can end with anything

        // get exact match
        PreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_SEL_FREEDOM_PRIVATE_TELE_EXACT);
        stmt->setString(0, tele_name_exact);
        stmt->setUInt32(1, source->GetSession()->GetAccountId());
        PreparedQueryResult result = WorldDatabase.Query(stmt);
        
        // if exact match failed, get first teleport, which name starts with given parameter
        if (!result) {
            stmt = WorldDatabase.GetPreparedStatement(WORLD_SEL_FREEDOM_PRIVATE_TELE);
            stmt->setString(0, tele_name_start);
            stmt->setUInt32(1, source->GetSession()->GetAccountId());
            result = WorldDatabase.Query(stmt);
        }

        if (!result)
        {
            handler->PSendSysMessage(
                "%s.freedom pteleport: %sTeleport not found.\n"
                "%sReason: %sNo teleport using that string was found in your private teleport list.\n"
                "%sHint: %sUse %s.f ptele list [$namePart] %scommand to search for available destinations.",
                MSG_COLOR_CHOCOLATE, MSG_COLOR_RED, MSG_COLOR_CHOCOLATE, MSG_COLOR_SUBWHITE, MSG_COLOR_CHOCOLATE, MSG_COLOR_SUBWHITE, MSG_COLOR_ORANGEY, MSG_COLOR_SUBWHITE);
            return true;
        }

        Field *fields = result->Fetch();

        if (source->IsInFlight())
        {
            source->GetMotionMaster()->MovementExpired();
            source->CleanupAfterTaxiFlight();
        }

        source->SaveRecallPosition();
        source->TeleportTo(uint32(fields[4].GetUInt16()), fields[0].GetFloat(), fields[1].GetFloat(), fields[2].GetFloat(), fields[3].GetFloat());

        handler->PSendSysMessage(
                "%s>>%s You have been successfully teleported to %s%s %slocation.", 
                MSG_COLOR_CHOCOLATE, MSG_COLOR_SUBWHITE, MSG_COLOR_ORANGEY, fields[5].GetCString(), MSG_COLOR_SUBWHITE);

        return true;
    }

    static bool HandleFreedomPrivateTeleAddCommand(ChatHandler* handler, const char* args)
    {
        if (!*args)
        {
            handler->PSendSysMessage(
                "%s.freedom pteleport add: %sPlease specify a name for the private teleport you want to add.\n"
                "%sSyntax: %s.freedom pteleport add $teleName",
                MSG_COLOR_CHOCOLATE, MSG_COLOR_SUBWHITE, MSG_COLOR_CHOCOLATE, MSG_COLOR_ORANGEY);
            return true;
        }

        std::string tele_name_new = strtok((char*)args, " ");
        Player* source = handler->GetSession()->GetPlayer();

        PreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_SEL_FREEDOM_PRIVATE_TELE_EXACT);
        stmt->setString(0, tele_name_new);
        stmt->setUInt32(1, source->GetSession()->GetAccountId());
        PreparedQueryResult result = WorldDatabase.Query(stmt);
        
        if (result) 
        {
            handler->PSendSysMessage(
                "%s.freedom pteleport add: %sAlready exists.\n"
                "%sReason: %sThe name of the private teleport you want to create already exists.\n" 
                "%sHint: %sUse a different name or delete the existing one using %s.f ptele del $name %scommand.",
                MSG_COLOR_CHOCOLATE, MSG_COLOR_RED, MSG_COLOR_CHOCOLATE, MSG_COLOR_SUBWHITE, MSG_COLOR_CHOCOLATE, MSG_COLOR_SUBWHITE, MSG_COLOR_ORANGEY, MSG_COLOR_SUBWHITE);
            return true;
        }

        stmt = WorldDatabase.GetPreparedStatement(WORLD_INS_FREEDOM_PRIVATE_TELE);
        stmt->setFloat(0, source->GetPositionX());
        stmt->setFloat(1, source->GetPositionY());
        stmt->setFloat(2, source->GetPositionZ());
        stmt->setFloat(3, source->GetOrientation());
        stmt->setUInt32(4, source->GetMapId());
        stmt->setString(5, tele_name_new);
        stmt->setUInt32(6, source->GetSession()->GetAccountId());
        WorldDatabase.Execute(stmt);

        handler->PSendSysMessage(
                "%s>>%s Teleport (Private-type) with the name %s%s%s successfully created.", 
                MSG_COLOR_CHOCOLATE, MSG_COLOR_SUBWHITE, MSG_COLOR_ORANGEY, tele_name_new.c_str(), MSG_COLOR_SUBWHITE);

        return true;
    }

    static bool HandleFreedomPrivateTeleDelCommand(ChatHandler* handler, const char* args)
    {
        if (!*args)
        {
            handler->PSendSysMessage(
                "%s.freedom pteleport delete: %sPlease specify a name of the private teleport you want to delete.\n"
                "%sSyntax: %s.freedom pteleport delete $teleName\n"
                "%sHint: %sUse %s.list ptele $namePart %scommand to search for currently owned teleports.",
                MSG_COLOR_CHOCOLATE, MSG_COLOR_SUBWHITE, MSG_COLOR_CHOCOLATE, MSG_COLOR_ORANGEY, MSG_COLOR_CHOCOLATE, MSG_COLOR_SUBWHITE, MSG_COLOR_ORANGEY, MSG_COLOR_SUBWHITE);
            return true;
        }

        std::string tele_name_delete = strtok((char*)args, " ");
        Player* source = handler->GetSession()->GetPlayer();

        PreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_SEL_FREEDOM_PRIVATE_TELE_EXACT);
        stmt->setString(0, tele_name_delete);
        stmt->setUInt32(1, source->GetSession()->GetAccountId());
        PreparedQueryResult result = WorldDatabase.Query(stmt);

        if (!result)
        {
            handler->PSendSysMessage(
                "%s.freedom pteleport delete: %sNot found.\n"
                "%sReason: %sThe name of the private teleport you want to delete is not found in your private teleport list.\n"
                "%sHint: %sRemember to specify full teleport's name, just part of the name will not work, however, case-sensitive name is not required.",
                MSG_COLOR_CHOCOLATE, MSG_COLOR_RED, MSG_COLOR_CHOCOLATE, MSG_COLOR_SUBWHITE, MSG_COLOR_CHOCOLATE, MSG_COLOR_SUBWHITE);
            return true;
        }

        stmt = WorldDatabase.GetPreparedStatement(WORLD_DEL_FREEDOM_PRIVATE_TELE);
        stmt->setString(0, tele_name_delete);
        stmt->setUInt32(1, source->GetSession()->GetAccountId());
        WorldDatabase.Execute(stmt);

        handler->PSendSysMessage(
                "%s>>%s Teleport (Private-type) with the name %s%s%s successfully deleted.", 
                 MSG_COLOR_CHOCOLATE, MSG_COLOR_SUBWHITE, MSG_COLOR_ORANGEY, tele_name_delete.c_str(), MSG_COLOR_SUBWHITE);

        return true;
    }

    static bool HandleFreedomPrivateTeleListCommand(ChatHandler* handler, const char* args)
    {
        std::string tele_name_start = "";
        uint32 account_id = handler->GetSession()->GetAccountId();
        std::string target_name = handler->GetSession()->GetPlayer()->GetName();
        PreparedStatement* stmt;

        char * params[3];
        params[0] = strtok((char*)args, " ");
        params[1] = strtok(NULL, " ");
        params[2] = strtok(NULL, " ");

        int param_index = 0;

        // check & set special parameters
        std::string temp = params[param_index] ? params[param_index] : "";

        if ((temp == "--p") && handler->GetSession()->GetSecurity() >= SEC_GAMEMASTER)
        {
            param_index++;
            account_id = NULL;
            
            if (!params[param_index])
            {
                handler->PSendSysMessage(
                    "%s.freedom ptele list [--p]: %sMissing argument.\n"
                    "%sReason: %sSpecial parameter %s--p%s requires an argument followed after it.",
                    MSG_COLOR_CHOCOLATE, MSG_COLOR_RED, MSG_COLOR_CHOCOLATE, MSG_COLOR_SUBWHITE, MSG_COLOR_ORANGEY, MSG_COLOR_SUBWHITE);
                return true;
            }
            if (isSpecialParam(params[param_index]))
            {
                handler->PSendSysMessage(
                    "%s.freedom ptele list [--p]: %sInvalid Argument.\n"
                    "%sReason: %sSpecial parameter (parameter which starts with %s--%s) cannot be used as an argument for another special parameter.",
                    MSG_COLOR_CHOCOLATE, MSG_COLOR_RED, MSG_COLOR_CHOCOLATE, MSG_COLOR_SUBWHITE, MSG_COLOR_ORANGEY, MSG_COLOR_SUBWHITE);
                return true;
            }

            target_name = params[param_index];

            stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_ACCOUNT_BY_NAME);
            stmt->setString(0, target_name);
            PreparedQueryResult result = CharacterDatabase.Query(stmt);
            if (result)
            {
                Field * fields = result->Fetch();
                account_id = fields[0].GetUInt32();
            }
            else
            {
                handler->PSendSysMessage(
                    "%s.freedom ptele list [--p]: %sNo player found.\n"
                    "%sReason: %sPlayer %s%s%s does not exist.",
                    MSG_COLOR_CHOCOLATE, MSG_COLOR_RED, MSG_COLOR_CHOCOLATE, MSG_COLOR_SUBWHITE, MSG_COLOR_ORANGEY, target_name.c_str(), MSG_COLOR_SUBWHITE);
                return true;
            }

            param_index++;
        }
        else if (isSpecialParam(temp))
        {
            handler->PSendSysMessage(
                "%s.freedom ptele list: %sInvalid special parameter.\n"
                "%sReason: %sSpecial parameter (parameter which starts with %s--%s) you used was incorrect (no such special parameter is available).\n"
                "%sHint: %sRemember that special parameters with their arguments have to be entered before command's normal parameters!",
                MSG_COLOR_CHOCOLATE, MSG_COLOR_RED, MSG_COLOR_CHOCOLATE, MSG_COLOR_SUBWHITE, MSG_COLOR_ORANGEY, MSG_COLOR_SUBWHITE, MSG_COLOR_CHOCOLATE, MSG_COLOR_SUBWHITE);
            return true;
        }

        if (params[param_index])
        {
            tele_name_start += params[param_index];
        }

        // refine query parameter for LIKE statement
        replaceAll(tele_name_start, "%", "\\%"); //disable inputted % wildcards
        replaceAll(tele_name_start, "_", "\\_"); //disable inputted _ wildcards
        tele_name_start += "%"; // ensure that tele_name_start can end with anything

        // get exact match
        stmt = WorldDatabase.GetPreparedStatement(WORLD_SEL_FREEDOM_PRIVATE_TELE);
        stmt->setString(0, tele_name_start);
        stmt->setUInt32(1, account_id);
        PreparedQueryResult result = WorldDatabase.Query(stmt);

        if (!result)
        {
            handler->PSendSysMessage(
                "%s.freedom ptele list: %sNo teleport(s) found.\n"
                "%sReason: %sNo teleports using that string (or blank) was found in private teleport list of %s%s%s.",
                MSG_COLOR_CHOCOLATE, MSG_COLOR_RED, MSG_COLOR_CHOCOLATE, MSG_COLOR_SUBWHITE, MSG_COLOR_ORANGEY, target_name.c_str(), MSG_COLOR_SUBWHITE);
            return true;
        }

        handler->PSendSysMessage(
            "%s>>%s Retrieving %s%u%s private teleport(s) of %s%s%s...",
            MSG_COLOR_CHOCOLATE, MSG_COLOR_SUBWHITE, MSG_COLOR_ORANGEY, uint32(result->GetRowCount()), MSG_COLOR_SUBWHITE, MSG_COLOR_ORANGEY, target_name.c_str(), MSG_COLOR_SUBWHITE);

        do
        {
            Field * fields = result->Fetch();
            std::string map_name = "<unknown>";

            MapEntry const* map = sMapStore.LookupEntry(uint32(fields[4].GetUInt16()));
            if (map)
                map_name = map->name;

            handler->PSendSysMessage(
                "%s> %s%s%s (Map: %s%s%s)",
                MSG_COLOR_CHOCOLATE, MSG_COLOR_ORANGEY, fields[5].GetCString(), MSG_COLOR_SUBWHITE, MSG_COLOR_ORANGEY, map_name.c_str(), MSG_COLOR_SUBWHITE);
        } while (result->NextRow());

        return true;
    }
    #pragma endregion LOCATIONAL
};

#endif