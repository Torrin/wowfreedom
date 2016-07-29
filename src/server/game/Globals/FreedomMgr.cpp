#include "FreedomMgr.h"
#include "Player.h"
#include "ObjectMgr.h"
#include "AccountMgr.h"
#include "MovementPackets.h"
#include "MoveSpline.h"
#include "MapManager.h"
#include "GameObject.h"
#include "Creature.h"
#include "Config.h"
#include "Transport.h"
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/tokenizer.hpp>

#pragma region ADVANCED_TOKENIZER
AdvancedArgumentTokenizer::AdvancedArgumentTokenizer(const std::string &src, bool preserveQuotes)
{
    bool trim = true;
    bool quoteMode = false;
    bool chatLinkMode = false;
    char cPrev = '\0';
    std::string token = "";
    normalParamString = src;
    boost::trim(normalParamString);

    for (auto it = src.begin(); it != src.end(); it++)
    {
        char c = *it;

        if (c == ' ' || c == '\t' || c == '\b') // whitespace case
        {
            if (trim) 
            {
                cPrev = c;
                continue;
            }

            if (quoteMode || chatLinkMode)
            {               
                token += c;
            }
            else
            {
                if (token != "")
                    m_storage.push_back(token);
                token = "";
                trim = true;
            }
        }
        else if (c == '\"') // quote case
        {
            if (quoteMode)
            {
                if (token != "")
                    m_storage.push_back(token);
                token = "";
                trim = true;
                quoteMode = false;
            }
            else
            {
                quoteMode = true;
                trim = false;
                if (preserveQuotes)
                    token += c;
            }
        }
        else if (c == 'c')
        {
            trim = false;
            token += c;

            if (cPrev == '|' && !quoteMode)
                chatLinkMode = true;            
        }
        else if (c == 'r')
        {                
            trim = false;
            token += c;

            if (cPrev == '|' && !quoteMode)
            {
                chatLinkMode = false;
                m_storage.push_back(token);
                token = "";
                trim = true;
            }
        }
        else
        {
            trim = false;
            token += c;
        }

        cPrev = c;
    }

    if (token != "")
        m_storage.push_back(token);
}

AdvancedArgumentTokenizer::~AdvancedArgumentTokenizer()
{
}

void AdvancedArgumentTokenizer::LoadModifier(std::string modifier, uint32 paramCount)
{
    // Do not override previously defined modifier due to potential missing parameters from normal token storage
    if (m_modifiers.find(modifier) != m_modifiers.end())
        return;

    ModifierValueStorageType valueStorage;
    valueStorage.first = paramCount;
    valueStorage.second = TokenStorage(paramCount);    

    bool modifierCheck = true;
    bool extract = false;
    int extractCount = paramCount;
    int modifierParamIndex = 0;
    int paramIndex = 0;
    std::vector<uint32> removeIndexes;

    // Copy modifier and its paramaters (if it has any) to modifier storage
    for (auto param : m_storage)
    {
        if (!IsModifier(param) && modifierCheck)
            break;
        else
            modifierCheck = false;

        if (param == modifier && !extract)
        {
            extract = true;
            removeIndexes.push_back(paramIndex);

            // remove from normalParamString
            boost::replace_first(normalParamString, param + (m_storage.size() > (paramIndex + 1) ? " " : ""), "");

            if (extractCount == 0)
                break;
        } 
        else if (extract && extractCount > 0)
        {
            valueStorage.second[modifierParamIndex] = param;
            removeIndexes.push_back(paramIndex);

            // remove from normalParamString
            boost::replace_first(normalParamString, param + (m_storage.size() > (paramIndex + 1) ? " " : ""), "");

            modifierParamIndex++;
            extractCount--;
        }

        paramIndex++;
    }

    boost::trim(normalParamString);

    // If extract is still false, then given modifier doesn't exist, remove it from modifier storage
    if (!extract)
    {
        return;        
    }

    m_modifiers[modifier] = valueStorage;

    // Remove modifier and its parameters (if it has any) from normal token storage
    int indexOffset = 0;
    for (auto index : removeIndexes)
    {
        m_storage.erase(m_storage.begin() + index - indexOffset);
        indexOffset++;
    }
}

std::string AdvancedArgumentTokenizer::GetModifierValue(std::string modifier, uint32 index)
{
    auto it = m_modifiers.find(modifier);
    if (it != m_modifiers.end())
    {
        if (index < it->second.second.size())
            return it->second.second[index];
    }

    return "";
}

std::string AdvancedArgumentTokenizer::ExtractChatLinkKey(std::string src, std::string type)
{
    return sFreedomMgr->GetChatLinkKey(src, type);
}
#pragma endregion

#pragma region FREEDOM_MANAGER
FreedomMgr::FreedomMgr()
{
    _phaseListStore = 
    {
        {1, 169},
        {2, 170},
        {4, 171},
        {8, 172},
        {16, 173},
        {32, 174},
        {64, 175},
        {128, 176},
        {256, 177}
    };
}

FreedomMgr::~FreedomMgr()
{
}

FreedomMgr* FreedomMgr::instance()
{
    static FreedomMgr instance;
    return &instance;
}

void FreedomMgr::LoadAllTables()
{
    uint32 oldMSTime = getMSTime();

    LoadPublicTeleports();
    LoadPrivateTeleports();
    LoadPublicSpells();
    LoadMorphs();
    LoadItemTemplateExtras();
    LoadGameObjectTemplateExtras();
    LoadGameObjectExtras();
    LoadCreatureExtras();
    LoadCreatureTemplateExtras();

    TC_LOG_INFO("server.loading", ">> Loaded FreedomMgr tables in %u ms", GetMSTimeDiffToNow(oldMSTime));
}

#pragma region PHASING
// | PhaseMask | PhaseID |
// | 1         | 169     |
// | 2         | 170     |
// | 4         | 171     |
// | 8         | 172     |
// | 16        | 173     |
// | 32        | 174     |
// | 64        | 175     |
// | 128       | 176     |
// | 256       | 177     |


int FreedomMgr::GetPhaseMask(uint32 phaseId)
{
    for (auto phase : _phaseListStore)
    {
        if (phase.second == phaseId)
            return phase.first;
    }

    return -1;
}

int FreedomMgr::GetPhaseId(uint32 phaseMask)
{
    for (auto phase : _phaseListStore)
    {
        if (phase.first == phaseMask)
            return phase.second;
    }

    return -1;
}

bool FreedomMgr::IsValidPhaseId(uint32 phaseId)
{
    for (auto phase : _phaseListStore)
    {
        if (phase.second == phaseId)
            return true;
    }

    return false;
}

bool FreedomMgr::IsValidPhaseMask(uint32 phaseMask)
{
    for (auto phase : _phaseListStore)
    {
        if (phase.first == phaseMask)
            return true;
    }

    return false;
}

void FreedomMgr::CreaturePhase(Creature* creature, uint32 phaseMask)
{
    if (!phaseMask)
        phaseMask = 1;

    creature->ClearPhases();

    for (int i = 1; i < 512; i = i << 1)
    {
        uint32 phase = phaseMask & i;

        if (phase)
            creature->SetInPhase(GetPhaseId(phase), false, true);
    }

    creature->SetPhaseMask(phaseMask, true);

    _creatureExtraStore[creature->GetSpawnId()].phaseMask = phaseMask;    
}

