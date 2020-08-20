﻿/*!
 * @brief ゲームデータ初期化2 / Initialization (part 2) -BEN-
 * @date 2014/01/28
 * @author
 * <pre>
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.  Other copyrights may also apply.
 * 2014 Deskull rearranged comment for Doxygen.\n
 * </pre>
 * @details
 * <pre>
 * This file is used to initialize various variables and arrays for the
 * Angband game.  Note the use of "fd_read()" and "fd_write()" to bypass
 * the common limitation of "read()" and "write()" to only 32767 bytes
 * at a time.
 * Several of the arrays for Angband are built from "template" files in
 * the "lib/file" directory, from which quick-load binary "image" files
 * are constructed whenever they are not present in the "lib/data"
 * directory, or if those files become obsolete, if we are allowed.
 * Warning -- the "ascii" file parsers use a minor hack to collect the
 * name and text information in a single pass.  Thus, the game will not
 * be able to load any template file with more than 20K of names or 60K
 * of text, even though technically, up to 64K should be legal.
 * The "init1.c" file is used only to parse the ascii template files,
 * to create the binary image files.  Note that the binary image files
 * are extremely system dependant.
 * </pre>
 */

#include "main/init.h"
#include "cmd-building/cmd-building.h"
#include "cmd-io/macro-util.h"
#include "info-reader/fixed-map-parser.h" // 相互参照、後で何とかする.
#include "dungeon/dungeon.h"
#include "dungeon/quest.h"
#include "floor/floor-town.h"
#include "floor/floor-util.h"
#include "floor/wild.h"
#include "game-option/option-flags.h"
#include "game-option/option-types-table.h"
#include "grid/feature.h"
#include "grid/grid.h"
#include "grid/trap.h"
#include "info-reader/artifact-reader.h"
#include "info-reader/dungeon-reader.h"
#include "info-reader/ego-reader.h"
#include "info-reader/feature-reader.h"
#include "info-reader/general-parser.h"
#include "info-reader/kind-reader.h"
#include "info-reader/magic-reader.h"
#include "info-reader/race-reader.h"
#include "info-reader/skill-reader.h"
#include "info-reader/parse-error-types.h"
#include "info-reader/vault-reader.h"
#include "io/files-util.h"
#include "io/read-pref-file.h"
#include "io/uid-checker.h"
#include "market/articles-on-sale.h"
#include "monster-race/monster-race.h"
#include "monster-race/race-flags7.h"
#include "object-enchant/object-ego.h"
#include "object/object-kind.h"
#include "player/player-class.h"
#include "player/player-skill.h"
#include "room/rooms-builder.h"
#include "room/rooms-vault.h"
#include "store/store-util.h"
#include "store/store.h"
#include "system/alloc-entries.h"
#include "system/angband-version.h"
#include "system/artifact-type-definition.h"
#include "system/building-type-definition.h"
#include "system/floor-type-definition.h"
#include "system/system-variables.h"
#include "term/gameterm.h"
#include "term/screen-processor.h"
#include "term/term-color-types.h"
#include "util/angband-files.h"
#include "util/quarks.h"
#include "util/string-processor.h"
#include "util/tag-sorter.h"
#include "view/display-messages.h"
#include "world/world.h"

#include <sys/stat.h>
#include <sys/types.h>

/*!
 * @brief マクロ登録の最大数 / Maximum number of macros (see "io.c")
 * @note Default: assume at most 256 macros are used
 */
static const int MACRO_MAX = 256;

static void put_title(void);

/*!
 * @brief 各データファイルを読み取るためのパスを取得する
 * Set the default paths to all of our important sub-directories.
 * @param libpath パス保管先の文字列
 * @param varpath Is the base path for directories that have files which
 * are not read-only: ANGBAND_DIR_APEX, ANGBAND_DIR_BONE, ANGBAND_DIR_DATA,
 * and ANGBAND_DIR_SAVE.  If the PRIVATE_USER_PATH preprocessor macro has not
 * been set, it is also used as the base path for ANGBAND_DIR_USER.
 * @return なし
 * @details
 * <pre>
 * The purpose of each sub-directory is described in "files.c".
 * The traditional behavior was to put all of the sub-directories within
 * one directory, "lib".  To get that behavior, pass the same string for
 * libpath and varpath.  Further customization may be done later in response
 * to command line options (most importantly for the "info", "user", and
 * "save" directories), but that is done after this function:  see
 * "change_path()" in "main.c".  libpath and varpath should end in the
 * appropriate "PATH_SEP" string.  All of the "sub-directory" paths
 * (created below or supplied by the user) will NOT end in the "PATH_SEP"
 * string, see the special "path_build()" function in "util.c" for more
 * information.
 * Hack -- first we free all the strings, since this is known
 * to succeed even if the strings have not been allocated yet,
 * as long as the variables start out as "NULL".  This allows
 * this function to be called multiple times, for example, to
 * try several base "path" values until a good one is found.
 * </pre>
 */
void init_file_paths(concptr libpath, concptr varpath)
{
#ifdef PRIVATE_USER_PATH
	char base[1024];
	char buf[1024];
#endif /* PRIVATE_USER_PATH */

	/*** Free everything ***/

	/* Free the main path */
	string_free(ANGBAND_DIR);

	/* Free the sub-paths */
	string_free(ANGBAND_DIR_APEX);
	string_free(ANGBAND_DIR_BONE);
	string_free(ANGBAND_DIR_DATA);
	string_free(ANGBAND_DIR_EDIT);
	string_free(ANGBAND_DIR_SCRIPT);
	string_free(ANGBAND_DIR_FILE);
	string_free(ANGBAND_DIR_HELP);
	string_free(ANGBAND_DIR_INFO);
	string_free(ANGBAND_DIR_SAVE);
	string_free(ANGBAND_DIR_USER);
	string_free(ANGBAND_DIR_XTRA);

	/*** Prepare the "path" ***/

	/* Hack -- save the main directory */
	ANGBAND_DIR = string_make(libpath);

	/*** Build the sub-directory names ***/

	ANGBAND_DIR_APEX = string_make(format("%sapex", varpath));
	ANGBAND_DIR_BONE = string_make(format("%sbone", varpath));
	ANGBAND_DIR_DATA = string_make(format("%sdata", varpath));
	ANGBAND_DIR_EDIT = string_make(format("%sedit", libpath));
	ANGBAND_DIR_SCRIPT = string_make(format("%sscript", libpath));
	ANGBAND_DIR_FILE = string_make(format("%sfile", libpath));
	ANGBAND_DIR_HELP = string_make(format("%shelp", libpath));
	ANGBAND_DIR_INFO = string_make(format("%sinfo", libpath));
	ANGBAND_DIR_PREF = string_make(format("%spref", libpath));
	ANGBAND_DIR_SAVE = string_make(format("%ssave", varpath));

#ifdef PRIVATE_USER_PATH
	/* Build the path to the user specific directory */
	path_parse(base, sizeof(base), PRIVATE_USER_PATH);
	path_build(buf, sizeof(buf), base, VERSION_NAME);

	ANGBAND_DIR_USER = string_make(buf);
#else /* PRIVATE_USER_PATH */
	ANGBAND_DIR_USER = string_make(format("%suser", varpath));
#endif /* PRIVATE_USER_PATH */

	ANGBAND_DIR_XTRA = string_make(format("%sxtra", libpath));
}


