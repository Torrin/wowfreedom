DELETE FROM wod_world.trinity_string
WHERE entry >= 100000;

-- Colors and color closure
SET @C_TITLE := '|cffd2691e';
SET @C_LINK := '|cffff4500';
SET @C_TEXT := '|cffbbbbbb';
SET @C_ERR := '|cffff0000';
SET @C_CLOSE := '|r'; -- stops/closes color tag

-- Templates
SET @T_CMD_INFO := CONCAT(@C_TITLE, '>> ', @C_CLOSE);
SET @T_CMD_SYNTAX := CONCAT(@C_TITLE, 'SYNTAX: ', @C_CLOSE);
SET @T_CMD_ERROR := CONCAT(@C_ERR, 'ERROR: ', @C_CLOSE);
SET @T_GLOBAL_GM := CONCAT(@C_LINK, '[GM-NOTIFY]', @C_CLOSE);

INSERT INTO wod_world.trinity_string (entry, content_default) VALUES
/* FREEDOM_CMDE_NOT_YET_IMPLEMENTED 					*/ (100000, CONCAT(@T_CMD_ERROR, @C_TEXT, 'This command isn\'t implemented yet.', @C_CLOSE)),
/* FREEDOM_CMDI_MONEY_RESET 								*/ (100001, CONCAT(@T_CMD_INFO, @C_TEXT, 'Your money has been reset.', @C_CLOSE)),
/* FREEDOM_CMDE_NOT_ENOUGH_PARAMS 						*/ (100002, CONCAT(@T_CMD_ERROR, @C_TEXT, 'Not enough parameters.', @C_CLOSE)),
/* FREEDOM_CMDH_GOTO_RELATIVE 							*/ (100003, CONCAT(@T_CMD_SYNTAX, '.goto $local_x [$local_y [$local_z [$local_deg]]]')),
/* FREEDOM_E_INVALID_MAP_COORD 							*/ (100004, CONCAT(@T_CMD_ERROR, @C_TEXT, 'Target map or coordinates is invalid (X: %f Y: %f MapId: %u).', @C_CLOSE)),
/* FREEDOM_CMDI_FLAG_FOR_CUSTOMIZATION					*/ (100005, CONCAT(@T_CMD_INFO, @C_TEXT, 'Your character has been successfully flagged for re-customization.', @C_CLOSE)),
/* FREEDOM_CMDI_FLAG_FOR_RACECHANGE						*/ (100006, CONCAT(@T_CMD_INFO, @C_TEXT, 'Your character has been successfully flagged for race change.', @C_CLOSE)),
/* FREEDOM_CMDI_FLAG_FOR_FACTIONCHANGE					*/ (100007, CONCAT(@T_CMD_INFO, @C_TEXT, 'Your character has been successfully flagged for faction change.', @C_CLOSE)),
/* FREEDOM_CMDI_FIX_ITEMS									*/ (100008, CONCAT(@T_CMD_INFO, @C_TEXT, 'All your character\'s items are repaired.', @C_CLOSE)),
/* FREEDOM_CMDH_WATERWALK 									*/ (100009, CONCAT(@T_CMD_SYNTAX, '.freedom waterwalk on/off')),
/* FREEDOM_CMDI_WATERWALK									*/ (100010, CONCAT(@T_CMD_INFO, @C_TEXT, 'Waterwalk toggle: ', @C_LINK, '%s', @C_CLOSE)),
/* FREEDOM_CMDH_DRUNK 										*/ (100011, CONCAT(@T_CMD_SYNTAX, '.freedom drunk $drunkPercentage ($drunkPercentage must be between 0 and 100)')),
/* FREEDOM_CMDI_DRUNK										*/ (100012, CONCAT(@T_CMD_INFO, @C_TEXT, 'Drunkenness level set to ', @C_LINK, '%d%%', @C_CLOSE)),
/* FREEDOM_CMDH_MOD_SPEED									*/ (100013, CONCAT(@T_CMD_SYNTAX, '.freedom swim/fly/walk/run/speed $speedValue')),
/* FREEDOM_CMDI_MOD_SPEED 									*/ (100014, CONCAT(@T_CMD_INFO, @C_TEXT, 'Movement speed of type ', @C_LINK, '%s', @C_TEXT, ' was set to value ', @C_LINK, '%.2f', @C_CLOSE)),
/* FREEDOM_CMDE_VALUE_OUT_OF_RANGE 						*/ (100015, CONCAT(@T_CMD_ERROR, @C_TEXT, 'Given value was out of allowed range (Max: ', @C_LINK, '%.2f', @C_TEXT, ', Min: ', @C_LINK, '%.2f', @C_TEXT, ').', @C_CLOSE)),
/* FREEDOM_CMDH_FLY 											*/ (100016, CONCAT(@T_CMD_SYNTAX, '.freedom fly on/off')),
/* FREEDOM_CMDI_FLY											*/ (100017, CONCAT(@T_CMD_INFO, @C_TEXT, 'Fly toggle: ', @C_LINK, '%s', @C_CLOSE)),
/* FREEDOM_CMDI_DEMORPH 									*/ (100018, CONCAT(@T_CMD_INFO, @C_TEXT, 'You have been successfully demorphed.', @C_CLOSE)),
/* FREEDOM_CMDH_SCALE										*/ (100019, CONCAT(@T_CMD_SYNTAX, '.freedom scale $scaleValue')),
/* FREEDOM_CMDI_SCALE										*/ (100020, CONCAT(@T_CMD_INFO, @C_TEXT, 'You have been scaled to value ', @C_LINK, '%.2f', @C_CLOSE)),
/* FREEDOM_CMDI_UNAURA										*/ (100021, CONCAT(@T_CMD_INFO, @C_TEXT, 'All auras have been successfully removed from you.', @C_CLOSE)),
/* FREEDOM_CMDI_REVIVE										*/ (100022, CONCAT(@T_CMD_INFO, @C_TEXT, 'You have successfully crawled back from the dead.', @C_CLOSE)),
/* FREEDOM_CMDE_REVIVE 										*/ (100023, CONCAT(@T_CMD_ERROR, @C_TEXT, 'You are already alive.', @C_CLOSE)),
/* FREEDOM_CMDH_SUMMON										*/ (100024, CONCAT(@T_CMD_SYNTAX, '.freedom summon $playerName')),
/* FREEDOM_CMDI_SUMMON										*/ (100025, CONCAT(@T_CMD_INFO, @C_TEXT, 'Summon request sent to player ', @C_LINK, '%s', @C_CLOSE)),
/* FREEDOM_CMDE_SUMMON_NOT_FOUND							*/ (100026, CONCAT(@T_CMD_ERROR, @C_TEXT, 'Player ', @C_LINK, '%s ', @C_TEXT, 'is either offline, currently loading or does not exist.', @C_CLOSE)),
/* FREEDOM_CMDE_SUMMON_IGNORE								*/ (100027, CONCAT(@T_CMD_ERROR, @C_TEXT, 'Player ', @C_LINK, '%s ', @C_TEXT, 'is ignoring you.', @C_CLOSE)),
/* FREEDOM_CMDE_SUMMON_GM_ON								*/ (100028, CONCAT(@T_CMD_ERROR, @C_TEXT, 'Player ', @C_LINK, '%s ', @C_TEXT, 'has GM tag ON.', @C_CLOSE)),
/* FREEDOM_CMDE_SUMMON_EVIL_TWIN							*/ (100029, CONCAT(@T_CMD_ERROR, @C_TEXT, 'Player ', @C_LINK, '%s ', @C_TEXT, 'has Evil Twin aura. Summoning is not possible.', @C_CLOSE)),
/* FREEDOM_CMDI_RELOAD										*/ (100030, CONCAT(@T_GLOBAL_GM, @C_TEXT, ' Freedom DB table ', @C_LINK, '%s ', @C_TEXT, 'has been reloaded.', @C_CLOSE)),
/* FREEDOM_CMDI_RELOAD_ALL									*/ (100031, CONCAT(@T_GLOBAL_GM, @C_TEXT, ' All freedom DB tables have been reloaded.', @C_CLOSE)),

(200000, 'Dummy');
-- Select new changes
SELECT * FROM wod_world.trinity_string
WHERE entry >= 100000;