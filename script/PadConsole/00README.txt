TITLE: KirikiriZ �����݊��f�o�b�O���[�U�C���^�[�t�F�[�X�E�B���h�E
AUTHOR: ������Ѓ����\�t�g �O�� ��

������͉����H

�g���g���Q�ɑ��݂����f�o�b�O�n�̃E�B���h�E�ނ�
win32dialog.dll �ɂĎ����������̂ɂȂ�܂��B

���L����������Ă��܂��B

	�E�R���\�[���E�B���h�E�iDebug.console�j
	�E�X�N���v�g�G�f�B�^�iShift+F2�ŕ\���������́j
	�EPad�N���X�i�ꕔ�v���p�e�B�ɂ��Ă͖��T�|�[�g�j

���L�͖������ł����C����Ή����邩������܂���B

	�E�R���g���[���iDebug.controller�j
	�E�Ď����E�B���h�E�iShift+F3�ŕ\���������́j

�Ȃ��C�e����ɂ��čו��܂Ō����Ƀ`�F�b�N�͂��Ă��܂���B
��肪�������ꍇ�� issue �o�^�����肢���܂��B


���t�@�C���̓��e�ɂ���

00README.txt		���̃t�@�C���ł�
data			�e�X�g�X�N���v�g�t�H���_�ł�
data/startup.tjs	�e�X�g�X�N���v�g�N���pTJS�t�@�C���ł�
data/win32dialog.tjs	win32dialog.dll�̃w���p�[TJS�ł�
data/k2debugui.tjs	�݊��R���\�[���E�B���h�E�̖{�̂ł�

	���������tjs�t�@�C���� UTF16LE-BOM �G���R�[�h�̂��߁C
	  -readencoding�I�v�V������UTF-8�ł�Shift_JIS�ł��ǂݍ��߂܂�

������m�F���@

����m�F�ɂ͑���Ȃ��t�@�C����p�ӂ��Ă��������K�v������܂��B

tvpwin32.exe		�f�o�b�O���O���肪�g�����ꂽ�ŐV�ł��g�p���Ă�������
plugin/menu.dll		�e�X�g�X�N���v�g�̓���m�F������ꍇ�ɕK�v�ł�
plugin/win32dialog.dll	�g���g���{�ƃ��|�W�g������o�C�i�����擾���Ă�������
			�� https://sv.kikyou.info/svn/kirikiri2/trunk/kirikiri2/bin/win32/plugin/win32dialog.dll

��L�t�@�C����u���ċN�����Ă��������ƁC
�e�X�g�E�B���h�E�Ƌ��ɃR���\�[�����\������܂��B


���g����

startup.tjs �Ȃǂ�

	Scripts.evalStorage("k2debugui.tjs");

�Ƃ��邱�Ƃ� Debug.console �� Pad �N���X���g�p�ł���悤�ɂȂ�܂��B

�R�}���h���C���I�v�V�����Łu-debugwin=no�v���w�肳��Ă����ꍇ�́C
Debug.console.visible�𖳎�����_�~�[�̃R���\�[���I�u�W�F�N�g����������܂��B


���݊����ɂ���

���S�ʁF

�EShift+F? �Ȃǂ̃z�b�g�L�[�ł̃f�o�b�O�E�B���h�E�ނ̕\��ON/OFF�͋@�\���܂���B
�@Window.onKeyDown �Ȃǂɕ\����؂�ւ���悤�ȍ׍H�����Ă��������B

�E���݁Ckrenvprf.kep�����̃f�o�b�O�E�B���h�E�ʒu���̕ۑ��@�\������܂���B
�@�ʂ̌`���ŏ�Ԃ�ۑ�����悤�ȑΉ����������ł��B

�E�f�o�b�O�E�B���h�E��ł̉E�N���b�N���j���[�̓���@�\�͂���܂���B

�E�E�B���h�E�̃A�C�R�����Ⴂ�܂��B

�E�e�L�X�g�̑I��͈͂̐F���Ⴂ�܂��B

�E�ʂ̃f�o�b�O�E�B���h�E���ŏ�����������
�@�^�X�N�o�[�ł͂Ȃ���ʍ����ɃX�g�b�N����܂��B

�EWindow���P�ł����O(Window.mainWindow���Ȃ����)��
�@�R���\�[�����̃f�o�b�O�E�B���h�E��\������ƁC
�@�ォ��Window������ĕ\�������ꍇ�ɁCWindow���̍ŏ����{�^����
�@�e�f�o�b�O�E�B���h�E�̍ŏ����ƘA�����܂���B
�@�܂��C�^�X�N�o�[�̃{�^�����ʂ̘g������ԂɂȂ�܂��B
���f�o�b�O�E�B���h�E�̐e�E�B���h�E���������߂ɋN���錻�ۂŁC�d�l�ł��B

�E�t��Window.mainWindow�������ԂŃf�o�b�O�E�B���h�E��\������ƁC
�@���̃E�B���h�E�͏��Window.mainWindow����O�ɕ\������Ă��܂��܂��B
���ݒ�ŉ���ł��Ȃ����m�F���iTVPApplicationWindow�������Ȃ������Ƃ̕��Q�H�j


���R���\�[���F

�E����C�����P�s���͂̃q�X�g���@�\������܂���B
���Ή��������B

�E�A���������O�̏o�͂̓A�C�h����ԂɂȂ��Ă���܂Ƃ߂Ĉꊇ�ŕ\������܂��B
���X�N���[���ŗ���郍�O���������ꍇ�͕ʓrcmd.exe�Ȃǂ��g�p���Ă�������

�E�P�s���͂�eval�����̌��ʕ\�����኱�قȂ�܂��B
�����ʂ�Object�̏ꍇ�ɏڍ׏�񂪕\������܂�

�E�P�s���͂�]���������ɗ�O�����������ꍇ�C���͗����N���A����܂���B
���C����O��Ƃ�������ɂȂ��Ă��܂�

�E���O�e�L�X�g�����N���b�N����ƂP�s���͂���t�H�[�J�X���O���
�@���O���ɃL�����b�g���\������Ă��܂��܂��B(Tab�L�[�ŕ��A�ł��܂��j

�E���O���X�N���[�����鎞�ɕ\�����`���t���ꍇ������܂�

�E���O�̏���s�����قȂ�܂�


��Pad�N���X�F

�E�X�e�[�^�X�o�[�ɃJ�[�\���ʒu(X:Y)�̕\���@�\�͂���܂���

�E���L�v���p�e�B�� new Pad() ������Cvisible=true����Ԃɂ̂ݕύX�\�ł��B
�@��x�\�������ォ��̕ύX�͂ł��܂���B

	fontFace
	fontSize, fontHeight
	fontBold
	wordWrap

�EfontHeight�v���p�e�B�͋��炭��ʂ�DPI��ύX�������ł�
�@�������s�N�Z���T�C�Y���w��ł��܂���B�܂��������w��ł��܂���B
�@�����fontSize�v���p�e�B���g�p���Ă��������B

�E���L�v���p�e�B�͖��T�|�[�g�ł��B�i�l�͐ݒ�ł��܂����Ӗ��������܂���j

	fileName
	fontItalic
	fontUnderline
	fontStrikeOut

�Evisible��true->false->true�Ƃ����ꍇ�ɃL�����b�g�ʒu��������
�@�e�L�X�g���S�I����ԂɂȂ��Ă��܂��܂��B


���X�N���v�g�G�f�B�^

�EPad�N���X�ɍ����̎��s�{�^����L���ɂ����o�[�W�����̃N���X��
  �uScriptEditorPad�v�N���X�Ƃ��Ď�������Ă��܂��B

�E����C��O�������Ɏ����ŃX�N���v�g�G�f�B�^�������ŕ\�����ăG���[�s��
�@�J�[�\�������킹��@�\�͂���܂���B
��Exception�N���X�ɃG���[�ʒu�Ȃǂ̏�񂪂Ȃ����߁C�{�̑��̑Ή���������


