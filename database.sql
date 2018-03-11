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

SET default_with_oids = false;

--
-- Name: account; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE account (
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

ALTER SEQUENCE account_id_seq OWNED BY account.id;


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
-- Name: spaceship; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE spaceship (
    id integer NOT NULL,
    name character varying(64) NOT NULL,
    script text,
    owner_id integer,
    last_update_date timestamp without time zone
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

ALTER SEQUENCE spaceship_id_seq OWNED BY spaceship.id;


--
-- Name: spaceship_modules; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE spaceship_modules (
    spaceship_id integer NOT NULL,
    module_id integer NOT NULL
);


--
-- Name: account id; Type: DEFAULT; Schema: public; Owner: -
--

ALTER TABLE ONLY account ALTER COLUMN id SET DEFAULT nextval('account_id_seq'::regclass);


--
-- Name: modules id; Type: DEFAULT; Schema: public; Owner: -
--

ALTER TABLE ONLY modules ALTER COLUMN id SET DEFAULT nextval('modules_id_seq'::regclass);


--
-- Name: spaceship id; Type: DEFAULT; Schema: public; Owner: -
--

ALTER TABLE ONLY spaceship ALTER COLUMN id SET DEFAULT nextval('spaceship_id_seq'::regclass);


--
-- Name: account account_email_key; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY account
    ADD CONSTRAINT account_email_key UNIQUE (email);


--
-- Name: account account_login_key; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY account
    ADD CONSTRAINT account_login_key UNIQUE (login);


--
-- Name: account account_pkey; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY account
    ADD CONSTRAINT account_pkey PRIMARY KEY (id);


--
-- Name: modules modules_pkey; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY modules
    ADD CONSTRAINT modules_pkey PRIMARY KEY (id);


--
-- Name: spaceship_modules spaceship_modules_pkey; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY spaceship_modules
    ADD CONSTRAINT spaceship_modules_pkey PRIMARY KEY (spaceship_id, module_id);


--
-- Name: spaceship spaceship_pkey; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY spaceship
    ADD CONSTRAINT spaceship_pkey PRIMARY KEY (id);


--
-- Name: spaceship_modules spaceship_modules_module_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY spaceship_modules
    ADD CONSTRAINT spaceship_modules_module_id_fkey FOREIGN KEY (module_id) REFERENCES modules(id) ON UPDATE CASCADE ON DELETE CASCADE;


--
-- Name: spaceship_modules spaceship_modules_spaceship_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY spaceship_modules
    ADD CONSTRAINT spaceship_modules_spaceship_id_fkey FOREIGN KEY (spaceship_id) REFERENCES spaceship(id) ON UPDATE CASCADE ON DELETE CASCADE;


--
-- Name: spaceship spaceship_owner_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY spaceship
    ADD CONSTRAINT spaceship_owner_id_fkey FOREIGN KEY (owner_id) REFERENCES account(id) ON UPDATE CASCADE ON DELETE CASCADE;


--
-- PostgreSQL database dump complete
--

