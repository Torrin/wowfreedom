#include "ScriptMgr.h"
#include "ObjectMgr.h"
#include "Chat.h"
#include "AccountMgr.h"
#include "Language.h"
#include "World.h"
#include "Player.h"
#include "Opcodes.h"

#ifdef FREEDOM_MOP_548_CODE

class fmisc_commandscript : public CommandScript
{
public:
    fmisc_commandscript() : CommandScript("fmisc_commandscript") { }

    ChatCommand* GetCommands() const
    {
        static ChatCommand commandTable[] =
        {
            { "pinfo",              rbac::RBAC_PERM_COMMAND_PINFO,                      true,  &HandlePInfoCommand,                 "", NULL },
            { "additem",            rbac::RBAC_PERM_COMMAND_ADDITEM,                    false, &HandleAddItemCommand,               "", NULL },
            { "hideitem",           rbac::RBAC_PERM_COMMAND_HIDEITEM,                   false, &HandleHideItemCommand,              "", NULL },
            { "unhideitem",         rbac::RBAC_PERM_COMMAND_UNHIDEITEM,                 false, &HandleUnHideItemCommand,            "", NULL },
            { NULL, 0, false, NULL, "", NULL }
        };

        return commandTable;
    }

    static bool HandleAddItemCommand(ChatHandler* handler, char const* args)
    {
        if (!*args) {
            handler->PSendSysMessage("Please specify Entry ID of the item to add to your inventory.");
            handler->PSendSysMessage("Hint: Use '.lookup item $itemNamePart' command to search for available items.");
            return true;
        }

        char const* id = handler->extractKeyFromLink((char*)args, "Hitem");
        uint32 itemId = uint32(atol(id));

        if (!itemId)
        {
            handler->PSendSysMessage("Invalid Entry ID.");
            return true;
        }

        char const* ccount = strtok(NULL, " ");

        int32 count = 1;

        if (ccount)
        {
            count = strtol(ccount, NULL, 10);
            if (count == 0) 
            {
                count = 1;
            }
        }

        Player* source = handler->GetSession()->GetPlayer();
        Player* target = handler->getSelectedPlayer();
        if (!target) 
        {
            target = source;
        }

        ItemTemplate const* itemTemplate = sObjectMgr->GetItemTemplate(itemId);
        if (!itemTemplate)
        {
            handler->PSendSysMessage("Item with such Entry ID does not exist.");
            return true;
        }

        // check if basic user, if so, then check if item is public
        if (!handler->HasPermission(rbac::RBAC_PERM_COMMAND_ADDHIDDENITEM)) 
        {
            target = source;

            PreparedStatement * stmt = WorldDatabase.GetPreparedStatement(WORLD_SEL_IS_PUBLIC_ITEM);
            stmt->setUInt32(0, itemId);
            
            PreparedQueryResult result = WorldDatabase.Query(stmt);
            
            if (!result)
            {
                handler->PSendSysMessage("This item is hidden. Contact the Staff if you wish to make it public.");
                return true;
            }
        }

        TC_LOG_DEBUG("misc", handler->GetTrinityString(LANG_ADDITEM), itemId, count);

        // Subtract
        if (count < 0)
        {
            target->DestroyItemCount(itemId, -count, true, false);
            handler->PSendSysMessage(LANG_REMOVEITEM, itemId, -count, handler->GetNameLink(target).c_str());
            return true;
        }

        // Adding items
        uint32 noSpaceForCount = 0;

        // check space and find places
        ItemPosCountVec dest;
        InventoryResult msg = target->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, itemId, count, &noSpaceForCount);
        if (msg != EQUIP_ERR_OK)                               // convert to possible store amount
            count -= noSpaceForCount;

        if (count == 0 || dest.empty())                         // can't add any
        {
            handler->PSendSysMessage(LANG_ITEM_CANNOT_CREATE, itemId, noSpaceForCount);
            handler->SetSentErrorMessage(true);
            return false;
        }

        Item* item = target->StoreNewItem(dest, itemId, true, Item::GenerateItemRandomPropertyId(itemId));

        // remove binding (let GM give it to another player later)
        if (source == target)
            for (ItemPosCountVec::const_iterator itr = dest.begin(); itr != dest.end(); ++itr)
                if (Item* item1 = source->GetItemByPos(itr->pos))
                    item1->SetBinding(false);