void FreedomMgr::GameObjectPhase(GameObject* go, uint32 phaseMask)
{
    if (!phaseMask)
        phaseMask = 1;

    go->ClearPhases();

    for (int i = 1; i < 512; i = i << 1)
    {
        uint32 phase = phaseMask & i;
        
        if (phase)
            go->SetInPhase(GetPhaseId(phase), false, true);
    }

    go->SetPhaseMask(phaseMask, true);

    _gameObjectExtraStore[go->GetSpawnId()].phaseMask = phaseMask;
}

void FreedomMgr::PlayerPhase(Player* player, uint32 phaseMask)
{
    if (!phaseMask)
        phaseMask = 1;

    player->ClearPhases();

    for (int i = 1; i < 512; i = i << 1)
    {
        uint32 phase = phaseMask & i;

        if (phase)
            player->SetInPhase(GetPhaseId(phase), false, true);
    }

    player->SetPhaseMask(phaseMask, true);

    _playerExtraDataStore[player->GetGUID().GetCounter()].phaseMask = phaseMask;
}

uint32 FreedomMgr::GetPlayerPhase(Player* player)
{
    return _playerExtraDataStore[player->GetGUID().GetCounter()].phaseMask;
}

#pragma endregion

#pragma region CREATURE
void FreedomMgr::LoadCreatureExtras()
{
    // clear current storage
    _creatureExtraStore.clear();

    // guid, scale, id_creator_bnet, id_creator_player, id_modifier_bnet, id_modifier_player, 
    // UNIX_TIMESTAMP(created), UNIX_TIMESTAMP(modified), phaseMask, displayLock, displayId, 
    // nativeDisplayId, genderLock, gender
    PreparedStatement* stmt = FreedomDatabase.GetPreparedStatement(FREEDOM_SEL_CREATUREEXTRA);
    PreparedQueryResult result = FreedomDatabase.Query(stmt);

    if (!result)
        return;

    do
    {
        Field * fields = result->Fetch();
        CreatureExtraData data;
        uint64 guid = fields[0].GetUInt64();
        data.scale = fields[1].GetFloat();
        data.creatorBnetAccId = fields[2].GetUInt32();
        data.creatorPlayerId = fields[3].GetUInt64();
        data.modifierBnetAccId = fields[4].GetUInt32();
        data.modifierPlayerId = fields[5].GetUInt64();
        data.created = fields[6].GetUInt64();
        data.modified = fields[7].GetUInt64();
        data.phaseMask = fields[8].GetUInt32();
        data.displayLock = fields[9].GetBool();
        data.displayId = fields[10].GetUInt32();
        data.nativeDisplayId = fields[11].GetUInt32();
        data.genderLock = fields[12].GetBool();
        data.gender = fields[13].GetUInt8();

        _creatureExtraStore[guid] = data;
    } while (result->NextRow());
}

void FreedomMgr::LoadCreatureTemplateExtras()
{
    // clear current storage
    _creatureTemplateExtraStore.clear();

    // id_entry, disabled
    PreparedStatement* stmt = FreedomDatabase.GetPreparedStatement(FREEDOM_SEL_CREATUREEXTRA_TEMPLATE);
    PreparedQueryResult result = FreedomDatabase.Query(stmt);

    if (!result)
        return;

    do
    {
        Field * fields = result->Fetch();
        CreatureTemplateExtraData data;
        uint32 entry = fields[0].GetUInt32();
        data.disabled = fields[1].GetBool();

        _creatureTemplateExtraStore[entry] = data;
    } while (result->NextRow());
}

void FreedomMgr::CreatureSetEmote(Creature* creature, uint32 emoteId)
{
    uint32 spawnId = creature->GetSpawnId();
    auto addonData = &(sObjectMgr->_creatureAddonStore[spawnId]);
    addonData->emote = emoteId;
    creature->SetUInt32Value(UNIT_NPC_EMOTESTATE, emoteId);
}

void FreedomMgr::CreatureSetMount(Creature* creature, uint32 mountId)
{
    uint32 spawnId = creature->GetSpawnId();
    auto addonData = &(sObjectMgr->_creatureAddonStore[spawnId]);
    addonData->mount = mountId;

    if (mountId)
        creature->Mount(mountId);
    else
        creature->Dismount();
}

void FreedomMgr::CreatureSetAuraToggle(Creature* creature, uint32 auraId, bool toggle)
{
    uint32 spawnId = creature->GetSpawnId();
    auto addonData = &(sObjectMgr->_creatureAddonStore[spawnId]);

    auto it = addonData->auras.begin();
    for (; it != addonData->auras.end(); it++)
    {
        if (*it == auraId && toggle) // refresh already existing aura
        {
            creature->AddAura(auraId, creature);
            return;
        }

        if (*it == auraId && !toggle) // we found auraId we want to remove
            break;
    }

    if (toggle)
    {
        creature->AddAura(auraId, creature);
        addonData->auras.push_back(auraId);
    }
    else if (it != addonData->auras.end())
    {
        creature->RemoveAura(auraId);
        addonData->auras.erase(it);
    }
}

void FreedomMgr::CreatureSetBytes1(Creature* creature, uint32 bytes1)
{
    uint32 spawnId = creature->GetSpawnId();
    auto addonData = &(sObjectMgr->_creatureAddonStore[spawnId]);
    addonData->bytes1 = bytes1;
    
    if (bytes1)
    {
        // 0 StandState
        // 1 FreeTalentPoints   Pet only, so always 0 for default creature
        // 2 StandFlags
        // 3 StandMiscFlags

        creature->SetByteValue(UNIT_FIELD_BYTES_1, 0, uint8(bytes1 & 0xFF));
        //SetByteValue(UNIT_FIELD_BYTES_1, 1, uint8((cainfo->bytes1 >> 8) & 0xFF));
        creature->SetByteValue(UNIT_FIELD_BYTES_1, 1, 0);
        creature->SetByteValue(UNIT_FIELD_BYTES_1, 2, uint8((bytes1 >> 16) & 0xFF));
        creature->SetByteValue(UNIT_FIELD_BYTES_1, 3, uint8((bytes1 >> 24) & 0xFF));

        if (uint8(bytes1 & 0xFF) == UNIT_STAND_STATE_DEAD)
            creature->SetUInt32Value(UNIT_NPC_EMOTESTATE, 0);

        //! Suspected correlation between UNIT_FIELD_BYTES_1, offset 3, value 0x2:
        //! If no inhabittype_fly (if no MovementFlag_DisableGravity or MovementFlag_CanFly flag found in sniffs)
        //! Check using InhabitType as movement flags are assigned dynamically
        //! basing on whether the creature is in air or not
        //! Set MovementFlag_Hover. Otherwise do nothing.
        if (creature->GetByteValue(UNIT_FIELD_BYTES_1, 3) & UNIT_BYTE1_FLAG_HOVER && !(creature->GetCreatureTemplate()->InhabitType & INHABIT_AIR))
            creature->AddUnitMovementFlag(MOVEMENTFLAG_HOVER);
    }
    else
    {
        creature->SetByteValue(UNIT_FIELD_BYTES_1, 0, 0);
        creature->SetByteValue(UNIT_FIELD_BYTES_1, 1, 0);
        creature->SetByteValue(UNIT_FIELD_BYTES_1, 2, 0);
        creature->SetByteValue(UNIT_FIELD_BYTES_1, 3, 0);
    }
}

