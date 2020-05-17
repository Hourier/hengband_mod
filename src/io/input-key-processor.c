﻿/*!
 * todo Ctrl+C がShift+Q に認識されている。仕様の可能性も高いが要確認
 * @brief キー入力に応じてゲーム内コマンドを実行する
 * @date 2020/05/10
 * @author Hourier
 */

#include "system/angband.h"
#include "io/input-key-processor.h"
#include "world/world.h"
#include "core/special-internal-keys.h"
#include "dungeon/dungeon.h"
#include "combat/snipe.h"
#include "autopick/autopick-pref-processor.h"
#include "market/store-util.h"
#include "main/sound-definitions-table.h"
#include "io/write-diary.h"
#include "inventory/player-inventory.h"
#include "player/player-effects.h"
#include "market/store.h" // do_cmd_store() がある。後で移設する.
#include "dungeon/quest.h" // do_cmd_quest() がある。後で移設する.
#include "floor/wild.h"
#include "spell/spells-object.h"
#include "mind/mind.h" // do_cmd_mind_browse() がある。後で移設する.
#include "mspell/monster-spell.h" // do_cmd_cast_learned() がある。後で移設する.
#include "mind/racial.h" // do_cmd_racial_power() がある。ファイル名変更？.
#include "view/display-main-window.h"
#include "knowledge/knowledge-autopick.h"
#include "knowledge/knowledge-quests.h"
#include "io/chuukei.h"
#include "player/player-move.h" // do_cmd_travel() がある。後で移設する.
#include "io/files.h"

#include "cmd/cmd-activate.h"
#include "cmd/cmd-autopick.h"
#include "cmd/cmd-diary.h"
#include "cmd/cmd-draw.h"
#include "cmd/cmd-dump.h"
#include "cmd/cmd-process-screen.h"
#include "cmd/cmd-eat.h"
#include "cmd/cmd-help.h"
#include "cmd/cmd-hissatsu.h"
#include "cmd/cmd-item.h"
#include "cmd/cmd-knowledge.h"
#include "cmd/cmd-magiceat.h"
#include "cmd/cmd-mane.h"
#include "cmd/cmd-macro.h"
#include "cmd/cmd-quaff.h"
#include "cmd/cmd-read.h"
#include "cmd/cmd-save.h"
#include "cmd/cmd-smith.h"
#include "cmd-spell.h"
#include "cmd/cmd-usestaff.h"
#include "cmd/cmd-zaprod.h"
#include "cmd/cmd-zapwand.h"
#include "cmd/cmd-pet.h"
#include "cmd/cmd-basic.h"
#include "cmd/cmd-visuals.h"
#include "wizard/wizard-spoiler.h"
#include "wizard/wizard-special-process.h"

// todo command-processor.h と util.h が相互依存している。後で別な場所に移す.
COMMAND_CODE now_message;

/*!
 * @brief ウィザードモードへの導入処理
 * / Verify use of "wizard" mode
 * @param player_ptr プレーヤーへの参照ポインタ
 * @return 実際にウィザードモードへ移行したらTRUEを返す。
 */
bool enter_wizard_mode(player_type* player_ptr)
{
    if (!current_world_ptr->noscore) {
        if (!allow_debug_opts || arg_wizard) {
            msg_print(_("ウィザードモードは許可されていません。 ", "Wizard mode is not permitted."));
            return FALSE;
        }

        msg_print(_("ウィザードモードはデバッグと実験のためのモードです。 ", "Wizard mode is for debugging and experimenting."));
        msg_print(_("一度ウィザードモードに入るとスコアは記録されません。", "The game will not be scored if you enter wizard mode."));
        msg_print(NULL);
        if (!get_check(_("本当にウィザードモードに入りたいのですか? ", "Are you sure you want to enter wizard mode? "))) {
            return FALSE;
        }

        exe_write_diary(player_ptr, DIARY_DESCRIPTION, 0, _("ウィザードモードに突入してスコアを残せなくなった。", "gave up recording score to enter wizard mode."));
        current_world_ptr->noscore |= 0x0002;
    }

    return TRUE;
}

