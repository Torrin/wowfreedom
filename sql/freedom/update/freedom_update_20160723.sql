ALTER TABLE `item_template_extra`
	ADD COLUMN `name` VARCHAR(255) NULL DEFAULT '' AFTER `hidden`,
	ADD COLUMN `inventory_type` INT UNSIGNED NULL DEFAULT '0' AFTER `name`,
	ADD COLUMN `class` INT UNSIGNED NULL DEFAULT '0' AFTER `inventory_type`,
	ADD COLUMN `subclass` INT UNSIGNED NULL DEFAULT '0' AFTER `class`;
	
ALTER TABLE `item_template_extra`
	ADD INDEX `NameIdx` (`name`);
