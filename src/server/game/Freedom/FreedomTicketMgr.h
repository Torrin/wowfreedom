#ifndef _FREEDOMTICKETMGR_H
#define _FREEDOMTICKETMGR_H

class FreedomTicketMgr
{
private:
    FreedomTicketMgr();
    ~FreedomTicketMgr();
public:
    static FreedomTicketMgr* instance();
};

#define sFreedomTicketMgr FreedomTicketMgr::instance()

#endif