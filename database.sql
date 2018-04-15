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

SET default_with_oids = false;

--
-- Name: accounts; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE accounts (
    id integer NOT NULL,
    login character varying(64) NOT NULL,
    display_name character varying(64) NOT NULL,
    password character varying(64) NOT NULL,
    password_salt character varying(64) NOT NULL,
    email character varying(64) NOT NULL,
    creation_date timestamp without time zone NOT NULL,
    last_login_date timestamp without time zone,
    permission_level smallint DEFAULT 10 NOT NULL
);


--
-- Name: account_id_seq; Type: SEQUENCE; Schema: public; Owner: -
--

CREATE SEQUENCE account_id_seq
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


--
-- Name: account_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: -
--

ALTER SEQUENCE account_id_seq OWNED BY accounts.id;


--
-- Name: collision_meshes; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE collision_meshes (
    id integer NOT NULL,
    file_path character varying(255) NOT NULL
);


--
-- Name: collision_mesh_id_seq; Type: SEQUENCE; Schema: public; Owner: -
--

CREATE SEQUENCE collision_mesh_id_seq
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


--
-- Name: collision_mesh_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: -
--

ALTER SEQUENCE collision_mesh_id_seq OWNED BY collision_meshes.id;


--
-- Name: modules; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE modules (
    id integer NOT NULL,
    class_name character varying(64) NOT NULL,
    class_info json NOT NULL,
    name character varying(64) NOT NULL,
    description text
);


--
-- Name: modules_id_seq; Type: SEQUENCE; Schema: public; Owner: -
--

CREATE SEQUENCE modules_id_seq
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


--
-- Name: modules_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: -
--

ALTER SEQUENCE modules_id_seq OWNED BY modules.id;


--
-- Name: spaceship_hulls; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE spaceship_hulls (
    id integer NOT NULL,
    name character varying NOT NULL,
    description text,
    collision_mesh integer NOT NULL,
    visual_mesh integer NOT NULL
);


--
-- Name: spaceship_hull_id_seq; Type: SEQUENCE; Schema: public; Owner: -
--

CREATE SEQUENCE spaceship_hull_id_seq
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


--
-- Name: spaceship_hull_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: -
--

ALTER SEQUENCE spaceship_hull_id_seq OWNED BY spaceship_hulls.id;


--
-- Name: spaceships; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE spaceships (
    id integer NOT NULL,
    name character varying(64) NOT NULL,
    script text,
    owner_id integer,
    last_update_date timestamp without time zone,
    spaceship_hull_id integer
);


--
-- Name: spaceship_id_seq; Type: SEQUENCE; Schema: public; Owner: -
--

CREATE SEQUENCE spaceship_id_seq
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


--
-- Name: spaceship_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: -
--

ALTER SEQUENCE spaceship_id_seq OWNED BY spaceships.id;


--
-- Name: spaceship_modules; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE spaceship_modules (
    spaceship_id integer NOT NULL,
    module_id integer NOT NULL
);


--
-- Name: visual_meshes; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE visual_meshes (
    id integer NOT NULL,
    file_path character varying(128) NOT NULL
);


--
-- Name: visual_meshes_id_seq; Type: SEQUENCE; Schema: public; Owner: -
--

CREATE SEQUENCE visual_meshes_id_seq
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


--
-- Name: visual_meshes_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: -
--

ALTER SEQUENCE visual_meshes_id_seq OWNED BY visual_meshes.id;


--
-- Name: accounts id; Type: DEFAULT; Schema: public; Owner: -
--

ALTER TABLE ONLY accounts ALTER COLUMN id SET DEFAULT nextval('account_id_seq'::regclass);


--
-- Name: collision_meshes id; Type: DEFAULT; Schema: public; Owner: -
--

ALTER TABLE ONLY collision_meshes ALTER COLUMN id SET DEFAULT nextval('collision_mesh_id_seq'::regclass);


--
-- Name: modules id; Type: DEFAULT; Schema: public; Owner: -
--

ALTER TABLE ONLY modules ALTER COLUMN id SET DEFAULT nextval('modules_id_seq'::regclass);


--
-- Name: spaceship_hulls id; Type: DEFAULT; Schema: public; Owner: -
--

ALTER TABLE ONLY spaceship_hulls ALTER COLUMN id SET DEFAULT nextval('spaceship_hull_id_seq'::regclass);


--
-- Name: spaceships id; Type: DEFAULT; Schema: public; Owner: -
--

ALTER TABLE ONLY spaceships ALTER COLUMN id SET DEFAULT nextval('spaceship_id_seq'::regclass);


--
-- Name: visual_meshes id; Type: DEFAULT; Schema: public; Owner: -
--

ALTER TABLE ONLY visual_meshes ALTER COLUMN id SET DEFAULT nextval('visual_meshes_id_seq'::regclass);


--
-- Name: accounts account_email_key; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY accounts
    ADD CONSTRAINT account_email_key UNIQUE (email);


--
-- Name: accounts account_login_key; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY accounts
    ADD CONSTRAINT account_login_key UNIQUE (login);


--
-- Name: accounts account_pkey; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY accounts
    ADD CONSTRAINT account_pkey PRIMARY KEY (id);


--
-- Name: collision_meshes collision_mesh_pkey; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY collision_meshes
    ADD CONSTRAINT collision_mesh_pkey PRIMARY KEY (id);


--
-- Name: modules modules_pkey; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY modules
    ADD CONSTRAINT modules_pkey PRIMARY KEY (id);


--
-- Name: spaceship_hulls spaceship_hull_pkey; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY spaceship_hulls
    ADD CONSTRAINT spaceship_hull_pkey PRIMARY KEY (id);


--
-- Name: spaceship_modules spaceship_modules_pkey; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY spaceship_modules
    ADD CONSTRAINT spaceship_modules_pkey PRIMARY KEY (spaceship_id, module_id);


--
-- Name: spaceships spaceship_pkey; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY spaceships
    ADD CONSTRAINT spaceship_pkey PRIMARY KEY (id);


--
-- Name: visual_meshes visual_meshes_pkey; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY visual_meshes
    ADD CONSTRAINT visual_meshes_pkey PRIMARY KEY (id);


--
-- Name: spaceship_hulls spaceship_hull_collision_mesh_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY spaceship_hulls
    ADD CONSTRAINT spaceship_hull_collision_mesh_fkey FOREIGN KEY (collision_mesh) REFERENCES collision_meshes(id) ON UPDATE CASCADE;


--
-- Name: spaceship_modules spaceship_modules_module_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY spaceship_modules
    ADD CONSTRAINT spaceship_modules_module_id_fkey FOREIGN KEY (module_id) REFERENCES modules(id) ON UPDATE CASCADE ON DELETE CASCADE;


--
-- Name: spaceship_modules spaceship_modules_spaceship_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY spaceship_modules
    ADD CONSTRAINT spaceship_modules_spaceship_id_fkey FOREIGN KEY (spaceship_id) REFERENCES spaceships(id) ON UPDATE CASCADE ON DELETE CASCADE;


--
-- Name: spaceships spaceship_owner_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY spaceships
    ADD CONSTRAINT spaceship_owner_id_fkey FOREIGN KEY (owner_id) REFERENCES accounts(id) ON UPDATE CASCADE ON DELETE CASCADE;


--
-- Name: spaceships spaceship_spaceship_hull_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY spaceships
    ADD CONSTRAINT spaceship_spaceship_hull_id_fkey FOREIGN KEY (spaceship_hull_id) REFERENCES spaceship_hulls(id) ON UPDATE CASCADE ON DELETE SET NULL;


--
-- PostgreSQL database dump complete
--