void FreedomMgr::CreatureSetBytes2(Creature* creature, uint32 bytes2)
{
    uint32 spawnId = creature->GetSpawnId();
    auto addonData = &(sObjectMgr->_creatureAddonStore[spawnId]);
    addonData->bytes2 = bytes2;

    if (bytes2)
    {
        // 0 SheathState
        // 1 Bytes2Flags
        // 2 UnitRename         Pet only, so always 0 for default creature
        // 3 ShapeshiftForm     Must be determined/set by shapeshift spell/aura

        creature->SetByteValue(UNIT_FIELD_BYTES_2, 0, uint8(bytes2 & 0xFF));
        //SetByteValue(UNIT_FIELD_BYTES_2, 1, uint8((cainfo->bytes2 >> 8) & 0xFF));
        //SetByteValue(UNIT_FIELD_BYTES_2, 2, uint8((cainfo->bytes2 >> 16) & 0xFF));
        creature->SetByteValue(UNIT_FIELD_BYTES_2, 2, 0);
        //SetByteValue(UNIT_FIELD_BYTES_2, 3, uint8((cainfo->bytes2 >> 24) & 0xFF));
        creature->SetByteValue(UNIT_FIELD_BYTES_2, 3, 0);
    }
    else
    {
        creature->SetByteValue(UNIT_FIELD_BYTES_2, 0, 0);
        creature->SetByteValue(UNIT_FIELD_BYTES_2, 2, 0);
        creature->SetByteValue(UNIT_FIELD_BYTES_2, 3, 0);
    }
}

void FreedomMgr::SetCreatureTemplateExtraDisabledFlag(uint32 entryId, bool disabled)
{
    auto it = _creatureTemplateExtraStore.find(entryId);
    if (it == _creatureTemplateExtraStore.end())
        return;

    _creatureTemplateExtraStore[entryId].disabled = disabled;

    // DB update
    PreparedStatement* stmt = FreedomDatabase.GetPreparedStatement(FREEDOM_UPD_CREATUREEXTRA_TEMPLATE);
    stmt->setBool(0, disabled);
    stmt->setUInt32(1, entryId);
    FreedomDatabase.Execute(stmt);
}

void FreedomMgr::SaveCreature(Creature* creature)
{
    creature->SaveToDB();

    // Save extra attached data if it exists
    auto it = _creatureExtraStore.find(creature->GetSpawnId());

    if (it != _creatureExtraStore.end())
    {
        int index = 0;
        CreatureExtraData data = it->second;
        PreparedStatement* stmt = FreedomDatabase.GetPreparedStatement(FREEDOM_REP_CREATUREEXTRA);
        stmt->setUInt64(index++, creature->GetSpawnId());
        stmt->setFloat(index++, data.scale);
        stmt->setUInt32(index++, data.creatorBnetAccId);
        stmt->setUInt64(index++, data.creatorPlayerId);
        stmt->setUInt32(index++, data.modifierBnetAccId);
        stmt->setUInt64(index++, data.modifierPlayerId);
        stmt->setUInt64(index++, data.created);
        stmt->setUInt64(index++, data.modified);
        stmt->setUInt32(index++, data.phaseMask);
        stmt->setBool(index++, data.displayLock);
        stmt->setUInt32(index++, data.displayId);
        stmt->setUInt32(index++, data.nativeDisplayId);
        stmt->setBool(index++, data.genderLock);
        stmt->setUInt8(index++, data.gender);

        FreedomDatabase.Execute(stmt);
    }
}

void FreedomMgr::CreatureSetModifyHistory(Creature* creature, Player* modifier)
{
    if (!creature || !modifier)
        return;

    CreatureExtraData data = _creatureExtraStore[creature->GetSpawnId()];
    data.modifierBnetAccId = modifier->GetSession()->GetBattlenetAccountId();
    data.modifierPlayerId = modifier->GetGUID().GetCounter();
    data.modified = time(NULL);
    _creatureExtraStore[creature->GetSpawnId()] = data;
}

void FreedomMgr::CreatureMove(Creature* creature, float x, float y, float z, float o)
{
    if (!creature)
        return;

    if (CreatureData const* data = sObjectMgr->GetCreatureData(creature->GetSpawnId()))
    {
        const_cast<CreatureData*>(data)->posX = x;
        const_cast<CreatureData*>(data)->posY = y;
        const_cast<CreatureData*>(data)->posZ = z;
        const_cast<CreatureData*>(data)->orientation = o;
    }

    //! If hovering, always increase our server-side Z position
    //! Client automatically projects correct position based on Z coord sent in monster move
    //! and UNIT_FIELD_HOVERHEIGHT sent in object updates
    if (creature->HasUnitMovementFlag(MOVEMENTFLAG_HOVER))
        z += creature->GetFloatValue(UNIT_FIELD_HOVERHEIGHT);
    creature->Relocate(x, y, z, o);
    creature->GetMotionMaster()->Initialize();

    if (creature->IsAlive())                            // dead creature will reset movement generator at respawn
    {
        creature->setDeathState(JUST_DIED);
        creature->Respawn();
    }

    PreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_UPD_CREATURE_POSITION);

    stmt->setFloat(0, x);
    stmt->setFloat(1, y);
    stmt->setFloat(2, z);
    stmt->setFloat(3, o);
    stmt->setUInt64(4, creature->GetSpawnId());

    WorldDatabase.Execute(stmt);

    creature->SendTeleportPacket(creature->GetPosition());
}

void FreedomMgr::CreatureTurn(Creature* creature, float o)
{
    CreatureMove(creature, creature->GetPositionX(), creature->GetPositionY(), creature->GetPositionZ(), o);
}

void FreedomMgr::CreatureScale(Creature* creature, float scale)
{
    if (!creature)
        return;

    float maxScale = sConfigMgr->GetFloatDefault("Freedom.Creature.MaxScale", 15.0f);
    float minScale = sConfigMgr->GetFloatDefault("Freedom.Creature.MinScale", 0.0f);

    if (scale < minScale)
        scale = minScale;
    if (scale > maxScale)
        scale = maxScale;
    
    creature->SetObjectScale(scale);
    _creatureExtraStore[creature->GetSpawnId()].scale = scale;
}

void FreedomMgr::CreatureDelete(Creature* creature)
{    
    creature->CombatStop();
    creature->DeleteFromDB();
    creature->AddObjectToRemoveList();
}

