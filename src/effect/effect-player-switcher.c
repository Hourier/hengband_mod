#include "angband.h"
#include "effect-player-util.h"
#include "effect/effect-player-switcher.h"
#include "spell/spells-type.h"
#include "player-damage.h"
#include "world.h"
#include "object-broken.h"
#include "player-effects.h"
#include "spells-status.h"
#include "artifact.h"
#include "player/mimic-info-table.h"
#include "monster-spell.h"
#include "mutation.h"
#include "object-curse.h"

// �ł�����4���f.
void effect_player_elements(player_type *target_ptr, effect_player_type *ep_ptr, concptr attack_message,
	HIT_POINT(*damage_func)(player_type*, HIT_POINT, concptr, int, bool))
{
	if (target_ptr->blind) msg_print(attack_message);

	ep_ptr->get_damage = (*damage_func)(target_ptr, ep_ptr->dam, ep_ptr->killer, ep_ptr->monspell, FALSE);
}


void effect_player_poison(player_type *target_ptr, effect_player_type *ep_ptr)
{
	bool double_resist = is_oppose_pois(target_ptr);
	if (target_ptr->blind) msg_print(_("�łōU�����ꂽ�I", "You are hit by poison!"));

	if (target_ptr->resist_pois) ep_ptr->dam = (ep_ptr->dam + 2) / 3;
	if (double_resist) ep_ptr->dam = (ep_ptr->dam + 2) / 3;

	if ((!(double_resist || target_ptr->resist_pois)) && one_in_(HURT_CHANCE) && !CHECK_MULTISHADOW(target_ptr))
	{
		do_dec_stat(target_ptr, A_CON);
	}

	ep_ptr->get_damage = take_hit(target_ptr, DAMAGE_ATTACK, ep_ptr->dam, ep_ptr->killer, ep_ptr->monspell);

	if (!(double_resist || target_ptr->resist_pois) && !CHECK_MULTISHADOW(target_ptr))
		set_poisoned(target_ptr, target_ptr->poisoned + randint0(ep_ptr->dam) + 10);
}


void effect_player_nuke(player_type *target_ptr, effect_player_type *ep_ptr)
{
	bool double_resist = is_oppose_pois(target_ptr);
	if (target_ptr->blind) msg_print(_("���˔\�ōU�����ꂽ�I", "You are hit by radiation!"));

	if (target_ptr->resist_pois) ep_ptr->dam = (2 * ep_ptr->dam + 2) / 5;
	if (double_resist) ep_ptr->dam = (2 * ep_ptr->dam + 2) / 5;

	ep_ptr->get_damage = take_hit(target_ptr, DAMAGE_ATTACK, ep_ptr->dam, ep_ptr->killer, ep_ptr->monspell);
	if ((double_resist || target_ptr->resist_pois) || CHECK_MULTISHADOW(target_ptr))
		return;

	set_poisoned(target_ptr, target_ptr->poisoned + randint0(ep_ptr->dam) + 10);
	if (one_in_(5)) /* 6 */
	{
		msg_print(_("��`�I�ȕϐg�𐋂����I", "You undergo a freakish metamorphosis!"));
		if (one_in_(4)) /* 4 */
			do_poly_self(target_ptr);
		else
			status_shuffle(target_ptr);
	}

	if (one_in_(6))
		inventory_damage(target_ptr, set_acid_destroy, 2);
}


void effect_player_missile(player_type *target_ptr, effect_player_type *ep_ptr)
{
	if (target_ptr->blind) msg_print(_("�����ōU�����ꂽ�I", "You are hit by something!"));

	ep_ptr->get_damage = take_hit(target_ptr, DAMAGE_ATTACK, ep_ptr->dam, ep_ptr->killer, ep_ptr->monspell);
}


void effect_player_holy_fire(player_type *target_ptr, effect_player_type *ep_ptr)
{
	if (target_ptr->blind) msg_print(_("�����ōU�����ꂽ�I", "You are hit by something!"));

	if (target_ptr->align > 10)
		ep_ptr->dam /= 2;
	else if (target_ptr->align < -10)
		ep_ptr->dam *= 2;

	ep_ptr->get_damage = take_hit(target_ptr, DAMAGE_ATTACK, ep_ptr->dam, ep_ptr->killer, ep_ptr->monspell);
}


void effect_player_hell_fire(player_type *target_ptr, effect_player_type *ep_ptr)
{
	if (target_ptr->blind) msg_print(_("�����ōU�����ꂽ�I", "You are hit by something!"));

	if (target_ptr->align > 10)
		ep_ptr->dam *= 2;

	ep_ptr->get_damage = take_hit(target_ptr, DAMAGE_ATTACK, ep_ptr->dam, ep_ptr->killer, ep_ptr->monspell);
}


void effect_player_arrow(player_type *target_ptr, effect_player_type *ep_ptr)
{
	if (target_ptr->blind)
	{
		msg_print(_("�����s�����̂ōU�����ꂽ�I", "You are hit by something sharp!"));
		ep_ptr->get_damage = take_hit(target_ptr, DAMAGE_ATTACK, ep_ptr->dam, ep_ptr->killer, ep_ptr->monspell);
		return;
	}
	
	if ((target_ptr->inventory_list[INVEN_RARM].name1 == ART_ZANTETSU) || (target_ptr->inventory_list[INVEN_LARM].name1 == ART_ZANTETSU))
	{
		msg_print(_("����a��̂Ă��I", "You cut down the arrow!"));
		return;
	}

	ep_ptr->get_damage = take_hit(target_ptr, DAMAGE_ATTACK, ep_ptr->dam, ep_ptr->killer, ep_ptr->monspell);
}


