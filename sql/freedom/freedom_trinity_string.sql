DELETE FROM wod_world.trinity_string
WHERE entry >= 100000;

-- Colors and color closure
SET @C_TITLE := '|cffd2691e';
SET @C_LINK := '|cffff4500';
SET @C_TEXT := '|cffbbbbbb';
SET @C_ERR := '|cffff0000';
SET @C_CLOSE := '|r';

-- Templates
SET @T_CMD_INFO := CONCAT(@C_TITLE, '>> ', @C_CLOSE);
SET @T_CMD_HELP := CONCAT(@C_TITLE, 'HELP: ', @C_CLOSE);
SET @T_CMD_ERROR := CONCAT(@C_ERR, 'ERROR: ', @C_CLOSE);

INSERT INTO wod_world.trinity_string (entry, content_default) VALUES
/* FREEDOM_CMDE_NOT_YET_IMPLEMENTED 					*/ (100000, CONCAT(@T_CMD_ERROR, @C_TEXT, 'This command isn\'t implemented yet.', @C_CLOSE)),
/* FREEDOM_CMDI_MONEY_RESET 								*/ (100001, CONCAT(@T_CMD_INFO, @C_TEXT, 'Your money has been reset.', @C_CLOSE)),
/* FREEDOM_CMDE_NOT_ENOUGH_PARAMS 						*/ (100002, CONCAT(@T_CMD_ERROR, @C_TEXT, 'Not enough parameters.', @C_CLOSE)),
/* FREEDOM_CMDH_GOTO_RELATIVE 							*/ (100003, CONCAT(@T_CMD_HELP, '.goto $local_x [$local_y [$local_z [$local_deg]]]')),
/* FREEDOM_E_INVALID_MAP_COORD 							*/ (100004, CONCAT(@T_CMD_ERROR, @C_TEXT, 'Target map or coordinates is invalid (X: %f Y: %f MapId: %u).', @C_CLOSE)),
/* FREEDOM_CMDI_FLAG_FOR_CUSTOMIZATION					*/ (100005, CONCAT(@T_CMD_INFO, @C_TEXT, 'Your character has been successfully flagged for re-customization.', @C_CLOSE)),
/* FREEDOM_CMDI_FLAG_FOR_RACECHANGE						*/ (100006, CONCAT(@T_CMD_INFO, @C_TEXT, 'Your character has been successfully flagged for race change.', @C_CLOSE)),
/* FREEDOM_CMDI_FLAG_FOR_FACTIONCHANGE					*/ (100007, CONCAT(@T_CMD_INFO, @C_TEXT, 'Your character has been successfully flagged for faction change.', @C_CLOSE)),
/* FREEDOM_CMDI_FIX_ITEMS									*/ (100008, CONCAT(@T_CMD_INFO, @C_TEXT, 'All your character\'s items are repaired.', @C_CLOSE)),
/* FREEDOM_CMDH_WATERWALK 									*/ (100009, CONCAT(@T_CMD_HELP, '.freedom waterwalk on/off')),
/* FREEDOM_CMDI_WATERWALK									*/ (100010, CONCAT(@T_CMD_INFO, @C_TEXT, 'Waterwalk toggle: ', @C_LINK, '%s', @C_CLOSE)),
/* FREEDOM_CMDH_DRUNK 										*/ (100011, CONCAT(@T_CMD_HELP, '.freedom drunk $drunkPercentage ($drunkPercentage must be between 0 and 100)')),
/* FREEDOM_CMDI_DRUNK										*/ (100012, CONCAT(@T_CMD_INFO, @C_TEXT, 'Drunkenness level set to ', @C_LINK, '%d%%', @C_CLOSE));

-- Select new changes
SELECT * FROM wod_world.trinity_string
WHERE entry >= 100000;