Creature* FreedomMgr::CreatureCreate(Player* creator, CreatureTemplate const* creatureTemplate)
{
    uint32 entryId = creatureTemplate->Entry;
    float x = creator->GetPositionX();
    float y = creator->GetPositionY();
    float z = creator->GetPositionZ();
    float o = creator->GetOrientation();
    Map* map = creator->GetMap();

    if (Transport* trans = creator->GetTransport())
    {
        ObjectGuid::LowType guid = map->GenerateLowGuid<HighGuid::Creature>();
        CreatureData& data = sObjectMgr->NewOrExistCreatureData(guid);
        data.id = entryId;
        data.phaseMask = creator->GetPhaseMask();
        data.posX = creator->GetTransOffsetX();
        data.posY = creator->GetTransOffsetY();
        data.posZ = creator->GetTransOffsetZ();
        data.orientation = creator->GetTransOffsetO();

        Creature* creature = trans->CreateNPCPassenger(guid, &data);

        creature->SaveToDB(trans->GetGOInfo()->moTransport.mapID, 1 << map->GetSpawnMode(), creator->GetPhaseMask());

        sObjectMgr->AddCreatureToGrid(guid, &data);
        return creature;
    }

    Creature* creature = new Creature();
    if (!creature->Create(map->GenerateLowGuid<HighGuid::Creature>(), map, creator->GetPhaseMask(), entryId, x, y, z, o))
    {
        delete creature;
        return nullptr;
    }

    creature->SaveToDB(map->GetId(), (1 << map->GetSpawnMode()), creator->GetPhaseMask());
    
    ObjectGuid::LowType db_guid = creature->GetSpawnId();
    sFreedomMgr->CreaturePhase(creature, creator->GetPhaseMask());
    sFreedomMgr->CreatureScale(creature, creature->GetObjectScale());

    // To call _LoadGoods(); _LoadQuests(); CreateTrainerSpells()
    // current "creature" variable is deleted and created fresh new, otherwise old values might trigger asserts or cause undefined behavior
    creature->CleanupsBeforeDelete();
    delete creature;
    creature = new Creature();
    if (!creature->LoadCreatureFromDB(db_guid, map))
    {
        delete creature;
        return nullptr;
    }

    // Creation history and straight update
    CreatureExtraData data;
    data.scale = creatureTemplate->scale;
    data.creatorBnetAccId = creator->GetSession()->GetBattlenetAccountId();
    data.creatorPlayerId = creator->GetGUID().GetCounter();
    data.modifierBnetAccId = creator->GetSession()->GetBattlenetAccountId();
    data.modifierPlayerId = creator->GetGUID().GetCounter();
    data.created = time(NULL);
    data.modified = time(NULL);
    data.phaseMask = creator->GetPhaseMask();
    _creatureExtraStore[creature->GetSpawnId()] = data;

    int index = 0;
    PreparedStatement* stmt = FreedomDatabase.GetPreparedStatement(FREEDOM_REP_CREATUREEXTRA);
    stmt->setUInt64(index++, creature->GetSpawnId());
    stmt->setFloat(index++, data.scale);
    stmt->setUInt32(index++, data.creatorBnetAccId);
    stmt->setUInt64(index++, data.creatorPlayerId);
    stmt->setUInt32(index++, data.modifierBnetAccId);
    stmt->setUInt64(index++, data.modifierPlayerId);
    stmt->setUInt64(index++, data.created);
    stmt->setUInt64(index++, data.modified);
    stmt->setUInt64(index++, data.phaseMask);
    stmt->setBool(index++, data.displayLock);
    stmt->setUInt32(index++, data.displayId);
    stmt->setUInt32(index++, data.nativeDisplayId);
    stmt->setBool(index++, data.genderLock);
    stmt->setUInt8(index++, data.gender);

    FreedomDatabase.Execute(stmt);

    sObjectMgr->AddCreatureToGrid(db_guid, sObjectMgr->GetCreatureData(db_guid));    
    return creature;
}

void FreedomMgr::CreatureRefresh(Creature* creature)
{
    ObjectGuid::LowType guidLow = creature->GetSpawnId();
    Map* map = creature->GetMap();
    map->GetObjectsStore().Remove<Creature>(creature->GetGUID());
    creature->DestroyForNearbyPlayers();

    auto newGuidLow = map->GenerateLowGuid<HighGuid::Creature>();
    auto newObjectGuid = ObjectGuid::Create<HighGuid::Creature>(map->GetId(), creature->GetEntry(), newGuidLow);

    creature->SetGuidValue(OBJECT_FIELD_GUID, newObjectGuid);
    creature->SetPackGUID(newObjectGuid);
    map->GetObjectsStore().Insert(newObjectGuid, creature);
}

CreatureExtraData const* FreedomMgr::GetCreatureExtraData(uint64 guid)
{
    auto it = _creatureExtraStore.find(guid);

    if (it != _creatureExtraStore.end())
    {
        return &it->second;
    }
    else
    {
        return nullptr;
    }
}

CreatureTemplateExtraData const* FreedomMgr::GetCreatureTemplateExtraData(uint32 entry)
{
    auto it = _creatureTemplateExtraStore.find(entry);

    if (it != _creatureTemplateExtraStore.end())
    {
        return &it->second;
    }
    else
    {
        return nullptr;
    }
}

Creature* FreedomMgr::GetAnyCreature(ObjectGuid::LowType lowguid)
{
    auto data = sObjectMgr->GetCreatureData(lowguid);
    if (!data)
        return nullptr;

    auto objectGuid = ObjectGuid::Create<HighGuid::Creature>(data->mapid, data->id, lowguid);
    Map* map = sMapMgr->FindMap(data->mapid, 0);

    if (!map)
        return nullptr;

    Creature* creature = map->GetCreature(objectGuid);

    // guid is DB guid of creature
    if (!creature)
    {
        auto bounds = map->GetCreatureBySpawnIdStore().equal_range(lowguid);
        if (bounds.first == bounds.second)
            return nullptr;

        return bounds.first->second;
    }

    return creature;
}

Creature* FreedomMgr::GetAnyCreature(Map* map, ObjectGuid::LowType lowguid, uint32 entry)
{
    auto objectGuid = ObjectGuid::Create<HighGuid::Creature>(map->GetId(), entry, lowguid);

    Creature* creature = map->GetCreature(objectGuid);

    // guid is DB guid of creature
    if (!creature)
    {
        auto bounds = map->GetCreatureBySpawnIdStore().equal_range(lowguid);
        if (bounds.first == bounds.second)
            return nullptr;

        return bounds.first->second;
    }

    return creature;
}

void FreedomMgr::SetCreatureSelectionForPlayer(ObjectGuid::LowType playerId, ObjectGuid::LowType creatureId)
{
    _playerExtraDataStore[playerId].selectedCreatureGuid = creatureId;
}

ObjectGuid::LowType FreedomMgr::GetSelectedCreatureGuidFromPlayer(ObjectGuid::LowType playerId)
{
    return _playerExtraDataStore[playerId].selectedCreatureGuid;
}
#pragma endregion

#pragma region GAMEOBJECT
void FreedomMgr::LoadGameObjectExtras()
{
    // clear current storage
    _gameObjectExtraStore.clear();

    // guid, scale, id_creator_bnet, id_creator_player, id_modifier_bnet, id_modifier_player, UNIX_TIMESTAMP(created), UNIX_TIMESTAMP(modified), phaseMask
    PreparedStatement* stmt = FreedomDatabase.GetPreparedStatement(FREEDOM_SEL_GAMEOBJECTEXTRA);
    PreparedQueryResult result = FreedomDatabase.Query(stmt);

    if (!result)
        return;

    do
    {
        Field * fields = result->Fetch();
        GameObjectExtraData data;
        uint64 guid = fields[0].GetUInt64();
        data.scale = fields[1].GetFloat();
        data.creatorBnetAccId = fields[2].GetUInt32();
        data.creatorPlayerId = fields[3].GetUInt64();
        data.modifierBnetAccId = fields[4].GetUInt32();
        data.modifierPlayerId = fields[5].GetUInt64();
        data.created = fields[6].GetUInt64();
        data.modified = fields[7].GetUInt64();
        data.phaseMask = fields[8].GetUInt32();

        _gameObjectExtraStore[guid] = data;
    } while (result->NextRow());
}