void effect_player_plasma(player_type *target_ptr, effect_player_type *ep_ptr)
{
	if (target_ptr->blind) msg_print(_("�����ƂĂ��M�����̂ōU�����ꂽ�I", "You are hit by something *HOT*!"));

	ep_ptr->get_damage = take_hit(target_ptr, DAMAGE_ATTACK, ep_ptr->dam, ep_ptr->killer, ep_ptr->monspell);

	if (!target_ptr->resist_sound && !CHECK_MULTISHADOW(target_ptr))
	{
		int plus_stun = (randint1((ep_ptr->dam > 40) ? 35 : (ep_ptr->dam * 3 / 4 + 5)));
		(void)set_stun(target_ptr, target_ptr->stun + plus_stun);
	}

	if (!(target_ptr->resist_fire || is_oppose_fire(target_ptr) || target_ptr->immune_fire))
		inventory_damage(target_ptr, set_acid_destroy, 3);
}


void effect_player_nether(player_type *target_ptr, effect_player_type *ep_ptr)
{
	if (target_ptr->blind) msg_print(_("�n���̗͂ōU�����ꂽ�I", "You are hit by nether forces!"));

	if (target_ptr->resist_neth)
	{
		if (!PRACE_IS_(target_ptr, RACE_SPECTRE))
			ep_ptr->dam *= 6; ep_ptr->dam /= (randint1(4) + 7);
	}
	else if (!CHECK_MULTISHADOW(target_ptr)) drain_exp(target_ptr, 200 + (target_ptr->exp / 100), 200 + (target_ptr->exp / 1000), 75);

	if (!PRACE_IS_(target_ptr, RACE_SPECTRE) || CHECK_MULTISHADOW(target_ptr))
	{
		ep_ptr->get_damage = take_hit(target_ptr, DAMAGE_ATTACK, ep_ptr->dam, ep_ptr->killer, ep_ptr->monspell);
		return;
	}

	msg_print(_("�C�����悭�Ȃ����B", "You feel invigorated!"));
	hp_player(target_ptr, ep_ptr->dam / 4);
	learn_spell(target_ptr, ep_ptr->monspell);
}


void effect_player_water(player_type *target_ptr, effect_player_type *ep_ptr)
{
	if (target_ptr->blind) msg_print(_("�������������̂ōU�����ꂽ�I", "You are hit by something wet!"));
	if (CHECK_MULTISHADOW(target_ptr))
	{
		ep_ptr->get_damage = take_hit(target_ptr, DAMAGE_ATTACK, ep_ptr->dam, ep_ptr->killer, ep_ptr->monspell);
		return;
	}

	if (!target_ptr->resist_sound && !target_ptr->resist_water)
	{
		set_stun(target_ptr, target_ptr->stun + randint1(40));
	}
	if (!target_ptr->resist_conf && !target_ptr->resist_water)
	{
		set_confused(target_ptr, target_ptr->confused + randint1(5) + 5);
	}

	if (one_in_(5) && !target_ptr->resist_water)
	{
		inventory_damage(target_ptr, set_cold_destroy, 3);
	}

	if (target_ptr->resist_water) ep_ptr->get_damage /= 4;

	ep_ptr->get_damage = take_hit(target_ptr, DAMAGE_ATTACK, ep_ptr->dam, ep_ptr->killer, ep_ptr->monspell);
}


void effect_player_chaos(player_type *target_ptr, effect_player_type *ep_ptr)
{
	if (target_ptr->blind) msg_print(_("�������̔g���ōU�����ꂽ�I", "You are hit by a wave of anarchy!"));
	if (target_ptr->resist_chaos)
	{
		ep_ptr->dam *= 6; ep_ptr->dam /= (randint1(4) + 7);
	}

	if (CHECK_MULTISHADOW(target_ptr))
	{
		ep_ptr->get_damage = take_hit(target_ptr, DAMAGE_ATTACK, ep_ptr->dam, ep_ptr->killer, ep_ptr->monspell);
		return;
	}

	if (!target_ptr->resist_conf)
	{
		(void)set_confused(target_ptr, target_ptr->confused + randint0(20) + 10);
	}
	if (!target_ptr->resist_chaos)
	{
		(void)set_image(target_ptr, target_ptr->image + randint1(10));
		if (one_in_(3))
		{
			msg_print(_("���Ȃ��̐g�̂̓J�I�X�̗͂ŔP���Ȃ���ꂽ�I", "Your body is twisted by chaos!"));
			(void)gain_mutation(target_ptr, 0);
		}
	}
	if (!target_ptr->resist_neth && !target_ptr->resist_chaos)
	{
		drain_exp(target_ptr, 5000 + (target_ptr->exp / 100), 500 + (target_ptr->exp / 1000), 75);
	}

	if (!target_ptr->resist_chaos || one_in_(9))
	{
		inventory_damage(target_ptr, set_elec_destroy, 2);
		inventory_damage(target_ptr, set_fire_destroy, 2);
	}

	ep_ptr->get_damage = take_hit(target_ptr, DAMAGE_ATTACK, ep_ptr->dam, ep_ptr->killer, ep_ptr->monspell);
}


