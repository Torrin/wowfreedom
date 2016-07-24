/********************************************************************************************************************************************************************************************
 * AUTHOR: Azeroc
 * Description: NPC & DK skin unlock script for CharSections.dbc of WoD 6.2.3. Requires table 'default_charsections' with default CharSections.dbc values
 * and table `freedom_charsections` with same structure as 'default_charsections'.
 */

/*
-- `default_charsections` / `freedom_charsections` table creation script
CREATE TABLE `default_charsections` (
	`id` INT(10) UNSIGNED NOT NULL,
	`race` INT(10) UNSIGNED NOT NULL DEFAULT '0',
	`gender` INT(10) UNSIGNED NOT NULL DEFAULT '0',
	`type` INT(10) UNSIGNED NOT NULL DEFAULT '0',
	`texture1` VARCHAR(255) NOT NULL DEFAULT '',
	`texture2` VARCHAR(255) NOT NULL DEFAULT '',
	`texture3` VARCHAR(255) NOT NULL DEFAULT '',
	`flags` INT(10) UNSIGNED NOT NULL DEFAULT '0',
	`variation_index` INT(10) UNSIGNED NOT NULL DEFAULT '0',
	`color_index` INT(10) UNSIGNED NOT NULL DEFAULT '0',
	PRIMARY KEY (`id`),
	INDEX `Index 2` (`race`),
	INDEX `Index 3` (`gender`),
	INDEX `Index 4` (`type`),
	INDEX `Index 5` (`flags`),
	INDEX `Index 6` (`variation_index`),
	INDEX `Index 7` (`color_index`)
)
COMMENT='CharSections.dbc'
COLLATE='utf8_general_ci'
ENGINE=InnoDB
;
*/

USE wod_dbc_db2;
SET SESSION sql_mode='NO_AUTO_VALUE_ON_ZERO'; -- prevent inserting auto-increment value on id = 0, because there is a valid entry from default_charsections with id = 0

-- Reset custom CharSections table by wiping it out and inserting default values
DELETE FROM freedom_charsections;
INSERT INTO freedom_charsections (id, race, gender, `type`, texture1, texture2, texture3, flags, variation_index, color_index)
SELECT id, race, gender, `type`, texture1, texture2, texture3, flags, variation_index, color_index 
FROM default_charsections a ORDER BY a.id;

-- GLOBAL VARIABLES
-- -- Genders
SET @GENDER_FEMALE := 1;
SET @GENDER_MALE := 0;

-- -- Flags
-- -- -- NOTE #1: DK flag restricts resource to only DK class
SET @FLAG_PLAYABLE := 1;
SET @FLAG_DEATH_KNIGHT := 4;
SET @FLAG_UNPLAYABLE := 8;
SET @FLAG_UNK1 := 16;

-- -- Normal skins (for goblins and worgen)
SET @TYPE_SKIN := 0;
SET @TYPE_FACE := 1;
SET @TYPE_FACIAL_HAIR := 2;
SET @TYPE_HAIR := 3;
SET @TYPE_UNDERWEAR := 4;

-- -- HD skins (vanilla + TBC races just need HD skins)
SET @TYPE_SKIN_HD := 5;
SET @TYPE_FACE_HD := 6;
SET @TYPE_FACIAL_HAIR_HD := 7;
SET @TYPE_HAIR_HD := 8;
SET @TYPE_UNDERWEAR_HD := 9;

-- Unlock DK hairstyles
UPDATE freedom_charsections a
SET a.flags = (a.flags & ~(@FLAG_DEATH_KNIGHT | @FLAG_UNPLAYABLE)) | @FLAG_UNK1 | @FLAG_PLAYABLE
WHERE a.`type` IN (@TYPE_FACIAL_HAIR, @TYPE_HAIR, @TYPE_FACIAL_HAIR_HD, @TYPE_HAIR_HD);

/********************************************************************************************************************************************************************************************
 * RACE: HUMAN (1)
 */
SET @RACE_HUMAN := 1;
SET @HUMAN_DK_VAR_INDEX_START := 15;

-- [Custom skins, excluding CHARACTER\\HUMAN\\MALE\\HUMANMALESKIN00_00_HD.BLP, because it is already defined as normal skin]
SET @HUMAN_CUSTOM_COLOR_VAR_OFFSET := 100;

-- -- Base skins
INSERT INTO freedom_charsections (race, gender, `type`, texture1, texture2, texture3, flags, variation_index, color_index)
SELECT a.race, a.gender, a.`type`, a.texture1, a.texture2, a.texture3, @FLAG_PLAYABLE | @FLAG_UNK1, a.variation_index, a.color_index + @HUMAN_CUSTOM_COLOR_VAR_OFFSET
FROM default_charsections a WHERE a.`type` = @TYPE_SKIN_HD AND a.race = @RACE_HUMAN AND a.flags = @FLAG_UNPLAYABLE AND a.texture1 != 'CHARACTER\\HUMAN\\MALE\\HUMANMALESKIN00_00_HD.BLP'
ORDER BY a.color_index, a.gender;

-- -- Face skins
INSERT INTO freedom_charsections (race, gender, `type`, texture1, texture2, texture3, flags, variation_index, color_index)
SELECT @RACE_HUMAN, b.gender, @TYPE_FACE_HD, 
'', 
if (b.gender = @GENDER_MALE,
	CONCAT('Character\\Human\\Male\\HumanMaleFaceUpper00_', SUBSTRING_INDEX(b.texture1, '_', -2)),
	CONCAT('Character\\Human\\Female\\HumanFemaleFaceUpper00_', SUBSTRING_INDEX(b.texture1, '_', -2))
),
'',
if (b.gender = @GENDER_MALE, 
	-- var_index 12-14 default male entries are all marked as DK ones, while female are just playable
	-- var_index 24-29 defaults only have female entries as DK, while male ones do not exist (mark newly, custom inserted as unplayable)
	if (a.variation_index IN (12, 13, 14), 
		@FLAG_PLAYABLE | @FLAG_DEATH_KNIGHT, 
		if (a.variation_index IN (24, 25, 26, 27, 28, 29), 
			@FLAG_UNPLAYABLE, 
			if (a.variation_index < @HUMAN_DK_VAR_INDEX_START, @FLAG_PLAYABLE, @FLAG_PLAYABLE | @FLAG_DEATH_KNIGHT)
		)
	),	
	if (a.variation_index IN (24, 25, 26, 27, 28, 29), 
		@FLAG_PLAYABLE | @FLAG_DEATH_KNIGHT, 
		if (a.variation_index < @HUMAN_DK_VAR_INDEX_START, @FLAG_PLAYABLE, @FLAG_PLAYABLE | @FLAG_DEATH_KNIGHT)
	)
), 
a.variation_index,
b.color_index + @HUMAN_CUSTOM_COLOR_VAR_OFFSET
FROM default_charsections a 
JOIN default_charsections b ON b.race = @RACE_HUMAN AND b.`type` = @TYPE_SKIN_HD AND b.flags = @FLAG_UNPLAYABLE AND b.texture1 != 'CHARACTER\\HUMAN\\MALE\\HUMANMALESKIN00_00_HD.BLP'
WHERE a.`type` = @TYPE_FACE_HD AND a.race = @RACE_HUMAN
GROUP BY a.variation_index, b.color_index, b.gender
ORDER BY a.variation_index, b.color_index, b.gender;

-- -- Underwear skins
INSERT INTO freedom_charsections (race, gender, `type`, texture1, texture2, texture3, flags, variation_index, color_index) 
SELECT @RACE_HUMAN, a.gender, @TYPE_UNDERWEAR_HD, 
if (a.gender = @GENDER_MALE,
	CONCAT('Character\\Human\\Male\\HumanMaleNakedPelvisSkin00_', SUBSTRING_INDEX(a.texture1, '_', -2)),
	CONCAT('Character\\Human\\Female\\HumanFemaleNakedPelvisSkin00_', SUBSTRING_INDEX(a.texture1, '_', -2))
),
if (a.gender = @GENDER_MALE,
	CONCAT('Character\\Human\\Male\\HumanMaleNakedTorsoSkin00_', SUBSTRING_INDEX(a.texture1, '_', -2)),
	CONCAT('Character\\Human\\Female\\HumanFemaleNakedTorsoSkin00_', SUBSTRING_INDEX(a.texture1, '_', -2))
), 
'', 
@FLAG_PLAYABLE | @FLAG_UNK1,
0,
a.color_index + @HUMAN_CUSTOM_COLOR_VAR_OFFSET
FROM default_charsections a
WHERE a.`type` = @TYPE_SKIN_HD AND a.race = @RACE_HUMAN AND a.flags = @FLAG_UNPLAYABLE AND a.texture1 != 'CHARACTER\\HUMAN\\MALE\\HUMANMALESKIN00_00_HD.BLP'
ORDER BY a.color_index;

-- [DK skin unlock]
SET @HUMAN_DK_SKIN_COLOR_VAR_OFFSET := 200;

-- -- Base skins
INSERT INTO freedom_charsections (race, gender, `type`, texture1, texture2, texture3, flags, variation_index, color_index)
SELECT a.race, a.gender, a.`type`, a.texture1, a.texture2, a.texture3, @FLAG_PLAYABLE | @FLAG_UNK1, a.variation_index, a.color_index + @HUMAN_DK_SKIN_COLOR_VAR_OFFSET
FROM default_charsections a WHERE a.`type` = @TYPE_SKIN_HD AND a.race = @RACE_HUMAN AND a.flags IN (@FLAG_PLAYABLE | @FLAG_UNK1, @FLAG_PLAYABLE | @FLAG_DEATH_KNIGHT)
ORDER BY a.color_index, a.gender;

-- -- Face skins
INSERT INTO freedom_charsections (race, gender, `type`, texture1, texture2, texture3, flags, variation_index, color_index)
SELECT @RACE_HUMAN, b.gender, @TYPE_FACE_HD, 
'', 
if (b.gender = @GENDER_MALE,
	CONCAT('Character\\Human\\Male\\HumanMaleFaceUpper', 
		LPAD(if (b.flags = @FLAG_PLAYABLE | @FLAG_UNK1, @HUMAN_DK_VAR_INDEX_START - 3, 0) + (a.variation_index % @HUMAN_DK_VAR_INDEX_START), 2, '00'), 
		'_', SUBSTRING_INDEX(b.texture1, '_', -2)
	),
	if (a.variation_index = 0 /*0 + 15 = 15*/ AND b.color_index = 8, -- character\human\female\HumanFemaleFaceUpper15_08_HD.blp isn't DK face, use next face as replacement
		'Character\\Human\\Female\\HumanFemaleFaceUpper16_08_HD.blp',
		CONCAT('Character\\Human\\Female\\HumanFemaleFaceUpper', 
			LPAD(if (b.flags = @FLAG_PLAYABLE | @FLAG_UNK1, @HUMAN_DK_VAR_INDEX_START, 0) + (a.variation_index % @HUMAN_DK_VAR_INDEX_START), 2, '00'), 
			'_', SUBSTRING_INDEX(b.texture1, '_', -2)
		)
	)
),
'',
if (b.gender = @GENDER_MALE, 
	-- var_index 12-14 default male entries are all marked as DK ones, while female are just playable
	-- var_index 24-29 defaults only have female entries as DK, while male ones do not exist (mark newly, custom inserted as unplayable)
	if (a.variation_index IN (12, 13, 14) OR a.variation_index >= @HUMAN_DK_VAR_INDEX_START, @FLAG_UNPLAYABLE, @FLAG_PLAYABLE),	
	if (a.variation_index < @HUMAN_DK_VAR_INDEX_START, @FLAG_PLAYABLE, @FLAG_UNPLAYABLE)
), 
a.variation_index,
b.color_index + @HUMAN_DK_SKIN_COLOR_VAR_OFFSET
FROM default_charsections a 
JOIN default_charsections b ON b.race = @RACE_HUMAN AND b.`type` = @TYPE_SKIN_HD AND b.flags IN (@FLAG_PLAYABLE | @FLAG_UNK1, @FLAG_PLAYABLE | @FLAG_DEATH_KNIGHT)
WHERE a.`type` = @TYPE_FACE_HD AND a.race = @RACE_HUMAN
GROUP BY a.variation_index, b.color_index, b.gender
ORDER BY a.variation_index, b.color_index, b.gender;

-- -- Underwear skins
INSERT INTO freedom_charsections (race, gender, `type`, texture1, texture2, texture3, flags, variation_index, color_index) 
SELECT @RACE_HUMAN, a.gender, @TYPE_UNDERWEAR_HD, 
if (a.gender = @GENDER_MALE,
	CONCAT('Character\\Human\\Male\\HumanMaleNakedPelvisSkin00_', SUBSTRING_INDEX(a.texture1, '_', -2)),
	CONCAT('Character\\Human\\Female\\HumanFemaleNakedPelvisSkin00_', SUBSTRING_INDEX(a.texture1, '_', -2))
),
if (a.gender = @GENDER_MALE,
	CONCAT('Character\\Human\\Male\\HumanMaleNakedTorsoSkin00_', SUBSTRING_INDEX(a.texture1, '_', -2)),
	CONCAT('Character\\Human\\Female\\HumanFemaleNakedTorsoSkin00_', SUBSTRING_INDEX(a.texture1, '_', -2))
), 
'', 
@FLAG_PLAYABLE | @FLAG_UNK1,
0,
a.color_index + @HUMAN_DK_SKIN_COLOR_VAR_OFFSET
FROM default_charsections a
WHERE a.`type` = @TYPE_SKIN_HD AND a.race = @RACE_HUMAN AND a.flags IN (@FLAG_PLAYABLE | @FLAG_UNK1, @FLAG_PLAYABLE | @FLAG_DEATH_KNIGHT)
ORDER BY a.color_index;