/*
 * Helper function for create_needed_dirs().  Copied over from PosChengband.
 */
bool dir_exists(concptr path)
{
    struct stat buf;
    if (stat(path, &buf) != 0)
	return FALSE;
#ifdef WIN32
    else if (buf.st_mode & S_IFDIR)
#else
    else if (S_ISDIR(buf.st_mode))
#endif
	return TRUE;
    else
	return FALSE;
}


/*
 * Helper function for create_needed_dirs().  Copied over from PosChengband
 * but use the global definition for the path separator rather than a local
 * one in PosChengband's code and check for paths that end with the path
 * separator.
 */
bool dir_create(concptr path)
{
#ifdef WIN32
    /* If the directory already exists then we're done */
    if (dir_exists(path)) return TRUE;
    return FALSE;
#else
    const char *ptr;
    char buf[1024];

    /* If the directory already exists then we're done */
    if (dir_exists(path)) return TRUE;
    /* Iterate through the path looking for path segements. At each step,
     * create the path segment if it doesn't already exist. */
    for (ptr = path; *ptr; ptr++)
        {
	    if (*ptr == PATH_SEP[0])
                {
		    /* Find the length of the parent path string */
		    size_t len = (size_t)(ptr - path);

		    /* Skip the initial slash */
		    if (len == 0) continue;
		    /* If this is a duplicate path separator, continue */
		    if (*(ptr - 1) == PATH_SEP[0]) continue;

		    /* We can't handle really big filenames */
		    if (len - 1 > 512) return FALSE;

		    /* Create the parent path string, plus null-padding */
		    angband_strcpy(buf, path, len + 1);

		    /* Skip if the parent exists */
		    if (dir_exists(buf)) continue;

		    /* The parent doesn't exist, so create it or fail */
		    if (mkdir(buf, 0755) != 0) return FALSE;
                }
        }
    /*
     * The path ends on a path separator so have created it already in
     * the loop above.
     */
    if (*(ptr-1) == PATH_SEP[0])
	{
	    return TRUE;
	}
    return mkdir(path, 0755) == 0 ? TRUE : FALSE;
#endif
}


/*
 * Create any missing directories. We create only those dirs which may be
 * empty (user/, save/, apex/, bone/, data/). Only user/ is created when
 * the PRIVATE_USER_PATH preprocessor macro has been set. The others are
 * assumed to contain required files and therefore must exist at startup
 * (edit/, pref/, file/, xtra/).
 *
 * ToDo: Only create the directories when actually writing files.
 * Copied over from PosChengband to support main-cocoa.m.  Dropped
 * creation of help/ (and removed it and info/ in the comment)
 * since init_file_paths() puts those in libpath which may not be writable
 * by the user running the application.  Added bone/ since
 * init_file_paths() puts that in varpath.
 */
void create_needed_dirs(void)
{
    char dirpath[1024];

    path_build(dirpath, sizeof(dirpath), ANGBAND_DIR_USER, "");
    if (!dir_create(dirpath)) quit_fmt("Cannot create '%s'", dirpath);

#ifndef PRIVATE_USER_PATH
    path_build(dirpath, sizeof(dirpath), ANGBAND_DIR_SAVE, "");
    if (!dir_create(dirpath)) quit_fmt("Cannot create '%s'", dirpath);

    path_build(dirpath, sizeof(dirpath), ANGBAND_DIR_APEX, "");
    if (!dir_create(dirpath)) quit_fmt("Cannot create '%s'", dirpath);

    path_build(dirpath, sizeof(dirpath), ANGBAND_DIR_BONE, "");
    if (!dir_create(dirpath)) quit_fmt("Cannot create '%s'", dirpath);

    path_build(dirpath, sizeof(dirpath), ANGBAND_DIR_DATA, "");
    if (!dir_create(dirpath)) quit_fmt("Cannot create '%s'", dirpath);
#endif /* ndef PRIVATE_USER_PATH */
}


/*
 * Hack -- help give useful error messages
 */
int error_idx; /*!< データ読み込み/初期化時に汎用的にエラーコードを保存するグローバル変数 */
int error_line; /*!< データ読み込み/初期化時に汎用的にエラー行数を保存するグローバル変数 */

/*!
 * エラーメッセージの名称定義 / Standard error message text
 */
concptr err_str[PARSE_ERROR_MAX] =
{
	NULL,
#ifdef JP
	"文法エラー",
	"古いファイル",
	"記録ヘッダがない",
	"不連続レコード",
	"おかしなフラグ存在",
	"未定義命令",
	"メモリ不足",
	"座標範囲外",
	"引数不足",
	"未定義地形タグ",
#else
	"parse error",
	"obsolete file",
	"missing record header",
	"non-sequential records",
	"invalid flag specification",
	"undefined directive",
	"out of memory",
	"coordinates out of bounds",
	"too few arguments",
	"undefined terrain tag",
#endif

};

/*
 * File headers
 */
static angband_header v_head; /*!< Vault情報のヘッダ構造体 */
static angband_header k_head; /*!< ペースアイテム情報のヘッダ構造体 */
static angband_header a_head; /*!< 固定アーティファクト情報のヘッダ構造体 */
static angband_header e_head; /*!< アイテムエゴ情報のヘッダ構造体 */
static angband_header r_head; /*!< モンスター種族情報のヘッダ構造体 */
static angband_header d_head; /*!< ダンジョン情報のヘッダ構造体 */
static angband_header s_head; /*!< プレイヤー職業技能情報のヘッダ構造体 */
static angband_header m_head; /*!< プレイヤー職業魔法情報のヘッダ構造体 */

