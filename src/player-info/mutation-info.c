#include "player-info/mutation-info.h"
#include "mutation/mutation-flag-types.h"
#include "player-info/self-info-util.h"
#include "player/player-status-flags.h"

void set_mutation_info_1(player_type *creature_ptr, self_info_type *si_ptr)
{
    if (creature_ptr->muta1 == 0)
        return;

    if (creature_ptr->muta1 & MUT1_SPIT_ACID)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͎_�𐁂������邱�Ƃ��ł���B(�_���[�W ���x��X1)", "You can spit acid (dam lvl).");

    if (creature_ptr->muta1 & MUT1_BR_FIRE)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͉��̃u���X��f�����Ƃ��ł���B(�_���[�W ���x��X2)", "You can breathe fire (dam lvl * 2).");

    if (creature_ptr->muta1 & MUT1_HYPN_GAZE)
        si_ptr->info[si_ptr->line++] = _("���Ȃ����ɂ݂͍Ö����ʂ����B", "Your gaze is hypnotic.");

    if (creature_ptr->muta1 & MUT1_TELEKINES)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͔O���͂������Ă���B", "You are telekinetic.");

    if (creature_ptr->muta1 & MUT1_VTELEPORT)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͎����̈ӎv�Ńe���|�[�g�ł���B", "You can teleport at will.");

    if (creature_ptr->muta1 & MUT1_MIND_BLST)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͐��_�U�����s����B(�_���[�W 3�`12d3)", "You can Mind Blast your enemies (3 to 12d3 dam).");

    if (creature_ptr->muta1 & MUT1_RADIATION)
        si_ptr->info[si_ptr->line++]
            = _("���Ȃ��͎����̈ӎv�ŋ������ː��𔭐����邱�Ƃ��ł���B(�_���[�W ���x��X2)", "You can emit hard radiation at will (dam lvl * 2).");

    if (creature_ptr->muta1 & MUT1_VAMPIRISM)
        si_ptr->info[si_ptr->line++] = _(
            "���Ȃ��͋z���S�̂悤�ɓG���琶���͂��z�����邱�Ƃ��ł���B(�_���[�W ���x��X2)", "Like a vampire, you can drain life from a foe (dam lvl * 2).");

    if (creature_ptr->muta1 & MUT1_SMELL_MET)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͋߂��ɂ���M���������������邱�Ƃ��ł���B", "You can smell nearby precious metal.");

    if (creature_ptr->muta1 & MUT1_SMELL_MON)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͋߂��̃����X�^�[�̑��݂����������邱�Ƃ��ł���B", "You can smell nearby monsters.");

    if (creature_ptr->muta1 & MUT1_BLINK)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͒Z���������e���|�[�g�ł���B", "You can teleport yourself short distances.");

    if (creature_ptr->muta1 & MUT1_EAT_ROCK)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͍d�����H�ׂ邱�Ƃ��ł���B", "You can consume solid rock.");

    if (creature_ptr->muta1 & MUT1_SWAP_POS)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͑��̎҂Əꏊ�����ւ�邱�Ƃ��ł���B", "You can switch locations with another being.");

    if (creature_ptr->muta1 & MUT1_SHRIEK)
        si_ptr->info[si_ptr->line++]
            = _("���Ȃ��͐g�̖т��悾���ѐ��𔭂��邱�Ƃ��ł���B(�_���[�W ���x��X2)", "You can emit a horrible shriek (dam 2 * lvl).");

    if (creature_ptr->muta1 & MUT1_ILLUMINE)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͖��邢��������Ƃ��ł���B", "You can emit bright light.");

    if (creature_ptr->muta1 & MUT1_DET_CURSE)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͎׈��Ȗ��@�̊댯�������Ƃ邱�Ƃ��ł���B", "You can feel the danger of evil magic.");

    if (creature_ptr->muta1 & MUT1_BERSERK)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͎����̈ӎv�ŋ����퓬��ԂɂȂ邱�Ƃ��ł���B", "You can drive yourself into a berserk frenzy.");

    if (creature_ptr->muta1 & MUT1_POLYMORPH)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͎����̈ӎu�ŕω��ł���B", "You can polymorph yourself at will.");

    if (creature_ptr->muta1 & MUT1_MIDAS_TCH)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͒ʏ�A�C�e�������ɕς��邱�Ƃ��ł���B", "You can turn ordinary items to gold.");

    if (creature_ptr->muta1 & MUT1_GROW_MOLD)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͎��͂ɃL�m�R�𐶂₷���Ƃ��ł���B", "You can cause mold to grow near you.");

    if (creature_ptr->muta1 & MUT1_RESIST)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͌��f�̍U���ɑ΂��Đg���d�����邱�Ƃ��ł���B", "You can harden yourself to the ravages of the elements.");

    if (creature_ptr->muta1 & MUT1_EARTHQUAKE)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͎��͂̃_���W��������󂳂��邱�Ƃ��ł���B", "You can bring down the dungeon around your ears.");

    if (creature_ptr->muta1 & MUT1_EAT_MAGIC)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͖��@�̃G�l���M�[�������̕��Ƃ��Ďg�p�ł���B", "You can consume magic energy for your own use.");

    if (creature_ptr->muta1 & MUT1_WEIGH_MAG)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͎����ɉe����^���閂�@�̗͂������邱�Ƃ��ł���B", "You can feel the strength of the magics affecting you.");

    if (creature_ptr->muta1 & MUT1_STERILITY)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͏W�c�I���B�s�\���N�������Ƃ��ł���B", "You can cause mass impotence.");

    if (creature_ptr->muta1 & MUT1_HIT_AND_AWAY)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͍U��������g����邽�ߓ����邱�Ƃ��ł���B", "You can run for your life after hitting something.");

    if (creature_ptr->muta1 & MUT1_DAZZLE)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͍����ƖӖڂ������N�������˔\�𔭐����邱�Ƃ��ł���B ", "You can emit confusing, blinding radiation.");

    if (creature_ptr->muta1 & MUT1_LASER_EYE)
        si_ptr->info[si_ptr->line++]
            = _("���Ȃ��͖ڂ��烌�[�U�[�����𔭂��邱�Ƃ��ł���B(�_���[�W ���x��X2)", "Your eyes can fire laser beams (dam 2 * lvl).");

    if (creature_ptr->muta1 & MUT1_RECALL)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͊X�ƃ_���W�����̊Ԃ��s�������邱�Ƃ��ł���B", "You can travel between town and the depths.");

    if (creature_ptr->muta1 & MUT1_BANISH)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͎׈��ȃ����X�^�[��n���ɗ��Ƃ����Ƃ��ł���B", "You can send evil creatures directly to Hell.");

    if (creature_ptr->muta1 & MUT1_COLD_TOUCH)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͓G��G���ē��点�邱�Ƃ��ł���B(�_���[�W ���x��X3)", "You can freeze things with a touch (dam 3 * lvl).");

    if (creature_ptr->muta1 & MUT1_LAUNCHER)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��̓A�C�e����͋��������邱�Ƃ��ł���B", "You can hurl objects with great force.");
}

void set_mutation_info_2(player_type *creature_ptr, self_info_type *si_ptr)
{
    if (creature_ptr->muta2 == 0)
        return;

    if (creature_ptr->muta2 & MUT2_BERS_RAGE)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͋���m���̔�����N�����B", "You are subject to berserker fits.");

    if (creature_ptr->muta2 & MUT2_COWARDICE)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͎��X���a�ɂȂ�B", "You are subject to cowardice.");

    if (creature_ptr->muta2 & MUT2_RTELEPORT)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��̓����_���Ƀe���|�[�g����B", "You may randomly teleport.");

    if (creature_ptr->muta2 & MUT2_ALCOHOL)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��̑̂̓A���R�[���𕪔傷��B", "Your body produces alcohol.");

    if (creature_ptr->muta2 & MUT2_HALLU)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͌��o�������N�������_�����ɐN����Ă���B", "You have a hallucinatory insanity.");

    if (creature_ptr->muta2 & MUT2_FLATULENT)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͐���ł��Ȃ�����ț��������B", "You are subject to uncontrollable flatulence.");

    if (creature_ptr->muta2 & MUT2_PROD_MANA)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͐���s�\�Ȗ��@�̃G�l���M�[�𔭂��Ă���B", "You produce magical energy uncontrollably.");

    if (creature_ptr->muta2 & MUT2_ATT_DEMON)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��̓f�[��������������B", "You attract demons.");

    if (creature_ptr->muta2 & MUT2_SCOR_TAIL)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��̓T�\���̐K���������Ă���B(�ŁA�_���[�W 3d7)", "You have a scorpion tail (poison, 3d7).");

    if (creature_ptr->muta2 & MUT2_HORNS)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͊p�������Ă���B(�_���[�W 2d6)", "You have horns (dam. 2d6).");

    if (creature_ptr->muta2 & MUT2_BEAK)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��̓N�`�o�V�������Ă���B(�_���[�W 2d4)", "You have a beak (dam. 2d4).");

    if (creature_ptr->muta2 & MUT2_SPEED_FLUX)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��̓����_���ɑ�����������x���������肷��B", "You move faster or slower randomly.");

    if (creature_ptr->muta2 & MUT2_BANISH_ALL)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͎��X�߂��̃����X�^�[�����ł�����B", "You sometimes cause nearby creatures to vanish.");

    if (creature_ptr->muta2 & MUT2_EAT_LIGHT)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͎��X���͂̌����z�����ĉh�{�ɂ���B", "You sometimes feed off of the light around you.");

    if (creature_ptr->muta2 & MUT2_TRUNK)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͏ۂ̂悤�ȕ@�������Ă���B(�_���[�W 1d4)", "You have an elephantine trunk (dam 1d4).");

    if (creature_ptr->muta2 & MUT2_ATT_ANIMAL)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͓�������������B", "You attract animals.");

    if (creature_ptr->muta2 & MUT2_TENTACLES)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͎׈��ȐG��������Ă���B(�_���[�W 2d5)", "You have evil looking tentacles (dam 2d5).");

    if (creature_ptr->muta2 & MUT2_RAW_CHAOS)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͂��΂��Ώ��J�I�X�ɕ�܂��B", "You occasionally are surrounded with raw chaos.");

    if (creature_ptr->muta2 & MUT2_NORMALITY)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͕ψق��Ă������A�񕜂��Ă��Ă���B", "You may be mutated, but you're recovering.");

    if (creature_ptr->muta2 & MUT2_WRAITH)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��̓��̂͗H�̉���������̉������肷��B", "You fade in and out of physical reality.");

    if (creature_ptr->muta2 & MUT2_POLY_WOUND)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��̌��N�̓J�I�X�̗͂ɉe�����󂯂�B", "Your health is subject to chaotic forces.");

    if (creature_ptr->muta2 & MUT2_WASTING)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͐��シ�鋰�낵���a�C�ɂ������Ă���B", "You have a horrible wasting disease.");

    if (creature_ptr->muta2 & MUT2_ATT_DRAGON)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��̓h���S������������B", "You attract dragons.");

    if (creature_ptr->muta2 & MUT2_WEIRD_MIND)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��̐��_�̓����_���Ɋg�債����k�������肵�Ă���B", "Your mind randomly expands and contracts.");

    if (creature_ptr->muta2 & MUT2_NAUSEA)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��݂͔̈��ɗ����������Ȃ��B", "You have a seriously upset stomach.");

    if (creature_ptr->muta2 & MUT2_CHAOS_GIFT)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��̓J�I�X�̎�숫������J���������Ƃ�B", "Chaos deities give you gifts.");

    if (creature_ptr->muta2 & MUT2_WALK_SHAD)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͂��΂��Α��́u�e�v�ɖ������ށB", "You occasionally stumble into other shadows.");

    if (creature_ptr->muta2 & MUT2_WARNING)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͓G�Ɋւ���x����������B", "You receive warnings about your foes.");

    if (creature_ptr->muta2 & MUT2_INVULN)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͎��X�����m�炸�ȋC���ɂȂ�B", "You occasionally feel invincible.");

    if (creature_ptr->muta2 & MUT2_SP_TO_HP)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͎��X�����ؓ��ɂǂ��Ɨ����B", "Your blood sometimes rushes to your muscles.");

    if (creature_ptr->muta2 & MUT2_HP_TO_SP)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͎��X���Ɍ����ǂ��Ɨ����B", "Your blood sometimes rushes to your head.");

    if (creature_ptr->muta2 & MUT2_DISARM)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͂悭�܂Â��ĕ��𗎂Ƃ��B", "You occasionally stumble and drop things.");
}

/* todo FEAELESS �t���O���L�q���Ė��Ȃ��Ǝv���� */
void set_mutation_info_3(player_type *creature_ptr, self_info_type *si_ptr)
{
    if (creature_ptr->muta3 == 0)
        return;

    if (creature_ptr->muta3 & MUT3_HYPER_STR)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͒��l�I�ɋ����B(�r��+4)", "You are superhumanly strong (+4 STR).");

    if (creature_ptr->muta3 & MUT3_PUNY)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͋��ゾ�B(�r��-4)", "You are puny (-4 STR).");

    if (creature_ptr->muta3 & MUT3_HYPER_INT)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��̔]�͐��̃R���s���[�^���B(�m�\������+4)", "Your brain is a living computer (+4 INT/WIS).");

    if (creature_ptr->muta3 & MUT3_MORONIC)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͐��_���ゾ�B(�m�\������-4)", "You are moronic (-4 INT/WIS).");

    if (creature_ptr->muta3 & MUT3_RESILIENT)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͔��Ƀ^�t���B(�ϋv+4)", "You are very resilient (+4 CON).");

    if (creature_ptr->muta3 & MUT3_XTRA_FAT)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͋ɒ[�ɑ����Ă���B(�ϋv+2,�X�s�[�h-2)", "You are extremely fat (+2 CON, -2 speed).");

    if (creature_ptr->muta3 & MUT3_ALBINO)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��̓A���r�m���B(�ϋv-4)", "You are an albino (-4 CON).");

    if (creature_ptr->muta3 & MUT3_FLESH_ROT)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��̓��͕̂��s���Ă���B(�ϋv-2,����-1)", "Your flesh is rotting (-2 CON, -1 CHR).");

    if (creature_ptr->muta3 & MUT3_SILLY_VOI)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��̐��͊Ԕ����ȃL�[�L�[�����B(����-4)", "Your voice is a silly squeak (-4 CHR).");

    if (creature_ptr->muta3 & MUT3_BLANK_FAC)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͂̂��؂�ڂ����B(����-1)", "Your face is featureless (-1 CHR).");

    if (creature_ptr->muta3 & MUT3_ILL_NORM)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͌��e�ɕ����Ă���B", "Your appearance is masked with illusion.");

    if (creature_ptr->muta3 & MUT3_XTRA_EYES)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͗]���ɓ�̖ڂ������Ă���B(�T��+15)", "You have an extra pair of eyes (+15 search).");

    if (creature_ptr->muta3 & MUT3_MAGIC_RES)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͖��@�ւ̑ϐ��������Ă���B", "You are resistant to magic.");

    if (creature_ptr->muta3 & MUT3_XTRA_NOIS)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͕ςȉ��𔭂��Ă���B(�B��-3)", "You make a lot of strange noise (-3 stealth).");

    if (creature_ptr->muta3 & MUT3_INFRAVIS)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͑f���炵���ԊO�����͂������Ă���B(+3)", "You have remarkable infravision (+3).");

    if (creature_ptr->muta3 & MUT3_XTRA_LEGS)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͗]���ɓ�{�̑��������Ă���B(����+3)", "You have an extra pair of legs (+3 speed).");

    if (creature_ptr->muta3 & MUT3_SHORT_LEG)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��̑��͒Z���ˋN���B(����-3)", "Your legs are short stubs (-3 speed).");

    if (creature_ptr->muta3 & MUT3_ELEC_TOUC)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��̌��ǂɂ͓d��������Ă���B", "Electricity is running through your veins.");

    if (creature_ptr->muta3 & MUT3_FIRE_BODY)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͉̑̂��ɂ܂�Ă���B", "Your body is enveloped in flames.");

    if (creature_ptr->muta3 & MUT3_WART_SKIN)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��̔��̓C�{�ɔ���Ă���B(����-2, AC+5)", "Your skin is covered with warts (-2 CHR, +5 AC).");

    if (creature_ptr->muta3 & MUT3_SCALES)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��̔��͗؂ɂȂ��Ă���B(����-1, AC+10)", "Your skin has turned into scales (-1 CHR, +10 AC).");

    if (creature_ptr->muta3 & MUT3_IRON_SKIN)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��̔��͓S�łł��Ă���B(��p-1, AC+25)", "Your skin is made of steel (-1 DEX, +25 AC).");

    if (creature_ptr->muta3 & MUT3_WINGS)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͉H�������Ă���B", "You have wings.");

    if (creature_ptr->muta3 & MUT3_FEARLESS) {
        /* Unnecessary */
    }

    if (creature_ptr->muta3 & MUT3_REGEN) {
        /* Unnecessary */
    }

    if (creature_ptr->muta3 & MUT3_ESP) {
        /* Unnecessary */
    }

    if (creature_ptr->muta3 & MUT3_LIMBER)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͔̑̂��ɂ��Ȃ₩���B(��p+3)", "Your body is very limber (+3 DEX).");

    if (creature_ptr->muta3 & MUT3_ARTHRITIS)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͂����֐߂ɒɂ݂������Ă���B(��p-3)", "Your joints ache constantly (-3 DEX).");

    if (creature_ptr->muta3 & MUT3_VULN_ELEM)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͌��f�̍U���Ɏア�B", "You are susceptible to damage from the elements.");

    if (creature_ptr->muta3 & MUT3_MOTION)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��̓���͐��m�ŗ͋����B(�B��+1)", "Your movements are precise and forceful (+1 STL).");

    if (has_good_luck(creature_ptr))
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͔����I�[���ɂ܂�Ă���B", "There is a white aura surrounding you.");

    if (creature_ptr->muta3 & MUT3_BAD_LUCK)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͍����I�[���ɂ܂�Ă���B", "There is a black aura surrounding you.");
}