/********************************************************************************************************************************************************************************************
 * RACE: ORC (2)
 */
SET @RACE_ORC := 2;
SET @ORC_DK_VAR_INDEX_START := 9;

-- [Custom skins]
SET @ORC_CUSTOM_COLOR_VAR_OFFSET := 100;

-- -- Base skins
INSERT INTO freedom_charsections (race, gender, `type`, texture1, texture2, texture3, flags, variation_index, color_index)
SELECT a.race, a.gender, a.`type`, a.texture1, a.texture2, a.texture3, @FLAG_PLAYABLE | @FLAG_UNK1, a.variation_index, a.color_index + @ORC_CUSTOM_COLOR_VAR_OFFSET
FROM default_charsections a WHERE a.`type` = @TYPE_SKIN_HD AND a.race = @RACE_ORC AND a.flags = @FLAG_UNPLAYABLE
ORDER BY a.color_index, a.gender;

-- -- Face skins
INSERT INTO freedom_charsections (race, gender, `type`, texture1, texture2, texture3, flags, variation_index, color_index)
SELECT @RACE_ORC, b.gender, @TYPE_FACE_HD, 
'', 
if (b.gender = @GENDER_MALE,
	CONCAT('Character\\Orc\\Male\\OrcMaleFaceUpper', LPAD(a.variation_index % @ORC_DK_VAR_INDEX_START, 2, '00'),'_', SUBSTRING_INDEX(b.texture1, '_', -2)),
	CONCAT('Character\\Orc\\Female\\OrcFemaleFaceUpper', LPAD(a.variation_index % @ORC_DK_VAR_INDEX_START, 2, '00'),'_', SUBSTRING_INDEX(b.texture1, '_', -2))
),
'',
if (a.variation_index < @ORC_DK_VAR_INDEX_START, @FLAG_PLAYABLE, @FLAG_PLAYABLE | @FLAG_DEATH_KNIGHT), 
a.variation_index,
b.color_index + @ORC_CUSTOM_COLOR_VAR_OFFSET
FROM default_charsections a 
JOIN default_charsections b ON b.race = @RACE_ORC AND b.`type` = @TYPE_SKIN_HD AND b.flags = @FLAG_UNPLAYABLE
WHERE a.`type` = @TYPE_FACE_HD AND a.race = @RACE_ORC
GROUP BY a.variation_index, b.color_index, b.gender
ORDER BY a.variation_index, b.color_index, b.gender;

-- -- Underwear skins
INSERT INTO freedom_charsections (race, gender, `type`, texture1, texture2, texture3, flags, variation_index, color_index) 
SELECT @RACE_ORC, a.gender, @TYPE_UNDERWEAR_HD, 
if (
	a.color_index = 30, -- colors don't match, use base skin color for underwear
	'', 
	if (a.gender = @GENDER_MALE,
		CONCAT('Character\\Orc\\Male\\OrcMaleNakedPelvisSkin00_', SUBSTRING_INDEX(a.texture1, '_', -2)),
		CONCAT('Character\\Orc\\Female\\OrcFemaleNakedPelvisSkin00_', SUBSTRING_INDEX(a.texture1, '_', -2))
	)
),
if (
	a.color_index = 30, -- colors don't match, use base skin color for underwear
	'', 
	if (a.gender = @GENDER_MALE,
		CONCAT('Character\\Orc\\Male\\OrcMaleNakedTorsoSkin00_', SUBSTRING_INDEX(a.texture1, '_', -2)),
		CONCAT('Character\\Orc\\Female\\OrcFemaleNakedTorsoSkin00_', SUBSTRING_INDEX(a.texture1, '_', -2))
	)
), 
'', 
@FLAG_PLAYABLE | @FLAG_UNK1,
0,
a.color_index + @ORC_CUSTOM_COLOR_VAR_OFFSET
FROM default_charsections a
WHERE a.`type` = @TYPE_SKIN_HD AND a.race = @RACE_ORC AND a.flags = @FLAG_UNPLAYABLE
ORDER BY a.color_index;

-- [DK skin unlock]
SET @ORC_DK_SKIN_COLOR_VAR_OFFSET := 200;

-- -- Base skins
INSERT INTO freedom_charsections (race, gender, `type`, texture1, texture2, texture3, flags, variation_index, color_index)
SELECT a.race, a.gender, a.`type`, a.texture1, a.texture2, a.texture3, @FLAG_PLAYABLE | @FLAG_UNK1, a.variation_index, a.color_index + @ORC_DK_SKIN_COLOR_VAR_OFFSET
FROM default_charsections a WHERE a.`type` = @TYPE_SKIN_HD AND a.race = @RACE_ORC AND a.flags IN (@FLAG_PLAYABLE | @FLAG_UNK1, @FLAG_PLAYABLE | @FLAG_DEATH_KNIGHT)
ORDER BY a.color_index, a.gender;

-- -- Face skins
INSERT INTO freedom_charsections (race, gender, `type`, texture1, texture2, texture3, flags, variation_index, color_index)
SELECT @RACE_ORC, b.gender, @TYPE_FACE_HD, 
'', 
if (a.variation_index < @ORC_DK_VAR_INDEX_START,
	if (b.gender = @GENDER_MALE,
		CONCAT('Character\\Orc\\Male\\OrcMaleFaceUpper', 
			LPAD(if (b.flags = @FLAG_PLAYABLE | @FLAG_UNK1, @ORC_DK_VAR_INDEX_START, 0) + (a.variation_index % @ORC_DK_VAR_INDEX_START), 2, '00'), 
			'_', SUBSTRING_INDEX(b.texture1, '_', -2)
		),
		CONCAT('Character\\Orc\\Female\\OrcFemaleFaceUpper', 
			LPAD(if (b.flags = @FLAG_PLAYABLE | @FLAG_UNK1, @ORC_DK_VAR_INDEX_START, 0) + (a.variation_index % @ORC_DK_VAR_INDEX_START), 2, '00'), 
			'_', SUBSTRING_INDEX(b.texture1, '_', -2)
		)
	),
	''
),
'', 
if (a.variation_index < @ORC_DK_VAR_INDEX_START, @FLAG_PLAYABLE, @FLAG_UNPLAYABLE), 
a.variation_index,
b.color_index + @ORC_DK_SKIN_COLOR_VAR_OFFSET
FROM default_charsections a 
JOIN default_charsections b ON b.race = @RACE_ORC AND b.`type` = @TYPE_SKIN_HD AND b.flags IN (@FLAG_PLAYABLE | @FLAG_UNK1, @FLAG_PLAYABLE | @FLAG_DEATH_KNIGHT)
WHERE a.`type` = @TYPE_FACE_HD AND a.race = @RACE_ORC
GROUP BY a.variation_index, b.color_index, b.gender
ORDER BY a.variation_index, b.color_index, b.gender;

-- -- Underwear skins
INSERT INTO freedom_charsections (race, gender, `type`, texture1, texture2, texture3, flags, variation_index, color_index) 
SELECT @RACE_ORC, a.gender, @TYPE_UNDERWEAR_HD, 
if (a.gender = @GENDER_MALE,
	CONCAT('Character\\Orc\\Male\\OrcMaleNakedPelvisSkin00_', SUBSTRING_INDEX(a.texture1, '_', -2)),
	CONCAT('Character\\Orc\\Female\\OrcFemaleNakedPelvisSkin00_', SUBSTRING_INDEX(a.texture1, '_', -2))
),
if (a.gender = @GENDER_MALE,
	CONCAT('Character\\Orc\\Male\\OrcMaleNakedTorsoSkin00_', SUBSTRING_INDEX(a.texture1, '_', -2)),
	CONCAT('Character\\Orc\\Female\\OrcFemaleNakedTorsoSkin00_', SUBSTRING_INDEX(a.texture1, '_', -2))
), 
'', 
@FLAG_PLAYABLE | @FLAG_UNK1,
0,
a.color_index + @ORC_DK_SKIN_COLOR_VAR_OFFSET 
FROM default_charsections a
WHERE a.`type` = @TYPE_SKIN_HD AND a.race = @RACE_ORC AND a.flags IN (@FLAG_PLAYABLE | @FLAG_DEATH_KNIGHT, @FLAG_PLAYABLE | @FLAG_UNK1)
ORDER BY a.color_index;

/********************************************************************************************************************************************************************************************
 * RACE: DWARF (3)
 */
SET @RACE_DWARF := 3;
SET @DWARF_DK_VAR_INDEX_START := 10;

-- [Custom skins]
SET @DWARF_CUSTOM_COLOR_VAR_OFFSET := 100;

-- -- Base skins
INSERT INTO freedom_charsections (race, gender, `type`, texture1, texture2, texture3, flags, variation_index, color_index)
SELECT a.race, a.gender, a.`type`, a.texture1, a.texture2, a.texture3, @FLAG_PLAYABLE | @FLAG_UNK1, a.variation_index, a.color_index + @DWARF_CUSTOM_COLOR_VAR_OFFSET
FROM default_charsections a WHERE a.`type` = @TYPE_SKIN_HD AND a.race = @RACE_DWARF AND a.flags = @FLAG_UNPLAYABLE
ORDER BY a.color_index, a.gender;

-- -- Face skins
INSERT INTO freedom_charsections (race, gender, `type`, texture1, texture2, texture3, flags, variation_index, color_index)
SELECT @RACE_DWARF, b.gender, @TYPE_FACE_HD, 
'', 
if (b.gender = @GENDER_MALE,
	CONCAT('Character\\Dwarf\\Male\\DwarfMaleFaceUpper', LPAD(a.variation_index, 2, '00'), '_', SUBSTRING_INDEX(b.texture1, '_', -2)),
	CONCAT('Character\\Dwarf\\Female\\DwarfFemaleFaceUpper', LPAD(a.variation_index, 2, '00'), '_', SUBSTRING_INDEX(b.texture1, '_', -2))
),
'', 
if (a.variation_index < @DWARF_DK_VAR_INDEX_START, @FLAG_PLAYABLE, @FLAG_PLAYABLE | @FLAG_DEATH_KNIGHT), 
a.variation_index,
b.color_index + @DWARF_CUSTOM_COLOR_VAR_OFFSET
FROM default_charsections a 
JOIN default_charsections b ON b.race = @RACE_DWARF AND b.`type` = @TYPE_SKIN_HD AND b.flags = @FLAG_UNPLAYABLE
WHERE a.`type` = @TYPE_FACE_HD AND a.race = @RACE_DWARF
GROUP BY a.variation_index, b.color_index, b.gender
ORDER BY a.variation_index, b.color_index, b.gender;

-- -- Underwear skins
INSERT INTO freedom_charsections (race, gender, `type`, texture1, texture2, texture3, flags, variation_index, color_index)
SELECT @RACE_DWARF, a.gender, @TYPE_UNDERWEAR_HD, 
if (a.gender = @GENDER_MALE, -- texture 1
	CONCAT('Character\\Dwarf\\Male\\DwarfMaleNakedPelvisSkin00_', SUBSTRING_INDEX(a.texture1, '_', -2)), 
	CONCAT('Character\\Dwarf\\Female\\DwarfFemaleNakedPelvisSkin00_', SUBSTRING_INDEX(a.texture1, '_', -2))
), 
if (a.gender = @GENDER_MALE, -- texture 2
	CONCAT('Character\\Dwarf\\Male\\DwarfMaleNakedTorsoSkin00_', SUBSTRING_INDEX(a.texture1, '_', -2)), 
	CONCAT('Character\\Dwarf\\Female\\DwarfFemaleNakedTorsoSkin00_', SUBSTRING_INDEX(a.texture1, '_', -2))
),  
'', 
@FLAG_PLAYABLE | @FLAG_UNK1, 
a.variation_index, 
a.color_index + @DWARF_CUSTOM_COLOR_VAR_OFFSET
FROM default_charsections a WHERE a.`type` = @TYPE_SKIN_HD AND a.race = @RACE_DWARF AND a.flags = @FLAG_UNPLAYABLE
ORDER BY a.color_index, a.gender;

-- [DK skin unlock]
SET @DWARF_DK_SKIN_COLOR_VAR_OFFSET := 200;

-- -- Base skins
INSERT INTO freedom_charsections (race, gender, `type`, texture1, texture2, texture3, flags, variation_index, color_index)
SELECT a.race, a.gender, a.`type`, a.texture1, a.texture2, a.texture3, @FLAG_PLAYABLE | @FLAG_UNK1, a.variation_index, a.color_index + @DWARF_DK_SKIN_COLOR_VAR_OFFSET
FROM default_charsections a WHERE a.`type` = @TYPE_SKIN_HD AND a.race = @RACE_DWARF AND a.flags IN (@FLAG_PLAYABLE | @FLAG_DEATH_KNIGHT, @FLAG_PLAYABLE | @FLAG_UNK1)
ORDER BY a.color_index, a.gender;