void FreedomMgr::LoadGameObjectTemplateExtras()
{
    // clear current storage
    _gameObjectTemplateExtraStore.clear();

    // id_entry, disabled, model_name, model_type, is_default
    PreparedStatement* stmt = FreedomDatabase.GetPreparedStatement(FREEDOM_SEL_GAMEOBJECTEXTRA_TEMPLATE);
    PreparedQueryResult result = FreedomDatabase.Query(stmt);

    if (!result)
        return;

    do
    {
        Field * fields = result->Fetch();
        GameObjectTemplateExtraData data;
        uint32 entry = fields[0].GetUInt32();
        data.disabled = fields[1].GetBool();
        data.modelName = fields[2].GetString();
        data.modelType = fields[3].GetString();
        data.isDefault = fields[4].GetBool();

        _gameObjectTemplateExtraStore[entry] = data;
    } while (result->NextRow());
}

GameObjectTemplateExtraData const* FreedomMgr::GetGameObjectTemplateExtraData(uint32 entry)
{
    auto it = _gameObjectTemplateExtraStore.find(entry);

    if (it != _gameObjectTemplateExtraStore.end())
    {
        return &it->second;
    }

    return nullptr;
}

void FreedomMgr::SetGameobjectTemplateExtraDisabledFlag(uint32 entryId, bool disabled)
{
    auto it = _gameObjectTemplateExtraStore.find(entryId);
    if (it == _gameObjectTemplateExtraStore.end())
        return;

    _gameObjectTemplateExtraStore[entryId].disabled = disabled;

    // DB update
    PreparedStatement* stmt = FreedomDatabase.GetPreparedStatement(FREEDOM_UPD_GAMEOBJECTEXTRA_TEMPLATE);
    stmt->setBool(0, disabled);
    stmt->setUInt32(1, entryId);
    FreedomDatabase.Execute(stmt);
}

void FreedomMgr::SaveGameObject(GameObject* go)
{
    go->SaveToDB();

    // Save extra attached data if it exists
    auto it = _gameObjectExtraStore.find(go->GetSpawnId());

    if (it != _gameObjectExtraStore.end())
    {
        int index = 0;
        GameObjectExtraData data = it->second;
        PreparedStatement* stmt = FreedomDatabase.GetPreparedStatement(FREEDOM_REP_GAMEOBJECTEXTRA);
        stmt->setUInt64(index++, go->GetSpawnId());
        stmt->setFloat(index++, data.scale);
        stmt->setUInt32(index++, data.creatorBnetAccId);
        stmt->setUInt64(index++, data.creatorPlayerId);
        stmt->setUInt32(index++, data.modifierBnetAccId);
        stmt->setUInt64(index++, data.modifierPlayerId);
        stmt->setUInt64(index++, data.created);
        stmt->setUInt64(index++, data.modified);
        stmt->setUInt32(index++, data.phaseMask);

        FreedomDatabase.Execute(stmt);
    }
}

GameObject* FreedomMgr::GetAnyGameObject(Map* objMap, ObjectGuid::LowType lowguid, uint32 entry)
{
    GameObject* obj = nullptr;

    obj = objMap->GetGameObject(ObjectGuid::Create<HighGuid::GameObject>(objMap->GetId(), entry, lowguid));

    // guid is DB guid of object
    if (!obj && sObjectMgr->GetGOData(lowguid))
    {
        auto bounds = objMap->GetGameObjectBySpawnIdStore().equal_range(lowguid);
        if (bounds.first == bounds.second)
            return nullptr;

        return bounds.first->second;
    }

    if (!obj)
        objMap->GetGameObject(ObjectGuid::Create<HighGuid::Transport>(lowguid));

    return obj;
}

void FreedomMgr::SetGameobjectSelectionForPlayer(ObjectGuid::LowType playerId, ObjectGuid::LowType gameobjectId)
{
    _playerExtraDataStore[playerId].selectedGameobjectGuid = gameobjectId;
}

ObjectGuid::LowType FreedomMgr::GetSelectedGameobjectGuidFromPlayer(ObjectGuid::LowType playerId)
{
    return _playerExtraDataStore[playerId].selectedGameobjectGuid;
}

// https://github.com/TrinityCore/TrinityCore/commit/e68ff4186e685de00362b12bc0b5084a4d6065dd
GameObject* FreedomMgr::GameObjectRefresh(GameObject* go)
{
    ObjectGuid::LowType guidLow = go->GetSpawnId();
    Map* map = go->GetMap();
    go->Delete();
    go = new GameObject();
    if (!go->LoadGameObjectFromDB(guidLow, map))
    {
        delete go;
        return nullptr;
    }

    return go;
}

void FreedomMgr::GameObjectMove(GameObject* go, float x, float y, float z, float o)
{
    go->Relocate(x, y, z, o);
    go->UpdateRotationFields();
}

void FreedomMgr::GameObjectTurn(GameObject* go, float o)
{
    GameObjectMove(go, go->GetPositionX(), go->GetPositionY(), go->GetPositionZ(), o);
}

void FreedomMgr::GameObjectScale(GameObject* go, float scale)
{
    if (!go)
        return;

    float maxScale = sConfigMgr->GetFloatDefault("Freedom.Gameobject.MaxScale", 15.0f);
    float minScale = sConfigMgr->GetFloatDefault("Freedom.Gameobject.MinScale", 0.0f);

    if (scale < minScale)
        scale = minScale;
    if (scale > maxScale)
        scale = maxScale;

    go->SetObjectScale(scale);
    _gameObjectExtraStore[go->GetSpawnId()].scale = scale;
}

void FreedomMgr::GameObjectDelete(GameObject* go)
{
    go->SetRespawnTime(0);
    go->Delete();
    go->DeleteFromDB();
}

void FreedomMgr::GameObjectSetModifyHistory(GameObject* go, Player* modifier)
{
    if (!go || !modifier)
        return;

    GameObjectExtraData data = _gameObjectExtraStore[go->GetSpawnId()];    
    data.modifierBnetAccId = modifier->GetSession()->GetBattlenetAccountId();
    data.modifierPlayerId = modifier->GetGUID().GetCounter();    
    data.modified = time(NULL);
    _gameObjectExtraStore[go->GetSpawnId()] = data;
}

