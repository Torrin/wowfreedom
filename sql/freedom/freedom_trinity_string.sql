DELETE FROM wod_world.trinity_string
WHERE entry >= 100000;

-- Colors and color closure
SET @C_TITLE := '|cffd2691e';
SET @C_LINK := '|cffff4500';
SET @C_TEXT := '|cffbbbbbb';
SET @C_ERR := '|cffff0000';
SET @C_CLOSE := '|r';

INSERT INTO wod_world.trinity_string (entry, content_default) VALUES
/* FREEDOM_CMDE_NOT_YET_IMPLEMENTED 					*/ (100000, CONCAT(@C_ERR, 'ERROR: ', @C_TEXT, 'This command isn\'t implemented yet.', @C_CLOSE)),
/* FREEDOM_CMDI_MONEY_RESET 								*/ (100001, CONCAT(@C_TEXT, 'Your money has been reset.', @C_CLOSE)),
/* FREEDOM_CMDE_NOT_ENOUGH_PARAMS 						*/ (100002, CONCAT(@C_ERR, 'ERROR: ', @C_CLOSE, 'Not enough parameters.')),
/* FREEDOM_CMDH_GOTO_RELATIVE 							*/ (100003, CONCAT(@C_TITLE, 'Command help: ', @C_CLOSE, '.goto $local_x [$local_y [$local_z [$local_deg]]]')),
/* FREEDOM_E_INVALID_MAP_COORD 							*/ (100004, CONCAT(@C_ERR, 'ERROR: ', @C_CLOSE, 'Target map or coordinates is invalid (X: %f Y: %f MapId: %u).'));

-- Select new changes
SELECT * FROM wod_world.trinity_string
WHERE entry >= 100000;