-- -- Face skins
INSERT INTO freedom_charsections (race, gender, `type`, texture1, texture2, texture3, flags, variation_index, color_index)
SELECT @RACE_DWARF, b.gender, @TYPE_FACE_HD, 
'', 
if (a.variation_index < @DWARF_DK_VAR_INDEX_START,
	if (b.gender = @GENDER_MALE,
		CONCAT('Character\\Dwarf\\Male\\DwarfMaleFaceUpper', 
			LPAD(if (b.flags = @FLAG_PLAYABLE | @FLAG_UNK1, @DWARF_DK_VAR_INDEX_START, 0) + (a.variation_index % @DWARF_DK_VAR_INDEX_START), 2, '00'), 
			'_', SUBSTRING_INDEX(b.texture1, '_', -2)
		),
		CONCAT('Character\\Dwarf\\Female\\DwarfFemaleFaceUpper', 
			LPAD(if (b.flags = @FLAG_PLAYABLE | @FLAG_UNK1, @DWARF_DK_VAR_INDEX_START, 0) + (a.variation_index % @DWARF_DK_VAR_INDEX_START), 2, '00'), 
			'_', SUBSTRING_INDEX(b.texture1, '_', -2)
		)
	),
	''
),
'', 
if (a.variation_index < @DWARF_DK_VAR_INDEX_START, @FLAG_PLAYABLE, @FLAG_UNPLAYABLE), 
a.variation_index,
b.color_index + @DWARF_DK_SKIN_COLOR_VAR_OFFSET
FROM default_charsections a 
JOIN default_charsections b ON b.race = @RACE_DWARF AND b.`type` = @TYPE_SKIN_HD AND b.flags IN (@FLAG_PLAYABLE | @FLAG_UNK1, @FLAG_PLAYABLE | @FLAG_DEATH_KNIGHT)
WHERE a.`type` = @TYPE_FACE_HD AND a.race = @RACE_DWARF
GROUP BY a.variation_index, b.color_index, b.gender
ORDER BY a.variation_index, b.color_index, b.gender;

-- -- Underwear skins
INSERT INTO freedom_charsections (race, gender, `type`, texture1, texture2, texture3, flags, variation_index, color_index) 
SELECT @RACE_DWARF, a.gender, @TYPE_UNDERWEAR_HD, 
if (a.gender = @GENDER_MALE,
	CONCAT('Character\\Dwarf\\Male\\DwarfMaleNakedPelvisSkin00_', SUBSTRING_INDEX(a.texture1, '_', -2)),
	CONCAT('Character\\Dwarf\\Female\\DwarfFemaleNakedPelvisSkin00_', SUBSTRING_INDEX(a.texture1, '_', -2))
),
if (a.gender = @GENDER_MALE,
	CONCAT('Character\\Dwarf\\Male\\DwarfMaleNakedTorsoSkin00_', SUBSTRING_INDEX(a.texture1, '_', -2)),
	CONCAT('Character\\Dwarf\\Female\\DwarfFemaleNakedTorsoSkin00_', SUBSTRING_INDEX(a.texture1, '_', -2))
), 
'', 
@FLAG_PLAYABLE | @FLAG_UNK1,
0,
a.color_index + @DWARF_DK_SKIN_COLOR_VAR_OFFSET
FROM default_charsections a
WHERE a.`type` = @TYPE_SKIN_HD AND a.race = @RACE_DWARF AND a.flags IN (@FLAG_PLAYABLE | @FLAG_DEATH_KNIGHT, @FLAG_PLAYABLE | @FLAG_UNK1)
ORDER BY a.color_index;

/********************************************************************************************************************************************************************************************
 * RACE: NIGHT ELF (4)
 */
SET @RACE_NELF := 4;
SET @NELF_DK_VAR_INDEX_START := 9;

-- [Custom skins]
SET @NELF_CUSTOM_COLOR_VAR_OFFSET := 100;

-- -- Base skins
INSERT INTO freedom_charsections (race, gender, `type`, texture1, texture2, texture3, flags, variation_index, color_index)
SELECT a.race, a.gender, a.`type`, a.texture1, a.texture2, a.texture3, @FLAG_PLAYABLE | @FLAG_UNK1, a.variation_index, a.color_index + @NELF_CUSTOM_COLOR_VAR_OFFSET
FROM default_charsections a WHERE a.`type` = @TYPE_SKIN_HD AND a.race = @RACE_NELF AND a.flags = @FLAG_UNPLAYABLE
ORDER BY a.color_index, a.gender;

-- -- Face skins
INSERT INTO freedom_charsections (race, gender, `type`, texture1, texture2, texture3, flags, variation_index, color_index)
SELECT @RACE_NELF, b.gender, @TYPE_FACE_HD, 
'', 
if (b.gender = @GENDER_MALE,
	CONCAT('Character\\NightElf\\Male\\NightElfMaleFaceUpper', LPAD(a.variation_index, 2, '00'), '_', SUBSTRING_INDEX(b.texture1, '_', -2)),
	CONCAT('Character\\NightElf\\Female\\NightElf_FemaleFaceUpper', LPAD(a.variation_index, 2, '00'), '_', SUBSTRING_INDEX(b.texture1, '_', -2))
),
'', 
if (a.variation_index < @NELF_DK_VAR_INDEX_START, @FLAG_PLAYABLE, @FLAG_PLAYABLE | @FLAG_DEATH_KNIGHT), 
a.variation_index,
b.color_index + @NELF_CUSTOM_COLOR_VAR_OFFSET
FROM default_charsections a 
JOIN default_charsections b ON b.race = @RACE_NELF AND b.`type` = @TYPE_SKIN_HD AND b.flags = @FLAG_UNPLAYABLE
WHERE a.`type` = @TYPE_FACE_HD AND a.race = @RACE_NELF
GROUP BY a.variation_index, b.color_index, b.gender
ORDER BY a.variation_index, b.color_index, b.gender;

-- -- Underwear skins
INSERT INTO freedom_charsections (race, gender, `type`, texture1, texture2, texture3, flags, variation_index, color_index)
SELECT @RACE_NELF, a.gender, @TYPE_UNDERWEAR_HD, 
if (a.gender = @GENDER_MALE, -- texture 1
	CONCAT('Character\\NightElf\\Male\\NightElfMaleNakedPelvisSkin00_', SUBSTRING_INDEX(a.texture1, '_', -2)), 
	CONCAT('Character\\NightElf\\Female\\NightElf_FemaleNakedPelvisSkin00_', SUBSTRING_INDEX(a.texture1, '_', -2))
), 
if (a.gender = @GENDER_MALE, -- texture 2
	CONCAT('Character\\NightElf\\Male\\NightElfMaleNakedTorsoSkin00_', SUBSTRING_INDEX(a.texture1, '_', -2)), 
	CONCAT('Character\\NightElf\\Female\\NightElf_FemaleNakedTorsoSkin00_', SUBSTRING_INDEX(a.texture1, '_', -2))
),  
'', 
@FLAG_PLAYABLE | @FLAG_UNK1, 
a.variation_index, 
a.color_index + @NELF_CUSTOM_COLOR_VAR_OFFSET
FROM default_charsections a WHERE a.`type` = @TYPE_SKIN_HD AND a.race = @RACE_NELF AND a.flags = @FLAG_UNPLAYABLE
ORDER BY a.color_index, a.gender;

-- [DK skin unlock]
SET @NELF_DK_COLOR_VAR_OFFSET := 200;

-- -- Base skins
INSERT INTO freedom_charsections (race, gender, `type`, texture1, texture2, texture3, flags, variation_index, color_index)
SELECT a.race, a.gender, a.`type`, a.texture1, a.texture2, a.texture3, @FLAG_PLAYABLE | @FLAG_UNK1, a.variation_index, a.color_index + @NELF_DK_COLOR_VAR_OFFSET
FROM default_charsections a WHERE a.`type` = @TYPE_SKIN_HD AND a.race = @RACE_NELF AND a.flags IN (@FLAG_PLAYABLE | @FLAG_DEATH_KNIGHT, @FLAG_PLAYABLE | @FLAG_UNK1)
ORDER BY a.color_index, a.gender;

-- -- Face skins
INSERT INTO freedom_charsections (race, gender, `type`, texture1, texture2, texture3, flags, variation_index, color_index)
SELECT @RACE_NELF, b.gender, @TYPE_FACE_HD, 
'', 
if (a.variation_index < @NELF_DK_VAR_INDEX_START,
	if (b.gender = @GENDER_MALE,
		CONCAT('Character\\NightElf\\Male\\NightElfMaleFaceUpper', 
			LPAD(if (b.flags = @FLAG_PLAYABLE | @FLAG_UNK1, @NELF_DK_VAR_INDEX_START, 0) + (a.variation_index % @NELF_DK_VAR_INDEX_START), 2, '00'), 
			'_', SUBSTRING_INDEX(b.texture1, '_', -2)
		),
		CONCAT('Character\\NightElf\\Female\\NightElf_FemaleFaceUpper', 
			LPAD(if (b.flags = @FLAG_PLAYABLE | @FLAG_UNK1, @NELF_DK_VAR_INDEX_START, 0) + (a.variation_index % @NELF_DK_VAR_INDEX_START), 2, '00'), 
			'_', SUBSTRING_INDEX(b.texture1, '_', -2)
		)
	),
	''
),
'', 
if (a.variation_index < @NELF_DK_VAR_INDEX_START, @FLAG_PLAYABLE, @FLAG_UNPLAYABLE), 
a.variation_index,
b.color_index + @NELF_DK_COLOR_VAR_OFFSET
FROM default_charsections a 
JOIN default_charsections b ON b.race = @RACE_NELF AND b.`type` = @TYPE_SKIN_HD AND b.flags IN (@FLAG_PLAYABLE | @FLAG_UNK1, @FLAG_PLAYABLE | @FLAG_DEATH_KNIGHT)
WHERE a.`type` = @TYPE_FACE_HD AND a.race = @RACE_NELF
GROUP BY a.variation_index, b.color_index, b.gender
ORDER BY a.variation_index, b.color_index, b.gender;

-- -- Underwear skins
INSERT INTO freedom_charsections (race, gender, `type`, texture1, texture2, texture3, flags, variation_index, color_index) 
SELECT @RACE_NELF, a.gender, @TYPE_UNDERWEAR_HD, 
if (a.gender = @GENDER_MALE,
	CONCAT('Character\\NightElf\\Male\\NightElfMaleNakedPelvisSkin00_', SUBSTRING_INDEX(a.texture1, '_', -2)),
	CONCAT('Character\\NightElf\\Female\\NightElf_FemaleNakedPelvisSkin00_', SUBSTRING_INDEX(a.texture1, '_', -2))
),
if (a.gender = @GENDER_MALE,
	CONCAT('Character\\NightElf\\Male\\NightElfMaleNakedTorsoSkin00_', SUBSTRING_INDEX(a.texture1, '_', -2)),
	CONCAT('Character\\NightElf\\Female\\NightElf_FemaleNakedTorsoSkin00_', SUBSTRING_INDEX(a.texture1, '_', -2))
), 
'', 
@FLAG_PLAYABLE | @FLAG_UNK1,
0,
a.color_index + @NELF_DK_COLOR_VAR_OFFSET
FROM default_charsections a
WHERE a.`type` = @TYPE_SKIN_HD AND a.race = @RACE_NELF AND a.flags IN (@FLAG_PLAYABLE | @FLAG_DEATH_KNIGHT, @FLAG_PLAYABLE | @FLAG_UNK1)
ORDER BY a.color_index;

/********************************************************************************************************************************************************************************************
 * RACE: FORSAKEN (5)
 */
SET @RACE_FORSAKEN := 5;
SET @FORSAKEN_DK_VAR_INDEX_START := 10;

-- [Custom skins]
SET @FORSAKEN_CUSTOM_COLOR_VAR_OFFSET := 100;
-- -- Forsaken do not have any NPC skins (@FLAG_UNPLAYABLE)

-- [DK skin unlock]
SET @FORSAKEN_DK_COLOR_VAR_OFFSET := 200;

-- -- Base skins
INSERT INTO freedom_charsections (race, gender, `type`, texture1, texture2, texture3, flags, variation_index, color_index)
SELECT a.race, a.gender, a.`type`, a.texture1, a.texture2, a.texture3, @FLAG_PLAYABLE | @FLAG_UNK1, a.variation_index, a.color_index + @FORSAKEN_DK_COLOR_VAR_OFFSET
FROM default_charsections a WHERE a.`type` = @TYPE_SKIN_HD AND a.race = @RACE_FORSAKEN AND a.flags IN (@FLAG_PLAYABLE | @FLAG_DEATH_KNIGHT, @FLAG_PLAYABLE | @FLAG_UNK1)
ORDER BY a.color_index, a.gender;