GameObject* FreedomMgr::GameObjectCreate(Player* creator, GameObjectTemplate const* gobTemplate, uint32 spawnTimeSecs)
{
    if (gobTemplate->displayId && !sGameObjectDisplayInfoStore.LookupEntry(gobTemplate->displayId))
    {
        // report to DB errors log as in loading case
        TC_LOG_ERROR("sql.sql", "Gameobject (Entry %u GoType: %u) have invalid displayId (%u), not spawned.", gobTemplate->entry, gobTemplate->type, gobTemplate->displayId);
        return nullptr;
    }

    float x, y, z, o;
    creator->GetPosition(x, y, z, o);
    Map* map = creator->GetMap();

    GameObject* object = new GameObject;

    if (!object->Create(gobTemplate->entry, map, 0, x, y, z, o, 0.0f, 0.0f, 0.0f, 0.0f, 0, GO_STATE_READY))
    {
        delete object;
        return nullptr;
    }

    if (spawnTimeSecs)
    {
        object->SetRespawnTime(spawnTimeSecs);
    }

    // fill the gameobject data and save to the db
    object->SaveToDB(map->GetId(), (1 << map->GetSpawnMode()), creator->GetPhaseMask());
    
    ObjectGuid::LowType spawnId = object->GetSpawnId();
    sFreedomMgr->GameObjectPhase(object, creator->GetPhaseMask());
    sFreedomMgr->GameObjectScale(object, object->GetObjectScale());

    // delete the old object and do a clean load from DB with a fresh new GameObject instance.
    // this is required to avoid weird behavior and memory leaks
    delete object;

    object = new GameObject();
    // this will generate a new guid if the object is in an instance
    if (!object->LoadGameObjectFromDB(spawnId, map))
    {
        delete object;
        return nullptr;
    }

    /// @todo is it really necessary to add both the real and DB table guid here ?
    sObjectMgr->AddGameobjectToGrid(spawnId, ASSERT_NOTNULL(sObjectMgr->GetGOData(spawnId)));

    // Creation history and straight update
    GameObjectExtraData data;
    data.scale = object->GetGOInfo()->size;
    data.creatorBnetAccId = creator->GetSession()->GetBattlenetAccountId();
    data.creatorPlayerId = creator->GetGUID().GetCounter();
    data.modifierBnetAccId = creator->GetSession()->GetBattlenetAccountId();
    data.modifierPlayerId = creator->GetGUID().GetCounter();
    data.created = time(NULL);
    data.modified = time(NULL);
    data.phaseMask = creator->GetPhaseMask();

    _gameObjectExtraStore[spawnId] = data;

    int index = 0;
    PreparedStatement* stmt = FreedomDatabase.GetPreparedStatement(FREEDOM_REP_GAMEOBJECTEXTRA);
    stmt->setUInt64(index++, spawnId);
    stmt->setFloat(index++, data.scale);
    stmt->setUInt32(index++, data.creatorBnetAccId);
    stmt->setUInt64(index++, data.creatorPlayerId);
    stmt->setUInt32(index++, data.modifierBnetAccId);
    stmt->setUInt64(index++, data.modifierPlayerId);
    stmt->setUInt64(index++, data.created);
    stmt->setUInt64(index++, data.modified);
    stmt->setUInt64(index++, data.phaseMask);

    FreedomDatabase.Execute(stmt);

    return object;
}

GameObjectExtraData const* FreedomMgr::GetGameObjectExtraData(uint64 guid)
{
    auto it = _gameObjectExtraStore.find(guid);

    if (it != _gameObjectExtraStore.end())
    {
        return &it->second;
    }
    else
    {
        return nullptr;
    }
}
#pragma endregion

#pragma region ITEMS
void FreedomMgr::LoadItemTemplateExtras()
{
    // clear current storage
    _itemTemplateExtraStore.clear();

    // entry_id, hidden
    PreparedStatement* stmt = FreedomDatabase.GetPreparedStatement(FREEDOM_SEL_ITEMTEMPLATEEXTRA);
    PreparedQueryResult result = FreedomDatabase.Query(stmt);

    if (!result)
        return;

    do
    {
        Field * fields = result->Fetch();
        ItemTemplateExtraData data;
        uint32 itemId = fields[0].GetUInt32();
        data.hidden = fields[1].GetBool();

        _itemTemplateExtraStore[itemId] = data;
    } while (result->NextRow());
}

ItemTemplateExtraData const* FreedomMgr::GetItemTemplateExtraById(uint32 itemId)
{
    auto it = _itemTemplateExtraStore.find(itemId);
    if (it != _itemTemplateExtraStore.end())
        return &it->second;
    else
        return nullptr;
}

void FreedomMgr::SetItemTemplateExtraHiddenFlag(uint32 itemId, bool hidden)
{
    auto it = _itemTemplateExtraStore.find(itemId);
    if (it == _itemTemplateExtraStore.end())
        return;

    _itemTemplateExtraStore[itemId].hidden = hidden;

    // DB update
    PreparedStatement* stmt = FreedomDatabase.GetPreparedStatement(FREEDOM_UPD_ITEMTEMPLATEEXTRA);
    stmt->setBool(0, hidden);
    stmt->setUInt32(1, itemId);
    FreedomDatabase.Execute(stmt);
}
#pragma endregion

#pragma region PLAYERS
void FreedomMgr::RemoveHoverFromPlayer(Player* player)
{
    Unit* source_unit = player->ToUnit();
    float hoverHeight = source_unit->GetFloatValue(UNIT_FIELD_HOVERHEIGHT);

    source_unit->RemoveUnitMovementFlag(MOVEMENTFLAG_HOVER);

    if (hoverHeight)
    {
        float newZ = source_unit->GetPositionZ() - hoverHeight;
        source_unit->UpdateAllowedPositionZ(source_unit->GetPositionX(), source_unit->GetPositionY(), newZ);
        source_unit->UpdateHeight(newZ);
    }

    WorldPackets::Movement::MoveSetFlag packet(SMSG_MOVE_UNSET_HOVERING);
    packet.MoverGUID = source_unit->GetGUID();
    source_unit->SendMessageToSet(packet.Write(), true);

    // Force player on the ground after removing hover
    source_unit->SetUnitMovementFlags(MOVEMENTFLAG_FALLING);
    WorldPackets::Movement::MoveUpdate moveUpdate;
    moveUpdate.movementInfo = &source_unit->m_movementInfo;
    source_unit->SendMessageToSet(moveUpdate.Write(), true);
}

void FreedomMgr::RemoveWaterwalkFromPlayer(Player* player)
{
    Unit* source_unit = player->ToUnit();
    source_unit->RemoveUnitMovementFlag(MOVEMENTFLAG_WATERWALKING);

    WorldPackets::Movement::MoveSetFlag packet(SMSG_MOVE_SET_LAND_WALK);
    packet.MoverGUID = source_unit->GetGUID();
    source_unit->SendMessageToSet(packet.Write(), true);
}

void FreedomMgr::RemoveFlyFromPlayer(Player* player)
{
    Unit* source_unit = player->ToUnit();
    source_unit->RemoveUnitMovementFlag(MOVEMENTFLAG_CAN_FLY | MOVEMENTFLAG_MASK_MOVING_FLY);

    if (!source_unit->IsLevitating())
        source_unit->SetFall(true);

    WorldPackets::Movement::MoveSetFlag packet(SMSG_MOVE_UNSET_CAN_FLY);
    packet.MoverGUID = source_unit->GetGUID();
    source_unit->SendMessageToSet(packet.Write(), true);
}
#pragma endregion

#pragma region MISC

std::string FreedomMgr::GetMapName(uint32 mapId)
{
    const MapEntry* map = sMapStore.LookupEntry(mapId);

    if (map)
        return map->MapName_lang;
    else
        return "Unknown";
}