void effect_player_shards(player_type *target_ptr, effect_player_type *ep_ptr)
{
	if (target_ptr->blind) msg_print(_("�����s�����̂ōU�����ꂽ�I", "You are hit by something sharp!"));
	if (target_ptr->resist_shard)
	{
		ep_ptr->dam *= 6; ep_ptr->dam /= (randint1(4) + 7);
	}
	else if (!CHECK_MULTISHADOW(target_ptr))
	{
		(void)set_cut(target_ptr, target_ptr->cut + ep_ptr->dam);
	}

	if (!target_ptr->resist_shard || one_in_(13))
		inventory_damage(target_ptr, set_cold_destroy, 2);

	ep_ptr->get_damage = take_hit(target_ptr, DAMAGE_ATTACK, ep_ptr->dam, ep_ptr->killer, ep_ptr->monspell);
}


void effect_player_sound(player_type *target_ptr, effect_player_type *ep_ptr)
{
	if (target_ptr->blind) msg_print(_("�����ōU�����ꂽ�I", "You are hit by a loud noise!"));
	if (target_ptr->resist_sound)
	{
		ep_ptr->dam *= 5; ep_ptr->dam /= (randint1(4) + 7);
	}
	else if (!CHECK_MULTISHADOW(target_ptr))
	{
		int plus_stun = (randint1((ep_ptr->dam > 90) ? 35 : (ep_ptr->dam / 3 + 5)));
		(void)set_stun(target_ptr, target_ptr->stun + plus_stun);
	}

	if (!target_ptr->resist_sound || one_in_(13))
		inventory_damage(target_ptr, set_cold_destroy, 2);

	ep_ptr->get_damage = take_hit(target_ptr, DAMAGE_ATTACK, ep_ptr->dam, ep_ptr->killer, ep_ptr->monspell);
}


void effect_player_confusion(player_type *target_ptr, effect_player_type *ep_ptr)
{
	if (target_ptr->blind) msg_print(_("��������������̂ōU�����ꂽ�I", "You are hit by something puzzling!"));
	if (target_ptr->resist_conf)
	{
		ep_ptr->dam *= 5; ep_ptr->dam /= (randint1(4) + 7);
	}
	else if (!CHECK_MULTISHADOW(target_ptr))
	{
		(void)set_confused(target_ptr, target_ptr->confused + randint1(20) + 10);
	}

	ep_ptr->get_damage = take_hit(target_ptr, DAMAGE_ATTACK, ep_ptr->dam, ep_ptr->killer, ep_ptr->monspell);
}


void effect_player_disenchant(player_type *target_ptr, effect_player_type *ep_ptr)
{
	if (target_ptr->blind) msg_print(_("���������Ȃ����̂ōU�����ꂽ�I", "You are hit by something static!"));
	if (target_ptr->resist_disen)
	{
		ep_ptr->dam *= 6; ep_ptr->dam /= (randint1(4) + 7);
	}
	else if (!CHECK_MULTISHADOW(target_ptr))
	{
		(void)apply_disenchant(target_ptr, 0);
	}

	ep_ptr->get_damage = take_hit(target_ptr, DAMAGE_ATTACK, ep_ptr->dam, ep_ptr->killer, ep_ptr->monspell);
}


void effect_player_nexus(player_type *target_ptr, effect_player_type *ep_ptr)
{
	if (target_ptr->blind) msg_print(_("������Ȃ��̂ōU�����ꂽ�I", "You are hit by something strange!"));
	if (target_ptr->resist_nexus)
	{
		ep_ptr->dam *= 6; ep_ptr->dam /= (randint1(4) + 7);
	}
	else if (!CHECK_MULTISHADOW(target_ptr))
	{
		apply_nexus(ep_ptr->m_ptr, target_ptr);
	}

	ep_ptr->get_damage = take_hit(target_ptr, DAMAGE_ATTACK, ep_ptr->dam, ep_ptr->killer, ep_ptr->monspell);
}

void effect_player_force(player_type *target_ptr, effect_player_type *ep_ptr)
{
	if (target_ptr->blind) msg_print(_("�^���G�l���M�[�ōU�����ꂽ�I", "You are hit by kinetic force!"));
	if (!target_ptr->resist_sound && !CHECK_MULTISHADOW(target_ptr))
	{
		(void)set_stun(target_ptr, target_ptr->stun + randint1(20));
	}

	ep_ptr->get_damage = take_hit(target_ptr, DAMAGE_ATTACK, ep_ptr->dam, ep_ptr->killer, ep_ptr->monspell);
}


void effect_player_rocket(player_type *target_ptr, effect_player_type *ep_ptr)
{
	if (target_ptr->blind) msg_print(_("�������������I", "There is an explosion!"));
	if (!target_ptr->resist_sound && !CHECK_MULTISHADOW(target_ptr))
	{
		(void)set_stun(target_ptr, target_ptr->stun + randint1(20));
	}

	if (target_ptr->resist_shard)
	{
		ep_ptr->dam /= 2;
	}
	else if (!CHECK_MULTISHADOW(target_ptr))
	{
		(void)set_cut(target_ptr, target_ptr->cut + (ep_ptr->dam / 2));
	}

	if (!target_ptr->resist_shard || one_in_(12))
	{
		inventory_damage(target_ptr, set_cold_destroy, 3);
	}

	ep_ptr->get_damage = take_hit(target_ptr, DAMAGE_ATTACK, ep_ptr->dam, ep_ptr->killer, ep_ptr->monspell);
}


