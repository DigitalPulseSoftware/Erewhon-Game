--
-- PostgreSQL database dump
--

-- Dumped from database version 10.3 (Debian 10.3-1.pgdg90+1)
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
-- Data for Name: collision_meshes; Type: TABLE DATA; Schema: public; Owner: lynix
--

INSERT INTO collision_meshes (id, file_path) VALUES (1, 'spaceship/spaceship.obj');
INSERT INTO collision_meshes (id, file_path) VALUES (2, 'gx7_interceptor/interceptor.obj');


--
-- Data for Name: modules; Type: TABLE DATA; Schema: public; Owner: lynix
--

INSERT INTO modules (id, class_name, class_info, name, description) VALUES (2, 'navigation', '{}', 'Basic navigation module', NULL);
INSERT INTO modules (id, class_name, class_info, name, description) VALUES (4, 'engine', '{}', 'Basic engine module', NULL);
INSERT INTO modules (id, class_name, class_info, name, description) VALUES (1, 'radar', '{
    "detectionRadius": 1000,
    "maxLockableTarget": 5
}', 'Basic radar module', NULL);
INSERT INTO modules (id, class_name, class_info, name, description) VALUES (3, 'weapon_plasmabeam', '{}', 'Plasma beam weapon module', NULL);
INSERT INTO modules (id, class_name, class_info, name, description) VALUES (5, 'weapon_torpedo', '{}', 'Torpedo weapon module', NULL);


--
-- Data for Name: spaceship_hulls; Type: TABLE DATA; Schema: public; Owner: lynix
--

INSERT INTO spaceship_hulls (id, name, description, collision_mesh, visual_mesh) VALUES (1, 'Default hull', NULL, 1, 1);


--
-- Data for Name: visual_meshes; Type: TABLE DATA; Schema: public; Owner: lynix
--

INSERT INTO visual_meshes (id, file_path) VALUES (1, 'spaceship/spaceship.obj');
INSERT INTO visual_meshes (id, file_path) VALUES (2, 'gx7_interceptor/interceptor.obj');
INSERT INTO visual_meshes (id, file_path) VALUES (3, 'ball/ball.obj');

--
-- Name: collision_mesh_id_seq; Type: SEQUENCE SET; Schema: public; Owner: lynix
--

SELECT pg_catalog.setval('collision_mesh_id_seq', 2, true);


--
-- Name: modules_id_seq; Type: SEQUENCE SET; Schema: public; Owner: lynix
--

SELECT pg_catalog.setval('modules_id_seq', 5, true);


--
-- Name: spaceship_hull_id_seq; Type: SEQUENCE SET; Schema: public; Owner: lynix
--

SELECT pg_catalog.setval('spaceship_hull_id_seq', 1, true);


--
-- Name: visual_meshes_id_seq; Type: SEQUENCE SET; Schema: public; Owner: lynix
--

SELECT pg_catalog.setval('visual_meshes_id_seq', 3, true);


--
-- PostgreSQL database dump complete
--

