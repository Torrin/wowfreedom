-- --------------------------------------------------------
-- Host:                         127.0.0.1
-- Server version:               5.6.19 - MySQL Community Server (GPL)
-- Server OS:                    Win64
-- HeidiSQL Version:             9.3.0.5056
-- --------------------------------------------------------

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET NAMES utf8 */;
/*!50503 SET NAMES utf8mb4 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;

-- Dumping structure for table freedom.morphs
DROP TABLE IF EXISTS `morphs`;
CREATE TABLE IF NOT EXISTS `morphs` (
  `guid` bigint(20) unsigned NOT NULL COMMENT 'CHARACTER identifier',
  `name` varchar(255) NOT NULL COMMENT 'Custom given name for morph',
  `id_display` int(10) unsigned NOT NULL DEFAULT '0',
  `id_bnet_gm` int(10) unsigned NOT NULL DEFAULT '0' COMMENT 'GM Account Identifier',
  PRIMARY KEY (`guid`,`id_display`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT='.f morph table';

-- Data exporting was unselected.
-- Dumping structure for table freedom.private_tele
DROP TABLE IF EXISTS `private_tele`;
CREATE TABLE IF NOT EXISTS `private_tele` (
  `name` varchar(255) NOT NULL DEFAULT '',
  `position_x` float NOT NULL DEFAULT '0',
  `position_y` float NOT NULL DEFAULT '0',
  `position_z` float NOT NULL DEFAULT '0',
  `orientation` float NOT NULL DEFAULT '0',
  `map` int(10) unsigned NOT NULL DEFAULT '0',
  `id_bnet_account` int(10) unsigned NOT NULL DEFAULT '0' COMMENT 'OWNER Account Identifier',
  PRIMARY KEY (`name`,`id_bnet_account`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT='.f pteleport table';

-- Data exporting was unselected.
-- Dumping structure for table freedom.public_spell
DROP TABLE IF EXISTS `public_spell`;
CREATE TABLE IF NOT EXISTS `public_spell` (
  `spell_id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `allow_targeting` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `name` varchar(255) NOT NULL DEFAULT '',
  `id_bnet_gm` int(10) unsigned NOT NULL DEFAULT '0' COMMENT 'GM Account Identifier',
  PRIMARY KEY (`spell_id`)
) ENGINE=InnoDB AUTO_INCREMENT=45949 DEFAULT CHARSET=utf8;

-- Data exporting was unselected.
-- Dumping structure for table freedom.public_tele
DROP TABLE IF EXISTS `public_tele`;
CREATE TABLE IF NOT EXISTS `public_tele` (
  `name` varchar(255) NOT NULL DEFAULT '',
  `position_x` float NOT NULL DEFAULT '0',
  `position_y` float NOT NULL DEFAULT '0',
  `position_z` float NOT NULL DEFAULT '0',
  `orientation` float NOT NULL DEFAULT '0',
  `map` int(10) unsigned NOT NULL DEFAULT '0',
  `id_bnet_gm` int(10) unsigned NOT NULL DEFAULT '0' COMMENT 'GM Account Identifier',
  PRIMARY KEY (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT='.f teleport table';

-- Data exporting was unselected.
/*!40101 SET SQL_MODE=IFNULL(@OLD_SQL_MODE, '') */;
/*!40014 SET FOREIGN_KEY_CHECKS=IF(@OLD_FOREIGN_KEY_CHECKS IS NULL, 1, @OLD_FOREIGN_KEY_CHECKS) */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