/*!
 * @brief デバッグコマンドへの導入処理
 * / Verify use of "debug" commands
 * @param player_ptr プレーヤーへの参照ポインタ
 * @return 実際にデバッグコマンドへ移行したらTRUEを返す。
 */
static bool enter_debug_mode(player_type* player_ptr)
{
    if (!current_world_ptr->noscore) {
        if (!allow_debug_opts) {
            msg_print(_("デバッグコマンドは許可されていません。 ", "Use of debug command is not permitted."));
            return FALSE;
        }

        msg_print(_("デバッグ・コマンドはデバッグと実験のためのコマンドです。 ", "The debug commands are for debugging and experimenting."));
        msg_print(_("デバッグ・コマンドを使うとスコアは記録されません。", "The game will not be scored if you use debug commands."));
        msg_print(NULL);
        if (!get_check(_("本当にデバッグ・コマンドを使いますか? ", "Are you sure you want to use debug commands? "))) {
            return FALSE;
        }

        exe_write_diary(player_ptr, DIARY_DESCRIPTION, 0, _("デバッグモードに突入してスコアを残せなくなった。", "gave up sending score to use debug commands."));
        current_world_ptr->noscore |= 0x0008;
    }

    return TRUE;
}

/*!
 * @brief プレイヤーから受けた入力コマンドの分岐処理。
 * / Parse and execute the current command Give "Warning" on illegal commands.
 * @todo Make some "blocks"
 * @return なし
 */
