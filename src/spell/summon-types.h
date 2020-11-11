#pragma once

/* summon_specific�Ŏ�舵����A�����̎�ʒ�` / Legal restrictions for "summon_specific()" */
typedef enum summon_type {
    SUMMON_NONE = 0,
	SUMMON_ANT = 11,  /*!< �����^�C�v: �A�� */
    SUMMON_SPIDER = 12, /*!< �����^�C�v: �w� */
    SUMMON_HOUND = 13, /*!< �����^�C�v: �n�E���h */
    SUMMON_HYDRA = 14, /*!< �����^�C�v: �q�h�� */
    SUMMON_ANGEL = 15, /*!< �����^�C�v: �V�g */
    SUMMON_DEMON = 16, /*!< �����^�C�v: ���� */
    SUMMON_UNDEAD = 17, /*!< �����^�C�v: �A���f�b�h */
    SUMMON_DRAGON = 18, /*!< �����^�C�v: �h���S�� */
    SUMMON_HI_UNDEAD = 21, /*!< �����^�C�v: ���͂ȃA���f�b�h */
    SUMMON_HI_DRAGON = 22, /*!< �����^�C�v: �Ñ�h���S�� */
    SUMMON_HI_DEMON = 23, /*!< �����^�C�v: �㋉�f�[���� */
    SUMMON_AMBERITES = 31, /*!< �����^�C�v: �A���o�[�̉��� */
    SUMMON_UNIQUE = 32, /*!< �����^�C�v: ���j�[�N */
    SUMMON_MOLD = 33, /*!< �����^�C�v: �J�r */
    SUMMON_BAT = 34, /*!< �����^�C�v: �R�E���� */
    SUMMON_QUYLTHULG = 35, /*!< �����^�C�v: �N�C���X���O */
    SUMMON_XXX1 = 36, /*!< �����^�C�v: ���g�p */
    SUMMON_COIN_MIMIC = 37, /*!< �����^�C�v: �N���[�s���O�E�R�C�� */
    SUMMON_MIMIC = 38, /*!< �����^�C�v: �~�~�b�N */
    SUMMON_CYBER = 39, /*!< �����^�C�v: �T�C�o�[�f�[���� */
    SUMMON_KIN = 40, /*!< �����^�C�v: �����҂̓��� */
    SUMMON_DAWN = 41, /*!< �����^�C�v: �ł̐�m */
    SUMMON_ANIMAL = 42, /*!< �����^�C�v: ���R�E�̓��� */
    SUMMON_ANIMAL_RANGER = 43, /*!< �����^�C�v: �����W���[�������R�E�̓��� */
    SUMMON_SMALL_MOAI = 44, /*!< �����^�C�v: �v�`���A�C */
    SUMMON_PHANTOM = 47, /*!< �����^�C�v: �S�[�X�g */
    SUMMON_TOTEM_MOAI = 48, /*!< �����^�C�v: �g�[�e�����A�C */
    SUMMON_BLUE_HORROR = 49, /*!< �����^�C�v: �u���[�E�z���[ */
    SUMMON_LIVING = 50, /*!< �����^�C�v: �����̂��郂���X�^�[ */
    SUMMON_HI_DRAGON_LIVING = 51, /*!< �����^�C�v: �����̂���Ñ�h���S�� */
    SUMMON_GOLEM = 52, /*!< �����^�C�v: �S�[���� */
    SUMMON_ELEMENTAL = 53, /*!< �����^�C�v: �G�������^�� */
    SUMMON_VORTEX = 54, /*!< �����^�C�v: �{���e�b�N�X */
    SUMMON_HYBRID = 55, /*!< �����^�C�v: �������� */
    SUMMON_BIRD = 56, /*!< �����^�C�v: �� */
    SUMMON_KAMIKAZE = 58, /*!< �����^�C�v: ���������X�^�[ */
    SUMMON_KAMIKAZE_LIVING = 59, /*!< �����^�C�v: �����̂��鎩�������X�^�[ */
    SUMMON_MANES = 60, /*!< �����^�C�v: �Ñ�̎��� */
    SUMMON_LOUSE = 61, /*!< �����^�C�v: �V���~ */
    SUMMON_GUARDIANS = 62, /*!< �����^�C�v: �_���W�����̎� */
    SUMMON_KNIGHTS = 63, /*!< �����^�C�v: ����p�R�m�n�����X�^�[ */
    SUMMON_EAGLES = 64, /*!< �����^�C�v: �h�n�����X�^�[ */
    SUMMON_PIRANHAS = 65, /*!< �����^�C�v: �s���j�A�E�g���b�v�p */
    SUMMON_ARMAGE_GOOD = 66, /*!< �����^�C�v: �n���}�Q�h���E�g���b�v�p�V�g�w�c */
    SUMMON_ARMAGE_EVIL = 67, /*!< �����^�C�v: �n���}�Q�h���E�g���b�v�p�����w�c */
} summon_type;