-- -- Face skins
INSERT INTO freedom_charsections (race, gender, `type`, texture1, texture2, texture3, flags, variation_index, color_index)
SELECT @RACE_FORSAKEN, b.gender, @TYPE_FACE_HD, 
'', 
if (a.variation_index < @FORSAKEN_DK_VAR_INDEX_START,
	if (b.gender = @GENDER_MALE,
		CONCAT('Character\\Scourge\\Male\\ScourgeMaleFaceUpper', 
			LPAD(if (b.flags = @FLAG_PLAYABLE | @FLAG_UNK1, @FORSAKEN_DK_VAR_INDEX_START, 0) + (a.variation_index % @FORSAKEN_DK_VAR_INDEX_START), 2, '00'), 
			'_', SUBSTRING_INDEX(b.texture1, '_', -2)
		),
		CONCAT('Character\\Scourge\\Female\\ScourgeFemaleFaceUpper', 
			LPAD(if (b.flags = @FLAG_PLAYABLE | @FLAG_UNK1, @FORSAKEN_DK_VAR_INDEX_START, 0) + (a.variation_index % @FORSAKEN_DK_VAR_INDEX_START), 2, '00'), 
			'_', SUBSTRING_INDEX(b.texture1, '_', -2)
		)
	),
	''
),
'', 
if (a.variation_index < @FORSAKEN_DK_VAR_INDEX_START, @FLAG_PLAYABLE, @FLAG_UNPLAYABLE), 
a.variation_index,
b.color_index + @FORSAKEN_DK_COLOR_VAR_OFFSET
FROM default_charsections a 
JOIN default_charsections b ON b.race = @RACE_FORSAKEN AND b.`type` = @TYPE_SKIN_HD AND b.flags IN (@FLAG_PLAYABLE | @FLAG_UNK1, @FLAG_PLAYABLE | @FLAG_DEATH_KNIGHT)
WHERE a.`type` = @TYPE_FACE_HD AND a.race = @RACE_FORSAKEN
GROUP BY a.variation_index, b.color_index, b.gender
ORDER BY a.variation_index, b.color_index, b.gender;

-- -- Underwear skins
INSERT INTO freedom_charsections (race, gender, `type`, texture1, texture2, texture3, flags, variation_index, color_index) 
SELECT @RACE_FORSAKEN, a.gender, @TYPE_UNDERWEAR_HD, 
if (a.gender = @GENDER_MALE,
	CONCAT('Character\\Scourge\\Male\\ScourgeMaleNakedPelvisSkin00_', SUBSTRING_INDEX(a.texture1, '_', -2)),
	CONCAT('Character\\Scourge\\Female\\ScourgeFemaleNakedPelvisSkin00_', SUBSTRING_INDEX(a.texture1, '_', -2))
),
if (a.gender = @GENDER_MALE,
	CONCAT('Character\\Scourge\\Male\\ScourgeMaleNakedTorsoSkin00_', SUBSTRING_INDEX(a.texture1, '_', -2)),
	CONCAT('Character\\Scourge\\Female\\ScourgeFemaleNakedTorsoSkin00_', SUBSTRING_INDEX(a.texture1, '_', -2))
), 
'', 
@FLAG_PLAYABLE | @FLAG_UNK1,
0,
a.color_index + @FORSAKEN_DK_COLOR_VAR_OFFSET
FROM default_charsections a
WHERE a.`type` = @TYPE_SKIN_HD AND a.race = @RACE_FORSAKEN AND a.flags IN (@FLAG_PLAYABLE | @FLAG_DEATH_KNIGHT, @FLAG_PLAYABLE | @FLAG_UNK1)
ORDER BY a.color_index;

/********************************************************************************************************************************************************************************************
 * RACE: TAUREN (6)
 */
SET @RACE_TAUREN := 6;
SET @TAUREN_DK_VAR_INDEX_START := 5;

-- [Custom skins]
SET @TAUREN_CUSTOM_COLOR_VAR_OFFSET := 100;

-- -- Base skins
INSERT INTO freedom_charsections (race, gender, `type`, texture1, texture2, texture3, flags, variation_index, color_index)
SELECT a.race, a.gender, a.`type`, a.texture1, a.texture2, a.texture3, @FLAG_PLAYABLE | @FLAG_UNK1, a.variation_index, a.color_index + @TAUREN_CUSTOM_COLOR_VAR_OFFSET
FROM default_charsections a WHERE a.`type` = @TYPE_SKIN_HD AND a.race = @RACE_TAUREN AND a.flags = @FLAG_UNPLAYABLE
ORDER BY a.color_index, a.gender;

-- -- Face skins
INSERT INTO freedom_charsections (race, gender, `type`, texture1, texture2, texture3, flags, variation_index, color_index)
SELECT @RACE_TAUREN, b.gender, @TYPE_FACE_HD, 
'', 
if (b.gender = @GENDER_MALE,
	CONCAT('Character\\Tauren\\Male\\TaurenMaleFaceUpper', LPAD(a.variation_index, 2, '00'), '_', SUBSTRING_INDEX(b.texture1, '_', -2)),
	CONCAT('Character\\Tauren\\Female\\TaurenFemaleFaceUpper', LPAD(a.variation_index, 2, '00'), '_', SUBSTRING_INDEX(b.texture1, '_', -2))
),
'', 
if (a.variation_index < @TAUREN_DK_VAR_INDEX_START, @FLAG_PLAYABLE, @FLAG_PLAYABLE | @FLAG_DEATH_KNIGHT), 
a.variation_index,
b.color_index + @TAUREN_CUSTOM_COLOR_VAR_OFFSET
FROM default_charsections a 
JOIN default_charsections b ON b.race = @RACE_TAUREN AND b.`type` = @TYPE_SKIN_HD AND b.flags = @FLAG_UNPLAYABLE
WHERE a.`type` = @TYPE_FACE_HD AND a.race = @RACE_TAUREN
GROUP BY a.variation_index, b.color_index, b.gender
ORDER BY a.variation_index, b.color_index, b.gender;

-- -- Underwear skins
INSERT INTO freedom_charsections (race, gender, `type`, texture1, texture2, texture3, flags, variation_index, color_index)
SELECT @RACE_TAUREN, a.gender, @TYPE_UNDERWEAR_HD, 
if (a.gender = @GENDER_MALE, -- texture 1
	CONCAT('Character\\Tauren\\Male\\TaurenMaleNakedPelvisSkin00_', SUBSTRING_INDEX(a.texture1, '_', -2)), 
	CONCAT('Character\\Tauren\\Female\\TaurenFemaleNakedPelvisSkin00_', SUBSTRING_INDEX(a.texture1, '_', -2))
), 
if (a.gender = @GENDER_MALE, -- texture 2
	CONCAT('Character\\Tauren\\Male\\TaurenMaleNakedTorsoSkin00_', SUBSTRING_INDEX(a.texture1, '_', -2)), 
	CONCAT('Character\\Tauren\\Female\\TaurenFemaleNakedTorsoSkin00_', SUBSTRING_INDEX(a.texture1, '_', -2))
),  
'', 
@FLAG_PLAYABLE | @FLAG_UNK1, 
a.variation_index, 
a.color_index + @TAUREN_CUSTOM_COLOR_VAR_OFFSET
FROM default_charsections a WHERE a.`type` = @TYPE_SKIN_HD AND a.race = @RACE_TAUREN AND a.flags = @FLAG_UNPLAYABLE
ORDER BY a.color_index, a.gender;

-- [DK skin unlock]
SET @TAUREN_DK_COLOR_VAR_OFFSET := 200;

-- -- Base skins
INSERT INTO freedom_charsections (race, gender, `type`, texture1, texture2, texture3, flags, variation_index, color_index)
SELECT a.race, a.gender, a.`type`, a.texture1, a.texture2, a.texture3, @FLAG_PLAYABLE | @FLAG_UNK1, a.variation_index, a.color_index + @TAUREN_DK_COLOR_VAR_OFFSET
FROM default_charsections a WHERE a.`type` = @TYPE_SKIN_HD AND a.race = @RACE_TAUREN AND a.flags IN (@FLAG_PLAYABLE | @FLAG_DEATH_KNIGHT, @FLAG_PLAYABLE | @FLAG_UNK1)
ORDER BY a.color_index, a.gender;

-- -- Face skins
INSERT INTO freedom_charsections (race, gender, `type`, texture1, texture2, texture3, flags, variation_index, color_index)
SELECT @RACE_TAUREN, b.gender, @TYPE_FACE_HD, 
'', 
if (a.variation_index < @TAUREN_DK_VAR_INDEX_START,
	if (b.gender = @GENDER_MALE,
		if (a.variation_index = 3 AND b.color_index = 6, -- TaurenMaleFaceUpper08_06_HD.blp isn't DK face, replace it with previous DK variation face (07 from 08)
			'Character\\Tauren\\Male\\TaurenMaleFaceUpper07_06_HD.blp',
			CONCAT('Character\\Tauren\\Male\\TaurenMaleFaceUpper', 
				LPAD(if (b.flags = @FLAG_PLAYABLE | @FLAG_UNK1, @TAUREN_DK_VAR_INDEX_START, 0) + (a.variation_index % @TAUREN_DK_VAR_INDEX_START), 2, '00'), 
				'_', SUBSTRING_INDEX(b.texture1, '_', -2)
			)
		),
		if (a.variation_index IN (3, 4), -- female tauren dont have DK faces for variation 3 + 5, 4 + 5, use previous face
			CONCAT('Character\\Tauren\\Female\\TaurenFemaleFaceUpper07_', SUBSTRING_INDEX(b.texture1, '_', -2)),
			CONCAT('Character\\Tauren\\Female\\TaurenFemaleFaceUpper', 
				LPAD(if (b.flags = @FLAG_PLAYABLE | @FLAG_UNK1, @TAUREN_DK_VAR_INDEX_START, 0) + (a.variation_index % @TAUREN_DK_VAR_INDEX_START), 2, '00'), 
				'_', SUBSTRING_INDEX(b.texture1, '_', -2)
			)
		)
	),
	''
),
'', 
if (a.variation_index < @TAUREN_DK_VAR_INDEX_START, @FLAG_PLAYABLE, @FLAG_UNPLAYABLE), 
a.variation_index,
b.color_index + @TAUREN_DK_COLOR_VAR_OFFSET
FROM default_charsections a 
JOIN default_charsections b ON b.race = @RACE_TAUREN AND b.`type` = @TYPE_SKIN_HD AND b.flags IN (@FLAG_PLAYABLE | @FLAG_UNK1, @FLAG_PLAYABLE | @FLAG_DEATH_KNIGHT)
WHERE a.`type` = @TYPE_FACE_HD AND a.race = @RACE_TAUREN
GROUP BY a.variation_index, b.color_index, b.gender
ORDER BY a.variation_index, b.color_index, b.gender;

-- -- Underwear skins
INSERT INTO freedom_charsections (race, gender, `type`, texture1, texture2, texture3, flags, variation_index, color_index) 
SELECT @RACE_TAUREN, a.gender, @TYPE_UNDERWEAR_HD, 
if (a.gender = @GENDER_MALE,
	CONCAT('Character\\Tauren\\Male\\TaurenMaleNakedPelvisSkin00_', SUBSTRING_INDEX(a.texture1, '_', -2)),
	CONCAT('Character\\Tauren\\Female\\TaurenFemaleNakedPelvisSkin00_', SUBSTRING_INDEX(a.texture1, '_', -2))
),
if (a.gender = @GENDER_MALE,
	CONCAT('Character\\Tauren\\Male\\TaurenMaleNakedTorsoSkin00_', SUBSTRING_INDEX(a.texture1, '_', -2)),
	CONCAT('Character\\Tauren\\Female\\TaurenFemaleNakedTorsoSkin00_', SUBSTRING_INDEX(a.texture1, '_', -2))
), 
'', 
@FLAG_PLAYABLE | @FLAG_UNK1,
0,
a.color_index + @TAUREN_DK_COLOR_VAR_OFFSET
FROM default_charsections a
WHERE a.`type` = @TYPE_SKIN_HD AND a.race = @RACE_TAUREN AND a.flags IN (@FLAG_PLAYABLE | @FLAG_DEATH_KNIGHT, @FLAG_PLAYABLE | @FLAG_UNK1)
ORDER BY a.color_index;

/********************************************************************************************************************************************************************************************
 * RACE: GNOME (7)
 */
 
SET @RACE_GNOME := 7;
SET @GNOME_DK_VAR_INDEX_START := 7;

-- [Custom skins]
SET @GNOME_CUSTOM_COLOR_VAR_OFFSET := 100;

-- -- Base skins
INSERT INTO freedom_charsections (race, gender, `type`, texture1, texture2, texture3, flags, variation_index, color_index)
SELECT a.race, a.gender, a.`type`, a.texture1, a.texture2, a.texture3, @FLAG_PLAYABLE | @FLAG_UNK1, a.variation_index, a.color_index + @GNOME_CUSTOM_COLOR_VAR_OFFSET
FROM default_charsections a WHERE a.`type` = @TYPE_SKIN_HD AND a.race = @RACE_GNOME AND a.flags = @FLAG_UNPLAYABLE
ORDER BY a.color_index, a.gender;

-- -- Face skins
INSERT INTO freedom_charsections (race, gender, `type`, texture1, texture2, texture3, flags, variation_index, color_index)
SELECT @RACE_GNOME, b.gender, @TYPE_FACE_HD, 
'', 
if (b.gender = @GENDER_MALE,
	CONCAT('Character\\Gnome\\Male\\GnomeMaleFaceUpper', LPAD(a.variation_index, 2, '00'), '_', SUBSTRING_INDEX(b.texture1, '_', -2)),
	CONCAT('Character\\Gnome\\Female\\GnomeFemaleFaceUpper', LPAD(a.variation_index, 2, '00'), '_', SUBSTRING_INDEX(b.texture1, '_', -2))
),
'', 
if (a.variation_index < @GNOME_DK_VAR_INDEX_START, @FLAG_PLAYABLE, @FLAG_PLAYABLE | @FLAG_DEATH_KNIGHT), 
a.variation_index,
b.color_index + @GNOME_CUSTOM_COLOR_VAR_OFFSET
FROM default_charsections a 
JOIN default_charsections b ON b.race = @RACE_GNOME AND b.`type` = @TYPE_SKIN_HD AND b.flags = @FLAG_UNPLAYABLE
WHERE a.`type` = @TYPE_FACE_HD AND a.race = @RACE_GNOME
GROUP BY a.variation_index, b.color_index, b.gender
ORDER BY a.variation_index, b.color_index, b.gender;

