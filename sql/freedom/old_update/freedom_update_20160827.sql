ALTER TABLE `gameobject_extra`
	ADD COLUMN `uses_quat` TINYINT UNSIGNED NOT NULL DEFAULT '0' AFTER `phaseMask`;
