#include "main/init-error-messages-table.h"

/*!
 * �G���[���b�Z�[�W�̖��̒�` / Standard error message text
 */
concptr err_str[PARSE_ERROR_MAX] = {
    NULL,
    _("���@�G���[", "parse error"),
    _("�Â��t�@�C��", "obsolete file"),
    _("�L�^�w�b�_���Ȃ�", "missing record header"),
    _("�s�A�����R�[�h", "non-sequential records"),
    _("�������ȃt���O����", "invalid flag specification"),
    _("����`����", "undefined directive"),
    _("�������s��", "out of memory"),
    _("���W�͈͊O", "coordinates out of bounds"),
    _("�����s��", "too few arguments"),
    _("����`�n�`�^�O", "undefined terrain tag"),
};