void process_command(player_type* creature_ptr)
{
    COMMAND_CODE old_now_message = now_message;
    repeat_check();
    now_message = 0;
    if ((creature_ptr->pclass == CLASS_SNIPER) && (creature_ptr->concent))
        creature_ptr->reset_concent = TRUE;

    floor_type* floor_ptr = creature_ptr->current_floor_ptr;
    switch (command_cmd) {
    case ESCAPE:
    case ' ':
    case '\r':
    case '\n': {
        /* Ignore */
        break;
    }
    case KTRL('W'): {
        if (current_world_ptr->wizard) {
            current_world_ptr->wizard = FALSE;
            msg_print(_("ウィザードモード解除。", "Wizard mode off."));
        } else if (enter_wizard_mode(creature_ptr)) {
            current_world_ptr->wizard = TRUE;
            msg_print(_("ウィザードモード突入。", "Wizard mode on."));
        }

        creature_ptr->update |= (PU_MONSTERS);
        creature_ptr->redraw |= (PR_TITLE);
        break;
    }
    case KTRL('A'): {
        if (enter_debug_mode(creature_ptr)) {
            do_cmd_debug(creature_ptr);
        }

        break;
    }
    case 'w': {
        if (!creature_ptr->wild_mode)
            do_cmd_wield(creature_ptr);

        break;
    }
    case 't': {
        if (!creature_ptr->wild_mode)
            do_cmd_takeoff(creature_ptr);

        break;
    }
    case 'd': {
        if (!creature_ptr->wild_mode)
            do_cmd_drop(creature_ptr);

        break;
    }
    case 'k': {
        do_cmd_destroy(creature_ptr);
        break;
    }
    case 'e': {
        do_cmd_equip(creature_ptr);
        break;
    }
    case 'i': {
        do_cmd_inven(creature_ptr);
        break;
    }
    case 'I': {
        do_cmd_observe(creature_ptr);
        break;
    }

    case KTRL('I'): {
        toggle_inventory_equipment(creature_ptr);
        break;
    }
    case '+': {
        if (!creature_ptr->wild_mode)
            do_cmd_alter(creature_ptr);

        break;
    }
    case 'T': {
        if (!creature_ptr->wild_mode)
            do_cmd_tunnel(creature_ptr);

        break;
    }
    case ';': {
        do_cmd_walk(creature_ptr, FALSE);
        break;
    }
    case '-': {
        do_cmd_walk(creature_ptr, TRUE);
        break;
    }
    case '.': {
        if (!creature_ptr->wild_mode)
            do_cmd_run(creature_ptr);

        break;
    }
    case ',': {
        do_cmd_stay(creature_ptr, always_pickup);
        break;
    }
    case 'g': {
        do_cmd_stay(creature_ptr, !always_pickup);
        break;
    }
    case 'R': {
        do_cmd_rest(creature_ptr);
        break;
    }
    case 's': {
        do_cmd_search(creature_ptr);
        break;
    }
    case 'S': {
        if (creature_ptr->action == ACTION_SEARCH)
            set_action(creature_ptr, ACTION_NONE);
        else
            set_action(creature_ptr, ACTION_SEARCH);

        break;
    }
    case SPECIAL_KEY_STORE: {
        do_cmd_store(creature_ptr);
        break;
    }
    case SPECIAL_KEY_BUILDING: {
        do_cmd_bldg(creature_ptr);
        break;
    }
    case SPECIAL_KEY_QUEST: {
        do_cmd_quest(creature_ptr);
        break;
    }
    case '<': {
        if (!creature_ptr->wild_mode && !floor_ptr->dun_level && !floor_ptr->inside_arena && !floor_ptr->inside_quest) {
            if (vanilla_town)
                break;

            if (creature_ptr->ambush_flag) {
                msg_print(_("襲撃から逃げるにはマップの端まで移動しなければならない。", "To flee the ambush you have to reach the edge of the map."));
                break;
            }

            if (creature_ptr->food < PY_FOOD_WEAK) {
                msg_print(_("その前に食事をとらないと。", "You must eat something here."));
                break;
            }

            change_wild_mode(creature_ptr, FALSE);
        } else
            do_cmd_go_up(creature_ptr);

        break;
    }
    case '>': {
        if (creature_ptr->wild_mode)
            change_wild_mode(creature_ptr, FALSE);
        else
            do_cmd_go_down(creature_ptr);

        break;
    }
    case 'o': {
        do_cmd_open(creature_ptr);
        break;
    }
    case 'c': {
        do_cmd_close(creature_ptr);
        break;
    }
    case 'j': {
        do_cmd_spike(creature_ptr);
        break;
    }
    case 'B': {
        do_cmd_bash(creature_ptr);
        break;
    }
    case 'D': {
        do_cmd_disarm(creature_ptr);
        break;
    }
    case 'G': {
        if ((creature_ptr->pclass == CLASS_SORCERER) || (creature_ptr->pclass == CLASS_RED_MAGE))
            msg_print(_("呪文を学習する必要はない！", "You don't have to learn spells!"));
        else if (creature_ptr->pclass == CLASS_SAMURAI)
            do_cmd_gain_hissatsu(creature_ptr);
        else if (creature_ptr->pclass == CLASS_MAGIC_EATER)
            import_magic_device(creature_ptr);
        else
            do_cmd_study(creature_ptr);

        break;
    }
    case 'b': {
        if ((creature_ptr->pclass == CLASS_MINDCRAFTER) || (creature_ptr->pclass == CLASS_BERSERKER) || (creature_ptr->pclass == CLASS_NINJA) || (creature_ptr->pclass == CLASS_MIRROR_MASTER))
            do_cmd_mind_browse(creature_ptr);
        else if (creature_ptr->pclass == CLASS_SMITH)
            do_cmd_kaji(creature_ptr, TRUE);
        else if (creature_ptr->pclass == CLASS_MAGIC_EATER)
            do_cmd_magic_eater(creature_ptr, TRUE, FALSE);
        else if (creature_ptr->pclass == CLASS_SNIPER)
            do_cmd_snipe_browse(creature_ptr);
        else
            do_cmd_browse(creature_ptr);

        break;
    }
    case 'm': {
        if (!creature_ptr->wild_mode) {
            if ((creature_ptr->pclass == CLASS_WARRIOR) || (creature_ptr->pclass == CLASS_ARCHER) || (creature_ptr->pclass == CLASS_CAVALRY)) {
                msg_print(_("呪文を唱えられない！", "You cannot cast spells!"));
            } else if (floor_ptr->dun_level && (d_info[creature_ptr->dungeon_idx].flags1 & DF1_NO_MAGIC) && (creature_ptr->pclass != CLASS_BERSERKER) && (creature_ptr->pclass != CLASS_SMITH)) {
                msg_print(_("ダンジョンが魔法を吸収した！", "The dungeon absorbs all attempted magic!"));
                msg_print(NULL);
            } else if (creature_ptr->anti_magic && (creature_ptr->pclass != CLASS_BERSERKER) && (creature_ptr->pclass != CLASS_SMITH)) {
                concptr which_power = _("魔法", "magic");
                if (creature_ptr->pclass == CLASS_MINDCRAFTER)
                    which_power = _("超能力", "psionic powers");
                else if (creature_ptr->pclass == CLASS_IMITATOR)
                    which_power = _("ものまね", "imitation");
                else if (creature_ptr->pclass == CLASS_SAMURAI)
                    which_power = _("必殺剣", "hissatsu");
                else if (creature_ptr->pclass == CLASS_MIRROR_MASTER)
                    which_power = _("鏡魔法", "mirror magic");
                else if (creature_ptr->pclass == CLASS_NINJA)
                    which_power = _("忍術", "ninjutsu");
                else if (mp_ptr->spell_book == TV_LIFE_BOOK)
                    which_power = _("祈り", "prayer");

                msg_format(_("反魔法バリアが%sを邪魔した！", "An anti-magic shell disrupts your %s!"), which_power);
                free_turn(creature_ptr);
            } else if (creature_ptr->shero && (creature_ptr->pclass != CLASS_BERSERKER)) {
                msg_format(_("狂戦士化していて頭が回らない！", "You cannot think directly!"));
                free_turn(creature_ptr);
            } else {
                if ((creature_ptr->pclass == CLASS_MINDCRAFTER) || (creature_ptr->pclass == CLASS_BERSERKER) || (creature_ptr->pclass == CLASS_NINJA) || (creature_ptr->pclass == CLASS_MIRROR_MASTER))
                    do_cmd_mind(creature_ptr);
                else if (creature_ptr->pclass == CLASS_IMITATOR)
                    do_cmd_mane(creature_ptr, FALSE);
                else if (creature_ptr->pclass == CLASS_MAGIC_EATER)
                    do_cmd_magic_eater(creature_ptr, FALSE, FALSE);
                else if (creature_ptr->pclass == CLASS_SAMURAI)
                    do_cmd_hissatsu(creature_ptr);
                else if (creature_ptr->pclass == CLASS_BLUE_MAGE)
                    do_cmd_cast_learned(creature_ptr);
                else if (creature_ptr->pclass == CLASS_SMITH)
                    do_cmd_kaji(creature_ptr, FALSE);
                else if (creature_ptr->pclass == CLASS_SNIPER)
                    do_cmd_snipe(creature_ptr);
                else
                    do_cmd_cast(creature_ptr);
            }
        }

        break;
    }
    case 'p': {
        do_cmd_pet(creature_ptr);
        break;
    }
    case '{': {
        do_cmd_inscribe(creature_ptr);
        break;
    }
    case '}': {
        do_cmd_uninscribe(creature_ptr);
        break;
    }
    case 'A': {
        do_cmd_activate(creature_ptr);
        break;
    }
    case 'E': {
        do_cmd_eat_food(creature_ptr);
        break;
    }
    case 'F': {
        do_cmd_refill(creature_ptr);
        break;
    }
    case 'f': {
        do_cmd_fire(creature_ptr, SP_NONE);
        break;
    }
    case 'v': {
        do_cmd_throw(creature_ptr, 1, FALSE, -1);
        break;
    }
    case 'a': {
        do_cmd_aim_wand(creature_ptr);
        break;
    }
    case 'z': {
        if (use_command && rogue_like_commands) {
            do_cmd_use(creature_ptr);
        } else {
            do_cmd_zap_rod(creature_ptr);
        }

        break;
    }
    case 'q': {
        do_cmd_quaff_potion(creature_ptr);
        break;
    }
    case 'r': {
        do_cmd_read_scroll(creature_ptr);
        break;
    }
    case 'u': {
        if (use_command && !rogue_like_commands)
            do_cmd_use(creature_ptr);
        else
            do_cmd_use_staff(creature_ptr);

        break;
    }
    case 'U': {
        do_cmd_racial_power(creature_ptr);
        break;
    }
    case 'M': {
        do_cmd_view_map(creature_ptr);
        break;
    }
    case 'L': {
        do_cmd_locate(creature_ptr);
        break;
    }
    case 'l': {
        do_cmd_look(creature_ptr);
        break;
    }
    case '*': {
        do_cmd_target(creature_ptr);
        break;
    }
    case '?': {
        do_cmd_help(creature_ptr);
        break;
    }
    case '/': {
        do_cmd_query_symbol(creature_ptr);
        break;
    }
    case 'C': {
        do_cmd_player_status(creature_ptr);
        break;
    }
    case '!': {
        (void)Term_user(0);
        break;
    }
    case '"': {
        do_cmd_pref(creature_ptr);
        break;
    }
    case '$': {
        do_cmd_reload_autopick(creature_ptr);
        break;
    }
    case '_': {
        do_cmd_edit_autopick(creature_ptr);
        break;
    }
    case '@': {
        do_cmd_macros(creature_ptr, process_autopick_file_command);
        break;
    }
    case '%': {
        do_cmd_visuals(creature_ptr, process_autopick_file_command);
        do_cmd_redraw(creature_ptr);
        break;
    }
    case '&': {
        do_cmd_colors(creature_ptr, process_autopick_file_command);
        do_cmd_redraw(creature_ptr);
        break;
    }
    case '=': {
        do_cmd_options();
        (void)combine_and_reorder_home(STORE_HOME);
        do_cmd_redraw(creature_ptr);
        break;
    }
    case ':': {
        do_cmd_note();
        break;
    }
    case 'V': {
        do_cmd_version();
        break;
    }
    case KTRL('F'): {
        do_cmd_feeling(creature_ptr);
        break;
    }
    case KTRL('O'): {
        do_cmd_message_one();
        break;
    }
    case KTRL('P'): {
        do_cmd_messages(old_now_message);
        break;
    }
    case KTRL('Q'): {
        do_cmd_checkquest(creature_ptr);
        break;
    }
    case KTRL('R'): {
        now_message = old_now_message;
        do_cmd_redraw(creature_ptr);
        break;
    }
    case KTRL('S'): {
        do_cmd_save_game(creature_ptr, FALSE);
        break;
    }
    case KTRL('T'): {
        do_cmd_time(creature_ptr);
        break;
    }
    case KTRL('X'):
    case SPECIAL_KEY_QUIT: {
        do_cmd_save_and_exit(creature_ptr);
        break;
    }
    case 'Q': {
        do_cmd_suicide(creature_ptr);
        break;
    }
    case '|': {
        do_cmd_diary(creature_ptr);
        break;
    }
    case '~': {
        do_cmd_knowledge(creature_ptr);
        break;
    }
    case '(': {
        do_cmd_load_screen();
        break;
    }
    case ')': {
        do_cmd_save_screen(creature_ptr, process_autopick_file_command);
        break;
    }
    case ']': {
        prepare_movie_hooks();
        break;
    }
    case KTRL('V'): {
        spoil_random_artifact(creature_ptr, "randifact.txt");
        break;
    }
    case '`': {
        if (!creature_ptr->wild_mode)
            do_cmd_travel(creature_ptr);
        if (creature_ptr->special_defense & KATA_MUSOU) {
            set_action(creature_ptr, ACTION_NONE);
        }

        break;
    }
    default: {
        if (flush_failure)
            flush();
        if (one_in_(2)) {
            char error_m[1024];
            sound(SOUND_ILLEGAL);
            if (!get_rnd_line(_("error_j.txt", "error.txt"), 0, error_m))
                msg_print(error_m);
        } else {
            prt(_(" '?' でヘルプが表示されます。", "Type '?' for help."), 0, 0);
        }

        break;
    }
    }

    if (!creature_ptr->energy_use && !now_message)
        now_message = old_now_message;
}
