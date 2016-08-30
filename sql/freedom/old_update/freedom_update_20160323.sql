CREATE TABLE `commands` (
	`command` VARCHAR(255) NOT NULL,
	`syntax` VARCHAR(255) NOT NULL DEFAULT '',
	`description` TEXT NULL,
	`gmlevel` INT(10) UNSIGNED NOT NULL DEFAULT '0',
	PRIMARY KEY (`command`)
)
COLLATE='utf8_general_ci'
ENGINE=InnoDB
;
