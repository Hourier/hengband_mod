#include "player-info/class-ability-info.h"
#include "player-info/self-info-util.h"
#include "realm/realm-names-table.h"
#include "realm/realm-types.h"

void set_class_ability_info(player_type *creature_ptr, self_info_type *si_ptr)
{
    switch (creature_ptr->pclass) {
    case CLASS_WARRIOR:
        if (creature_ptr->lev > 39)
            si_ptr->info[si_ptr->line++]
                = _("���Ȃ��̓����_���ȕ����ɑ΂��Đ���U�����邱�Ƃ��ł���B(75 MP)", "You can attack some random directions simultaneously (cost 75).");

        break;
    case CLASS_HIGH_MAGE:
        if (creature_ptr->realm1 == REALM_HEX)
            break;
        /* Fall through */
    case CLASS_MAGE:
    case CLASS_SORCERER:
        if (creature_ptr->lev > 24)
            si_ptr->info[si_ptr->line++] = _("���Ȃ��̓A�C�e���̖��͂��z�����邱�Ƃ��ł���B(1 MP)", "You can absorb charges from an item (cost 1).");

        break;
    case CLASS_PRIEST:
        if (is_good_realm(creature_ptr->realm1)) {
            if (creature_ptr->lev > 34)
                si_ptr->info[si_ptr->line++] = _("���Ȃ��͕�����j�����邱�Ƃ��ł���B(70 MP)", "You can bless a weapon (cost 70).");

            break;
        }

        if (creature_ptr->lev > 41)
            si_ptr->info[si_ptr->line++]
                = _("���Ȃ��͎���̂��ׂẴ����X�^�[���U�����邱�Ƃ��ł���B(40 MP)", "You can damage all monsters in sight (cost 40).");

        break;
    case CLASS_ROGUE:
        if (creature_ptr->lev > 7)
            si_ptr->info[si_ptr->line++]
                = _("���Ȃ��͍U�����đ����ɓ����邱�Ƃ��ł���B(12 MP)", "You can hit a monster and teleport away simultaneously (cost 12).");

        break;
    case CLASS_RANGER:
        if (creature_ptr->lev > 14)
            si_ptr->info[si_ptr->line++] = _("���Ȃ��͉����𒲍����邱�Ƃ��ł���B(20 MP)", "You can probe monsters (cost 20).");

        break;
    case CLASS_PALADIN:
        if (is_good_realm(creature_ptr->realm1)) {
            if (creature_ptr->lev > 29) {
                si_ptr->info[si_ptr->line++] = _("���Ȃ��͐��Ȃ鑄������Ƃ��ł���B(30 MP)", "You can fire a holy spear (cost 30).");
            }

            break;
        }

        if (creature_ptr->lev > 29)
            si_ptr->info[si_ptr->line++]
                = _("���Ȃ��͐����͂����������鑄������Ƃ��ł���B(30 MP)", "You can fire a spear which drains vitality (cost 30).");

        break;
    case CLASS_WARRIOR_MAGE:
        if (creature_ptr->lev > 24) {
            si_ptr->info[si_ptr->line++] = _("���Ȃ��͂g�o���l�o�ɕϊ����邱�Ƃ��ł���B(0 MP)", "You can convert HP to SP (cost 0).");
            si_ptr->info[si_ptr->line++] = _("���Ȃ��͂l�o���g�o�ɕϊ����邱�Ƃ��ł���B(0 MP)", "You can convert SP to HP (cost 0).");
        }

        break;
    case CLASS_CHAOS_WARRIOR:
        if (creature_ptr->lev > 39) {
            si_ptr->info[si_ptr->line++]
                = _("���Ȃ��͎��͂ɉ�����f�킷���𔭐������邱�Ƃ��ł���B(50 MP)", "You can radiate light which confuses nearby monsters (cost 50).");
        }

        break;
    case CLASS_MONK:
        if (creature_ptr->lev > 24)
            si_ptr->info[si_ptr->line++] = _("���Ȃ��͍\���邱�Ƃ��ł���B(0 MP)", "You can assume a special stance (cost 0).");

        if (creature_ptr->lev > 29)
            si_ptr->info[si_ptr->line++] = _("���Ȃ��͒ʏ��2�{�̍U�����s�����Ƃ��ł���B(30 MP)", "You can perform two attacks at the same time (cost 30).");

        break;
    case CLASS_MINDCRAFTER:
    case CLASS_FORCETRAINER:
        if (creature_ptr->lev > 14)
            si_ptr->info[si_ptr->line++]
                = _("���Ȃ��͐��_���W�����Ăl�o���񕜂����邱�Ƃ��ł���B(0 MP)", "You can concentrate to regenerate your mana (cost 0).");

        break;
    case CLASS_TOURIST:
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͎ʐ^���B�e���邱�Ƃ��ł���B(0 MP)", "You can take a photograph (cost 0).");
        if (creature_ptr->lev > 24)
            si_ptr->info[si_ptr->line++] = _("���Ȃ��̓A�C�e�������S�ɊӒ肷�邱�Ƃ��ł���B(20 MP)", "You can *identify* items (cost 20).");

        break;
    case CLASS_IMITATOR:
        if (creature_ptr->lev > 29)
            si_ptr->info[si_ptr->line++] = _("���Ȃ��͉����̓���U�����_���[�W2�{�ł܂˂邱�Ƃ��ł���B(100 MP)",
                "You can imitate monster's special attacks with double damage (cost 100).");

        break;
    case CLASS_BEASTMASTER:
        si_ptr->info[si_ptr->line++]
            = _("���Ȃ���1�̂̐����̂��郂���X�^�[���x�z���邱�Ƃ��ł���B(���x��/4 MP)", "You can dominate a monster (cost level/4).");
        if (creature_ptr->lev > 29)
            si_ptr->info[si_ptr->line++] = _("���Ȃ��͎��E���̐����̂��郂���X�^�[���x�z���邱�Ƃ��ł���B((���x��+20)/2 MP)",
                "You can dominate living monsters in sight (cost (level+20)/4).");

        break;
    case CLASS_MAGIC_EATER:
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͏�/���@�_/���b�h�̖��͂������̂��̂ɂ��邱�Ƃ��ł���B", "You can absorb a staff, wand or rod itself.");
        break;
    case CLASS_RED_MAGE:
        if (creature_ptr->lev > 47)
            si_ptr->info[si_ptr->line++] = _("���Ȃ���1�^�[����2�񖂖@�������邱�Ƃ��ł���B(20 MP)", "You can cast two spells simultaneously (cost 20).");

        break;
    case CLASS_SAMURAI:
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͐��_���W�����ċC�����𗭂߂邱�Ƃ��ł���B", "You can concentrate to regenerate your mana.");
        if (creature_ptr->lev > 24)
            si_ptr->info[si_ptr->line++] = _("���Ȃ��͓���Ȍ^�ō\���邱�Ƃ��ł���B", "You can assume a special stance.");

        break;
    case CLASS_BLUE_MAGE:
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͑���Ɏg��ꂽ���@���w�Ԃ��Ƃ��ł���B", "You can study spells which your enemy casts on you.");
        break;
    case CLASS_CAVALRY:
        if (creature_ptr->lev > 9)
            si_ptr->info[si_ptr->line++]
                = _("���Ȃ��̓����X�^�[�ɏ���Ė�����y�b�g�ɂ��邱�Ƃ��ł���B", "You can ride on a hostile monster to forcibly turn it into a pet.");

        break;
    case CLASS_BERSERKER:
        if (creature_ptr->lev > 9)
            si_ptr->info[si_ptr->line++] = _("���Ȃ��͊X�ƃ_���W�����̊Ԃ��s�������邱�Ƃ��ł���B", "You can travel between town and the depths.");

        break;
    case CLASS_MIRROR_MASTER:
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͋������o�����Ƃ��ł���B(2 MP)", "You can create a Mirror (cost 2).");
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͋������邱�Ƃ��ł���B(0 MP)", "You can break distant Mirrors (cost 0).");
        break;
    case CLASS_NINJA:
        if (creature_ptr->lev > 19)
            si_ptr->info[si_ptr->line++] = _("���Ȃ��͑f�����ړ����邱�Ƃ��ł���B", "You can walk extremely fast.");

        break;
    }
}