-- -- Underwear skins
INSERT INTO freedom_charsections (race, gender, `type`, texture1, texture2, texture3, flags, variation_index, color_index)
SELECT @RACE_GNOME, a.gender, @TYPE_UNDERWEAR_HD, 
if (a.gender = @GENDER_MALE, -- texture 1
	CONCAT('Character\\Gnome\\Male\\GnomeMaleNakedPelvisSkin00_', SUBSTRING_INDEX(a.texture1, '_', -2)), 
	CONCAT('Character\\Gnome\\Female\\GnomeFemaleNakedPelvisSkin00_', SUBSTRING_INDEX(a.texture1, '_', -2))
), 
if (a.gender = @GENDER_MALE, -- texture 2
	CONCAT('Character\\Gnome\\Male\\GnomeMaleNakedTorsoSkin00_', SUBSTRING_INDEX(a.texture1, '_', -2)), 
	CONCAT('Character\\Gnome\\Female\\GnomeFemaleNakedTorsoSkin00_', SUBSTRING_INDEX(a.texture1, '_', -2))
),  
'', 
@FLAG_PLAYABLE | @FLAG_UNK1, 
a.variation_index, 
a.color_index + @GNOME_CUSTOM_COLOR_VAR_OFFSET
FROM default_charsections a WHERE a.`type` = @TYPE_SKIN_HD AND a.race = @RACE_GNOME AND a.flags = @FLAG_UNPLAYABLE
ORDER BY a.color_index, a.gender;

-- [DK skin unlock]
SET @GNOME_DK_COLOR_VAR_OFFSET := 200;

-- -- Base skins
INSERT INTO freedom_charsections (race, gender, `type`, texture1, texture2, texture3, flags, variation_index, color_index)
SELECT a.race, a.gender, a.`type`, a.texture1, a.texture2, a.texture3, @FLAG_PLAYABLE | @FLAG_UNK1, a.variation_index, a.color_index + @GNOME_DK_COLOR_VAR_OFFSET
FROM default_charsections a WHERE a.`type` = @TYPE_SKIN_HD AND a.race = @RACE_GNOME AND a.flags IN (@FLAG_PLAYABLE | @FLAG_DEATH_KNIGHT, @FLAG_PLAYABLE | @FLAG_UNK1)
ORDER BY a.color_index, a.gender;

-- -- Face skins
INSERT INTO freedom_charsections (race, gender, `type`, texture1, texture2, texture3, flags, variation_index, color_index)
SELECT @RACE_GNOME, b.gender, @TYPE_FACE_HD, 
'', 
if (a.variation_index < @GNOME_DK_VAR_INDEX_START,
	if (b.gender = @GENDER_MALE,
		CONCAT('Character\\Gnome\\Male\\GnomeMaleFaceUpper', 
			LPAD(if (b.flags = @FLAG_PLAYABLE | @FLAG_UNK1, @GNOME_DK_VAR_INDEX_START, 0) + (a.variation_index % @GNOME_DK_VAR_INDEX_START), 2, '00'), 
			'_', SUBSTRING_INDEX(b.texture1, '_', -2)
		),
		CONCAT('Character\\Gnome\\Female\\GnomeFemaleFaceUpper', 
			LPAD(if (b.flags = @FLAG_PLAYABLE | @FLAG_UNK1, @GNOME_DK_VAR_INDEX_START, 0) + (a.variation_index % @GNOME_DK_VAR_INDEX_START), 2, '00'), 
			'_', SUBSTRING_INDEX(b.texture1, '_', -2)
		)
	),
	''
),
'', 
if (a.variation_index < @GNOME_DK_VAR_INDEX_START, @FLAG_PLAYABLE, @FLAG_UNPLAYABLE), 
a.variation_index,
b.color_index + @GNOME_DK_COLOR_VAR_OFFSET
FROM default_charsections a 
JOIN default_charsections b ON b.race = @RACE_GNOME AND b.`type` = @TYPE_SKIN_HD AND b.flags IN (@FLAG_PLAYABLE | @FLAG_UNK1, @FLAG_PLAYABLE | @FLAG_DEATH_KNIGHT)
WHERE a.`type` = @TYPE_FACE_HD AND a.race = @RACE_GNOME
GROUP BY a.variation_index, b.color_index, b.gender
ORDER BY a.variation_index, b.color_index, b.gender;

-- -- Underwear skins
INSERT INTO freedom_charsections (race, gender, `type`, texture1, texture2, texture3, flags, variation_index, color_index) 
SELECT @RACE_GNOME, a.gender, @TYPE_UNDERWEAR_HD, 
if (a.gender = @GENDER_MALE,
	CONCAT('Character\\Gnome\\Male\\GnomeMaleNakedPelvisSkin00_', SUBSTRING_INDEX(a.texture1, '_', -2)),
	CONCAT('Character\\Gnome\\Female\\GnomeFemaleNakedPelvisSkin00_', SUBSTRING_INDEX(a.texture1, '_', -2))
),
if (a.gender = @GENDER_MALE,
	CONCAT('Character\\Gnome\\Male\\GnomeMaleNakedTorsoSkin00_', SUBSTRING_INDEX(a.texture1, '_', -2)),
	CONCAT('Character\\Gnome\\Female\\GnomeFemaleNakedTorsoSkin00_', SUBSTRING_INDEX(a.texture1, '_', -2))
), 
'', 
@FLAG_PLAYABLE | @FLAG_UNK1,
0,
a.color_index + @GNOME_DK_COLOR_VAR_OFFSET
FROM default_charsections a
WHERE a.`type` = @TYPE_SKIN_HD AND a.race = @RACE_GNOME AND a.flags IN (@FLAG_PLAYABLE | @FLAG_DEATH_KNIGHT, @FLAG_PLAYABLE | @FLAG_UNK1)
ORDER BY a.color_index;

/********************************************************************************************************************************************************************************************
 * RACE: TROLL (8)
 */
 
SET @RACE_TROLL := 8;
SET @TROLL_DK_VAR_INDEX_START := 6;

-- [Custom skins]
SET @TROLL_CUSTOM_COLOR_VAR_OFFSET := 100;

-- -- Base skins
INSERT INTO freedom_charsections (race, gender, `type`, texture1, texture2, texture3, flags, variation_index, color_index)
SELECT a.race, a.gender, a.`type`, a.texture1, a.texture2, a.texture3, @FLAG_PLAYABLE | @FLAG_UNK1, a.variation_index, a.color_index + @TROLL_CUSTOM_COLOR_VAR_OFFSET
FROM default_charsections a WHERE a.`type` = @TYPE_SKIN_HD AND a.race = @RACE_TROLL AND a.flags = @FLAG_UNPLAYABLE
ORDER BY a.color_index, a.gender;

-- -- Face skins
INSERT INTO freedom_charsections (race, gender, `type`, texture1, texture2, texture3, flags, variation_index, color_index)
SELECT @RACE_TROLL, b.gender, @TYPE_FACE_HD, 
'', 
if (b.gender = @GENDER_MALE,
	CONCAT('Character\\Troll\\Male\\TrollMaleFaceUpper', LPAD(a.variation_index, 2, '00'), '_', SUBSTRING_INDEX(b.texture1, '_', -2)),
	CONCAT('Character\\Troll\\Female\\TrollFemaleFaceUpper', LPAD(a.variation_index, 2, '00'), '_', SUBSTRING_INDEX(b.texture1, '_', -2))
),
'', 
if (a.variation_index < @TROLL_DK_VAR_INDEX_START, @FLAG_PLAYABLE, @FLAG_PLAYABLE | @FLAG_DEATH_KNIGHT), 
a.variation_index,
b.color_index + @TROLL_CUSTOM_COLOR_VAR_OFFSET
FROM default_charsections a 
JOIN default_charsections b ON b.race = @RACE_TROLL AND b.`type` = @TYPE_SKIN_HD AND b.flags = @FLAG_UNPLAYABLE
WHERE a.`type` = @TYPE_FACE_HD AND a.race = @RACE_TROLL
GROUP BY a.variation_index, b.color_index, b.gender
ORDER BY a.variation_index, b.color_index, b.gender;

-- -- Underwear skins
INSERT INTO freedom_charsections (race, gender, `type`, texture1, texture2, texture3, flags, variation_index, color_index)
SELECT @RACE_TROLL, a.gender, @TYPE_UNDERWEAR_HD, 
if (a.gender = @GENDER_MALE, -- texture 1
	CONCAT('Character\\Troll\\Male\\TrollMaleNakedPelvisSkin00_', SUBSTRING_INDEX(a.texture1, '_', -2)), 
	CONCAT('Character\\Troll\\Female\\TrollFemaleNakedPelvisSkin00_', SUBSTRING_INDEX(a.texture1, '_', -2))
), 
if (a.gender = @GENDER_MALE, -- texture 2
	CONCAT('Character\\Troll\\Male\\TrollMaleNakedTorsoSkin00_', SUBSTRING_INDEX(a.texture1, '_', -2)), 
	CONCAT('Character\\Troll\\Female\\TrollFemaleNakedTorsoSkin00_', SUBSTRING_INDEX(a.texture1, '_', -2))
),  
'', 
@FLAG_PLAYABLE | @FLAG_UNK1, 
a.variation_index, 
a.color_index + @TROLL_CUSTOM_COLOR_VAR_OFFSET
FROM default_charsections a WHERE a.`type` = @TYPE_SKIN_HD AND a.race = @RACE_TROLL AND a.flags = @FLAG_UNPLAYABLE
ORDER BY a.color_index, a.gender;

-- [DK skin unlock]
SET @TROLL_DK_COLOR_VAR_OFFSET := 200;

-- -- Base skins
INSERT INTO freedom_charsections (race, gender, `type`, texture1, texture2, texture3, flags, variation_index, color_index)
SELECT a.race, a.gender, a.`type`, a.texture1, a.texture2, a.texture3, @FLAG_PLAYABLE | @FLAG_UNK1, a.variation_index, a.color_index + @TROLL_DK_COLOR_VAR_OFFSET
FROM default_charsections a WHERE a.`type` = @TYPE_SKIN_HD AND a.race = @RACE_TROLL AND a.flags IN (@FLAG_PLAYABLE | @FLAG_DEATH_KNIGHT, @FLAG_PLAYABLE | @FLAG_UNK1)
ORDER BY a.color_index, a.gender;

-- -- Face skins
INSERT INTO freedom_charsections (race, gender, `type`, texture1, texture2, texture3, flags, variation_index, color_index)
SELECT @RACE_TROLL, b.gender, @TYPE_FACE_HD, 
'', 
if (a.variation_index < @TROLL_DK_VAR_INDEX_START,
	if (b.gender = @GENDER_MALE,
		if (a.variation_index = 5 AND b.gender = @GENDER_MALE AND b.flags = @FLAG_PLAYABLE | @FLAG_UNK1, -- troll male doesnt have dk variation = 10, replace with previous var
			CONCAT('Character\\Troll\\Male\\TrollMaleFaceUpper09_', SUBSTRING_INDEX(b.texture1, '_', -2)),
			CONCAT('Character\\Troll\\Male\\TrollMaleFaceUpper', 
				LPAD(if (b.flags = @FLAG_PLAYABLE | @FLAG_UNK1, @TROLL_DK_VAR_INDEX_START - 1, 0) + (a.variation_index % @TROLL_DK_VAR_INDEX_START), 2, '00'), 
				'_', SUBSTRING_INDEX(b.texture1, '_', -2)
			)
		),
		CONCAT('Character\\Troll\\Female\\TrollFemaleFaceUpper', 
			LPAD(if (b.flags = @FLAG_PLAYABLE | @FLAG_UNK1, @TROLL_DK_VAR_INDEX_START, 0) + (a.variation_index % @TROLL_DK_VAR_INDEX_START), 2, '00'), 
			'_', SUBSTRING_INDEX(b.texture1, '_', -2)
		)
	),
	''
),
'', 
if (a.variation_index < @TROLL_DK_VAR_INDEX_START, @FLAG_PLAYABLE, @FLAG_UNPLAYABLE), 
a.variation_index,
b.color_index + @TROLL_DK_COLOR_VAR_OFFSET
FROM default_charsections a 
JOIN default_charsections b ON b.race = @RACE_TROLL AND b.`type` = @TYPE_SKIN_HD AND b.flags IN (@FLAG_PLAYABLE | @FLAG_UNK1, @FLAG_PLAYABLE | @FLAG_DEATH_KNIGHT)
WHERE a.`type` = @TYPE_FACE_HD AND a.race = @RACE_TROLL
GROUP BY a.variation_index, b.color_index, b.gender
ORDER BY a.variation_index, b.color_index, b.gender;