        if (count > 0 && item)
        {
            source->SendNewItem(item, count, false, true);
            if (source != target)
                target->SendNewItem(item, count, true, false);
        }

        if (noSpaceForCount > 0)
            handler->PSendSysMessage(LANG_ITEM_CANNOT_CREATE, itemId, noSpaceForCount);

        return true;
    }

    static bool HandleHideItemCommand(ChatHandler* handler, char const* args)
    {
        if (!*args) {
            handler->PSendSysMessage("Please specify Entry ID of the item to hide from public list.");
            handler->PSendSysMessage("Hint: Use '.lookup item $itemNamePart' command to search for available items.");
            return true;
        }

        char const* id = handler->extractKeyFromLink((char*)args, "Hitem");
        uint32 itemId = uint32(atol(id));

        if (!itemId)
        {
            handler->PSendSysMessage("Invalid Entry ID.");
            return true;
        }

        ItemTemplate const* itemTemplate = sObjectMgr->GetItemTemplate(itemId);
        if (!itemTemplate)
        {
            handler->PSendSysMessage("Item with such Entry ID does not exist.");
            return true;
        }

        PreparedStatement * stmt = WorldDatabase.GetPreparedStatement(WORLD_UPD_ITEM_VISIBILITY);
        stmt->setUInt8(0, 0);
        stmt->setUInt32(1, itemId);
        WorldDatabase.Execute(stmt);

        handler->PSendSysMessage(">> Item has been successfully hidden/restricted from public.");

        return true;
    }

    static bool HandleUnHideItemCommand(ChatHandler* handler, char const* args)
    {
        if (!*args) {
            handler->PSendSysMessage("Please specify Entry ID of the item to unhide.");
            handler->PSendSysMessage("Hint: Use '.lookup item $itemNamePart' command to search for available items.");
            return true;
        }

        char const* id = handler->extractKeyFromLink((char*)args, "Hitem");
        uint32 itemId = uint32(atol(id));

        if (!itemId)
        {
            handler->PSendSysMessage("Invalid Entry ID.");
            return true;
        }

        ItemTemplate const* itemTemplate = sObjectMgr->GetItemTemplate(itemId);
        if (!itemTemplate)
        {
            handler->PSendSysMessage("Item with such Entry ID does not exist.");
            return true;
        }

        PreparedStatement * stmt = WorldDatabase.GetPreparedStatement(WORLD_UPD_ITEM_VISIBILITY);
        stmt->setUInt8(0, 1);
        stmt->setUInt32(1, itemId);
        WorldDatabase.Execute(stmt);

        handler->PSendSysMessage(">> Item has been successfully made public.");

        return true;
    }

    /**
    * @name Player command: .pinfo
    * @date 05/19/2013
    *
    * @brief Prints information about a character and it's linked account to the commander
    *
    * Non-applying information, e.g. a character that is not in gm mode right now or
    * that is not banned/muted, is not printed
    *
    * This can be done either by giving a name or by targeting someone, else, it'll use the commander
    *
    * @param args name   Prints information according to the given name to the commander
    *             target Prints information on the target to the commander
    *             none   No given args results in printing information on the commander
    *
    * @return Several pieces of information about the character and the account
    **/

    static bool HandlePInfoCommand(ChatHandler* handler, char const* args)
    {
        // Define ALL the player variables!
        Player* target;
        uint64 targetGuid;
        std::string targetName;

        // To make sure we get a target, we convert our guid to an omniversal...
        uint32 parseGUID = MAKE_NEW_GUID(atol((char*)args), 0, HIGHGUID_PLAYER);

        // ... and make sure we get a target, somehow.
        if (sObjectMgr->GetPlayerNameByGUID(parseGUID, targetName))
        {
            target = sObjectMgr->GetPlayerByLowGUID(parseGUID);
            targetGuid = parseGUID;
        }
        // if not, then return false. Which shouldn't happen, now should it ?
        else if (!handler->extractPlayerTarget((char*)args, &target, &targetGuid, &targetName))
            return false;

        /* The variables we extract for the command. They are
        * default as "does not exist" to prevent problems
        * The output is printed in the follow manner:
        *
        * Player %s %s (guid: %u)                   - I.    LANG_PINFO_PLAYER
        * ** GM Mode active, Phase: -1              - II.   LANG_PINFO_GM_ACTIVE (if GM)
        * ** Banned: (Type, Reason, Time, By)       - III.  LANG_PINFO_BANNED (if banned)
        * ** Muted: (Time, Reason, By)              - IV.   LANG_PINFO_MUTED (if muted)
        * * Account: %s (id: %u), GM Level: %u      - V.    LANG_PINFO_ACC_ACCOUNT
        * * Last Login: %u (Failed Logins: %u)      - VI.   LANG_PINFO_ACC_LASTLOGIN
        * * Uses OS: %s - Latency: %u ms            - VII.  LANG_PINFO_ACC_OS
        * * Registration Email: %s - Email: %s      - VIII. LANG_PINFO_ACC_REGMAILS
        * * Last IP: %u (Locked: %s)                - IX.   LANG_PINFO_ACC_IP
        * * Level: %u (%u/%u XP (%u XP left)        - X.    LANG_PINFO_CHR_LEVEL
        * * Race: %s %s, Class %s                   - XI.   LANG_PINFO_CHR_RACE
        * * Alive ?: %s                             - XII.  LANG_PINFO_CHR_ALIVE
        * * Phase: %s                               - XIII. LANG_PINFO_CHR_PHASE (if not GM)
        * * Money: %ug%us%uc                        - XIV.  LANG_PINFO_CHR_MONEY
        * * Map: %s, Area: %s                       - XV.   LANG_PINFO_CHR_MAP
        * * Guild: %s (Id: %u)                      - XVI.  LANG_PINFO_CHR_GUILD (if in guild)
        * ** Rank: %s                               - XVII. LANG_PINFO_CHR_GUILD_RANK (if in guild)
        * ** Note: %s                               - XVIII.LANG_PINFO_CHR_GUILD_NOTE (if in guild and has note)
        * ** O. Note: %s                            - XVIX. LANG_PINFO_CHR_GUILD_ONOTE (if in guild and has officer note)
        * * Played time: %s                         - XX.   LANG_PINFO_CHR_PLAYEDTIME
        * * Mails: %u Read/%u Total                 - XXI.  LANG_PINFO_CHR_MAILS (if has mails)
        *
        * Not all of them can be moved to the top. These should
        * place the most important ones to the head, though.
        *
        * For a cleaner overview, I segment each output in Roman numerals
        */

        // Account data print variables
        std::string userName = handler->GetTrinityString(LANG_ERROR);
        uint32 accId = 0;
        uint32 lowguid = GUID_LOPART(targetGuid);
        std::string eMail = handler->GetTrinityString(LANG_ERROR);
        std::string regMail = handler->GetTrinityString(LANG_ERROR);
        uint32 security = 0;
        std::string lastIp = handler->GetTrinityString(LANG_ERROR);
        uint8 locked = 0;
        std::string lastLogin = handler->GetTrinityString(LANG_ERROR);
        uint32 failedLogins = 0;
        uint32 latency = 0;
        std::string OS = "None";

        // Mute data print variables
        int64 muteTime = -1;
        std::string muteReason = "unknown";
        std::string muteBy = "unknown";

        // Ban data print variables
        int64 banTime = -1;
        std::string banType = "None";
        std::string banReason = "Unknown";
        std::string bannedBy = "Unknown";

        // Character data print variables
        uint8 raceid, classid = 0; //RACE_NONE, CLASS_NONE
        std::string raceStr, classStr = "None";
        uint8 gender = 0;
        int8 locale = handler->GetSessionDbcLocale();
        std::string genderStr = handler->GetTrinityString(LANG_ERROR);
        uint32 totalPlayerTime = 0;
        uint8 level = 0;
        std::string alive = handler->GetTrinityString(LANG_ERROR);
        uint32 money = 0;
        uint32 xp = 0;
        uint32 xptotal = 0;

        // Position data print
        uint32 mapId;
        uint32 areaId;
        uint32 phase = 0;
        std::string areaName = "<unknown>";
        std::string zoneName = "<unknown>";

        // Guild data print variables defined so that they exist, but are not necessarily used
        uint32 guildId = 0;
        uint8 guildRankId = 0;
        std::string guildName;
        std::string guildRank;
        std::string note;
        std::string officeNote;

        // Mail data print is only defined if you have a mail

        if (target)
        {
            // check online security
            if (handler->HasLowerSecurity(target, 0))
                return false;

            accId = target->GetSession()->GetAccountId();
            money = target->GetMoney();
            totalPlayerTime = target->GetTotalPlayedTime();
            level = target->getLevel();
            latency = target->GetSession()->GetLatency();
            raceid = target->getRace();
            classid = target->getClass();
            muteTime = target->GetSession()->m_muteTime;
            mapId = target->GetMapId();
            areaId = target->GetAreaId();
            alive = target->IsAlive() ? "Yes" : "No";
            gender = target->getGender();
            phase = target->GetPhaseMask();
        }
        // get additional information from DB
        else
        {
            // check offline security
            if (handler->HasLowerSecurity(NULL, targetGuid))
                return false;

            // Query informations from the DB
            PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHAR_PINFO);
            stmt->setUInt32(0, lowguid);
            PreparedQueryResult result = CharacterDatabase.Query(stmt);

            if (!result)
                return false;

            Field* fields = result->Fetch();
            totalPlayerTime = fields[0].GetUInt32();
            level = fields[1].GetUInt8();
            money = fields[2].GetUInt32();
            accId = fields[3].GetUInt32();
            raceid = fields[4].GetUInt8();
            classid = fields[5].GetUInt8();
            mapId = fields[6].GetUInt16();
            areaId = fields[7].GetUInt16();
            gender = fields[8].GetUInt8();
            uint32 health = fields[9].GetUInt32();
            uint32 playerFlags = fields[10].GetUInt32();

            if (!health || playerFlags & PLAYER_FLAGS_GHOST)
                alive = "No";
            else
                alive = "Yes";
        }

        // Query the prepared statement for login data
        PreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_SEL_PINFO);
        stmt->setInt32(0, int32(realmID));
        stmt->setUInt32(1, accId);
        PreparedQueryResult result = LoginDatabase.Query(stmt);

        if (result)
        {
            Field* fields = result->Fetch();
            userName = fields[0].GetString();
            security = fields[1].GetUInt8();

            // Only fetch these fields if commander has sufficient rights)
            if (handler->HasPermission(rbac::RBAC_PERM_COMMANDS_PINFO_CHECK_PERSONAL_DATA) && // RBAC Perm. 48, Role 39
                (!handler->GetSession() || handler->GetSession()->GetSecurity() >= AccountTypes(security)))
            {
                eMail = fields[2].GetString();
                regMail = fields[3].GetString();
                lastIp = fields[4].GetString();
                lastLogin = fields[5].GetString();

                uint32 ip = inet_addr(lastIp.c_str());
                EndianConvertReverse(ip);

                // If ip2nation table is populated, it displays the country
                PreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_SEL_IP2NATION_COUNTRY);
                stmt->setUInt32(0, ip);
                if (PreparedQueryResult result2 = LoginDatabase.Query(stmt))
                {
                    Field* fields2 = result2->Fetch();
                    lastIp.append(" (");
                    lastIp.append(fields2[0].GetString());
                    lastIp.append(")");
                }
            }
            else
            {
                eMail = "Unauthorized";
                lastIp = "Unauthorized";
                lastLogin = "Unauthorized";
            }
            muteTime = fields[6].GetUInt64();
            muteReason = fields[7].GetString();
            muteBy = fields[8].GetString();
            failedLogins = fields[9].GetUInt32();
            locked = fields[10].GetUInt8();
            OS = fields[11].GetString();
        }

        // Creates a chat link to the character. Returns nameLink
        std::string nameLink = handler->playerLink(targetName);

        // Returns banType, banTime, bannedBy, banreason
        PreparedStatement* stmt2 = LoginDatabase.GetPreparedStatement(LOGIN_SEL_PINFO_BANS);
        stmt2->setUInt32(0, accId);
        PreparedQueryResult result2 = LoginDatabase.Query(stmt2);
        if (!result2)
        {
            banType = "Character";
            stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_PINFO_BANS);
            stmt->setUInt32(0, lowguid);
            result2 = CharacterDatabase.Query(stmt);
        }

        if (result2)
        {
            Field* fields = result2->Fetch();
            banTime = int64(fields[1].GetUInt64() ? 0 : fields[0].GetUInt32());
            bannedBy = fields[2].GetString();
            banReason = fields[3].GetString();
        }

        // Can be used to query data from World database
        stmt2 = WorldDatabase.GetPreparedStatement(WORLD_SEL_REQ_XP);
        stmt2->setUInt8(0, level);
        PreparedQueryResult result3 = WorldDatabase.Query(stmt2);

        if (result3)
        {
            Field* fields = result3->Fetch();
            xptotal = fields[0].GetUInt32();
        }

        // Can be used to query data from Characters database
        stmt2 = CharacterDatabase.GetPreparedStatement(CHAR_SEL_PINFO_XP);
        stmt2->setUInt32(0, lowguid);
        PreparedQueryResult result4 = CharacterDatabase.Query(stmt2);

        if (result4)
        {
            Field* fields = result4->Fetch();
            xp = fields[0].GetUInt32(); // Used for "current xp" output and "%u XP Left" calculation
            uint32 gguid = fields[1].GetUInt32(); // We check if have a guild for the person, so we might not require to query it at all

            if (gguid != 0)
            {
                // Guild Data - an own query, because it may not happen.
                PreparedStatement* stmt3 = CharacterDatabase.GetPreparedStatement(CHAR_SEL_GUILD_MEMBER_EXTENDED);
                stmt3->setUInt32(0, lowguid);
                PreparedQueryResult result5 = CharacterDatabase.Query(stmt3);
                if (result5)
                {
                    Field* fields = result5->Fetch();
                    guildId = fields[0].GetUInt32();
                    guildName = fields[1].GetString();
                    guildRank = fields[2].GetString();
                    guildRankId = fields[3].GetUInt8();
                    note = fields[4].GetString();
                    officeNote = fields[5].GetString();
                }
            }
        }

        // Initiate output
        // Output I. LANG_PINFO_PLAYER
        handler->PSendSysMessage("PLAYER: %s %s (guid: %u)", target ? "" : handler->GetTrinityString(LANG_OFFLINE), nameLink.c_str(), lowguid);

        // Output II. LANG_PINFO_GM_ACTIVE if character is gamemaster
        if (target && target->IsGameMaster())
            handler->PSendSysMessage("> > GM Mode active, Phase: -1");

        // Output III. LANG_PINFO_BANNED if ban exists and is applied
        if (banTime >= 0) 
        {
            handler->PSendSysMessage("> > %sBANNED", MSG_COLOR_RED);
            handler->PSendSysMessage("> > > Type: %s", banType.c_str());
            handler->PSendSysMessage("> > > Reason: %s", banReason.c_str());
            handler->PSendSysMessage("> > > Time: %s", banTime > 0 ? secsToTimeString(banTime - time(NULL), true).c_str() : "permanently");
            handler->PSendSysMessage("> > > By: %s", bannedBy.c_str());
        }
  

        // Output IV. LANG_PINFO_MUTED if mute is applied
        if (muteTime > 0)
        {
            handler->PSendSysMessage("> > %sMUTED", MSG_COLOR_RED);
            handler->PSendSysMessage("> > > Reason: %s", muteReason.c_str());
            handler->PSendSysMessage("> > > Time: %s", secsToTimeString(muteTime - time(NULL), true).c_str());
            handler->PSendSysMessage("> > > By: %s", muteBy.c_str());
        }

        // Output V. LANG_PINFO_ACC_ACCOUNT
        handler->PSendSysMessage("> Account: %s (id: %u), GM Level: %u", userName.c_str(), accId, security);

        // Output VI. LANG_PINFO_ACC_LASTLOGIN
        handler->PSendSysMessage("> Last Login: %s (Failed Logins: %u) ", lastLogin.c_str(), failedLogins);

        // Output VII. LANG_PINFO_ACC_OS
        handler->PSendSysMessage("> Uses OS: %s - Latency: %u ms", OS.c_str(), latency);

        // Output VIII. LANG_PINFO_ACC_REGMAILS
        handler->PSendSysMessage("> Email: %s ", eMail.c_str());

        // Output IX. LANG_PINFO_ACC_IP
        handler->PSendSysMessage("> Last IP: %s (Locked: %s) ", lastIp.c_str(), locked ? "Yes" : "No");

        // Output X. LANG_PINFO_CHR_LEVEL
        handler->PSendSysMessage("> Level: %u", level);

        // Output XI. LANG_PINFO_CHR_RACE
        raceStr = GetRaceName(raceid, locale);
        classStr = GetClassName(classid, locale);
        handler->PSendSysMessage("> Race: %s %s, Class %s", (gender == 0) ? "Male" : "Female", raceStr.c_str(), classStr.c_str());

        // Output XII. LANG_PINFO_CHR_ALIVE
        handler->PSendSysMessage("> Is Alive? - %s", alive.c_str());

        // Output XIII. LANG_PINFO_CHR_PHASE if player is not in GM mode (GM is in every phase)
        if (target && !target->IsGameMaster())                            // IsInWorld() returns false on loadingscreen, so it's more
            handler->PSendSysMessage("> Phase: %u", phase);        // precise than just target (safer ?).
        // However, as we usually just require a target here, we use target instead.
        // Output XIV. LANG_PINFO_CHR_MONEY
        uint32 gold = money / GOLD;
        uint32 silv = (money % GOLD) / SILVER;
        uint32 copp = (money % GOLD) % SILVER;
        handler->PSendSysMessage("> Money: %ug%us%uc ", gold, silv, copp);

        // Position data
        MapEntry const* map = sMapStore.LookupEntry(mapId);
        AreaTableEntry const* area = GetAreaEntryByAreaID(areaId);
        if (area)
        {
            areaName = area->area_name;

            AreaTableEntry const* zone = GetAreaEntryByAreaID(area->zone);
            if (zone)
                zoneName = zone->area_name;
        }

        if (map) 
        {
            handler->PSendSysMessage("> Map: %s (ID: %u)", map->name, mapId);
            handler->PSendSysMessage("> Zone: %s (ID: %u)", (!zoneName.empty() ? zoneName.c_str() : "<Unknown>"), area ? area->zone : 0);
            handler->PSendSysMessage("> Area: %s (ID: %u)", (!areaName.empty() ? areaName.c_str() : "<Unknown>"), area ? area->ID : 0);
        }


        // Output XVII. - XVIX. if they are not empty
        if (!guildName.empty())
        {
            handler->PSendSysMessage("> Guild: %s (ID: %u)", guildName.c_str(), guildId);
            handler->PSendSysMessage("> > Rank: %s (ID: %u)", guildRank.c_str(), (uint32) guildRankId);
            if (!note.empty())
                handler->PSendSysMessage("> > Note: %s", note.c_str());
            if (!officeNote.empty())
                handler->PSendSysMessage("> > Officer Note: %s", officeNote.c_str());
        }

        // Output XX. LANG_PINFO_CHR_PLAYEDTIME
        handler->PSendSysMessage("> Played time: %s", (secsToTimeString(totalPlayerTime, true, true)).c_str());

        // Mail Data - an own query, because it may or may not be useful.
        // SQL: "SELECT SUM(CASE WHEN (checked & 1) THEN 1 ELSE 0 END) AS 'readmail', COUNT(*) AS 'totalmail' FROM mail WHERE `receiver` = ?"
        PreparedStatement* stmt4 = CharacterDatabase.GetPreparedStatement(CHAR_SEL_PINFO_MAILS);
        stmt4->setUInt32(0, lowguid);
        PreparedQueryResult result6 = CharacterDatabase.Query(stmt4);
        if (result6)
        {
            // Define the variables, so the compiler knows they exist
            uint32 rmailint = 0;

            // Fetch the fields - readmail is a SUM(x) and given out as char! Thus...
            // ... while totalmail is a COUNT(x), which is given out as INt64, which we just convert on fetch...
            Field* fields = result6->Fetch();
            std::string readmail = fields[0].GetString();
            uint32 totalmail = uint32(fields[1].GetUInt64());

            // ... we have to convert it from Char to int. We can use totalmail as it is
            rmailint = atol(readmail.c_str());

            // Output XXI. LANG_INFO_CHR_MAILS if at least one mail is given
            if (totalmail >= 1)
                handler->PSendSysMessage("> Mails (Read/Total): %u/%u", rmailint, totalmail);
        }

        return true;
    }
};

void AddSC_fmisc_commandscript()
{
    new fmisc_commandscript();
}

#endif