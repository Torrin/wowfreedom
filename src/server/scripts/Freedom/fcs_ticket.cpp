#include "ScriptMgr.h"
#include "ObjectMgr.h"
#include "MapManager.h"
#include "SupportMgr.h"
#include "Chat.h"
#include "Language.h"
#include "Player.h"
#include "Transport.h"
#include "ArgumentTokenizer.h"

class fticket_commandscript : public CommandScript
{
public:
    fticket_commandscript() : CommandScript("fticket_commandscript") { }

    std::vector<ChatCommand> GetCommands() const override
    {
        static std::vector<ChatCommand> ticketMyListCommandTable =
        {
            { "recent",     rbac::RBAC_FPERM_COMMAND_FREEDOM_UTILITIES,     false, &HandleTicketMyListRecentCommand,            "" },
            { "active",     rbac::RBAC_FPERM_COMMAND_FREEDOM_UTILITIES,     false, &HandleTicketMyListActiveCommand,            "" },
            { "closed",     rbac::RBAC_FPERM_COMMAND_FREEDOM_UTILITIES,     false, &HandleTicketMyListClosedCommand,            "" },
        };

        static std::vector<ChatCommand> ticketGMListCommandTable =
        {
            { "active",     rbac::RBAC_FPERM_ADMINISTRATION,                false, &HandleTicketGMListActiveCommand,            "" },
            { "unassigned", rbac::RBAC_FPERM_ADMINISTRATION,                false, &HandleTicketGMListUnassignedCommand,        "" },
            { "myassigned", rbac::RBAC_FPERM_ADMINISTRATION,                false, &HandleTicketGMListMyAssignedCommand,        "" },
            { "assigned",   rbac::RBAC_FPERM_ADMINISTRATION,                false, &HandleTicketGMListAssignedCommand,          "" },
            { "closed",     rbac::RBAC_FPERM_ADMINISTRATION,                false, &HandleTicketGMListClosedCommand,            "" },
        };

        static std::vector<ChatCommand> ticketCommandTable =
        {
            { "create",     rbac::RBAC_FPERM_COMMAND_FREEDOM_UTILITIES,     false, &HandleTicketCreateCommand,                  "" },
            { "edit",       rbac::RBAC_FPERM_COMMAND_FREEDOM_UTILITIES,     false, &HandleTicketEditCommand,                    "" },
            { "abandon",    rbac::RBAC_FPERM_COMMAND_FREEDOM_UTILITIES,     false, &HandleTicketAbandonCommand,                 "" },
            { "close",      rbac::RBAC_FPERM_ADMINISTRATION,                false, &HandleTicketCloseCommand,                   "" },
            { "assign",     rbac::RBAC_FPERM_ADMINISTRATION,                false, &HandleTicketAssignCommand,                  "" },
            { "view",       rbac::RBAC_FPERM_COMMAND_FREEDOM_UTILITIES,     false, &HandleTicketViewCommand,                    "" },
            { "mylist",     rbac::RBAC_FPERM_COMMAND_FREEDOM_UTILITIES,     false, NULL,                                        "", ticketMyListCommandTable },
            { "gmlist",     rbac::RBAC_FPERM_ADMINISTRATION,                false, NULL,                                        "", ticketGMListCommandTable },
        };

        static std::vector<ChatCommand> commandTable =
        {
            { "ticket",     rbac::RBAC_FPERM_COMMAND_FREEDOM_UTILITIES,     false, NULL,                                        "", ticketCommandTable },
        };

        return commandTable;
    }

#pragma region TICKET
    static bool HandleTicketCreateCommand(ChatHandler * handler, char const* args)
    {
        handler->PSendSysMessage(FREEDOM_CMDE_NOT_YET_IMPLEMENTED);
        return true;
    }

    static bool HandleTicketEditCommand(ChatHandler * handler, char const* args)
    {
        handler->PSendSysMessage(FREEDOM_CMDE_NOT_YET_IMPLEMENTED);
        return true;
    }

    static bool HandleTicketAbandonCommand(ChatHandler * handler, char const* args)
    {
        handler->PSendSysMessage(FREEDOM_CMDE_NOT_YET_IMPLEMENTED);
        return true;
    }

    static bool HandleTicketCloseCommand(ChatHandler * handler, char const* args)
    {
        handler->PSendSysMessage(FREEDOM_CMDE_NOT_YET_IMPLEMENTED);
        return true;
    }

    static bool HandleTicketAssignCommand(ChatHandler * handler, char const* args)
    {
        handler->PSendSysMessage(FREEDOM_CMDE_NOT_YET_IMPLEMENTED);
        return true;
    }

    static bool HandleTicketViewCommand(ChatHandler * handler, char const* args)
    {
        handler->PSendSysMessage(FREEDOM_CMDE_NOT_YET_IMPLEMENTED);
        return true;
    }
#pragma endregion

#pragma region TICKET -> GLOBAL LIST
    static bool HandleTicketGMListActiveCommand(ChatHandler * handler, char const* args)
    {
        handler->PSendSysMessage(FREEDOM_CMDE_NOT_YET_IMPLEMENTED);
        return true;
    }

    static bool HandleTicketGMListUnassignedCommand(ChatHandler * handler, char const* args)
    {
        handler->PSendSysMessage(FREEDOM_CMDE_NOT_YET_IMPLEMENTED);
        return true;
    }

    static bool HandleTicketGMListMyAssignedCommand(ChatHandler * handler, char const* args)
    {
        handler->PSendSysMessage(FREEDOM_CMDE_NOT_YET_IMPLEMENTED);
        return true;
    }

    static bool HandleTicketGMListAssignedCommand(ChatHandler * handler, char const* args)
    {
        handler->PSendSysMessage(FREEDOM_CMDE_NOT_YET_IMPLEMENTED);
        return true;
    }

    static bool HandleTicketGMListClosedCommand(ChatHandler * handler, char const* args)
    {
        handler->PSendSysMessage(FREEDOM_CMDE_NOT_YET_IMPLEMENTED);
        return true;
    }
#pragma endregion

#pragma region TICKET -> MYLIST
    static bool HandleTicketMyListRecentCommand(ChatHandler * handler, char const* args)
    {
        handler->PSendSysMessage(FREEDOM_CMDE_NOT_YET_IMPLEMENTED);
        return true;
    }

    static bool HandleTicketMyListActiveCommand(ChatHandler * handler, char const* args)
    {
        handler->PSendSysMessage(FREEDOM_CMDE_NOT_YET_IMPLEMENTED);
        return true;
    }

    static bool HandleTicketMyListClosedCommand(ChatHandler * handler, char const* args)
    {
        handler->PSendSysMessage(FREEDOM_CMDE_NOT_YET_IMPLEMENTED);
        return true;
    }
#pragma endregion
};

void AddSC_fticket_commandscript()
{
    new fticket_commandscript();
}