-- -- Underwear skins
INSERT INTO freedom_charsections (race, gender, `type`, texture1, texture2, texture3, flags, variation_index, color_index) 
SELECT @RACE_TROLL, a.gender, @TYPE_UNDERWEAR_HD, 
if (a.gender = @GENDER_MALE,
	CONCAT('Character\\Troll\\Male\\TrollMaleNakedPelvisSkin00_', SUBSTRING_INDEX(a.texture1, '_', -2)),
	CONCAT('Character\\Troll\\Female\\TrollFemaleNakedPelvisSkin00_', SUBSTRING_INDEX(a.texture1, '_', -2))
),
if (a.gender = @GENDER_MALE,
	CONCAT('Character\\Troll\\Male\\TrollMaleNakedTorsoSkin00_', SUBSTRING_INDEX(a.texture1, '_', -2)),
	CONCAT('Character\\Troll\\Female\\TrollFemaleNakedTorsoSkin00_', SUBSTRING_INDEX(a.texture1, '_', -2))
), 
'', 
@FLAG_PLAYABLE | @FLAG_UNK1,
0,
a.color_index + @TROLL_DK_COLOR_VAR_OFFSET
FROM default_charsections a
WHERE a.`type` = @TYPE_SKIN_HD AND a.race = @RACE_TROLL AND a.flags IN (@FLAG_PLAYABLE | @FLAG_DEATH_KNIGHT, @FLAG_PLAYABLE | @FLAG_UNK1)
ORDER BY a.color_index;

/********************************************************************************************************************************************************************************************
 * RACE: GOBLIN (9)
 */
 
SET @RACE_GOBLIN := 9;
SET @GOBLIN_DK_VAR_INDEX_START := 10;

-- [Custom skins]
SET @GOBLIN_CUSTOM_COLOR_VAR_OFFSET := 100;

-- -- Base skins
INSERT INTO freedom_charsections (race, gender, `type`, texture1, texture2, texture3, flags, variation_index, color_index)
SELECT a.race, a.gender, a.`type`, a.texture1, a.texture2, a.texture3, @FLAG_PLAYABLE | @FLAG_UNK1, a.variation_index, a.color_index + @GOBLIN_CUSTOM_COLOR_VAR_OFFSET
FROM default_charsections a WHERE a.`type` = @TYPE_SKIN AND a.race = @RACE_GOBLIN AND a.flags = @FLAG_UNPLAYABLE
ORDER BY a.color_index, a.gender;

-- -- Face skins
INSERT INTO freedom_charsections (race, gender, `type`, texture1, texture2, texture3, flags, variation_index, color_index)
SELECT @RACE_GOBLIN, b.gender, @TYPE_FACE, 
if (b.gender = @GENDER_MALE,
	CONCAT('Character\\Goblin\\Male\\GoblinMaleFaceLower', LPAD(a.variation_index, 2, '00'), '_', SUBSTRING_INDEX(b.texture1, '_', -1)),
	CONCAT('Character\\Goblin\\Female\\GoblinFemaleFaceLower', LPAD(a.variation_index, 2, '00'), '_', SUBSTRING_INDEX(b.texture1, '_', -1))
), 
if (b.gender = @GENDER_MALE,
	CONCAT('Character\\Goblin\\Male\\GoblinMaleFaceUpper', LPAD(a.variation_index, 2, '00'), '_', SUBSTRING_INDEX(b.texture1, '_', -1)),
	CONCAT('Character\\Goblin\\Female\\GoblinFemaleFaceUpper', LPAD(a.variation_index, 2, '00'), '_', SUBSTRING_INDEX(b.texture1, '_', -1))
),
'', 
if (a.variation_index < @GOBLIN_DK_VAR_INDEX_START, 
	if (b.gender = @GENDER_MALE AND a.variation_index > 4, @FLAG_PLAYABLE | @FLAG_DEATH_KNIGHT, @FLAG_PLAYABLE), 
	if (b.gender = @GENDER_MALE, @FLAG_UNPLAYABLE, @FLAG_PLAYABLE | @FLAG_DEATH_KNIGHT)
), 
a.variation_index,
b.color_index + @GOBLIN_CUSTOM_COLOR_VAR_OFFSET
FROM default_charsections a 
JOIN default_charsections b ON b.race = @RACE_GOBLIN AND b.`type` = @TYPE_SKIN AND b.flags = @FLAG_UNPLAYABLE
WHERE a.`type` = @TYPE_FACE AND a.race = @RACE_GOBLIN
GROUP BY a.variation_index, b.color_index, b.gender
ORDER BY a.variation_index, b.color_index, b.gender;

-- -- Underwear skins
INSERT INTO freedom_charsections (race, gender, `type`, texture1, texture2, texture3, flags, variation_index, color_index)
SELECT @RACE_GOBLIN, a.gender, @TYPE_UNDERWEAR, 
if (a.gender = @GENDER_MALE, -- texture 1
	CONCAT('Character\\Goblin\\Male\\GoblinMaleNakedPelvisSkin00_', SUBSTRING_INDEX(a.texture1, '_', -1)), 
	CONCAT('Character\\Goblin\\Female\\GoblinFemaleNakedPelvisSkin00_', SUBSTRING_INDEX(a.texture1, '_', -1))
), 
if (a.gender = @GENDER_MALE, -- texture 2
	CONCAT('Character\\Goblin\\Male\\GoblinMaleNakedTorsoSkin00_', SUBSTRING_INDEX(a.texture1, '_', -1)), 
	CONCAT('Character\\Goblin\\Female\\GoblinFemaleNakedTorsoSkin00_', SUBSTRING_INDEX(a.texture1, '_', -1))
),  
'', 
@FLAG_PLAYABLE | @FLAG_UNK1, 
a.variation_index, 
a.color_index + @GOBLIN_CUSTOM_COLOR_VAR_OFFSET
FROM default_charsections a WHERE a.`type` = @TYPE_SKIN AND a.race = @RACE_GOBLIN AND a.flags = @FLAG_UNPLAYABLE
ORDER BY a.color_index, a.gender;

-- [DK skin unlock]
SET @GOBLIN_DK_COLOR_VAR_OFFSET := 200;

-- -- Base skins
INSERT INTO freedom_charsections (race, gender, `type`, texture1, texture2, texture3, flags, variation_index, color_index)
SELECT a.race, a.gender, a.`type`, a.texture1, a.texture2, a.texture3, @FLAG_PLAYABLE | @FLAG_UNK1, a.variation_index, a.color_index + @GOBLIN_DK_COLOR_VAR_OFFSET
FROM default_charsections a WHERE a.`type` = @TYPE_SKIN AND a.race = @RACE_GOBLIN AND a.flags IN (@FLAG_PLAYABLE | @FLAG_DEATH_KNIGHT, @FLAG_PLAYABLE | @FLAG_UNK1)
ORDER BY a.color_index, a.gender;

-- -- Face skins
INSERT INTO freedom_charsections (race, gender, `type`, texture1, texture2, texture3, flags, variation_index, color_index)
SELECT @RACE_GOBLIN, b.gender, @TYPE_FACE, 
if (a.variation_index < @GOBLIN_DK_VAR_INDEX_START,
	if (b.gender = @GENDER_MALE,
		if (b.flags = @FLAG_PLAYABLE | @FLAG_DEATH_KNIGHT AND a.variation_index > 2, -- special DK skins of male doesnt have proper face above variation 2, use var 0 as replacement
			CONCAT('Character\\Goblin\\Male\\GoblinMaleFaceLower00_', SUBSTRING_INDEX(b.texture1, '_', -1)),
			CONCAT('Character\\Goblin\\Male\\GoblinMaleFaceLower', 
				LPAD((a.variation_index % @GOBLIN_DK_VAR_INDEX_START), 2, '00'), 
				'_', LPAD(b.color_index + if (b.flags = @FLAG_PLAYABLE | @FLAG_UNK1, 12, 0), 2, '00'), '.blp'
			)
		),
		if (a.variation_index > 7 AND b.flags = @FLAG_PLAYABLE | @FLAG_UNK1,
			CONCAT('Character\\Goblin\\Female\\GoblinFemaleFaceLower17_', SUBSTRING_INDEX(b.texture1, '_', -1)),
			CONCAT('Character\\Goblin\\Female\\GoblinFemaleFaceLower', 
				LPAD(if (b.flags = @FLAG_PLAYABLE | @FLAG_UNK1, @GOBLIN_DK_VAR_INDEX_START, 0) + (a.variation_index % @GOBLIN_DK_VAR_INDEX_START), 2, '00'), 
				'_', SUBSTRING_INDEX(b.texture1, '_', -1)
			)
		)
	),
	''
),
if (a.variation_index < @GOBLIN_DK_VAR_INDEX_START,
	if (b.gender = @GENDER_MALE,
		if (b.flags = @FLAG_PLAYABLE | @FLAG_DEATH_KNIGHT AND a.variation_index > 2, -- special DK skins of male doesnt have proper face above variation 2, use var 0 as replacement
			CONCAT('Character\\Goblin\\Male\\GoblinMaleFaceUpper00_', SUBSTRING_INDEX(b.texture1, '_', -1)),			
			CONCAT('Character\\Goblin\\Male\\GoblinMaleFaceUpper', 
				LPAD(a.variation_index % @GOBLIN_DK_VAR_INDEX_START, 2, '00'), 
				'_', LPAD(b.color_index + if (b.flags = @FLAG_PLAYABLE | @FLAG_UNK1, 12, 0), 2, '00'), '.blp'
			)
		),
		if (a.variation_index > 7 AND b.flags = @FLAG_PLAYABLE | @FLAG_UNK1,
			CONCAT('Character\\Goblin\\Female\\GoblinFemaleFaceUpper07_', SUBSTRING_INDEX(b.texture1, '_', -1)),
			CONCAT('Character\\Goblin\\Female\\GoblinFemaleFaceUpper', 
				LPAD(if (b.flags = @FLAG_PLAYABLE | @FLAG_UNK1, @GOBLIN_DK_VAR_INDEX_START, 0) + (a.variation_index % @GOBLIN_DK_VAR_INDEX_START), 2, '00'), 
				'_', SUBSTRING_INDEX(b.texture1, '_', -1)
			)
		)
	),
	''
),
'', 
if (a.variation_index < (@GOBLIN_DK_VAR_INDEX_START), 
	if (b.gender = @GENDER_MALE AND a.variation_index > 4, @FLAG_UNPLAYABLE, @FLAG_PLAYABLE), 
	@FLAG_UNPLAYABLE
), 
a.variation_index,
b.color_index + @GOBLIN_DK_COLOR_VAR_OFFSET
FROM default_charsections a 
JOIN default_charsections b ON b.race = @RACE_GOBLIN AND b.`type` = @TYPE_SKIN AND b.flags IN (@FLAG_PLAYABLE | @FLAG_UNK1, @FLAG_PLAYABLE | @FLAG_DEATH_KNIGHT)
WHERE a.`type` = @TYPE_FACE AND a.race = @RACE_GOBLIN
GROUP BY a.variation_index, b.color_index, b.gender
ORDER BY a.variation_index, b.color_index, b.gender;

-- -- Underwear skins
INSERT INTO freedom_charsections (race, gender, `type`, texture1, texture2, texture3, flags, variation_index, color_index) 
SELECT @RACE_GOBLIN, a.gender, @TYPE_UNDERWEAR, 
if (a.gender = @GENDER_MALE,
	CONCAT('Character\\Goblin\\Male\\GoblinMaleNakedPelvisSkin00_', SUBSTRING_INDEX(a.texture1, '_', -1)),
	CONCAT('Character\\Goblin\\Female\\GoblinFemaleNakedPelvisSkin00_', SUBSTRING_INDEX(a.texture1, '_', -1))
),
if (a.gender = @GENDER_MALE,
	CONCAT('Character\\Goblin\\Male\\GoblinMaleNakedTorsoSkin00_', SUBSTRING_INDEX(a.texture1, '_', -1)),
	CONCAT('Character\\Goblin\\Female\\GoblinFemaleNakedTorsoSkin00_', SUBSTRING_INDEX(a.texture1, '_', -1))
), 
'', 
@FLAG_PLAYABLE | @FLAG_UNK1,
0,
a.color_index + @GOBLIN_DK_COLOR_VAR_OFFSET
FROM default_charsections a
WHERE a.`type` = @TYPE_SKIN AND a.race = @RACE_GOBLIN AND a.flags IN (@FLAG_PLAYABLE | @FLAG_DEATH_KNIGHT, @FLAG_PLAYABLE | @FLAG_UNK1)
ORDER BY a.color_index;

/********************************************************************************************************************************************************************************************
 * RACE: BLOOD ELF (10)
 */
 
SET @RACE_BELF := 10;
SET @BELF_DK_VAR_INDEX_START := 10;

-- [Custom skins]
SET @BELF_CUSTOM_COLOR_VAR_OFFSET := 100;

-- -- Base skins
INSERT INTO freedom_charsections (race, gender, `type`, texture1, texture2, texture3, flags, variation_index, color_index)
SELECT a.race, a.gender, a.`type`, a.texture1, a.texture2, a.texture3, @FLAG_PLAYABLE | @FLAG_UNK1, a.variation_index, a.color_index + @BELF_CUSTOM_COLOR_VAR_OFFSET
FROM default_charsections a WHERE a.`type` = @TYPE_SKIN_HD AND a.race = @RACE_BELF AND a.flags = @FLAG_UNPLAYABLE
ORDER BY a.color_index, a.gender;