/*!
 * @brief テキストファイルとrawファイルの更新時刻を比較する
 * Find the default paths to all of our important sub-directories.
 * @param fd ファイルディスクリプタ
 * @param template_file ファイル名
 * @return テキストの方が新しいか、rawファイルがなく更新の必要がある場合-1、更新の必要がない場合0。
 */
static errr check_modification_date(int fd, concptr template_file)
{
	struct stat txt_stat, raw_stat;
	char buf[1024];
	path_build(buf, sizeof(buf), ANGBAND_DIR_EDIT, template_file);

	/* Access stats on text file */
	if (stat(buf, &txt_stat))
	{
		return 0;
	}

	/* Access stats on raw file */
	if (fstat(fd, &raw_stat))
	{
		return -1;
	}

	/* Ensure text file is not newer than raw file */
	if (txt_stat.st_mtime > raw_stat.st_mtime)
	{
		return -1;
	}

	return 0;
}


/*** Initialize from binary image files ***/

/*!
 * @brief rawファイルからのデータの読み取り処理
 * Initialize the "*_info" array, by parsing a binary "image" file
 * @param fd ファイルディスクリプタ
 * @param head rawファイルのヘッダ
 * @return エラーコード
 */
static errr init_info_raw(int fd, angband_header *head)
{
	angband_header test;

	/* Read and Verify the header */
	if (fd_read(fd, (char*)(&test), sizeof(angband_header)) ||
		(test.v_major != head->v_major) ||
		(test.v_minor != head->v_minor) ||
		(test.v_patch != head->v_patch) ||
		(test.info_num != head->info_num) ||
		(test.info_len != head->info_len) ||
		(test.head_size != head->head_size) ||
		(test.info_size != head->info_size))
	{
		/* Error */
		return -1;
	}

	/* Accept the header */
	(*head) = test;

	/* Allocate the "*_info" array */
	C_MAKE(head->info_ptr, head->info_size, char);

	/* Read the "*_info" array */
	fd_read(fd, head->info_ptr, head->info_size);

	if (head->name_size)
	{
		/* Allocate the "*_name" array */
		C_MAKE(head->name_ptr, head->name_size, char);

		/* Read the "*_name" array */
		fd_read(fd, head->name_ptr, head->name_size);
	}

	if (head->text_size)
	{
		/* Allocate the "*_text" array */
		C_MAKE(head->text_ptr, head->text_size, char);

		/* Read the "*_text" array */
		fd_read(fd, head->text_ptr, head->text_size);
	}

	if (head->tag_size)
	{
		/* Allocate the "*_tag" array */
		C_MAKE(head->tag_ptr, head->tag_size, char);

		/* Read the "*_tag" array */
		fd_read(fd, head->tag_ptr, head->tag_size);
	}

	return 0;
}



/*!
 * @brief ヘッダ構造体の更新
 * Initialize the header of an *_info.raw file.
 * @param head rawファイルのヘッダ
 * @param num データ数
 * @param len データの長さ
 * @return エラーコード
 */
static void init_header(angband_header *head, IDX num, int len)
{
	/* Save the "version" */
	head->v_major = FAKE_VER_MAJOR;
	head->v_minor = FAKE_VER_MINOR;
	head->v_patch = FAKE_VER_PATCH;
	head->v_extra = 0;

	/* Save the "record" information */
	head->info_num = (IDX)num;
	head->info_len = len;

	/* Save the size of "*_head" and "*_info" */
	head->head_size = sizeof(angband_header);
	head->info_size = head->info_num * head->info_len;
}


static void update_header(angband_header *head, void **info, char **name, char **text, char **tag)
{
	if (info) *info = head->info_ptr;
	if (name) *name = head->name_ptr;
	if (text) *text = head->text_ptr;
	if (tag)  *tag = head->tag_ptr;
}


/*!
 * @brief ヘッダ構造体の更新
 * Initialize the "*_info" array
 * @param filename ファイル名(拡張子txt/raw)
 * @param head 処理に用いるヘッダ構造体
 * @param info データ保管先の構造体ポインタ
 * @param name 名称用可変文字列の保管先
 * @param text テキスト用可変文字列の保管先
 * @param tag タグ用可変文字列の保管先
 * @return エラーコード
 * @note
 * Note that we let each entry have a unique "name" and "text" string,
 * even if the string happens to be empty (everyone has a unique '\0').
 */
