-- --------------------------------------------------------
-- Host:                         127.0.0.1
-- Server version:               5.6.19 - MySQL Community Server (GPL)
-- Server OS:                    Win64
-- HeidiSQL Version:             9.3.0.4984
-- --------------------------------------------------------

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET NAMES utf8mb4 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;

-- Dumping structure for table freedom.freedom_tele
DROP TABLE IF EXISTS `freedom_tele`;
CREATE TABLE IF NOT EXISTS `freedom_tele` (
  `id` mediumint(8) unsigned NOT NULL AUTO_INCREMENT,
  `name` varchar(100) NOT NULL DEFAULT '',
  `position_x` float NOT NULL DEFAULT '0',
  `position_y` float NOT NULL DEFAULT '0',
  `position_z` float NOT NULL DEFAULT '0',
  `orientation` float NOT NULL DEFAULT '0',
  `map` smallint(5) unsigned NOT NULL DEFAULT '0',
  `gm_uid` int(10) unsigned NOT NULL DEFAULT '0' COMMENT 'GM Account Identifier',
  PRIMARY KEY (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT='.f teleport table';

-- Data exporting was unselected.


-- Dumping structure for table freedom.user
DROP TABLE IF EXISTS `user`;
CREATE TABLE IF NOT EXISTS `user` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `username` varchar(32) NOT NULL,
  `email` varchar(255) NOT NULL DEFAULT '',
  `password_hash` varchar(40) NOT NULL,
  `email_confirm` tinyint(4) NOT NULL DEFAULT '0',
  `security_stamp` varchar(255) NOT NULL DEFAULT '',
  PRIMARY KEY (`id`),
  UNIQUE KEY `UNQ_USERNAME` (`username`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- Data exporting was unselected.
/*!40101 SET SQL_MODE=IFNULL(@OLD_SQL_MODE, '') */;
/*!40014 SET FOREIGN_KEY_CHECKS=IF(@OLD_FOREIGN_KEY_CHECKS IS NULL, 1, @OLD_FOREIGN_KEY_CHECKS) */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