-- -- Face skins
INSERT INTO freedom_charsections (race, gender, `type`, texture1, texture2, texture3, flags, variation_index, color_index)
SELECT @RACE_BELF, b.gender, @TYPE_FACE_HD, 
'', 
if (b.gender = @GENDER_MALE,
	CONCAT('Character\\BloodElf\\Male\\BloodElfMaleFaceUpper', LPAD(a.variation_index, 2, '00'), '_', SUBSTRING_INDEX(b.texture1, '_', -2)),
	CONCAT('Character\\BloodElf\\Female\\BloodElfFemaleFaceUpper', LPAD(a.variation_index, 2, '00'), '_', SUBSTRING_INDEX(b.texture1, '_', -2))
),
'', 
if (a.variation_index < @BELF_DK_VAR_INDEX_START, @FLAG_PLAYABLE, @FLAG_PLAYABLE | @FLAG_DEATH_KNIGHT), 
a.variation_index,
b.color_index + @BELF_CUSTOM_COLOR_VAR_OFFSET
FROM default_charsections a 
JOIN default_charsections b ON b.race = @RACE_BELF AND b.`type` = @TYPE_SKIN_HD AND b.flags = @FLAG_UNPLAYABLE
WHERE a.`type` = @TYPE_FACE_HD AND a.race = @RACE_BELF
GROUP BY a.variation_index, b.color_index, b.gender
ORDER BY a.variation_index, b.color_index, b.gender;

-- -- Underwear skins
INSERT INTO freedom_charsections (race, gender, `type`, texture1, texture2, texture3, flags, variation_index, color_index)
SELECT @RACE_BELF, a.gender, @TYPE_UNDERWEAR_HD, 
if (a.gender = @GENDER_MALE, -- texture 1
	CONCAT('Character\\BloodElf\\Male\\BloodElfMaleNakedPelvisSkin00_', SUBSTRING_INDEX(a.texture1, '_', -2)), 
	CONCAT('Character\\BloodElf\\Female\\BloodElfFemaleNakedPelvisSkin00_', SUBSTRING_INDEX(a.texture1, '_', -2))
), 
if (a.gender = @GENDER_MALE, -- texture 2
	CONCAT('Character\\BloodElf\\Male\\BloodElfMaleNakedTorsoSkin00_', SUBSTRING_INDEX(a.texture1, '_', -2)), 
	CONCAT('Character\\BloodElf\\Female\\BloodElfFemaleNakedTorsoSkin00_', SUBSTRING_INDEX(a.texture1, '_', -2))
),  
'', 
@FLAG_PLAYABLE | @FLAG_UNK1, 
a.variation_index, 
a.color_index + @BELF_CUSTOM_COLOR_VAR_OFFSET
FROM default_charsections a WHERE a.`type` = @TYPE_SKIN_HD AND a.race = @RACE_BELF AND a.flags = @FLAG_UNPLAYABLE
ORDER BY a.color_index, a.gender;

-- [DK skin unlock]
SET @BELF_DK_COLOR_VAR_OFFSET := 200;

-- -- Base skins
INSERT INTO freedom_charsections (race, gender, `type`, texture1, texture2, texture3, flags, variation_index, color_index)
SELECT a.race, a.gender, a.`type`, a.texture1, a.texture2, a.texture3, @FLAG_PLAYABLE | @FLAG_UNK1, a.variation_index, a.color_index + @BELF_DK_COLOR_VAR_OFFSET
FROM default_charsections a WHERE a.`type` = @TYPE_SKIN_HD AND a.race = @RACE_BELF AND a.flags IN (@FLAG_PLAYABLE | @FLAG_DEATH_KNIGHT, @FLAG_PLAYABLE | @FLAG_UNK1)
ORDER BY a.color_index, a.gender;

-- -- Face skins
INSERT INTO freedom_charsections (race, gender, `type`, texture1, texture2, texture3, flags, variation_index, color_index)
SELECT @RACE_BELF, b.gender, @TYPE_FACE_HD, 
'', 
if (a.variation_index < @BELF_DK_VAR_INDEX_START,
	if (b.gender = @GENDER_MALE,
		CONCAT('Character\\BloodElf\\Male\\BloodElfMaleFaceUpper', 
			LPAD(if (b.flags = @FLAG_PLAYABLE | @FLAG_UNK1, @BELF_DK_VAR_INDEX_START, 0) + (a.variation_index % @BELF_DK_VAR_INDEX_START), 2, '00'), 
			'_', SUBSTRING_INDEX(b.texture1, '_', -2)
		),
		CONCAT('Character\\BloodElf\\Female\\BloodElfFemaleFaceUpper', 
			LPAD(if (b.flags = @FLAG_PLAYABLE | @FLAG_UNK1, @BELF_DK_VAR_INDEX_START, 0) + (a.variation_index % @BELF_DK_VAR_INDEX_START), 2, '00'), 
			'_', SUBSTRING_INDEX(b.texture1, '_', -2)
		)
	),
	''
),
'', 
if (a.variation_index < @BELF_DK_VAR_INDEX_START, @FLAG_PLAYABLE, @FLAG_UNPLAYABLE), 
a.variation_index,
b.color_index + @BELF_DK_COLOR_VAR_OFFSET
FROM default_charsections a 
JOIN default_charsections b ON b.race = @RACE_BELF AND b.`type` = @TYPE_SKIN_HD AND b.flags IN (@FLAG_PLAYABLE | @FLAG_UNK1, @FLAG_PLAYABLE | @FLAG_DEATH_KNIGHT)
WHERE a.`type` = @TYPE_FACE_HD AND a.race = @RACE_BELF
GROUP BY a.variation_index, b.color_index, b.gender
ORDER BY a.variation_index, b.color_index, b.gender;

-- -- Underwear skins
INSERT INTO freedom_charsections (race, gender, `type`, texture1, texture2, texture3, flags, variation_index, color_index) 
SELECT @RACE_BELF, a.gender, @TYPE_UNDERWEAR_HD, 
if (a.gender = @GENDER_MALE,
	CONCAT('Character\\BloodElf\\Male\\BloodElfMaleNakedPelvisSkin00_', SUBSTRING_INDEX(a.texture1, '_', -2)),
	CONCAT('Character\\BloodElf\\Female\\BloodElfFemaleNakedPelvisSkin00_', SUBSTRING_INDEX(a.texture1, '_', -2))
),
if (a.gender = @GENDER_MALE,
	CONCAT('Character\\BloodElf\\Male\\BloodElfMaleNakedTorsoSkin00_', SUBSTRING_INDEX(a.texture1, '_', -2)),
	CONCAT('Character\\BloodElf\\Female\\BloodElfFemaleNakedTorsoSkin00_', SUBSTRING_INDEX(a.texture1, '_', -2))
), 
'', 
@FLAG_PLAYABLE | @FLAG_UNK1,
0,
a.color_index + @BELF_DK_COLOR_VAR_OFFSET
FROM default_charsections a
WHERE a.`type` = @TYPE_SKIN_HD AND a.race = @RACE_BELF AND a.flags IN (@FLAG_PLAYABLE | @FLAG_DEATH_KNIGHT, @FLAG_PLAYABLE | @FLAG_UNK1)
ORDER BY a.color_index;

/********************************************************************************************************************************************************************************************
 * RACE: DRAENEI (11)
 */
 
SET @RACE_DRAENEI := 11;
SET @DRAENEI_DK_VAR_INDEX_START := 10;

-- [Custom skins]
SET @DRAENEI_CUSTOM_COLOR_VAR_OFFSET := 100;

-- -- Base skins
INSERT INTO freedom_charsections (race, gender, `type`, texture1, texture2, texture3, flags, variation_index, color_index)
SELECT a.race, a.gender, a.`type`, a.texture1, a.texture2, a.texture3, @FLAG_PLAYABLE | @FLAG_UNK1, a.variation_index, a.color_index + @DRAENEI_CUSTOM_COLOR_VAR_OFFSET
FROM default_charsections a WHERE a.`type` = @TYPE_SKIN_HD AND a.race = @RACE_DRAENEI AND a.flags = @FLAG_UNPLAYABLE
ORDER BY a.color_index, a.gender;

-- -- Face skins
INSERT INTO freedom_charsections (race, gender, `type`, texture1, texture2, texture3, flags, variation_index, color_index)
SELECT @RACE_DRAENEI, b.gender, @TYPE_FACE_HD, 
'', 
if (b.gender = @GENDER_MALE,
	CONCAT('Character\\Draenei\\Male\\DraeneiMaleFaceUpper', LPAD(a.variation_index, 2, '00'), '_', SUBSTRING_INDEX(b.texture1, '_', -2)),
	CONCAT('Character\\Draenei\\Female\\DraeneiFemaleFaceUpper', LPAD(a.variation_index, 2, '00'), '_', SUBSTRING_INDEX(b.texture1, '_', -2))
),
'', 
if (a.variation_index < @DRAENEI_DK_VAR_INDEX_START, @FLAG_PLAYABLE, @FLAG_PLAYABLE | @FLAG_DEATH_KNIGHT), 
a.variation_index,
b.color_index + @DRAENEI_CUSTOM_COLOR_VAR_OFFSET
FROM default_charsections a 
JOIN default_charsections b ON b.race = @RACE_DRAENEI AND b.`type` = @TYPE_SKIN_HD AND b.flags = @FLAG_UNPLAYABLE
WHERE a.`type` = @TYPE_FACE_HD AND a.race = @RACE_DRAENEI
GROUP BY a.variation_index, b.color_index, b.gender
ORDER BY a.variation_index, b.color_index, b.gender;

-- -- Underwear skins
INSERT INTO freedom_charsections (race, gender, `type`, texture1, texture2, texture3, flags, variation_index, color_index)
SELECT @RACE_DRAENEI, a.gender, @TYPE_UNDERWEAR_HD, 
if (a.gender = @GENDER_MALE, -- texture 1
	CONCAT('Character\\Draenei\\Male\\DraeneiMaleNakedPelvisSkin00_', SUBSTRING_INDEX(a.texture1, '_', -2)), 
	CONCAT('Character\\Draenei\\Female\\DraeneiFemaleNakedPelvisSkin00_', SUBSTRING_INDEX(a.texture1, '_', -2))
), 
if (a.gender = @GENDER_MALE, -- texture 2
	CONCAT('Character\\Draenei\\Male\\DraeneiMaleNakedTorsoSkin00_', SUBSTRING_INDEX(a.texture1, '_', -2)), 
	CONCAT('Character\\Draenei\\Female\\DraeneiFemaleNakedTorsoSkin00_', SUBSTRING_INDEX(a.texture1, '_', -2))
),  
'', 
@FLAG_PLAYABLE | @FLAG_UNK1, 
a.variation_index, 
a.color_index + @DRAENEI_CUSTOM_COLOR_VAR_OFFSET
FROM default_charsections a WHERE a.`type` = @TYPE_SKIN_HD AND a.race = @RACE_DRAENEI AND a.flags = @FLAG_UNPLAYABLE
ORDER BY a.color_index, a.gender;

-- [DK skin unlock]
SET @DRAENEI_DK_COLOR_VAR_OFFSET := 200;

-- -- Base skins
INSERT INTO freedom_charsections (race, gender, `type`, texture1, texture2, texture3, flags, variation_index, color_index)
SELECT a.race, a.gender, a.`type`, a.texture1, a.texture2, a.texture3, @FLAG_PLAYABLE | @FLAG_UNK1, a.variation_index, a.color_index + @DRAENEI_DK_COLOR_VAR_OFFSET
FROM default_charsections a WHERE a.`type` = @TYPE_SKIN_HD AND a.race = @RACE_DRAENEI AND a.flags IN (@FLAG_PLAYABLE | @FLAG_DEATH_KNIGHT, @FLAG_PLAYABLE | @FLAG_UNK1)
ORDER BY a.color_index, a.gender;

-- -- Face skins
INSERT INTO freedom_charsections (race, gender, `type`, texture1, texture2, texture3, flags, variation_index, color_index)
SELECT @RACE_DRAENEI, b.gender, @TYPE_FACE_HD, 
'', 
if (a.variation_index < @DRAENEI_DK_VAR_INDEX_START,
	if (b.gender = @GENDER_MALE,
		CONCAT('Character\\Draenei\\Male\\DraeneiMaleFaceUpper', 
			LPAD(if (b.flags = @FLAG_PLAYABLE | @FLAG_UNK1, @DRAENEI_DK_VAR_INDEX_START, 0) + (a.variation_index % @DRAENEI_DK_VAR_INDEX_START), 2, '00'), 
			'_', SUBSTRING_INDEX(b.texture1, '_', -2)
		),
		CONCAT('Character\\Draenei\\Female\\DraeneiFemaleFaceUpper', 
			LPAD(if (b.flags = @FLAG_PLAYABLE | @FLAG_UNK1, @DRAENEI_DK_VAR_INDEX_START, 0) + (a.variation_index % @DRAENEI_DK_VAR_INDEX_START), 2, '00'), 
			'_', SUBSTRING_INDEX(b.texture1, '_', -2)
		)
	),
	''
),
'', 
if (a.variation_index < @DRAENEI_DK_VAR_INDEX_START, @FLAG_PLAYABLE, @FLAG_UNPLAYABLE), 
a.variation_index,
b.color_index + @DRAENEI_DK_COLOR_VAR_OFFSET
FROM default_charsections a 
JOIN default_charsections b ON b.race = @RACE_DRAENEI AND b.`type` = @TYPE_SKIN_HD AND b.flags IN (@FLAG_PLAYABLE | @FLAG_UNK1, @FLAG_PLAYABLE | @FLAG_DEATH_KNIGHT)
WHERE a.`type` = @TYPE_FACE_HD AND a.race = @RACE_DRAENEI
GROUP BY a.variation_index, b.color_index, b.gender
ORDER BY a.variation_index, b.color_index, b.gender;