static errr init_info(player_type *player_ptr, concptr filename, angband_header *head, void **info, char **name, char **text, char **tag)
{
	/* General buffer */
	char buf[1024];

	/*** Load the binary image file ***/
	path_build(buf, sizeof(buf), ANGBAND_DIR_DATA, format(_("%s_j.raw", "%s.raw"), filename));

	/* Attempt to open the "raw" file */
	int fd = fd_open(buf, O_RDONLY);

	/* Process existing "raw" file */
	errr err = 1;
	if (fd >= 0)
	{
		err = check_modification_date(fd, format("%s.txt", filename));

		/* Attempt to parse the "raw" file */
		if (!err)
			err = init_info_raw(fd, head);
		(void)fd_close(fd);
	}

	/* Do we have to parse the *.txt file? */
	BIT_FLAGS file_permission = 0644;
	if (err == 0)
	{
		update_header(head, info, name, text, tag);
		return 0;
	}

	/*** Make the fake arrays ***/
	C_MAKE(head->info_ptr, head->info_size, char);

	/* Hack -- make "fake" arrays */
	if (name) C_MAKE(head->name_ptr, FAKE_NAME_SIZE, char);
	if (text) C_MAKE(head->text_ptr, FAKE_TEXT_SIZE, char);
	if (tag)  C_MAKE(head->tag_ptr, FAKE_TAG_SIZE, char);

	if (info) (*info) = head->info_ptr;
	if (name) (*name) = head->name_ptr;
	if (text) (*text) = head->text_ptr;
	if (tag)  (*tag) = head->tag_ptr;

	/*** Load the ascii template file ***/
	path_build(buf, sizeof(buf), ANGBAND_DIR_EDIT, format("%s.txt", filename));
	FILE *fp;
	fp = angband_fopen(buf, "r");

	/* Parse it */
	if (!fp) quit(format(_("'%s.txt'ファイルをオープンできません。", "Cannot open '%s.txt' file."), filename));

	/* Parse the file */
	err = init_info_txt(fp, buf, head, head->parse_info_txt);
	angband_fclose(fp);

	/* Errors */
	if (err)
	{
		concptr oops;

#ifdef JP
		/* Error string */
		oops = (((err > 0) && (err < PARSE_ERROR_MAX)) ? err_str[err] : "未知の");

		msg_format("'%s.txt'ファイルの %d 行目にエラー。", filename, error_line);
		msg_format("レコード %d は '%s' エラーがあります。", error_idx, oops);
		msg_format("構文 '%s'。", buf);
		msg_print(NULL);

		/* Quit */
		quit(format("'%s.txt'ファイルにエラー", filename));
#else
		/* Error string */
		oops = (((err > 0) && (err < PARSE_ERROR_MAX)) ? err_str[err] : "unknown");

		msg_format("Error %d at line %d of '%s.txt'.", err, error_line, filename);
		msg_format("Record %d contains a '%s' error.", error_idx, oops);
		msg_format("Parsing '%s'.", buf);
		msg_print(NULL);

		/* Quit */
		quit(format("Error in '%s.txt' file.", filename));
#endif
	}

	/*** Make final retouch on fake tags ***/
	if (head->retouch)
	{
		(*head->retouch)(head);
	}

	/*** Dump the binary image file ***/
	path_build(buf, sizeof(buf), ANGBAND_DIR_DATA, format(_("%s_j.raw", "%s.raw"), filename));

	/* Grab permissions */
	safe_setuid_grab(player_ptr);

	/* Kill the old file */
	(void)fd_kill(buf);

	/* Attempt to create the raw file */
	fd = fd_make(buf, file_permission);

	/* Drop permissions */
	safe_setuid_drop();

	/* Dump to the file */
	if (fd >= 0)
	{
		/* Dump it */
		fd_write(fd, (concptr)(head), head->head_size);

		/* Dump the "*_info" array */
		fd_write(fd, head->info_ptr, head->info_size);

		/* Dump the "*_name" array */
		fd_write(fd, head->name_ptr, head->name_size);

		/* Dump the "*_text" array */
		fd_write(fd, head->text_ptr, head->text_size);

		/* Dump the "*_tag" array */
		fd_write(fd, head->tag_ptr, head->tag_size);

		/* Close */
		(void)fd_close(fd);
	}

	/*** Kill the fake arrays ***/

	/* Free the "*_info" array */
	C_KILL(head->info_ptr, head->info_size, char);

	/* Hack -- Free the "fake" arrays */
	if (name) C_KILL(head->name_ptr, FAKE_NAME_SIZE, char);
	if (text) C_KILL(head->text_ptr, FAKE_TEXT_SIZE, char);
	if (tag)  C_KILL(head->tag_ptr, FAKE_TAG_SIZE, char);

	/*** Load the binary image file ***/
	path_build(buf, sizeof(buf), ANGBAND_DIR_DATA, format(_("%s_j.raw", "%s.raw"), filename));

	/* Attempt to open the "raw" file */
	fd = fd_open(buf, O_RDONLY);

	/* Process existing "raw" file */
	if (fd < 0) quit(format(_("'%s_j.raw'ファイルをロードできません。", "Cannot load '%s.raw' file."), filename));

	/* Attempt to parse the "raw" file */
	err = init_info_raw(fd, head);
	(void)fd_close(fd);

	/* Error */
	if (err) quit(format(_("'%s_j.raw'ファイルを解析できません。", "Cannot parse '%s.raw' file."), filename));

	update_header(head, info, name, text, tag);
	return 0;
}


/*!
 * @brief 地形情報読み込みのメインルーチン /
 * Initialize the "f_info" array
 * @return エラーコード
 */
static errr init_f_info(player_type *player_ptr)
{
	/* Init the header */
	init_header(&f_head, max_f_idx, sizeof(feature_type));

	/* Save a pointer to the parsing function */
	f_head.parse_info_txt = parse_f_info;

	/* Save a pointer to the retouch fake tags */
	f_head.retouch = retouch_f_info;

	return init_info(player_ptr, "f_info", &f_head,
		(void*)&f_info, &f_name, NULL, &f_tag);
}


/*!
 * @brief ベースアイテム情報読み込みのメインルーチン /
 * Initialize the "k_info" array
 * @return エラーコード
 */
static errr init_k_info(player_type *player_ptr)
{
	/* Init the header */
	init_header(&k_head, max_k_idx, sizeof(object_kind));

	/* Save a pointer to the parsing function */
	k_head.parse_info_txt = parse_k_info;

	return init_info(player_ptr, "k_info", &k_head,
		(void*)&k_info, &k_name, &k_text, NULL);
}


/*!
 * @brief 固定アーティファクト情報読み込みのメインルーチン /
 * Initialize the "a_info" array
 * @return エラーコード
 */
static errr init_a_info(player_type *player_ptr)
{
	/* Init the header */
	init_header(&a_head, max_a_idx, sizeof(artifact_type));

	/* Save a pointer to the parsing function */
	a_head.parse_info_txt = parse_a_info;

	return init_info(player_ptr, "a_info", &a_head,
		(void*)&a_info, &a_name, &a_text, NULL);
}


/*!
 * @brief 固定アーティファクト情報読み込みのメインルーチン /
 * Initialize the "e_info" array
 * @return エラーコード
 */
static errr init_e_info(player_type *player_ptr)
{
	/* Init the header */
	init_header(&e_head, max_e_idx, sizeof(ego_item_type));

	/* Save a pointer to the parsing function */
	e_head.parse_info_txt = parse_e_info;

	return init_info(player_ptr, "e_info", &e_head,
		(void*)&e_info, &e_name, &e_text, NULL);
}


/*!
 * @brief モンスター種族情報読み込みのメインルーチン /
 * Initialize the "r_info" array
 * @return エラーコード
 */
static errr init_r_info(player_type *player_ptr)
{
	/* Init the header */
	init_header(&r_head, max_r_idx, sizeof(monster_race));

	/* Save a pointer to the parsing function */
	r_head.parse_info_txt = parse_r_info;

	return init_info(player_ptr, "r_info", &r_head,
		(void*)&r_info, &r_name, &r_text, NULL);
}


/*!
 * @brief ダンジョン情報読み込みのメインルーチン /
 * Initialize the "d_info" array
 * @return エラーコード
 */
