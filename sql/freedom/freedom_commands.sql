-- Reset table
DELETE FROM freedom.commands;

SET @GMLEVEL_PLAYER := 0;
SET @GMLEVEL_MODERATOR := 1;
SET @GMLEVEL_GM := 2;
SET @GMLEVEL_ADMIN := 3;

-- Copy over data from TC's wod_world.command table with some formatting
-- -- Note: Ensure that there are no multiple rbac link entries for one command
INSERT INTO freedom.commands (command, syntax, description, gmlevel)
SELECT 
a.name, 
REPLACE(REPLACE(REPLACE(SUBSTRING_INDEX(a.`help`, '\n',  1), '#', '$'), '<', '[$'), '>', ']') as `syntax`, 
REPLACE(TRIM(TRIM('\n' FROM SUBSTRING(REPLACE(a.`help`, '\r', ''), (INSTR(REPLACE(a.`help`, '\r', ''), '\n'))))), '#', '$') as `description`, 
d.secId
FROM wod_world.command a
JOIN wod_auth.rbac_linked_permissions b ON a.permission = b.linkedId
JOIN wod_auth.rbac_permissions c ON b.id = c.id
JOIN wod_auth.rbac_default_permissions d ON c.id = d.permissionId;

-- Fix some entries from copy/paste
REPLACE INTO freedom.commands (command, syntax, description, gmlevel) VALUES
('gobject add temp', 'Syntax: .gobject add temp $id [$spawntimeSecs]', 'Adds a temporary gameobject that is not saved to DB.', @GMLEVEL_MODERATOR),
('reload conditions', 'Syntax: .reload conditions', 'Reload conditions table.', @GMLEVEL_ADMIN),
('reload spell_linked_spell', 'Syntax: .reload spell_linked_spell', 'Reloads the spell_linked_spell DB table.', @GMLEVEL_ADMIN),
('server set loglevel', 'Syntax: .server set loglevel $facility $name $loglevel', '$facility can take the values: appender (a) or logger (l). $loglevel can take the values: disabled (0), trace (1), debug (2), info (3), warn (4), error (5) or fatal (6)', @GMLEVEL_ADMIN),
('reload gameobject_questender', 'Syntax: .reload gameobject_questender', 'Reload gameobject_questender table.', @GMLEVEL_ADMIN),
('reload waypoint_data', 'Syntax: .reload waypoint_data', 'Reload waypoint_data table.', @GMLEVEL_ADMIN),
('learn all recipes', 'Syntax: .learn all recipes [$profession]', 'Learns all recipes of specified profession and sets skill level to max.\nExample: .learn all recipes enchanting', @GMLEVEL_GM),
('gobject info', 'Syntax: .gobject info [$object_entry]', 'Query Gameobject information for selected gameobject or given entry.', @GMLEVEL_GM);
-- ('xxx', 'xxx', 'xxx', @GMLEVEL_GM);

-- Remove unused entries
DELETE FROM freedom.commands
WHERE command IN ('modify spell');

-- Update .modify commands
UPDATE freedom.commands a SET 
a.command = REPLACE(a.command, ' speed', ''),
a.syntax = REPLACE(a.syntax, 'modify speed', 'modify')
WHERE a.command LIKE 'modify%' AND a.command != 'modify speed';
UPDATE freedom.commands a SET
a.syntax = 'Syntax: .modify all $rate'
WHERE a.command = 'modify all';
UPDATE freedom.commands a SET
a.syntax = CONCAT('Syntax: ', a.syntax)
WHERE a.command IN ('modify fly', 'modify scale');

-- Update .goto commands
UPDATE freedom.commands a SET
a.command = REPLACE(a.command, 'go', 'goto'),
a.syntax = REPLACE(a.syntax, '.go ', '.goto '),
a.description = REPLACE(REPLACE(a.description, '.go ', '.goto '), '.help go', '.help goto')
WHERE a.command LIKE 'go %' OR a.command = 'go';

/* FREEDOM CUSTOM COMMAND HELP */
REPLACE INTO freedom.commands (command, syntax, description, gmlevel) VALUES
('goto', 'Syntax: .goto $local_x [$local_y [$local_z [$local_deg]]]', 'Teleport to coordinates relative of your character.\n$local_x is forward(+)/backward(+)\n$local_y is left(+)/right(-)\n$local_z is up(+)/down(-)\n$local_deg is turn left(+)/turn right(-)', @GMLEVEL_MODERATOR),
('goto local', 'Syntax: .goto local $local_x [$local_y [$local_z [$local_deg]]]', 'Teleport to coordinates relative of your character.\n$local_x is forward(+)/backward(+)\n$local_y is left(+)/right(-)\n$local_z is up(+)/down(-)\n$local_deg is turn left(+)/turn right(-)', @GMLEVEL_MODERATOR),
('goto relative', 'Syntax: .goto relative $local_x [$local_y [$local_z [$local_deg]]]', 'Teleport to coordinates relative of your character.\n$local_x is forward(+)/backward(+)\n$local_y is left(+)/right(-)\n$local_z is up(+)/down(-)\n$local_deg is turn left(+)/turn right(-)', @GMLEVEL_MODERATOR)
;

SELECT * FROM freedom.commands ORDER BY command;