-- -- Underwear skins
INSERT INTO freedom_charsections (race, gender, `type`, texture1, texture2, texture3, flags, variation_index, color_index) 
SELECT @RACE_DRAENEI, a.gender, @TYPE_UNDERWEAR_HD, 
if (a.gender = @GENDER_MALE,
	CONCAT('Character\\Draenei\\Male\\DraeneiMaleNakedPelvisSkin00_', SUBSTRING_INDEX(a.texture1, '_', -2)),
	CONCAT('Character\\Draenei\\Female\\DraeneiFemaleNakedPelvisSkin00_', SUBSTRING_INDEX(a.texture1, '_', -2))
),
if (a.gender = @GENDER_MALE,
	CONCAT('Character\\Draenei\\Male\\DraeneiMaleNakedTorsoSkin00_', SUBSTRING_INDEX(a.texture1, '_', -2)),
	CONCAT('Character\\Draenei\\Female\\DraeneiFemaleNakedTorsoSkin00_', SUBSTRING_INDEX(a.texture1, '_', -2))
), 
'', 
@FLAG_PLAYABLE | @FLAG_UNK1,
0,
a.color_index + @DRAENEI_DK_COLOR_VAR_OFFSET
FROM default_charsections a
WHERE a.`type` = @TYPE_SKIN_HD AND a.race = @RACE_DRAENEI AND a.flags IN (@FLAG_PLAYABLE | @FLAG_DEATH_KNIGHT, @FLAG_PLAYABLE | @FLAG_UNK1)
ORDER BY a.color_index;

/********************************************************************************************************************************************************************************************
 * RACE: Worgen (22)
 */
 
SET @RACE_WORGEN := 22;
SET @WORGEN_DK_VAR_INDEX_START := 14;

-- [Custom skins]
SET @WORGEN_CUSTOM_COLOR_VAR_OFFSET := 100;
-- -- Worgen do not have any NPC skins (@FLAG_UNPLAYABLE)

-- [DK skin unlock]
SET @WORGEN_DK_COLOR_VAR_OFFSET := 200;

-- -- Base skins
INSERT INTO freedom_charsections (race, gender, `type`, texture1, texture2, texture3, flags, variation_index, color_index)
SELECT a.race, a.gender, a.`type`, a.texture1, a.texture2, a.texture3, @FLAG_PLAYABLE | @FLAG_UNK1, a.variation_index, a.color_index + @WORGEN_DK_COLOR_VAR_OFFSET
FROM default_charsections a WHERE a.`type` = @TYPE_SKIN AND a.race = @RACE_WORGEN AND a.flags IN (@FLAG_PLAYABLE | @FLAG_DEATH_KNIGHT, @FLAG_PLAYABLE | @FLAG_UNK1)
ORDER BY a.color_index, a.gender;

-- -- Face skins
INSERT INTO freedom_charsections (race, gender, `type`, texture1, texture2, texture3, flags, variation_index, color_index)
SELECT @RACE_WORGEN, b.gender, @TYPE_FACE, 
if (a.variation_index < @WORGEN_DK_VAR_INDEX_START,
	if (b.gender = @GENDER_MALE,
		CONCAT('Character\\Worgen\\Male\\WorgenMaleFaceLower', 
			LPAD(if (b.flags = @FLAG_PLAYABLE | @FLAG_UNK1, @WORGEN_DK_VAR_INDEX_START - 9, 0) + (a.variation_index % @WORGEN_DK_VAR_INDEX_START), 2, '00'), 
			'_', SUBSTRING_INDEX(b.texture1, '_', -1)
		),
		if (a.variation_index NOT IN (7, 9, 13) AND b.color_index IN (9, 10, 11), -- fill first worgen female DK face in variations where no female DK face exists
			CONCAT('Character\Worgen\Female\WorgenFemaleFaceLower07_', SUBSTRING_INDEX(b.texture1, '_', -1)),
			CONCAT('Character\\Worgen\\Female\\WorgenFemaleFaceLower', 
				LPAD(if (b.flags = @FLAG_PLAYABLE | @FLAG_UNK1, @WORGEN_DK_VAR_INDEX_START, 0) + (a.variation_index % @WORGEN_DK_VAR_INDEX_START), 2, '00'), 
				'_', SUBSTRING_INDEX(b.texture1, '_', -1)
			)
		)
	),
	''
), 
if (a.variation_index < @WORGEN_DK_VAR_INDEX_START,
	if (b.gender = @GENDER_MALE,
		CONCAT('Character\\Worgen\\Male\\WorgenMaleFaceUpper', 
			LPAD(if (b.flags = @FLAG_PLAYABLE | @FLAG_UNK1, @WORGEN_DK_VAR_INDEX_START - 9, 0) + (a.variation_index % @WORGEN_DK_VAR_INDEX_START), 2, '00'), 
			'_', SUBSTRING_INDEX(b.texture1, '_', -1)
		),
		if (a.variation_index NOT IN (7, 9, 13) AND b.color_index IN (9, 10, 11), -- fill first worgen female DK face in variations where no female DK face exists
			CONCAT('Character\Worgen\Female\WorgenFemaleFaceLower07_', SUBSTRING_INDEX(b.texture1, '_', -1)),
			CONCAT('Character\\Worgen\\Female\\WorgenFemaleFaceUpper', 
				LPAD(if (b.flags = @FLAG_PLAYABLE | @FLAG_UNK1, @WORGEN_DK_VAR_INDEX_START, 0) + (a.variation_index % @WORGEN_DK_VAR_INDEX_START), 2, '00'), 
				'_', SUBSTRING_INDEX(b.texture1, '_', -1)
			)
		)
	),
	''
),
'', 
if (a.variation_index < @WORGEN_DK_VAR_INDEX_START, 
	if (b.gender = @GENDER_MALE AND a.variation_index > 4, @FLAG_UNPLAYABLE, @FLAG_PLAYABLE), 
	@FLAG_UNPLAYABLE
), 
a.variation_index,
b.color_index + @WORGEN_DK_COLOR_VAR_OFFSET
FROM default_charsections a 
JOIN default_charsections b ON b.race = @RACE_WORGEN AND b.`type` = @TYPE_SKIN AND b.flags IN (@FLAG_PLAYABLE | @FLAG_UNK1, @FLAG_PLAYABLE | @FLAG_DEATH_KNIGHT)
WHERE a.`type` = @TYPE_FACE AND a.race = @RACE_WORGEN
GROUP BY a.variation_index, b.color_index, b.gender
ORDER BY a.variation_index, b.color_index, b.gender;

-- -- Underwear skins
INSERT INTO freedom_charsections (race, gender, `type`, texture1, texture2, texture3, flags, variation_index, color_index) 
SELECT @RACE_WORGEN, a.gender, @TYPE_UNDERWEAR, 
if (a.gender = @GENDER_MALE,
	CONCAT('Character\\Worgen\\Male\\WorgenMaleNakedPelvisSkin00_', SUBSTRING_INDEX(a.texture1, '_', -1)),
	CONCAT('Character\\Worgen\\Female\\WorgenFemaleNakedPelvisSkin00_', SUBSTRING_INDEX(a.texture1, '_', -1))
),
if (a.gender = @GENDER_MALE,
	CONCAT('Character\\Worgen\\Male\\WorgenMaleNakedTorsoSkin00_', SUBSTRING_INDEX(a.texture1, '_', -1)),
	CONCAT('Character\\Worgen\\Female\\WorgenFemaleNakedTorsoSkin00_', SUBSTRING_INDEX(a.texture1, '_', -1))
), 
'', 
@FLAG_PLAYABLE | @FLAG_UNK1,
0,
a.color_index + @WORGEN_DK_COLOR_VAR_OFFSET
FROM default_charsections a
WHERE a.`type` = @TYPE_SKIN AND a.race = @RACE_WORGEN AND a.flags IN (@FLAG_PLAYABLE | @FLAG_DEATH_KNIGHT, @FLAG_PLAYABLE | @FLAG_UNK1)
ORDER BY a.color_index;

/********************************************************************************************************************************************************************************************
 * RACE: Gilnean (23)
 */
 
SET @RACE_GILNEAN := 23;
SET @GILNEAN_DK_VAR_INDEX_START := 15;

-- [Custom skins]
SET @GILNEAN_CUSTOM_COLOR_VAR_OFFSET := 100;
-- -- Gilnean do not have any NPC skins (@FLAG_UNPLAYABLE)

-- [DK skin unlock]
SET @GILNEAN_DK_COLOR_VAR_OFFSET := 200;

-- -- Base skins
INSERT INTO freedom_charsections (race, gender, `type`, texture1, texture2, texture3, flags, variation_index, color_index)
SELECT a.race, a.gender, a.`type`, a.texture1, a.texture2, a.texture3, @FLAG_PLAYABLE | @FLAG_UNK1, a.variation_index, a.color_index + @GILNEAN_DK_COLOR_VAR_OFFSET
FROM default_charsections a WHERE a.`type` = @TYPE_SKIN_HD AND a.race = @RACE_GILNEAN AND a.flags IN (@FLAG_PLAYABLE | @FLAG_DEATH_KNIGHT, @FLAG_PLAYABLE | @FLAG_UNK1)
ORDER BY a.color_index, a.gender;

-- -- Face skins
INSERT INTO freedom_charsections (race, gender, `type`, texture1, texture2, texture3, flags, variation_index, color_index)
SELECT @RACE_GILNEAN, b.gender, @TYPE_FACE_HD, 
'', 
if (a.variation_index < @GILNEAN_DK_VAR_INDEX_START,
	if (b.gender = @GENDER_MALE,
		CONCAT('Character\\Human\\Male\\HumanMaleFaceUpper', 
			LPAD(if (b.flags = @FLAG_PLAYABLE | @FLAG_UNK1, @GILNEAN_DK_VAR_INDEX_START, 0) + (a.variation_index % @GILNEAN_DK_VAR_INDEX_START), 2, '00'), 
			'_', SUBSTRING_INDEX(b.texture1, '_', -2)
		),
		if (FALSE AND a.variation_index = 0 /*0 + 15 = 15*/ AND b.color_index = 8, -- character\human\female\HumanFemaleFaceUpper15_08_HD.blp isn't DK face, use next face as replacement
			'Character\\Human\\Female\\HumanFemaleFaceUpper16_08_HD.blp',
			CONCAT('Character\\Human\\Female\\HumanFemaleFaceUpper', 
				LPAD(if (b.flags = @FLAG_PLAYABLE | @FLAG_UNK1, @GILNEAN_DK_VAR_INDEX_START, 0) + (a.variation_index % @GILNEAN_DK_VAR_INDEX_START), 2, '00'), 
				'_', SUBSTRING_INDEX(b.texture1, '_', -2)
			)
		)
	),
	''
),
'', 
if (a.variation_index < @GILNEAN_DK_VAR_INDEX_START, 
	if (b.gender = @GENDER_MALE AND a.variation_index > 4, @FLAG_UNPLAYABLE, @FLAG_PLAYABLE), 
	@FLAG_UNPLAYABLE
), 
a.variation_index,
b.color_index + @GILNEAN_DK_COLOR_VAR_OFFSET
FROM default_charsections a 
JOIN default_charsections b ON b.race = @RACE_GILNEAN AND b.`type` = @TYPE_SKIN_HD AND b.flags IN (@FLAG_PLAYABLE | @FLAG_UNK1, @FLAG_PLAYABLE | @FLAG_DEATH_KNIGHT)
WHERE a.`type` = @TYPE_FACE_HD AND a.race = @RACE_GILNEAN
GROUP BY a.variation_index, b.color_index, b.gender
ORDER BY a.variation_index, b.color_index, b.gender;

-- -- Underwear skins
INSERT INTO freedom_charsections (race, gender, `type`, texture1, texture2, texture3, flags, variation_index, color_index) 
SELECT @RACE_GILNEAN, a.gender, @TYPE_UNDERWEAR_HD, 
if (a.gender = @GENDER_MALE,
	CONCAT('Character\\Human\\Male\\HumanMaleNakedPelvisSkin00_', SUBSTRING_INDEX(a.texture1, '_', -2)),
	CONCAT('Character\\Human\\Female\\HumanFemaleNakedPelvisSkin00_', SUBSTRING_INDEX(a.texture1, '_', -2))
),
if (a.gender = @GENDER_MALE,
	CONCAT('Character\\Human\\Male\\HumanMaleNakedTorsoSkin00_', SUBSTRING_INDEX(a.texture1, '_', -2)),
	CONCAT('Character\\Human\\Female\\HumanFemaleNakedTorsoSkin00_', SUBSTRING_INDEX(a.texture1, '_', -2))
), 
'', 
@FLAG_PLAYABLE | @FLAG_UNK1,
0,
a.color_index + @GILNEAN_DK_COLOR_VAR_OFFSET
FROM default_charsections a
WHERE a.`type` = @TYPE_SKIN_HD AND a.race = @RACE_GILNEAN AND a.flags IN (@FLAG_PLAYABLE | @FLAG_DEATH_KNIGHT, @FLAG_PLAYABLE | @FLAG_UNK1)
ORDER BY a.color_index;