static errr init_d_info(player_type *player_ptr)
{
	/* Init the header */
	init_header(&d_head, current_world_ptr->max_d_idx, sizeof(dungeon_type));

	/* Save a pointer to the parsing function */
	d_head.parse_info_txt = parse_d_info;

	return init_info(player_ptr, "d_info", &d_head,
		(void*)&d_info, &d_name, &d_text, NULL);
}


/*!
 * @brief Vault情報読み込みのメインルーチン /
 * Initialize the "v_info" array
 * @return エラーコード
 * @note
 * Note that we let each entry have a unique "name" and "text" string,
 * even if the string happens to be empty (everyone has a unique '\0').
 */
errr init_v_info(player_type *player_ptr)
{
	/* Init the header */
	init_header(&v_head, max_v_idx, sizeof(vault_type));

	/* Save a pointer to the parsing function */
	v_head.parse_info_txt = parse_v_info;

	return init_info(player_ptr, "v_info", &v_head,
		(void*)&v_info, &v_name, &v_text, NULL);
}


/*!
 * @brief 職業技能情報読み込みのメインルーチン /
 * Initialize the "s_info" array
 * @return エラーコード
 */
static errr init_s_info(player_type *player_ptr)
{
	/* Init the header */
	init_header(&s_head, MAX_CLASS, sizeof(skill_table));

	/* Save a pointer to the parsing function */
	s_head.parse_info_txt = parse_s_info;

	return init_info(player_ptr, "s_info", &s_head,
		(void*)&s_info, NULL, NULL, NULL);
}


/*!
 * @brief 職業魔法情報読み込みのメインルーチン /
 * Initialize the "m_info" array
 * @return エラーコード
 */
static errr init_m_info(player_type *player_ptr)
{
	/* Init the header */
	init_header(&m_head, MAX_CLASS, sizeof(player_magic));

	/* Save a pointer to the parsing function */
	m_head.parse_info_txt = parse_m_info;

	return init_info(player_ptr, "m_info", &m_head,
		(void*)&m_info, NULL, NULL, NULL);
}


/*!
 * @brief 基本情報読み込みのメインルーチン /
 * Initialize misc. values
 * @param player_ptr プレーヤーへの参照ポインタ
 * @return エラーコード
 */
static errr init_misc(player_type *player_ptr)
{
	return parse_fixed_map(player_ptr, "misc.txt", 0, 0, 0, 0);
}


/*!
 * @brief 町情報読み込みのメインルーチン /
 * Initialize town array
 * @return エラーコード
 */
static errr init_towns(void)
{
	/* Allocate the towns */
	C_MAKE(town_info, max_towns, town_type);

	for (int i = 1; i < max_towns; i++)
	{
		/*** Prepare the Stores ***/

		/* Allocate the stores */
		C_MAKE(town_info[i].store, MAX_STORES, store_type);

		/* Fill in each store */
		for (int j = 0; j < MAX_STORES; j++)
		{
			/* Access the store */
			store_type *store_ptr = &town_info[i].store[j];

			if ((i > 1) && (j == STORE_MUSEUM || j == STORE_HOME)) continue;

			/* Assume full stock */

			/*
			 * 我が家が 20 ページまで使える隠し機能のための準備。
			 * オプションが有効でもそうでなくても一応スペースを作っておく。
			 */
			if (j == STORE_HOME)
			{
				store_ptr->stock_size = (STORE_INVEN_MAX * 10);
			}
			else if (j == STORE_MUSEUM)
			{
				store_ptr->stock_size = (STORE_INVEN_MAX * 50);
			}
			else
			{
				store_ptr->stock_size = STORE_INVEN_MAX;
			}

			/* Allocate the stock */
			C_MAKE(store_ptr->stock, store_ptr->stock_size, object_type);

			/* No table for the black market or home */
			if ((j == STORE_BLACK) || (j == STORE_HOME) || (j == STORE_MUSEUM)) continue;

			/* Assume full table */
			store_ptr->table_size = STORE_CHOICES;

			/* Allocate the stock */
			C_MAKE(store_ptr->table, store_ptr->table_size, s16b);

			/* Scan the choices */
			for (int k = 0; k < STORE_CHOICES; k++)
			{
				KIND_OBJECT_IDX k_idx;

				/* Extract the tval/sval codes */
				int tv = store_table[j][k][0];
				int sv = store_table[j][k][1];

				/* Look for it */
				for (k_idx = 1; k_idx < max_k_idx; k_idx++)
				{
					object_kind *k_ptr = &k_info[k_idx];

					/* Found a match */
					if ((k_ptr->tval == tv) && (k_ptr->sval == sv)) break;
				}

				/* Catch errors */
				if (k_idx == max_k_idx) continue;

				/* Add that item index to the table */
				store_ptr->table[store_ptr->table_num++] = k_idx;
			}
		}
	}

	return 0;
}

/*!
 * @brief 店情報初期化のメインルーチン /
 * Initialize buildings
 * @return エラーコード
 */
errr init_buildings(void)
{
	for (int i = 0; i < MAX_BLDG; i++)
	{
		building[i].name[0] = '\0';
		building[i].owner_name[0] = '\0';
		building[i].owner_race[0] = '\0';

		for (int j = 0; j < 8; j++)
		{
			building[i].act_names[j][0] = '\0';
			building[i].member_costs[j] = 0;
			building[i].other_costs[j] = 0;
			building[i].letters[j] = 0;
			building[i].actions[j] = 0;
			building[i].action_restr[j] = 0;
		}

		for (int j = 0; j < MAX_CLASS; j++)
		{
			building[i].member_class[j] = 0;
		}

		for (int j = 0; j < MAX_RACES; j++)
		{
			building[i].member_race[j] = 0;
		}

		for (int j = 0; j < MAX_MAGIC + 1; j++)
		{
			building[i].member_realm[j] = 0;
		}
	}

	return 0;
}


/*!
 * @brief クエスト情報初期化のメインルーチン /
 * Initialize quest array
 * @return エラーコード
 */
static errr init_quests(void)
{
	/* Allocate the quests */
	C_MAKE(quest, max_q_idx, quest_type);

	/* Set all quest to "untaken" */
	for (int i = 0; i < max_q_idx; i++)
	{
		quest[i].status = QUEST_STATUS_UNTAKEN;
	}

	return 0;
}

/*!
 * @brief その他の初期情報更新 /
 * Initialize some other arrays
 * @return エラーコード
 */
