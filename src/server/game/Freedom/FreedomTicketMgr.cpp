#include "FreedomTicketMgr.h"

#pragma region FREEDOM_TICKET_MANAGER
FreedomTicketMgr::FreedomTicketMgr()
{
}

FreedomTicketMgr::~FreedomTicketMgr()
{
}

FreedomTicketMgr* FreedomTicketMgr::instance()
{
    static FreedomTicketMgr instance;
    return &instance;
}

#pragma endregion