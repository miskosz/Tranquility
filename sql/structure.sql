-- phpMyAdmin SQL Dump
-- version 3.3.2deb1ubuntu1
-- http://www.phpmyadmin.net
--
-- Host: localhost
-- Generation Time: Apr 16, 2012 at 12:10 PM
-- Server version: 5.1.61
-- PHP Version: 5.3.2-1ubuntu4.14

SET FOREIGN_KEY_CHECKS=0;
SET SQL_MODE="NO_AUTO_VALUE_ON_ZERO";


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;

--
-- Database: `tranquility`
--

-- --------------------------------------------------------

--
-- Table structure for table `graph`
--

DROP TABLE IF EXISTS `graph`;
CREATE TABLE IF NOT EXISTS `graph` (
  `id_graph` int(11) NOT NULL AUTO_INCREMENT,
  `nvertices` int(11) NOT NULL,
  `ascii_code` tinytext NOT NULL,
  `black_faces` tinytext,
  `white_faces` tinytext,
  `last_computed` timestamp NULL DEFAULT NULL,
  PRIMARY KEY (`id_graph`)
) ENGINE=InnoDB  DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `triangulation`
--

DROP TABLE IF EXISTS `triangulation`;
CREATE TABLE IF NOT EXISTS `triangulation` (
  `id_triangulation` int(11) NOT NULL AUTO_INCREMENT,
  `id_graph` int(11) NOT NULL,
  `vertex_color` enum('black','white') DEFAULT NULL,
  `size` int(11) DEFAULT NULL,
  `degenerated` tinyint(1) DEFAULT NULL,
  `coordinates` tinytext,
  PRIMARY KEY (`id_triangulation`,`id_graph`),
  KEY `fk_triangulation_graph` (`id_graph`)
) ENGINE=InnoDB  DEFAULT CHARSET=utf8;

--
-- Constraints for dumped tables
--

--
-- Constraints for table `triangulation`
--
ALTER TABLE `triangulation`
  ADD CONSTRAINT `fk_triangulation_graph` FOREIGN KEY (`id_graph`) REFERENCES `graph` (`id_graph`) ON DELETE NO ACTION ON UPDATE NO ACTION;
SET FOREIGN_KEY_CHECKS=1;