static errr init_other(player_type *player_ptr)
{
	player_ptr->current_floor_ptr = &floor_info; // TODO:本当はこんなところで初期化したくない
	floor_type *floor_ptr = player_ptr->current_floor_ptr;

	/*** Prepare the "dungeon" information ***/

	/* Allocate and Wipe the object list */
	C_MAKE(floor_ptr->o_list, current_world_ptr->max_o_idx, object_type);

	/* Allocate and Wipe the monster list */
	C_MAKE(floor_ptr->m_list, current_world_ptr->max_m_idx, monster_type);

	/* Allocate and Wipe the monster process list */
	for (int i = 0; i < MAX_MTIMED; i++)
	{
		C_MAKE(floor_ptr->mproc_list[i], current_world_ptr->max_m_idx, s16b);
	}

	/* Allocate and Wipe the max dungeon level */
	C_MAKE(max_dlv, current_world_ptr->max_d_idx, DEPTH);

	for (int i = 0; i < MAX_HGT; i++)
	{
		C_MAKE(floor_ptr->grid_array[i], MAX_WID, grid_type);
	}

	/*** Prepare the various "bizarre" arrays ***/

	/* Macro variables */
	C_MAKE(macro__pat, MACRO_MAX, concptr);
	C_MAKE(macro__act, MACRO_MAX, concptr);
	C_MAKE(macro__cmd, MACRO_MAX, bool);

	/* Macro action buffer */
	C_MAKE(macro__buf, 1024, char);

	/* Quark variables */
	quark_init();

	/* Message variables */
	C_MAKE(message__ptr, MESSAGE_MAX, u32b);
	C_MAKE(message__buf, MESSAGE_BUF, char);

	/* Hack -- No messages yet */
	message__tail = MESSAGE_BUF;

	/*** Prepare the options ***/

	/* Scan the options */
	for (int i = 0; option_info[i].o_desc; i++)
	{
		int os = option_info[i].o_set;
		int ob = option_info[i].o_bit;

		/* Set the "default" options */
		if (!option_info[i].o_var) continue;

		/* Accept */
		option_mask[os] |= (1L << ob);

		/* Set */
		if (option_info[i].o_norm)
		{
			/* Set */
			option_flag[os] |= (1L << ob);
		}
		else
		{
			option_flag[os] &= ~(1L << ob);
		}
	}

	/* Analyze the windows */
	for (int n = 0; n < 8; n++)
	{
		/* Analyze the options */
		for (int i = 0; i < 32; i++)
		{
			/* Accept */
			if (window_flag_desc[i])
			{
				/* Accept */
				window_mask[n] |= (1L << i);
			}
		}
	}

	/*
	 *  Set the "default" window flags
	 *  Window 1 : Display messages
	 *  Window 2 : Display inven/equip
	 */
	window_flag[1] = 1L << A_MAX;
	window_flag[2] = 1L << 0;

	/*** Pre-allocate space for the "format()" buffer ***/

	/* Hack -- Just call the "format()" function */
	(void)format("%s (%s).", "Mr.Hoge", MAINTAINER);
	return 0;
}


/*!
 * @brief オブジェクト配列を初期化する /
 * Initialize some other arrays
 * @return エラーコード
 */
static errr init_object_alloc(void)
{
	s16b aux[MAX_DEPTH];
	(void)C_WIPE(&aux, MAX_DEPTH, s16b);

	s16b num[MAX_DEPTH];
	(void)C_WIPE(&num, MAX_DEPTH, s16b);

	/* Free the old "alloc_kind_table" (if it exists) */
	if (alloc_kind_table)
	{
		C_KILL(alloc_kind_table, alloc_kind_size, alloc_entry);
	}

	/* Size of "alloc_kind_table" */
	alloc_kind_size = 0;

	/* Scan the objects */
	for (int i = 1; i < max_k_idx; i++)
	{
		object_kind *k_ptr;
		k_ptr = &k_info[i];

		/* Scan allocation pairs */
		for (int j = 0; j < 4; j++)
		{
			/* Count the "legal" entries */
			if (k_ptr->chance[j])
			{
				/* Count the entries */
				alloc_kind_size++;

				/* Group by level */
				num[k_ptr->locale[j]]++;
			}
		}
	}

	/* Collect the level indexes */
	for (int i = 1; i < MAX_DEPTH; i++)
	{
		/* Group by level */
		num[i] += num[i - 1];
	}

	if (!num[0]) quit(_("町のアイテムがない！", "No town objects!"));

	/*** Initialize object allocation info ***/

	/* Allocate the alloc_kind_table */
	C_MAKE(alloc_kind_table, alloc_kind_size, alloc_entry);

	/* Access the table entry */
	alloc_entry *table;
	table = alloc_kind_table;

	/* Scan the objects */
	for (int i = 1; i < max_k_idx; i++)
	{
		object_kind *k_ptr;
		k_ptr = &k_info[i];

		/* Scan allocation pairs */
		for (int j = 0; j < 4; j++)
		{
			/* Count the "legal" entries */
			if (k_ptr->chance[j] == 0) continue;

			/* Extract the base level */
			int x = k_ptr->locale[j];

			/* Extract the base probability */
			int p = (100 / k_ptr->chance[j]);

			/* Skip entries preceding our locale */
			int y = (x > 0) ? num[x - 1] : 0;

			/* Skip previous entries at this locale */
			int z = y + aux[x];

			/* Load the entry */
			table[z].index = (KIND_OBJECT_IDX)i;
			table[z].level = (DEPTH)x;
			table[z].prob1 = (PROB)p;
			table[z].prob2 = (PROB)p;
			table[z].prob3 = (PROB)p;

			/* Another entry complete for this locale */
			aux[x]++;
		}
	}

	return 0;
}


/*!
 * @brief モンスター配列と生成テーブルを初期化する /
 * Initialize some other arrays
 * @return エラーコード
 */
