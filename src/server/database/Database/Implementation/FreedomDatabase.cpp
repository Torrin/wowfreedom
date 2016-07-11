/*
 * Copyright (C) 2008-2016 TrinityCore <http://www.trinitycore.org/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "FreedomDatabase.h"

void FreedomDatabaseConnection::DoPrepareStatements()
{
    if (!m_reconnecting)
        m_stmts.resize(MAX_FREEDOMDATABASE_STATEMENTS);

    // SELECTS
    PrepareStatement(FREEDOM_SEL_PUBLIC_TELE, "SELECT name, position_x, position_y, position_z, orientation, map, id_bnet_gm FROM public_tele ORDER BY name", CONNECTION_SYNCH);
    PrepareStatement(FREEDOM_SEL_PRIVATE_TELE, "SELECT name, position_x, position_y, position_z, orientation, map, id_bnet_account FROM private_tele ORDER BY id_bnet_account, name", CONNECTION_SYNCH);
    PrepareStatement(FREEDOM_SEL_PUBLIC_SPELL, "SELECT spell_id, allow_targeting, name, id_bnet_gm FROM public_spell", CONNECTION_SYNCH);
    PrepareStatement(FREEDOM_SEL_MORPHS, "SELECT guid, name, id_display, id_bnet_gm FROM morphs ORDER BY guid, name", CONNECTION_SYNCH);
    PrepareStatement(FREEDOM_SEL_ITEMTEMPLATEEXTRA, "SELECT entry_id, hidden FROM item_template_extra", CONNECTION_SYNCH);
    PrepareStatement(FREEDOM_SEL_GAMEOBJECTEXTRA, "SELECT guid, scale, id_creator_bnet, id_creator_player, id_modifier_bnet, id_modifier_player, UNIX_TIMESTAMP(created), UNIX_TIMESTAMP(modified) FROM gameobject_extra", CONNECTION_SYNCH);

    // INSERTS
    PrepareStatement(FREEDOM_INS_PUBLIC_TELE, "INSERT INTO public_tele (name, position_x, position_y, position_z, orientation, map, id_bnet_gm) VALUES (?, ?, ?, ?, ?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(FREEDOM_INS_PRIVATE_TELE, "INSERT INTO private_tele (name, position_x, position_y, position_z, orientation, map, id_bnet_account) VALUES (?, ?, ?, ?, ?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(FREEDOM_INS_PUBLIC_SPELL, "INSERT INTO public_spell (spell_id, allow_targeting, name, id_bnet_gm) VALUES (?, ?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(FREEDOM_INS_MORPHS, "INSERT INTO morphs (guid, name, id_display, id_bnet_gm) VALUES (?, ?, ?, ?)", CONNECTION_ASYNC);

    // DELETIONS
    PrepareStatement(FREEDOM_DEL_PUBLIC_TELE_NAME, "DELETE FROM public_tele WHERE name = ?", CONNECTION_ASYNC);
    PrepareStatement(FREEDOM_DEL_PRIVATE_TELE_NAME, "DELETE FROM private_tele WHERE name = ? AND id_bnet_account = ?", CONNECTION_ASYNC);
    PrepareStatement(FREEDOM_DEL_PUBLIC_SPELL_ID, "DELETE FROM public_spell WHERE spell_id = ?", CONNECTION_ASYNC);
    PrepareStatement(FREEDOM_DEL_MORPHS_NAME, "DELETE FROM morphs WHERE name = ? AND guid = ?", CONNECTION_ASYNC);
    PrepareStatement(FREEDOM_DEL_GAMEOBJECTEXTRA, "DELETE FROM gameobject_extra WHERE guid = ?", CONNECTION_ASYNC);

    // UPDATES
    PrepareStatement(FREEDOM_UPD_ITEMTEMPLATEEXTRA, "UPDATE item_template_extra SET hidden = ? WHERE entry_id = ?", CONNECTION_ASYNC);

    // REPLACES
    PrepareStatement(FREEDOM_REP_GAMEOBJECTEXTRA, "REPLACE INTO gameobject_extra VALUES (?, ?, ?, ?, ?, ?, from_unixtime(?), from_unixtime(?))", CONNECTION_ASYNC);
}
