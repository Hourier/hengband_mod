#include "angband.h"
#include "autopick/autopick-finder.h"
#include "autopick/autopick-util.h"
#include "autopick/autopick-matcher.h"
#include "object-flavor.h"

/*
 * @brief �^����ꂽ�A�C�e���������E���̃��X�g�ɓo�^����Ă��邩�ǂ�������������
 * @param player_ptr �v���[���[�ւ̎Q�ƃ|�C���^
 * @o_ptr �A�C�e���ւ̎Q�ƃ|�C���^
 * @return �����E���̃��X�g�ɓo�^����Ă����炻�̓o�^�ԍ��A�Ȃ�������-1
 * @details
 * A function for Auto-picker/destroyer
 * Examine whether the object matches to the list of keywords or not.
 */
int find_autopick_list(player_type *player_ptr, object_type *o_ptr)
{
	GAME_TEXT o_name[MAX_NLEN];
	if (o_ptr->tval == TV_GOLD) return -1;

	object_desc(player_ptr, o_name, o_ptr, (OD_NO_FLAVOR | OD_OMIT_PREFIX | OD_NO_PLURAL));
	str_tolower(o_name);
	for (int i = 0; i < max_autopick; i++)
	{
		autopick_type *entry = &autopick_list[i];
		if (is_autopick_match(player_ptr, o_ptr, entry, o_name))
			return i;
	}

	return -1;
}