static errr init_alloc(void)
{
	monster_race *r_ptr;
	tag_type *elements;

	/* Allocate the "r_info" array */
	C_MAKE(elements, max_r_idx, tag_type);

	/* Scan the monsters */
	for (int i = 1; i < max_r_idx; i++)
	{
		elements[i].tag = r_info[i].level;
		elements[i].index = i;
	}

	tag_sort(elements, max_r_idx);

	/*** Initialize monster allocation info ***/

	/* Size of "alloc_race_table" */
	alloc_race_size = max_r_idx;

	/* Allocate the alloc_race_table */
	C_MAKE(alloc_race_table, alloc_race_size, alloc_entry);

	/* Scan the monsters */
	for (int i = 1; i < max_r_idx; i++)
	{
		/* Get the i'th race */
		r_ptr = &r_info[elements[i].index];

		/* Count valid pairs */
		if (r_ptr->rarity == 0) continue;

		/* Extract the base level */
		int x = r_ptr->level;

		/* Extract the base probability */
		int p = (100 / r_ptr->rarity);

		/* Load the entry */
		alloc_race_table[i].index = (KIND_OBJECT_IDX)elements[i].index;
		alloc_race_table[i].level = (DEPTH)x;
		alloc_race_table[i].prob1 = (PROB)p;
		alloc_race_table[i].prob2 = (PROB)p;
		alloc_race_table[i].prob3 = (PROB)p;
	}

	/* Free the "r_info" array */
	C_KILL(elements, max_r_idx, tag_type);
	(void)init_object_alloc();
	return 0;
}


/*!
 * @brief 画面左下にシステムメッセージを表示する /
 * Hack -- take notes on line 23
 * @return なし
 */
static void init_note(concptr str)
{
	term_erase(0, 23, 255);
	term_putstr(20, 23, -1, TERM_WHITE, str);
	term_fresh();
}


/*!
 * @brief 全ゲームデータ読み込みのサブルーチン /
 * Hack -- Explain a broken "lib" folder and quit (see below).
 * @return なし
 * @note
 * <pre>
 * This function is "messy" because various things
 * may or may not be initialized, but the "plog()" and "quit()"
 * functions are "supposed" to work under any conditions.
 * </pre>
 */
static void init_angband_aux(concptr why)
{
	plog(why);

#ifdef JP
	/* Explain */
	plog("'lib'ディレクトリが存在しないか壊れているようです。");

	/* More details */
	plog("ひょっとするとアーカイブが正しく解凍されていないのかもしれません。");

	/* Explain */
	plog("該当する'README'ファイルを読んで確認してみて下さい。");

	/* Quit with error */
	quit("致命的なエラー。");
#else
	/* Explain */
	plog("The 'lib' directory is probably missing or broken.");

	/* More details */
	plog("Perhaps the archive was not extracted correctly.");

	/* Explain */
	plog("See the 'README' file for more information.");

	/* Quit with error */
	quit("Fatal Error.");
#endif

}


/*!
 * @brief 全ゲームデータ読み込みのメインルーチン /
 * Hack -- main Angband initialization entry point
 * @return なし
 * @note
 * <pre>
 * This function is "messy" because various things
 * may or may not be initialized, but the "plog()" and "quit()"
 * functions are "supposed" to work under any conditions.
 * Verify some files, display the "news.txt" file, create
 * the high score file, initialize all internal arrays, and
 * load the basic "user pref files".
 * Be very careful to keep track of the order in which things
 * are initialized, in particular, the only thing *known* to
 * be available when this function is called is the "z-term.c"
 * package, and that may not be fully initialized until the
 * end of this function, when the default "user pref files"
 * are loaded and "term_xtra(TERM_XTRA_REACT,0)" is called.
 * Note that this function attempts to verify the "news" file,
 * and the game aborts (cleanly) on failure, since without the
 * "news" file, it is likely that the "lib" folder has not been
 * correctly located.  Otherwise, the news file is displayed for
 * the user.
 * Note that this function attempts to verify (or create) the
 * "high score" file, and the game aborts (cleanly) on failure,
 * since one of the most common "extraction" failures involves
 * failing to extract all sub-directories (even empty ones), such
 * as by failing to use the "-d" option of "pkunzip", or failing
 * to use the "save empty directories" option with "Compact Pro".
 * This error will often be caught by the "high score" creation
 * code below, since the "lib/apex" directory, being empty in the
 * standard distributions, is most likely to be "lost", making it
 * impossible to create the high score file.
 * Note that various things are initialized by this function,
 * including everything that was once done by "init_some_arrays".
 * This initialization involves the parsing of special files
 * in the "lib/data" and sometimes the "lib/edit" directories.
 * Note that the "template" files are initialized first, since they
 * often contain errors.  This means that macros and message recall
 * and things like that are not available until after they are done.
 * We load the default "user pref files" here in case any "color"
 * changes are needed before character creation.
 * Note that the "graf-xxx.prf" file must be loaded separately,
 * if needed, in the first (?) pass through "TERM_XTRA_REACT".
 * </pre>
 */
