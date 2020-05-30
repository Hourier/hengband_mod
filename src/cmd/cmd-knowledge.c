﻿#include "system/angband.h"
#include "cmd/cmd-knowledge.h"
#include "cmd/cmd-draw.h"
#include "grid/feature.h"
#include "knowledge/knowledge-autopick.h"
#include "knowledge/knowledge-experiences.h"
#include "knowledge/knowledge-features.h"
#include "knowledge/knowledge-inventory.h"
#include "knowledge/knowledge-items.h"
#include "knowledge/knowledge-mutations.h"
#include "knowledge/knowledge-monsters.h"
#include "knowledge/knowledge-quests.h"
#include "knowledge/knowledge-self.h"
#include "knowledge/knowledge-uniques.h"

/*
 * Interact with "knowledge"
 */
void do_cmd_knowledge(player_type *creature_ptr)
{
	int i, p = 0;
	bool need_redraw = FALSE;
	FILE_TYPE(FILE_TYPE_TEXT);
	screen_save();
	while (TRUE)
	{
		Term_clear();
		prt(format(_("%d/2 ページ", "page %d/2"), (p + 1)), 2, 65);
		prt(_("現在の知識を確認する", "Display current knowledge"), 3, 0);
		if (p == 0)
		{
			prt(_("(1) 既知の伝説のアイテム                 の一覧", "(1) Display known artifacts"), 6, 5);
			prt(_("(2) 既知のアイテム                       の一覧", "(2) Display known objects"), 7, 5);
			prt(_("(3) 既知の生きているユニーク・モンスター の一覧", "(3) Display remaining uniques"), 8, 5);
			prt(_("(4) 既知のモンスター                     の一覧", "(4) Display known monster"), 9, 5);
			prt(_("(5) 倒した敵の数                         の一覧", "(5) Display kill count"), 10, 5);
			if (!vanilla_town) prt(_("(6) 賞金首                               の一覧", "(6) Display wanted monsters"), 11, 5);
			prt(_("(7) 現在のペット                         の一覧", "(7) Display current pets"), 12, 5);
			prt(_("(8) 我が家のアイテム                     の一覧", "(8) Display home inventory"), 13, 5);
			prt(_("(9) *鑑定*済み装備の耐性                 の一覧", "(9) Display *identified* equip."), 14, 5);
			prt(_("(0) 地形の表示文字/タイル                の一覧", "(0) Display terrain symbols."), 15, 5);
		}
		else
		{
			prt(_("(a) 自分に関する情報                     の一覧", "(a) Display about yourself"), 6, 5);
			prt(_("(b) 突然変異                             の一覧", "(b) Display mutations"), 7, 5);
			prt(_("(c) 武器の経験値                         の一覧", "(c) Display weapon proficiency"), 8, 5);
			prt(_("(d) 魔法の経験値                         の一覧", "(d) Display spell proficiency"), 9, 5);
			prt(_("(e) 技能の経験値                         の一覧", "(e) Display misc. proficiency"), 10, 5);
			prt(_("(f) プレイヤーの徳                       の一覧", "(f) Display virtues"), 11, 5);
			prt(_("(g) 入ったダンジョン                     の一覧", "(g) Display dungeons"), 12, 5);
			prt(_("(h) 実行中のクエスト                     の一覧", "(h) Display current quests"), 13, 5);
			prt(_("(i) 現在の自動拾い/破壊設定              の一覧", "(i) Display auto pick/destroy"), 14, 5);
		}

		prt(_("-続く-", "-more-"), 17, 8);
		prt(_("ESC) 抜ける", "ESC) Exit menu"), 21, 1);
		prt(_("SPACE) 次ページ", "SPACE) Next page"), 21, 30);
		prt(_("コマンド:", "Command: "), 20, 0);
		i = inkey();

		if (i == ESCAPE) break;
		switch (i)
		{
		case ' ': /* Page change */
		case '-':
			p = 1 - p;
			break;
		case '1': /* Artifacts */
			do_cmd_knowledge_artifacts(creature_ptr);
			break;
		case '2': /* Objects */
			do_cmd_knowledge_objects(creature_ptr, &need_redraw, FALSE, -1);
			break;
		case '3': /* Uniques */
			do_cmd_knowledge_uniques(creature_ptr);
			break;
		case '4': /* Monsters */
			do_cmd_knowledge_monsters(creature_ptr, &need_redraw, FALSE, -1);
			break;
		case '5': /* Kill count  */
			do_cmd_knowledge_kill_count(creature_ptr);
			break;
		case '6': /* wanted */
			if (!vanilla_town) do_cmd_knowledge_bounty(creature_ptr);
			break;
		case '7': /* Pets */
			do_cmd_knowledge_pets(creature_ptr);
			break;
		case '8': /* Home */
			do_cmd_knowledge_home(creature_ptr);
			break;
		case '9': /* Resist list */
			do_cmd_knowledge_inventory(creature_ptr);
			break;
		case '0': /* Feature list */
		{
			IDX lighting_level = F_LIT_STANDARD;
			do_cmd_knowledge_features(&need_redraw, FALSE, -1, &lighting_level);
		}
		break;
		/* Next page */
		case 'a': /* Max stat */
			do_cmd_knowledge_stat(creature_ptr);
			break;
		case 'b': /* Mutations */
			do_cmd_knowledge_mutations(creature_ptr);
			break;
		case 'c': /* weapon-exp */
			do_cmd_knowledge_weapon_exp(creature_ptr);
			break;
		case 'd': /* spell-exp */
			do_cmd_knowledge_spell_exp(creature_ptr);
			break;
		case 'e': /* skill-exp */
			do_cmd_knowledge_skill_exp(creature_ptr);
			break;
		case 'f': /* Virtues */
			do_cmd_knowledge_virtues(creature_ptr);
			break;
		case 'g': /* Dungeon */
			do_cmd_knowledge_dungeon(creature_ptr);
			break;
		case 'h': /* Quests */
			do_cmd_knowledge_quests(creature_ptr);
			break;
		case 'i': /* Autopick */
			do_cmd_knowledge_autopick(creature_ptr);
			break;
		default: /* Unknown option */
			bell();
		}

		msg_erase();
	}

	screen_load();
	if (need_redraw) do_cmd_redraw(creature_ptr);
}