std::string FreedomMgr::GetChatLinkKey(std::string const &src, std::string type)
{
    if (src.empty())
        return "";

    std::string typePart = "|" + type + ":";
    std::string key = "";
    std::size_t pos = src.find(typePart); // find start pos of "|Hkeytype:" fragment first

    // Check for plain number first
    std::string plainNumber = src;
    boost::trim(plainNumber); // trim spaces
    plainNumber = plainNumber.substr(0, plainNumber.find(' ')); // get first token in case src had multiple ones
    if (isNumeric(plainNumber.c_str()))
        return plainNumber;

    // Do ChatLink check
    if (pos != std::string::npos)
    {
        auto it = src.begin();
        std::advance(it, pos + typePart.length());
        
        // if key part iteration ends without encountering ':' or '|', 
        // then link was malformed and we return empty string later on
        for (; it != src.end(); it++)
        {
            char c = *it;

            if (c == ':' || c == '|')
                return key;

            key += c;
        }
    }

    return "";
}

std::string FreedomMgr::ToDateTimeString(time_t t)
{
    tm aTm;
    localtime_r(&t, &aTm);
    //       YYYY   year
    //       MM     month (2 digits 01-12)
    //       DD     day (2 digits 01-31)
    //       HH     hour (2 digits 00-23)
    //       MM     minutes (2 digits 00-59)
    //       SS     seconds (2 digits 00-59)
    char buf[20];
    snprintf(buf, 20, "%04d-%02d-%02d %02d:%02d:%02d", aTm.tm_year + 1900, aTm.tm_mon + 1, aTm.tm_mday, aTm.tm_hour, aTm.tm_min, aTm.tm_sec);
    return std::string(buf);
}

std::string FreedomMgr::ToDateString(time_t t)
{
    tm aTm;
    localtime_r(&t, &aTm);
    //       YYYY   year
    //       MM     month (2 digits 01-12)
    //       DD     day (2 digits 01-31)
    //       HH     hour (2 digits 00-23)
    //       MM     minutes (2 digits 00-59)
    //       SS     seconds (2 digits 00-59)
    char buf[14];
    snprintf(buf, 14, "%04d-%02d-%02d", aTm.tm_year + 1900, aTm.tm_mon + 1, aTm.tm_mday);
    return std::string(buf);
}

#pragma endregion

#pragma region PUBLIC_TELEPORT

PublicTeleData const* FreedomMgr::GetPublicTeleport(std::string const& name)
{
    auto it = std::find_if(_publicTeleStore.begin(), _publicTeleStore.end(), [name](PublicTeleData t)->bool { return boost::iequals(name, t.name); });

    if (it != _publicTeleStore.end())
        return &(*it);
    else
        return nullptr;
}

PublicTeleData const* FreedomMgr::GetFirstClosestPublicTeleport(std::string const& name)
{
    // Priority to exact match
    auto it = std::find_if(_publicTeleStore.begin(), _publicTeleStore.end(), [name](PublicTeleData t)->bool { return boost::iequals(name, t.name); });

    if (it != _publicTeleStore.end())
    {
        return &(*it);
    }
    else // Find first starts-with match, if exact match fails
    {
        it = std::find_if(_publicTeleStore.begin(), _publicTeleStore.end(), [name](PublicTeleData t)->bool { return boost::istarts_with(t.name, name); });

        if (it != _publicTeleStore.end())
            return &(*it);
        else
            return nullptr;
    }
}

void FreedomMgr::AddPublicTeleport(PublicTeleData const& data)
{
    _publicTeleStore.push_back(data);

    // name, position_x, position_y, position_z, orientation, map, id_bnet_gm
    int index = 0;
    PreparedStatement* stmt = FreedomDatabase.GetPreparedStatement(FREEDOM_INS_PUBLIC_TELE);
    stmt->setString(index++, data.name);
    stmt->setFloat(index++, data.x);
    stmt->setFloat(index++, data.y);
    stmt->setFloat(index++, data.z);
    stmt->setFloat(index++, data.o);
    stmt->setUInt32(index++, data.map);
    stmt->setUInt32(index++, data.gmBnetAccId);
    FreedomDatabase.Execute(stmt);
}

void FreedomMgr::DeletePublicTeleport(std::string const& name)
{
    auto it = std::find_if(_publicTeleStore.begin(), _publicTeleStore.end(), [name](PublicTeleData t)->bool { return boost::iequals(name, t.name); });

    if (it != _publicTeleStore.end())
    {             
        // WHERE name = ?
        PreparedStatement* stmt = FreedomDatabase.GetPreparedStatement(FREEDOM_DEL_PUBLIC_TELE_NAME);
        stmt->setString(0, name);
        FreedomDatabase.Execute(stmt);

        _publicTeleStore.erase(it);
    }
}

void FreedomMgr::LoadPublicTeleports()
{
    // clear current storage
    _publicTeleStore.clear();

    // name, position_x, position_y, position_z, orientation, map, id_bnet_gm
    PreparedStatement* stmt = FreedomDatabase.GetPreparedStatement(FREEDOM_SEL_PUBLIC_TELE);
    PreparedQueryResult result = FreedomDatabase.Query(stmt);

    if (!result)
        return;

    do
    {
        Field * fields = result->Fetch();
        PublicTeleData data;
        data.name = fields[0].GetString();
        data.x = fields[1].GetFloat();
        data.y = fields[2].GetFloat();
        data.z = fields[3].GetFloat();
        data.o = fields[4].GetFloat();
        data.map = fields[5].GetUInt32();        
        data.gmBnetAccId = fields[6].GetUInt32();

        _publicTeleStore.push_back(data);
    } while (result->NextRow());
}

#pragma endregion

#pragma region PRIVATE_TELEPORT

PrivateTeleData const* FreedomMgr::GetPrivateTeleport(uint32 bnetAccountId, std::string const& name)
{
    auto it = std::find_if(_privateTeleStore[bnetAccountId].begin(), _privateTeleStore[bnetAccountId].end(), [name](PrivateTeleData t)->bool { return boost::iequals(name, t.name); });

    if (it != _privateTeleStore[bnetAccountId].end())
        return &(*it);
    else
        return nullptr;
}

PrivateTeleData const* FreedomMgr::GetFirstClosestPrivateTeleport(uint32 bnetAccountId, std::string const& name)
{
    // Priority to exact match
    auto it = std::find_if(_privateTeleStore[bnetAccountId].begin(), _privateTeleStore[bnetAccountId].end(), [name](PrivateTeleData t)->bool { return boost::iequals(name, t.name); });

    if (it != _privateTeleStore[bnetAccountId].end())
    {
        return &(*it);
    }
    else // Find first starts-with match, if exact match fails
    {
        it = std::find_if(_privateTeleStore[bnetAccountId].begin(), _privateTeleStore[bnetAccountId].end(), [name](PrivateTeleData t)->bool { return boost::istarts_with(t.name, name); });

        if (it != _privateTeleStore[bnetAccountId].end())
            return &(*it);
        else
            return nullptr;
    }
}

void FreedomMgr::AddPrivateTeleport(uint32 bnetAccountId, PrivateTeleData const& data)
{
    _privateTeleStore[bnetAccountId].push_back(data);

    // position_x, position_y, position_z, orientation, map, name, id_bnet_gm
    int index = 0;
    PreparedStatement* stmt = FreedomDatabase.GetPreparedStatement(FREEDOM_INS_PRIVATE_TELE);
    stmt->setString(index++, data.name);
    stmt->setFloat(index++, data.x);
    stmt->setFloat(index++, data.y);
    stmt->setFloat(index++, data.z);
    stmt->setFloat(index++, data.o);
    stmt->setUInt32(index++, data.map);
    stmt->setUInt32(index++, bnetAccountId);
    FreedomDatabase.Execute(stmt);
}