void effect_player_inertial(player_type *target_ptr, effect_player_type *ep_ptr)
{
	if (target_ptr->blind) msg_print(_("�����x�����̂ōU�����ꂽ�I", "You are hit by something slow!"));
	if (!CHECK_MULTISHADOW(target_ptr)) (void)set_slow(target_ptr, target_ptr->slow + randint0(4) + 4, FALSE);

	ep_ptr->get_damage = take_hit(target_ptr, DAMAGE_ATTACK, ep_ptr->dam, ep_ptr->killer, ep_ptr->monspell);
}


void effect_player_lite(player_type *target_ptr, effect_player_type *ep_ptr)
{
	if (target_ptr->blind) msg_print(_("�����ōU�����ꂽ�I", "You are hit by something!"));
	if (target_ptr->resist_lite)
	{
		ep_ptr->dam *= 4; ep_ptr->dam /= (randint1(4) + 7);
	}
	else if (!target_ptr->blind && !target_ptr->resist_blind && !CHECK_MULTISHADOW(target_ptr))
	{
		(void)set_blind(target_ptr, target_ptr->blind + randint1(5) + 2);
	}

	if (PRACE_IS_(target_ptr, RACE_VAMPIRE) || (target_ptr->mimic_form == MIMIC_VAMPIRE))
	{
		if (!CHECK_MULTISHADOW(target_ptr)) msg_print(_("���œ��̂��ł����ꂽ�I", "The light scorches your flesh!"));
		ep_ptr->dam *= 2;
	}
	else if (PRACE_IS_(target_ptr, RACE_S_FAIRY))
	{
		ep_ptr->dam = ep_ptr->dam * 4 / 3;
	}

	if (target_ptr->wraith_form) ep_ptr->dam *= 2;
	ep_ptr->get_damage = take_hit(target_ptr, DAMAGE_ATTACK, ep_ptr->dam, ep_ptr->killer, ep_ptr->monspell);

	if (!target_ptr->wraith_form || CHECK_MULTISHADOW(target_ptr))
		return;

	target_ptr->wraith_form = 0;
	msg_print(_("�M���̂��ߔ񕨎��I�ȉe�̑��݂ł����Ȃ��Ȃ����B",
		"The light forces you out of your incorporeal shadow form."));

	target_ptr->redraw |= (PR_MAP | PR_STATUS);
	target_ptr->update |= (PU_MONSTERS);
	target_ptr->window |= (PW_OVERHEAD | PW_DUNGEON);
}


void effect_player_dark(player_type *target_ptr, effect_player_type *ep_ptr)
{
	if (target_ptr->blind) msg_print(_("�����ōU�����ꂽ�I", "You are hit by something!"));
	if (target_ptr->resist_dark)
	{
		ep_ptr->dam *= 4; ep_ptr->dam /= (randint1(4) + 7);

		if (PRACE_IS_(target_ptr, RACE_VAMPIRE) || (target_ptr->mimic_form == MIMIC_VAMPIRE) || target_ptr->wraith_form) ep_ptr->dam = 0;
	}
	else if (!target_ptr->blind && !target_ptr->resist_blind && !CHECK_MULTISHADOW(target_ptr))
	{
		(void)set_blind(target_ptr, target_ptr->blind + randint1(5) + 2);
	}

	ep_ptr->get_damage = take_hit(target_ptr, DAMAGE_ATTACK, ep_ptr->dam, ep_ptr->killer, ep_ptr->monspell);
}


static void effect_player_time_one_disability(player_type *target_ptr)
{
	int k = 0;
	concptr act = NULL;
	switch (randint1(6))
	{
	case 1: k = A_STR; act = _("����", "strong"); break;
	case 2: k = A_INT; act = _("������", "bright"); break;
	case 3: k = A_WIS; act = _("������", "wise"); break;
	case 4: k = A_DEX; act = _("��p��", "agile"); break;
	case 5: k = A_CON; act = _("���N��", "hale"); break;
	case 6: k = A_CHR; act = _("������", "beautiful"); break;
	}

	msg_format(_("���Ȃ��͈ȑO�ق�%s�Ȃ��Ȃ��Ă��܂���...�B", "You're not as %s as you used to be..."), act);
	target_ptr->stat_cur[k] = (target_ptr->stat_cur[k] * 3) / 4;
	if (target_ptr->stat_cur[k] < 3) target_ptr->stat_cur[k] = 3;

	target_ptr->update |= (PU_BONUS);
}


static void effect_player_time_all_disabilities(player_type *target_ptr)
{
	msg_print(_("���Ȃ��͈ȑO�قǗ͋����Ȃ��Ȃ��Ă��܂���...�B", "You're not as powerful as you used to be..."));
	for (int k = 0; k < A_MAX; k++)
	{
		target_ptr->stat_cur[k] = (target_ptr->stat_cur[k] * 7) / 8;
		if (target_ptr->stat_cur[k] < 3)
			target_ptr->stat_cur[k] = 3;
	}

	target_ptr->update |= (PU_BONUS);
}


