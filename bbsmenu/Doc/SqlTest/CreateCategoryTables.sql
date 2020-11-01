--######################
--#####テーブル生成#####
--######################

--カテゴリ情報テーブル
CREATE TABLE CATEGORY_INFO_TABLE(
        ID                      INTEGER PRIMARY KEY AUTOINCREMENT,
        CATEGORY_NAME           TEXT    NOT NULL);

--板情報テーブル
CREATE TABLE BOARD_INFO_TABLE(
        ID                      INTEGER PRIMARY KEY AUTOINCREMENT,
        CATEGORY_ID             INTEGER REFERENCES CATEGORY_INFO_TABLE(ID),
        BOARD_NAME              TEXT    NOT NULL,
        THREAD_LIST_URL         TEXT    NOT NULL,
        BOARD_TYPE              INTEGER NOT NULL);

--無視カテゴリ情報テーブル
CREATE TABLE IGNORE_CATEGORY_INFO_TABLE(
        ID                      INTEGER PRIMARY KEY AUTOINCREMENT,
        IGNORE_CATEGORY_NAME    TEXT NOT NULL);

--無視板情報テーブル
CREATE TABLE IGNORE_BOARD_INFO_TABLE(
        ID                      INTEGER PRIMARY KEY AUTOINCREMENT,
        IGNORE_BOARD_NAME       TEXT    NOT NULL);

