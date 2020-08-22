#include "player-info/race-ability-info.h"
#include "player-info/self-info-util.h"

void set_race_ability_info(player_type *creature_ptr, self_info_type *si_ptr)
{
    switch (creature_ptr->prace) {
    case RACE_NIBELUNG:
    case RACE_DWARF:
        if (creature_ptr->lev > 4)
            si_ptr->info[si_ptr->line++] = _("���Ȃ���㩂ƃh�A�ƊK�i�����m�ł���B(5 MP)", "You can find traps, doors and stairs (cost 5).");

        break;
    case RACE_HOBBIT:
        if (creature_ptr->lev > 14)
            si_ptr->info[si_ptr->line++] = _("���Ȃ��͐H���𐶐��ł���B(10 MP)", "You can produce food (cost 10).");

        break;
    case RACE_GNOME:
        if (creature_ptr->lev > 4) {
            sprintf(si_ptr->plev_buf, _("���Ȃ��͔͈� %d �ȓ��Ƀe���|�[�g�ł���B(%d MP)", "You can teleport, range %d (cost %d)."), (1 + creature_ptr->lev),
                (5 + (creature_ptr->lev / 5)));
            si_ptr->info[si_ptr->line++] = si_ptr->plev_buf;
        }

        break;
    case RACE_HALF_ORC:
        if (creature_ptr->lev > 2)
            si_ptr->info[si_ptr->line++] = _("���Ȃ��͋��|�������ł���B(5 MP)", "You can remove fear (cost 5).");

        break;
    case RACE_HALF_TROLL:
        if (creature_ptr->lev > 9)
            si_ptr->info[si_ptr->line++] = _("���Ȃ��͋��\�����邱�Ƃ��ł���B(12 MP) ", "You can enter a berserk fury (cost 12).");

        break;
    case RACE_AMBERITE:
        if (creature_ptr->lev > 29)
            si_ptr->info[si_ptr->line++] = _("���Ȃ��̓V���h�E�V�t�g���邱�Ƃ��ł���B(50 MP)", "You can Shift Shadows (cost 50).");

        if (creature_ptr->lev > 39)
            si_ptr->info[si_ptr->line++] = _("���Ȃ��́u�p�^�[���v��S�ɕ`���ĕ������Ƃ��ł���B(75 MP)", "You can mentally Walk the Pattern (cost 75).");

        break;
    case RACE_BARBARIAN:
        if (creature_ptr->lev > 7)
            si_ptr->info[si_ptr->line++] = _("���Ȃ��͋��\�����邱�Ƃ��ł���B(10 MP) ", "You can enter a berserk fury (cost 10).");

        break;
    case RACE_HALF_OGRE:
        if (creature_ptr->lev > 24)
            si_ptr->info[si_ptr->line++] = _("���Ȃ��͔����̃��[�����d�|���邱�Ƃ��ł���B(35 MP)", "You can set an Explosive Rune (cost 35).");

        break;
    case RACE_HALF_GIANT:
        if (creature_ptr->lev > 19)
            si_ptr->info[si_ptr->line++] = _("���Ȃ��͐΂̕ǂ��󂷂��Ƃ��ł���B(10 MP)", "You can break stone walls (cost 10).");

        break;
    case RACE_HALF_TITAN:
        if (creature_ptr->lev > 34)
            si_ptr->info[si_ptr->line++] = _("���Ȃ��̓����X�^�[���X�L�������邱�Ƃ��ł���B(20 MP)", "You can probe monsters (cost 20).");

        break;
    case RACE_CYCLOPS:
        if (creature_ptr->lev > 19) {
            sprintf(si_ptr->plev_buf, _("���Ȃ��� %d �_���[�W�̊�΂𓊂��邱�Ƃ��ł���B(15 MP)", "You can throw a boulder, dam. %d (cost 15)."),
                3 * creature_ptr->lev);
            si_ptr->info[si_ptr->line++] = si_ptr->plev_buf;
        }

        break;
    case RACE_YEEK:
        if (creature_ptr->lev > 14)
            si_ptr->info[si_ptr->line++] = _("���Ȃ��͋��|���ĂыN�������ѐ��𔭂��邱�Ƃ��ł���B(15 MP)", "You can make a terrifying scream (cost 15).");

        break;
    case RACE_KLACKON:
        if (creature_ptr->lev > 8) {
            sprintf(si_ptr->plev_buf, _("���Ȃ��� %d �_���[�W�̎_�𐁂������邱�Ƃ��ł���B(9 MP)", "You can spit acid, dam. %d (cost 9)."), creature_ptr->lev);
            si_ptr->info[si_ptr->line++] = si_ptr->plev_buf;
        }

        break;
    case RACE_KOBOLD:
        if (creature_ptr->lev > 11) {
            sprintf(si_ptr->plev_buf, _("���Ȃ��� %d �_���[�W�̓Ŗ�𓊂��邱�Ƃ��ł���B(8 MP)", "You can throw a dart of poison, dam. %d (cost 8)."),
                creature_ptr->lev);
            si_ptr->info[si_ptr->line++] = si_ptr->plev_buf;
        }

        break;
    case RACE_DARK_ELF:
        if (creature_ptr->lev > 1) {
            sprintf(si_ptr->plev_buf, _("���Ȃ��� %d �_���[�W�̃}�W�b�N�E�~�T�C���̎������g����B(2 MP)", "You can cast a Magic Missile, dam %d (cost 2)."),
                (3 + ((creature_ptr->lev - 1) / 5)));
            si_ptr->info[si_ptr->line++] = si_ptr->plev_buf;
        }

        break;
    case RACE_DRACONIAN:
        sprintf(si_ptr->plev_buf, _("���Ȃ��� %d �_���[�W�̃u���X��f�����Ƃ��ł���B(%d MP)", "You can breathe, dam. %d (cost %d)."), 2 * creature_ptr->lev,
            creature_ptr->lev);
        si_ptr->info[si_ptr->line++] = si_ptr->plev_buf;
        break;
    case RACE_MIND_FLAYER:
        if (creature_ptr->lev > 14)
            sprintf(si_ptr->plev_buf, _("���Ȃ��� %d �_���[�W�̐��_�U�������邱�Ƃ��ł���B(12 MP)", "You can mind blast your enemies, dam %d (cost 12)."),
                creature_ptr->lev);

        si_ptr->info[si_ptr->line++] = si_ptr->plev_buf;
        break;
    case RACE_IMP:
        if (creature_ptr->lev > 29) {
            sprintf(si_ptr->plev_buf, _("���Ȃ��� %d �_���[�W�̃t�@�C�A�E�{�[���̎������g����B(15 MP)", "You can cast a Fire Ball, dam. %d (cost 15)."),
                creature_ptr->lev);
            si_ptr->info[si_ptr->line++] = si_ptr->plev_buf;
            break;
        }

        if (creature_ptr->lev > 8) {
            sprintf(si_ptr->plev_buf, _("���Ȃ��� %d �_���[�W�̃t�@�C�A�E�{���g�̎������g����B(15 MP)", "You can cast a Fire Bolt, dam. %d (cost 15)."),
                creature_ptr->lev);
            si_ptr->info[si_ptr->line++] = si_ptr->plev_buf;
        }

        break;
    case RACE_GOLEM:
        if (creature_ptr->lev > 19)
            si_ptr->info[si_ptr->line++]
                = _("���Ȃ��� d20+30 �^�[���̊Ԕ���΂ɕω���������B(15 MP)", "You can turn your skin to stone, dur d20+30 (cost 15).");

        break;
    case RACE_ZOMBIE:
    case RACE_SKELETON:
        if (creature_ptr->lev > 29)
            si_ptr->info[si_ptr->line++] = _("���Ȃ��͎������o���l���񕜂��邱�Ƃ��ł���B(30 MP)", "You can restore lost experience (cost 30).");

        break;
    case RACE_VAMPIRE:
        if (creature_ptr->lev <= 1)
            break;

        sprintf(si_ptr->plev_buf, _("���Ȃ��͓G���� %d-%d HP �̐����͂��z���ł���B(%d MP)", "You can steal life from a foe, dam. %d-%d (cost %d)."),
            creature_ptr->lev + MAX(1, creature_ptr->lev / 10), creature_ptr->lev + creature_ptr->lev * MAX(1, creature_ptr->lev / 10),
            1 + (creature_ptr->lev / 3));
        si_ptr->info[si_ptr->line++] = si_ptr->plev_buf;
        break;
    case RACE_SPECTRE:
        if (creature_ptr->lev > 3)
            si_ptr->info[si_ptr->line++] = _("���Ȃ��͋�������œG�����|�����邱�Ƃ��ł���B(3 MP)", "You can wail to terrify your enemies (cost 3).");

        break;
    case RACE_SPRITE:
        if (creature_ptr->lev > 11)
            si_ptr->info[si_ptr->line++]
                = _("���Ȃ��͓G�𖰂点�閂�@�̕��𓊂��邱�Ƃ��ł���B(12 MP)", "You can throw magical dust which induces sleep (cost 12).");

        break;
    case RACE_BALROG:
        sprintf(si_ptr->plev_buf, _("���Ȃ��� %d �_���[�W�̒n�����Ή��̃u���X��f�����Ƃ��ł���B(%d MP)", "You can breathe nether, dam. %d (cost %d)."),
            3 * creature_ptr->lev, 10 + creature_ptr->lev / 3);
        si_ptr->info[si_ptr->line++] = si_ptr->plev_buf;
        break;
    case RACE_KUTAR:
        if (creature_ptr->lev > 19)
            si_ptr->info[si_ptr->line++] = _("���Ȃ��� d20+30 �^�[���̊ԉ��ɐL�т邱�Ƃ��ł���B(15 MP)", "You can expand horizontally, dur d20+30 (cost 15).");

        break;
    case RACE_ANDROID:
        if (creature_ptr->lev < 10)
            sprintf(si_ptr->plev_buf, _("���Ȃ��� %d �_���[�W�̃��C�K���������Ƃ��ł���B(7 MP)", "You can fire a ray gun with damage %d (cost 7)."),
                (creature_ptr->lev + 1) / 2);
        else if (creature_ptr->lev < 25)
            sprintf(si_ptr->plev_buf, _("���Ȃ��� %d �_���[�W�̃u���X�^�[�������Ƃ��ł���B(13 MP)", "You can fire a blaster with damage %d (cost 13)."),
                creature_ptr->lev);
        else if (creature_ptr->lev < 35)
            sprintf(si_ptr->plev_buf, _("���Ȃ��� %d �_���[�W�̃o�Y�[�J�������Ƃ��ł���B(26 MP)", "You can fire a bazooka with damage %d (cost 26)."),
                creature_ptr->lev * 2);
        else if (creature_ptr->lev < 45)
            sprintf(si_ptr->plev_buf,
                _("���Ȃ��� %d �_���[�W�̃r�[���L���m���������Ƃ��ł���B(40 MP)", "You can fire a beam cannon with damage %d (cost 40)."),
                creature_ptr->lev * 2);
        else
            sprintf(si_ptr->plev_buf, _("���Ȃ��� %d �_���[�W�̃��P�b�g�������Ƃ��ł���B(60 MP)", "You can fire a rocket with damage %d (cost 60)."),
                creature_ptr->lev * 5);

        si_ptr->info[si_ptr->line++] = si_ptr->plev_buf;
        break;
    default:
        break;
    }
}