void init_angband(player_type *player_ptr, void(*process_autopick_file_command)(char*))
{
	/*** Verify the "news" file ***/
	char buf[1024];
	path_build(buf, sizeof(buf), ANGBAND_DIR_FILE, _("news_j.txt", "news.txt"));

	/* Attempt to open the file */
	int fd = fd_open(buf, O_RDONLY);

	/* Failure */
	if (fd < 0)
	{
		char why[1024];

		sprintf(why, _("'%s'ファイルにアクセスできません!", "Cannot access the '%s' file!"), buf);

		/* Crash and burn */
		init_angband_aux(why);
	}

	(void)fd_close(fd);

	/*** Display the "news" file ***/
	term_clear();
	path_build(buf, sizeof(buf), ANGBAND_DIR_FILE, _("news_j.txt", "news.txt"));

	/* Open the News file */
	FILE *fp;
	fp = angband_fopen(buf, "r");

	/* Dump */
	if (fp)
	{
		/* Dump the file to the screen */
		int i = 0;
		while (0 == angband_fgets(fp, buf, sizeof(buf)))
		{
			/* Display and advance */
			term_putstr(0, i++, -1, TERM_WHITE, buf);
		}

		angband_fclose(fp);
	}

	term_flush();

	/*** Verify (or create) the "high score" file ***/
	path_build(buf, sizeof(buf), ANGBAND_DIR_APEX, "scores.raw");

	/* Attempt to open the high score file */
	fd = fd_open(buf, O_RDONLY);

	BIT_FLAGS file_permission = 0664;
	if (fd < 0)
	{
		/* Grab permissions */
		safe_setuid_grab(player_ptr);

		/* Create a new high score file */
		fd = fd_make(buf, file_permission);

		/* Drop permissions */
		safe_setuid_drop();

		/* Failure */
		if (fd < 0)
		{
			char why[1024];

			sprintf(why, _("'%s'ファイルを作成できません!", "Cannot create the '%s' file!"), buf);

			/* Crash and burn */
			init_angband_aux(why);
		}
	}

	(void)fd_close(fd);

	put_title();

	/*** Initialize some arrays ***/

	/* Initialize misc. values */
	init_note(_("[変数を初期化しています...(その他)", "[Initializing values... (misc)]"));
	if (init_misc(player_ptr)) quit(_("その他の変数を初期化できません", "Cannot initialize misc. values"));

	init_note(_("[データの初期化中... (地形)]", "[Initializing arrays... (features)]"));
	if (init_f_info(player_ptr)) quit(_("地形初期化不能", "Cannot initialize features"));
	if (init_feat_variables()) quit(_("地形初期化不能", "Cannot initialize features"));

	/* Initialize object info */
	init_note(_("[データの初期化中... (アイテム)]", "[Initializing arrays... (objects)]"));
	if (init_k_info(player_ptr)) quit(_("アイテム初期化不能", "Cannot initialize objects"));

	/* Initialize artifact info */
	init_note(_("[データの初期化中... (伝説のアイテム)]", "[Initializing arrays... (artifacts)]"));
	if (init_a_info(player_ptr)) quit(_("伝説のアイテム初期化不能", "Cannot initialize artifacts"));


	/* Initialize ego-item info */
	init_note(_("[データの初期化中... (名のあるアイテム)]", "[Initializing arrays... (ego-items)]"));
	if (init_e_info(player_ptr)) quit(_("名のあるアイテム初期化不能", "Cannot initialize ego-items"));


	/* Initialize monster info */
	init_note(_("[データの初期化中... (モンスター)]", "[Initializing arrays... (monsters)]"));
	if (init_r_info(player_ptr)) quit(_("モンスター初期化不能", "Cannot initialize monsters"));


	/* Initialize dungeon info */
	init_note(_("[データの初期化中... (ダンジョン)]", "[Initializing arrays... (dungeon)]"));
	if (init_d_info(player_ptr)) quit(_("ダンジョン初期化不能", "Cannot initialize dungeon"));
	{
		for (int i = 1; i < current_world_ptr->max_d_idx; i++)
			if (d_info[i].final_guardian)
				r_info[d_info[i].final_guardian].flags7 |= RF7_GUARDIAN;
	}

	/* Initialize magic info */
	init_note(_("[データの初期化中... (魔法)]", "[Initializing arrays... (magic)]"));
	if (init_m_info(player_ptr)) quit(_("魔法初期化不能", "Cannot initialize magic"));

	/* Initialize weapon_exp info */
	init_note(_("[データの初期化中... (熟練度)]", "[Initializing arrays... (skill)]"));
	if (init_s_info(player_ptr)) quit(_("熟練度初期化不能", "Cannot initialize skill"));

	/* Initialize wilderness array */
	init_note(_("[配列を初期化しています... (荒野)]", "[Initializing arrays... (wilderness)]"));

	if (init_wilderness()) quit(_("荒野を初期化できません", "Cannot initialize wilderness"));

	/* Initialize town array */
	init_note(_("[配列を初期化しています... (街)]", "[Initializing arrays... (towns)]"));
	if (init_towns()) quit(_("街を初期化できません", "Cannot initialize towns"));

	/* Initialize building array */
	init_note(_("[配列を初期化しています... (建物)]", "[Initializing arrays... (buildings)]"));
	if (init_buildings()) quit(_("建物を初期化できません", "Cannot initialize buildings"));

	/* Initialize quest array */
	init_note(_("[配列を初期化しています... (クエスト)]", "[Initializing arrays... (quests)]"));
	if (init_quests()) quit(_("クエストを初期化できません", "Cannot initialize quests"));

	/* Initialize vault info */
	if (init_v_info(player_ptr)) quit(_("vault 初期化不能", "Cannot initialize vaults"));

	/* Initialize some other arrays */
	init_note(_("[データの初期化中... (その他)]", "[Initializing arrays... (other)]"));
	if (init_other(player_ptr)) quit(_("その他のデータ初期化不能", "Cannot initialize other stuff"));

	/* Initialize some other arrays */
	init_note(_("[データの初期化中... (アロケーション)]", "[Initializing arrays... (alloc)]"));
	if (init_alloc()) quit(_("アロケーション・スタッフ初期化不能", "Cannot initialize alloc stuff"));

	/*** Load default user pref files ***/

	/* Initialize feature info */
	init_note(_("[ユーザー設定ファイルを初期化しています...]", "[Initializing user pref files...]"));

	/* Access the "basic" pref file */
	strcpy(buf, "pref.prf");

	/* Process that file */
	process_pref_file(player_ptr, buf, process_autopick_file_command);

	/* Access the "basic" system pref file */
	sprintf(buf, "pref-%s.prf", ANGBAND_SYS);

	/* Process that file */
	process_pref_file(player_ptr, buf, process_autopick_file_command);

	init_note(_("[初期化終了]", "[Initialization complete]"));
}


/*!
 * @brief タイトル記述
 * @return なし
 */
static void put_title(void)
{
	char title[120];
#if H_VER_EXTRA > 0
	sprintf(title, _("変愚蛮怒 %d.%d.%d.%d(%s)", "Hengband %d.%d.%d.%d(%s)"), H_VER_MAJOR, H_VER_MINOR, H_VER_PATCH, H_VER_EXTRA,
#else
	sprintf(title, _("変愚蛮怒 %d.%d.%d(%s)", "Hengband %d.%d.%d(%s)"), H_VER_MAJOR, H_VER_MINOR, H_VER_PATCH,
#endif
		IS_STABLE_VERSION ? _("安定版", "Stable") : _("開発版", "Developing"));
	int col = (80 - strlen(title)) / 2;
	col = col < 0 ? 0 : col;
    const int VER_INFO_ROW = 3; //!< タイトル表記(行)
	prt(title, VER_INFO_ROW, col);
}


/*!
 * @brief チェックサム情報を出力 / Get check sum in string form
 * @return チェックサム情報の文字列
 */
concptr get_check_sum(void)
{
	return format("%02x%02x%02x%02x%02x%02x%02x%02x%02x",
		f_head.v_extra,
		k_head.v_extra,
		a_head.v_extra,
		e_head.v_extra,
		r_head.v_extra,
		d_head.v_extra,
		m_head.v_extra,
		s_head.v_extra,
		v_head.v_extra);
}
