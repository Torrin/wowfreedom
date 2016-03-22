DELETE FROM trinity_string
WHERE entry >= 100000;

-- Colors and color closure
SET @C_TITLE := '|cffd2691e';
SET @C_LINK := '|cffff4500';
SET @C_TEXT := '|cffbbbbbb';
SET @C_ERR := '|cffff0000';
SET @C_CLOSE := '|r';

INSERT INTO wod_world.trinity_string (entry, content_default) VALUES
/* FREEDOM_CMD_NOT_YET_IMPLEMENTED */ 	(100000, CONCAT(@C_ERR, 'ERROR: ', @C_TEXT, 'This command isn\'t implemented yet.', @C_CLOSE)),
/* FREEDOM_CMD_MONEY_RESET */ 			(100001, CONCAT(@C_TEXT, 'Your money has been reset.', @C_CLOSE));

-- Select new changes
SELECT * FROM trinity_string
WHERE entry >= 100000;