static void effect_player_time_addition(player_type *target_ptr)
{
	switch (randint1(10))
	{
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
	{
		if (target_ptr->prace == RACE_ANDROID) break;

		msg_print(_("�l�����t�߂肵���C������B", "You feel like a chunk of the past has been ripped away."));
		lose_exp(target_ptr, 100 + (target_ptr->exp / 100) * MON_DRAIN_LIFE);
		break;
	}
	case 6:
	case 7:
	case 8:
	case 9:
		effect_player_time_one_disability(target_ptr);
		break;
	case 10:
		effect_player_time_all_disabilities(target_ptr);
		break;
	}
}


void effect_player_time(player_type *target_ptr, effect_player_type *ep_ptr)
{
	if (target_ptr->blind) msg_print(_("�ߋ�����̏Ռ��ɍU�����ꂽ�I", "You are hit by a blast from the past!"));

	if (target_ptr->resist_time)
	{
		ep_ptr->dam *= 4;
		ep_ptr->dam /= (randint1(4) + 7);
		msg_print(_("���Ԃ��ʂ�߂��Ă����C������B", "You feel as if time is passing you by."));
		ep_ptr->get_damage = take_hit(target_ptr, DAMAGE_ATTACK, ep_ptr->dam, ep_ptr->killer, ep_ptr->monspell);
		return;
	}

	if (CHECK_MULTISHADOW(target_ptr))
	{
		ep_ptr->get_damage = take_hit(target_ptr, DAMAGE_ATTACK, ep_ptr->dam, ep_ptr->killer, ep_ptr->monspell);
		return;
	}

	effect_player_time_addition(target_ptr);
	ep_ptr->get_damage = take_hit(target_ptr, DAMAGE_ATTACK, ep_ptr->dam, ep_ptr->killer, ep_ptr->monspell);
}


void effect_player_gravity(player_type *target_ptr, effect_player_type *ep_ptr)
{
	if (target_ptr->blind) msg_print(_("�����d�����̂ōU�����ꂽ�I", "You are hit by something heavy!"));
	msg_print(_("���ӂ̏d�͂��䂪�񂾁B", "Gravity warps around you."));

	if (!CHECK_MULTISHADOW(target_ptr))
	{
		teleport_player(target_ptr, 5, TELEPORT_PASSIVE);
		if (!target_ptr->levitation)
			(void)set_slow(target_ptr, target_ptr->slow + randint0(4) + 4, FALSE);
		if (!(target_ptr->resist_sound || target_ptr->levitation))
		{
			int plus_stun = (randint1((ep_ptr->dam > 90) ? 35 : (ep_ptr->dam / 3 + 5)));
			(void)set_stun(target_ptr, target_ptr->stun + plus_stun);
		}
	}

	if (target_ptr->levitation)
	{
		ep_ptr->dam = (ep_ptr->dam * 2) / 3;
	}

	if (!target_ptr->levitation || one_in_(13))
	{
		inventory_damage(target_ptr, set_cold_destroy, 2);
	}

	ep_ptr->get_damage = take_hit(target_ptr, DAMAGE_ATTACK, ep_ptr->dam, ep_ptr->killer, ep_ptr->monspell);
}


/*!
 * @brief ���@�̌��ʂɂ���ėl�X�ȃ��b�Z�[���o�͂�����^����_���[�W�̑������s�����肷��
 * @param target_ptr �v���[���[�ւ̎Q�ƃ|�C���^
 * @param em_ptr �v���[���[���ʍ\���̂ւ̎Q�ƃ|�C���^
 * @return �Ȃ�
 */
