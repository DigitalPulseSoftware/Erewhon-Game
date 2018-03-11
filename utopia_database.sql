--
-- PostgreSQL database dump
--

-- Dumped from database version 10.1
-- Dumped by pg_dump version 10.1

SET statement_timeout = 0;
SET lock_timeout = 0;
SET idle_in_transaction_session_timeout = 0;
SET client_encoding = 'UTF8';
SET standard_conforming_strings = on;
SET check_function_bodies = false;
SET client_min_messages = warning;
SET row_security = off;

SET search_path = public, pg_catalog;

--
-- Data for Name: modules; Type: TABLE DATA; Schema: public; Owner: lynix
--

INSERT INTO modules (id, class_name, class_info, name, description) VALUES (2, 'navigation', '{}', 'Basic navigation module', NULL);
INSERT INTO modules (id, class_name, class_info, name, description) VALUES (3, 'weapon', '{}', 'Basic weapon module', NULL);
INSERT INTO modules (id, class_name, class_info, name, description) VALUES (4, 'engine', '{}', 'Basic engine module', NULL);
INSERT INTO modules (id, class_name, class_info, name, description) VALUES (1, 'radar', '{
    "detectionRadius": 1000,
    "maxLockableTarget": 5
}', 'Basic radar module', NULL);

--
-- Name: modules_id_seq; Type: SEQUENCE SET; Schema: public; Owner: lynix
--

SELECT pg_catalog.setval('modules_id_seq', 4, true);


--
-- Name: spaceship_id_seq; Type: SEQUENCE SET; Schema: public; Owner: lynix
--

SELECT pg_catalog.setval('spaceship_id_seq', 0, true);


--
-- PostgreSQL database dump complete
--