void FreedomMgr::DeletePrivateTeleport(uint32 bnetAccountId, std::string const& name)
{
    auto it = std::find_if(_privateTeleStore[bnetAccountId].begin(), _privateTeleStore[bnetAccountId].end(), [name](PrivateTeleData t)->bool { return boost::iequals(name, t.name); });

    if (it != _privateTeleStore[bnetAccountId].end())
    {
        // WHERE name = ? AND id_bnet_account = ?
        PreparedStatement* stmt = FreedomDatabase.GetPreparedStatement(FREEDOM_DEL_PRIVATE_TELE_NAME);
        stmt->setString(0, name);
        stmt->setUInt32(1, bnetAccountId);
        FreedomDatabase.Execute(stmt);

        _privateTeleStore[bnetAccountId].erase(it);
    }
}

void FreedomMgr::LoadPrivateTeleports()
{
    // clear current storage
    _privateTeleStore.clear();

    // name, position_x, position_y, position_z, orientation, map, id_bnet_account
    PreparedStatement* stmt = FreedomDatabase.GetPreparedStatement(FREEDOM_SEL_PRIVATE_TELE);
    PreparedQueryResult result = FreedomDatabase.Query(stmt);

    if (!result)
        return;

    do
    {
        Field * fields = result->Fetch();
        PrivateTeleData data;
        data.name = fields[0].GetString();
        data.x = fields[1].GetFloat();
        data.y = fields[2].GetFloat();
        data.z = fields[3].GetFloat();
        data.o = fields[4].GetFloat();
        data.map = fields[5].GetUInt32();
        uint32 bnetAccId = fields[6].GetUInt32();

        _privateTeleStore[bnetAccId].push_back(data);
    } while (result->NextRow());
}

#pragma endregion

#pragma region PUBLIC_SPELL

PublicSpellData const* FreedomMgr::GetPublicSpell(uint32 spellId)
{
    auto it = _publicSpellStore.find(spellId);
    if (it != _publicSpellStore.end())
        return &it->second;
    else
        return nullptr;
}

void FreedomMgr::AddPublicSpell(uint32 spellId, PublicSpellData const& data)
{
    _publicSpellStore[spellId] = data;

    // spell_id, allow_targeting, name, id_bnet_gm
    int index = 0;
    PreparedStatement* stmt = FreedomDatabase.GetPreparedStatement(FREEDOM_INS_PUBLIC_SPELL);
    stmt->setUInt32(index++, spellId);
    stmt->setUInt8(index++, data.targetOthers);
    stmt->setString(index++, data.name);
    stmt->setUInt32(index++, data.gmBnetAccId);
    FreedomDatabase.Execute(stmt);
}

void FreedomMgr::DeletePublicSpell(uint32 spellId)
{
    if (_publicSpellStore.find(spellId) != _publicSpellStore.end())
    {
        // WHERE spell_id = ?
        PreparedStatement* stmt = FreedomDatabase.GetPreparedStatement(FREEDOM_DEL_PUBLIC_SPELL_ID);
        stmt->setUInt32(0, spellId);
        FreedomDatabase.Execute(stmt);

        _publicSpellStore.erase(spellId);
    }
}

void FreedomMgr::LoadPublicSpells()
{
    // clear current storage
    _publicSpellStore.clear();

    // spell_id, allow_targeting, name, id_bnet_gm
    PreparedStatement* stmt = FreedomDatabase.GetPreparedStatement(FREEDOM_SEL_PUBLIC_SPELL);
    PreparedQueryResult result = FreedomDatabase.Query(stmt);

    if (!result)
        return;

    do
    {
        Field * fields = result->Fetch();
        PublicSpellData data;
        uint32 spellId = fields[0].GetUInt32();
        data.targetOthers = fields[1].GetUInt8();
        data.name = fields[2].GetString();
        data.gmBnetAccId = fields[3].GetUInt32();

        _publicSpellStore[spellId] = data;
    } while (result->NextRow());
}

#pragma endregion

#pragma region MORPHS

MorphData const* FreedomMgr::GetMorphByName(ObjectGuid::LowType playerId, std::string const& name)
{
    auto it = std::find_if(_playerExtraDataStore[playerId].morphDataStore.begin(), _playerExtraDataStore[playerId].morphDataStore.end(), [name](MorphData m)->bool { return boost::iequals(name, m.name); });

    if (it != _playerExtraDataStore[playerId].morphDataStore.end())
        return &(*it);
    else
        return nullptr;
}

MorphData const* FreedomMgr::GetMorphByDisplayId(ObjectGuid::LowType playerId, uint32 displayId)
{
    auto it = std::find_if(_playerExtraDataStore[playerId].morphDataStore.begin(), _playerExtraDataStore[playerId].morphDataStore.end(), [displayId](MorphData m)->bool { return m.displayId == displayId; });

    if (it != _playerExtraDataStore[playerId].morphDataStore.end())
        return &(*it);
    else
        return nullptr;
}

void FreedomMgr::AddMorph(ObjectGuid::LowType playerId, MorphData const& data)
{
    _playerExtraDataStore[playerId].morphDataStore.push_back(data);

    // guid, name, id_display, id_bnet_gm
    int index = 0;
    PreparedStatement* stmt = FreedomDatabase.GetPreparedStatement(FREEDOM_INS_MORPHS);
    stmt->setUInt64(index++, playerId);
    stmt->setString(index++, data.name);
    stmt->setUInt32(index++, data.displayId);
    stmt->setUInt32(index++, data.gmBnetAccId);
    FreedomDatabase.Execute(stmt);
}

void FreedomMgr::DeleteMorphByName(ObjectGuid::LowType playerId, std::string const& name)
{
    auto it = std::find_if(_playerExtraDataStore[playerId].morphDataStore.begin(), _playerExtraDataStore[playerId].morphDataStore.end(), [name](MorphData m)->bool { return boost::iequals(name, m.name); });

    if (it != _playerExtraDataStore[playerId].morphDataStore.end())
    {
        PreparedStatement* stmt = FreedomDatabase.GetPreparedStatement(FREEDOM_DEL_MORPHS_NAME);
        stmt->setString(0, name);
        stmt->setUInt64(1, playerId);
        FreedomDatabase.Execute(stmt);

        _playerExtraDataStore[playerId].morphDataStore.erase(it);
    }
}

void FreedomMgr::LoadMorphs()
{
    // clear current Morph storage for each player
    for (auto it : _playerExtraDataStore)
    {
        it.second.morphDataStore.clear();
    }

    // guid, name, id_display, id_bnet_gm
    PreparedStatement* stmt = FreedomDatabase.GetPreparedStatement(FREEDOM_SEL_MORPHS);
    PreparedQueryResult result = FreedomDatabase.Query(stmt);

    if (!result)
        return;

    do
    {
        Field * fields = result->Fetch();
        MorphData data;
        ObjectGuid::LowType charGuid = fields[0].GetUInt64();
        data.name = fields[1].GetString();
        data.displayId = fields[2].GetUInt32();
        data.gmBnetAccId = fields[3].GetUInt32();        

        _playerExtraDataStore[charGuid].morphDataStore.push_back(data);
    } while (result->NextRow());
}

#pragma endregion

#pragma endregion