void switch_effects_player(player_type *target_ptr, effect_player_type *ep_ptr)
{
	switch (ep_ptr->effect_type)
	{
	case GF_ACID:
		effect_player_elements(target_ptr, ep_ptr, _("�_�ōU�����ꂽ�I", "You are hit by acid!"), acid_dam);
		return;
	case GF_FIRE:
		effect_player_elements(target_ptr, ep_ptr, _("�Ή��ōU�����ꂽ�I", "You are hit by fire!"), fire_dam);
		return;
	case GF_COLD:
		effect_player_elements(target_ptr, ep_ptr, _("��C�ōU�����ꂽ�I", "You are hit by cold!"), cold_dam);
		return;
	case GF_ELEC:
		effect_player_elements(target_ptr, ep_ptr, _("�d���ōU�����ꂽ�I", "You are hit by lightning!"), elec_dam);
		return;
	case GF_POIS:
		effect_player_poison(target_ptr, ep_ptr);
		return;
	case GF_NUKE:
		effect_player_nuke(target_ptr, ep_ptr);
		return;
	case GF_MISSILE:
		effect_player_missile(target_ptr, ep_ptr);
		return;
	case GF_HOLY_FIRE:
		effect_player_holy_fire(target_ptr, ep_ptr);
		return;
	case GF_HELL_FIRE:
		effect_player_hell_fire(target_ptr, ep_ptr);
		return;
	case GF_ARROW:
		effect_player_arrow(target_ptr, ep_ptr);
		return;
	case GF_PLASMA:
		effect_player_plasma(target_ptr, ep_ptr);
		return;
	case GF_NETHER:
		effect_player_nether(target_ptr, ep_ptr);
		return;
	case GF_WATER:
		effect_player_water(target_ptr, ep_ptr);
		return;
	case GF_CHAOS:
		effect_player_chaos(target_ptr, ep_ptr);
		return;
	case GF_SHARDS:
		effect_player_shards(target_ptr, ep_ptr);
		return;
	case GF_SOUND:
		effect_player_sound(target_ptr, ep_ptr);
		return;
	case GF_CONFUSION:
		effect_player_confusion(target_ptr, ep_ptr);
		return;
	case GF_DISENCHANT:
		effect_player_disenchant(target_ptr, ep_ptr);
		return;
	case GF_NEXUS:
		effect_player_nexus(target_ptr, ep_ptr);
		return;
	case GF_FORCE:
		effect_player_force(target_ptr, ep_ptr);
		return;
	case GF_ROCKET:
		effect_player_rocket(target_ptr, ep_ptr);
		return;
	case GF_INERTIAL:
		effect_player_inertial(target_ptr, ep_ptr);
		return;
	case GF_LITE:
		effect_player_lite(target_ptr, ep_ptr);
		return;
	case GF_DARK:
		effect_player_dark(target_ptr, ep_ptr);
		return;
	case GF_TIME:
		effect_player_time(target_ptr, ep_ptr);
		return;
	case GF_GRAVITY:
		effect_player_gravity(target_ptr, ep_ptr);
		return;
	case GF_DISINTEGRATE:
	{
		if (target_ptr->blind) msg_print(_("�����ȃG�l���M�[�ōU�����ꂽ�I", "You are hit by pure energy!"));

		ep_ptr->get_damage = take_hit(target_ptr, DAMAGE_ATTACK, ep_ptr->dam, ep_ptr->killer, ep_ptr->monspell);
		break;
	}
	case GF_OLD_HEAL:
	{
		if (target_ptr->blind) msg_print(_("���炩�̍U���ɂ���ċC�����悭�Ȃ����B", "You are hit by something invigorating!"));

		(void)hp_player(target_ptr, ep_ptr->dam);
		ep_ptr->dam = 0;
		break;
	}
	case GF_OLD_SPEED:
	{
		if (target_ptr->blind) msg_print(_("�����ōU�����ꂽ�I", "You are hit by something!"));
		(void)set_fast(target_ptr, target_ptr->fast + randint1(5), FALSE);
		ep_ptr->dam = 0;
		break;
	}
	case GF_OLD_SLOW:
	{
		if (target_ptr->blind) msg_print(_("�����x�����̂ōU�����ꂽ�I", "You are hit by something slow!"));
		(void)set_slow(target_ptr, target_ptr->slow + randint0(4) + 4, FALSE);
		break;
	}
	case GF_OLD_SLEEP:
	{
		if (target_ptr->free_act)  break;
		if (target_ptr->blind) msg_print(_("�����Ă��܂����I", "You fall asleep!"));

		if (ironman_nightmare)
		{
			msg_print(_("���낵�����i�����ɕ�����ł����B", "A horrible vision enters your mind."));
			/* Have some nightmares */
			sanity_blast(target_ptr, NULL, FALSE);
		}

		set_paralyzed(target_ptr, target_ptr->paralyzed + ep_ptr->dam);
		ep_ptr->dam = 0;
		break;
	}
	case GF_MANA:
	case GF_SEEKER:
	case GF_SUPER_RAY:
	{
		if (target_ptr->blind) msg_print(_("���@�̃I�[���ōU�����ꂽ�I", "You are hit by an aura of magic!"));

		ep_ptr->get_damage = take_hit(target_ptr, DAMAGE_ATTACK, ep_ptr->dam, ep_ptr->killer, ep_ptr->monspell);
		break;
	}
	case GF_PSY_SPEAR:
	{
		if (target_ptr->blind) msg_print(_("�G�l���M�[�̉�ōU�����ꂽ�I", "You are hit by an energy!"));

		ep_ptr->get_damage = take_hit(target_ptr, DAMAGE_FORCE, ep_ptr->dam, ep_ptr->killer, ep_ptr->monspell);
		break;
	}
	case GF_METEOR:
	{
		if (target_ptr->blind) msg_print(_("�������󂩂炠�Ȃ��̓���ɗ����Ă����I", "Something falls from the sky on you!"));

		ep_ptr->get_damage = take_hit(target_ptr, DAMAGE_ATTACK, ep_ptr->dam, ep_ptr->killer, ep_ptr->monspell);
		if (!target_ptr->resist_shard || one_in_(13))
		{
			if (!target_ptr->immune_fire) inventory_damage(target_ptr, set_fire_destroy, 2);
			inventory_damage(target_ptr, set_cold_destroy, 2);
		}

		break;
	}
	case GF_ICE:
	{
		if (target_ptr->blind) msg_print(_("�����s���₽�����̂ōU�����ꂽ�I", "You are hit by something sharp and cold!"));

		ep_ptr->get_damage = cold_dam(target_ptr, ep_ptr->dam, ep_ptr->killer, ep_ptr->monspell, FALSE);
		if (CHECK_MULTISHADOW(target_ptr)) break;

		if (!target_ptr->resist_shard)
		{
			(void)set_cut(target_ptr, target_ptr->cut + damroll(5, 8));
		}

		if (!target_ptr->resist_sound)
		{
			(void)set_stun(target_ptr, target_ptr->stun + randint1(15));
		}

		if ((!(target_ptr->resist_cold || is_oppose_cold(target_ptr))) || one_in_(12))
		{
			if (!target_ptr->immune_cold) inventory_damage(target_ptr, set_cold_destroy, 3);
		}

		break;
	}
	case GF_DEATH_RAY:
	{
		if (target_ptr->blind) msg_print(_("�������ɗ₽�����̂ōU�����ꂽ�I", "You are hit by something extremely cold!"));

		if (target_ptr->mimic_form)
		{
			if (!(mimic_info[target_ptr->mimic_form].MIMIC_FLAGS & MIMIC_IS_NONLIVING))
				ep_ptr->get_damage = take_hit(target_ptr, DAMAGE_ATTACK, ep_ptr->dam, ep_ptr->killer, ep_ptr->monspell);

			break;
		}

		switch (target_ptr->prace)
		{
		case RACE_GOLEM:
		case RACE_SKELETON:
		case RACE_ZOMBIE:
		case RACE_VAMPIRE:
		case RACE_DEMON:
		case RACE_SPECTRE:
		{
			ep_ptr->dam = 0;
			break;
		}
		default:
		{
			ep_ptr->get_damage = take_hit(target_ptr, DAMAGE_ATTACK, ep_ptr->dam, ep_ptr->killer, ep_ptr->monspell);
			break;
		}
		}

		break;
	}
	case GF_DRAIN_MANA:
	{
		if (CHECK_MULTISHADOW(target_ptr))
		{
			msg_print(_("�U���͌��e�ɖ������A���Ȃ��ɂ͓͂��Ȃ������B", "The attack hits Shadow, but you are unharmed!"));
			ep_ptr->dam = 0;
			break;
		}

		if (target_ptr->csp == 0)
		{
			ep_ptr->dam = 0;
			break;
		}

		if (ep_ptr->who > 0)
			msg_format(_("%^s�ɐ��_�G�l���M�[���z������Ă��܂����I", "%^s draws psychic energy from you!"), ep_ptr->m_name);
		else
			msg_print(_("���_�G�l���M�[���z������Ă��܂����I", "Your psychic energy is drawn!"));

		if (ep_ptr->dam >= target_ptr->csp)
		{
			ep_ptr->dam = target_ptr->csp;
			target_ptr->csp = 0;
			target_ptr->csp_frac = 0;
		}
		else
		{
			target_ptr->csp -= ep_ptr->dam;
		}

		learn_spell(target_ptr, ep_ptr->monspell);
		target_ptr->redraw |= (PR_MANA);
		target_ptr->window |= (PW_PLAYER | PW_SPELL);

		if ((ep_ptr->who <= 0) || (ep_ptr->m_ptr->hp >= ep_ptr->m_ptr->maxhp))
		{
			ep_ptr->dam = 0;
			break;
		}

		ep_ptr->m_ptr->hp += ep_ptr->dam;
		if (ep_ptr->m_ptr->hp > ep_ptr->m_ptr->maxhp) ep_ptr->m_ptr->hp = ep_ptr->m_ptr->maxhp;

		if (target_ptr->health_who == ep_ptr->who) target_ptr->redraw |= (PR_HEALTH);
		if (target_ptr->riding == ep_ptr->who) target_ptr->redraw |= (PR_UHEALTH);

		if (ep_ptr->m_ptr->ml)
		{
			msg_format(_("%^s�͋C�����ǂ��������B", "%^s appears healthier."), ep_ptr->m_name);
		}

		ep_ptr->dam = 0;
		break;
	}
	case GF_MIND_BLAST:
	{
		if ((randint0(100 + ep_ptr->rlev / 2) < MAX(5, target_ptr->skill_sav)) && !CHECK_MULTISHADOW(target_ptr))
		{
			msg_print(_("���������͂𒵂˕Ԃ����I", "You resist the effects!"));
			learn_spell(target_ptr, ep_ptr->monspell);
			break;
		}

		if (CHECK_MULTISHADOW(target_ptr))
		{
			ep_ptr->get_damage = take_hit(target_ptr, DAMAGE_ATTACK, ep_ptr->dam, ep_ptr->killer, ep_ptr->monspell);
			break;
		}

		msg_print(_("��I�G�l���M�[�Ő��_���U�����ꂽ�B", "Your mind is blasted by psionic energy."));
		if (!target_ptr->resist_conf)
		{
			(void)set_confused(target_ptr, target_ptr->confused + randint0(4) + 4);
		}

		if (!target_ptr->resist_chaos && one_in_(3))
		{
			(void)set_image(target_ptr, target_ptr->image + randint0(250) + 150);
		}

		target_ptr->csp -= 50;
		if (target_ptr->csp < 0)
		{
			target_ptr->csp = 0;
			target_ptr->csp_frac = 0;
		}

		target_ptr->redraw |= PR_MANA;
		ep_ptr->get_damage = take_hit(target_ptr, DAMAGE_ATTACK, ep_ptr->dam, ep_ptr->killer, ep_ptr->monspell);
		break;
	}
	case GF_BRAIN_SMASH:
	{
		if ((randint0(100 + ep_ptr->rlev / 2) < MAX(5, target_ptr->skill_sav)) && !CHECK_MULTISHADOW(target_ptr))
		{
			msg_print(_("���������͂𒵂˕Ԃ����I", "You resist the effects!"));
			learn_spell(target_ptr, ep_ptr->monspell);
			break;
		}

		if (!CHECK_MULTISHADOW(target_ptr))
		{
			msg_print(_("��I�G�l���M�[�Ő��_���U�����ꂽ�B", "Your mind is blasted by psionic energy."));

			target_ptr->csp -= 100;
			if (target_ptr->csp < 0)
			{
				target_ptr->csp = 0;
				target_ptr->csp_frac = 0;
			}
			target_ptr->redraw |= PR_MANA;
		}

		ep_ptr->get_damage = take_hit(target_ptr, DAMAGE_ATTACK, ep_ptr->dam, ep_ptr->killer, ep_ptr->monspell);
		if (CHECK_MULTISHADOW(target_ptr)) break;

		if (!target_ptr->resist_blind)
		{
			(void)set_blind(target_ptr, target_ptr->blind + 8 + randint0(8));
		}

		if (!target_ptr->resist_conf)
		{
			(void)set_confused(target_ptr, target_ptr->confused + randint0(4) + 4);
		}

		if (!target_ptr->free_act)
		{
			(void)set_paralyzed(target_ptr, target_ptr->paralyzed + randint0(4) + 4);
		}

		(void)set_slow(target_ptr, target_ptr->slow + randint0(4) + 4, FALSE);

		while (randint0(100 + ep_ptr->rlev / 2) > (MAX(5, target_ptr->skill_sav)))
			(void)do_dec_stat(target_ptr, A_INT);
		while (randint0(100 + ep_ptr->rlev / 2) > (MAX(5, target_ptr->skill_sav)))
			(void)do_dec_stat(target_ptr, A_WIS);

		if (!target_ptr->resist_chaos)
		{
			(void)set_image(target_ptr, target_ptr->image + randint0(250) + 150);
		}

		break;
	}
	case GF_CAUSE_1:
	{
		if ((randint0(100 + ep_ptr->rlev / 2) < target_ptr->skill_sav) && !CHECK_MULTISHADOW(target_ptr))
		{
			msg_print(_("���������͂𒵂˕Ԃ����I", "You resist the effects!"));
			learn_spell(target_ptr, ep_ptr->monspell);
		}
		else
		{
			if (!CHECK_MULTISHADOW(target_ptr)) curse_equipment(target_ptr, 15, 0);
			ep_ptr->get_damage = take_hit(target_ptr, DAMAGE_ATTACK, ep_ptr->dam, ep_ptr->killer, ep_ptr->monspell);
		}
		break;
	}
	case GF_CAUSE_2:
	{
		if ((randint0(100 + ep_ptr->rlev / 2) < target_ptr->skill_sav) && !CHECK_MULTISHADOW(target_ptr))
		{
			msg_print(_("���������͂𒵂˕Ԃ����I", "You resist the effects!"));
			learn_spell(target_ptr, ep_ptr->monspell);
		}
		else
		{
			if (!CHECK_MULTISHADOW(target_ptr)) curse_equipment(target_ptr, 25, MIN(ep_ptr->rlev / 2 - 15, 5));
			ep_ptr->get_damage = take_hit(target_ptr, DAMAGE_ATTACK, ep_ptr->dam, ep_ptr->killer, ep_ptr->monspell);
		}
		break;
	}
	case GF_CAUSE_3:
	{
		if ((randint0(100 + ep_ptr->rlev / 2) < target_ptr->skill_sav) && !CHECK_MULTISHADOW(target_ptr))
		{
			msg_print(_("���������͂𒵂˕Ԃ����I", "You resist the effects!"));
			learn_spell(target_ptr, ep_ptr->monspell);
		}
		else
		{
			if (!CHECK_MULTISHADOW(target_ptr)) curse_equipment(target_ptr, 33, MIN(ep_ptr->rlev / 2 - 15, 15));
			ep_ptr->get_damage = take_hit(target_ptr, DAMAGE_ATTACK, ep_ptr->dam, ep_ptr->killer, ep_ptr->monspell);
		}
		break;
	}
	case GF_CAUSE_4:
	{
		if ((randint0(100 + ep_ptr->rlev / 2) < target_ptr->skill_sav) && !(ep_ptr->m_ptr->r_idx == MON_KENSHIROU) && !CHECK_MULTISHADOW(target_ptr))
		{
			msg_print(_("��������E�𒵂˕Ԃ����I", "You resist the effects!"));
			learn_spell(target_ptr, ep_ptr->monspell);
		}
		else
		{
			ep_ptr->get_damage = take_hit(target_ptr, DAMAGE_ATTACK, ep_ptr->dam, ep_ptr->killer, ep_ptr->monspell);
			if (!CHECK_MULTISHADOW(target_ptr)) (void)set_cut(target_ptr, target_ptr->cut + damroll(10, 10));
		}

		break;
	}
	case GF_HAND_DOOM:
	{
		if ((randint0(100 + ep_ptr->rlev / 2) < target_ptr->skill_sav) && !CHECK_MULTISHADOW(target_ptr))
		{
			msg_print(_("���������͂𒵂˕Ԃ����I", "You resist the effects!"));
			learn_spell(target_ptr, ep_ptr->monspell);
		}
		else
		{
			if (!CHECK_MULTISHADOW(target_ptr))
			{
				msg_print(_("���Ȃ��͖������܂��Ă����悤�Ɋ������I", "You feel your life fade away!"));
				curse_equipment(target_ptr, 40, 20);
			}

			ep_ptr->get_damage = take_hit(target_ptr, DAMAGE_ATTACK, ep_ptr->dam, ep_ptr->m_name, ep_ptr->monspell);

			if (target_ptr->chp < 1) target_ptr->chp = 1;
		}

		break;
	}
	default:
	{
		ep_ptr->dam = 0;
		break;
	}
	}
}
