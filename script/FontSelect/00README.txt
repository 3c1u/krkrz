TITLE: KirikiriZ �����t�H���g�I���_�C�A���O�݊��X�N���v�g
AUTHOR: ������Ѓ����\�t�g �O�� ��

������͉����H

�t�H���g�I���_�C�A���O�̌݊��ł� win32dialog.dll �ɂĎ����������̂ɂȂ�܂��B
Font�C���X�^���X����e���C�����擾�ł��Ȃ����߁C�Ăяo�����@���኱�قȂ�܂����C
����ȊO�͂ق� Font.doUserSelect �Ɠ����悤�Ɏg���܂��B
�i�������ׂ����I�v�V�����ɂ��Ă͍ו��܂Ō����Ƀ`�F�b�N�͂��Ă��܂���j


���t�@�C���̓��e�ɂ���

00README.txt		���̃t�@�C���ł�
data			�e�X�g�X�N���v�g�t�H���_�ł�
data/startup.tjs	�e�X�g�X�N���v�g�N���pTJS�t�@�C���ł�
data/win32dialog.tjs	win32dialog.dll�̃w���p�[TJS�ł�
data/fontselect.tjs	�t�H���g�I���_�C�A���O�̖{�̂ł�
data/inputstring.tjs	System.inputString �݊��̃X�N���v�g�{�̂ł�


������m�F���@

����m�F�ɂ͑���Ȃ��t�@�C����p�ӂ��Ă��������K�v������܂��B

tvpwin32.exe		krkrz_20130702.zip�̃o�[�W�����œ���m�F���Ă���܂�
plugin/menu.dll		�V
plugin/win32dialog.dll	�g���g���{�ƃ��|�W�g������o�C�i�����擾���Ă�������
			�� https://sv.kikyou.info/svn/kirikiri2/trunk/kirikiri2/bin/win32/plugin/win32dialog.dll

��L�t�@�C����u���ċN�����Ă��������ƁC�e�X�g�E�B���h�E���\������܂��B

File���j���[��

	�EFontSelect(GDI)
	�EFontSelect(FreeType)

��I�Ԃ��ƂŃt�H���g�I���_�C�A���O���\������܂��B
GDI/FreeType�̓��X�^���C�U�̃o�[�W�����̈Ⴂ�ɂ����̂ł��B

���������j���[��Option��I�ԂƁC�t�H���g�I����flags��ҏW���邱�Ƃ��ł��܂��B
�iSystem.inputString�̃T���v���ɂ��Ȃ��Ă��܂��j


�����m�̕s�

krkrz_20130702�̃o�C�i���ł͉��L�̕s�������܂��B

�EGDI���X�^���C�U�̏ꍇ�C�ꕔ�̕������������\���ɂȂ�
���ǂ������@���ł����Ȃ邩�͕s���B��xFreeType�ł��J������ɂ�������J���ƂȂ�₷���͗l

�EFreeType�͈ꕔ�t�H���g�̃����_�����O�ŗ�O���������Đ������\������Ȃ�
��drawText�ŁuFont Rasterize error.�v�Ƃ����G���[���o�܂�

�EWindow.menu.visible = true �ɂ���ƃ��j���[���\������Ȃ�
��menu.dll�������N���ď��Ȃ��Ƃ�1��add���ꂽ���_�ŕ\�������悤�Ȃ̂Ŗ��͂Ȃ��̂ł����C
���݊��̊ϓ_���猩��Ə��X���̂悤�Ɏv���܂��B


��API�ɂ���

----------------------------------------------------------------
��System.doFontSelect

�E�@�\/�Ӗ�
�@�t�H���g�I���_�C�A���O�{�b�N�X�̕\���i�݊������j

�E�^�C�v
�@System�N���X�̃��\�b�h

�E�\��
�@doFontSelect(layer, flags, caption, prompt, sample)

�E����
�@layer : �ΏۂƂȂ� font �I�u�W�F�N�g�������C���[���w�肵�܂�
�@flags, caption, prompt, sample : Font.doUserSelect �Ɠ����p�����[�^�ł�

�E�߂�l�i���j
�@���[�U�� OK �{�^����I�������^�������̓t�H���g���X�g���_�u���N���b�N�����ꍇ��
�@���̃t�H���g�t�F�C�X���̂̕����񂪕Ԃ�܂��B
�@����ȊO�̃L�����Z����������ꂽ�ꍇ�� void ���Ԃ�܂��B

�E����
�@���[�U�Ƀt�H���g��I�������邽�߂̃_�C�A���O�{�b�N�X���J���܂��B
�@�f�t�H���g�� layer.font.face �̃t�H���g���I�����ꂽ��ԂɂȂ��Ă��܂��B
�@fsfUseFontFace �� flags �Ɏw�肵���ꍇ�́Alayer.font.height ���Q�Ƃ���A
�@���̑傫���ŕ\������܂��B�i�傫������Ƒ��삪����ɂȂ�̂Œ��ӂ��Ă��������j

�@�g�p�ɍۂ��ẮA
	fontselect.tjs  (���������p�X���ɃX�N���v�g��z�u)
	win32dialog.tjs (�V)
	win32dialog.dll (����̃v���O�C���t�H���_�ɔz�u)
�@��3�t�@�C�����K�v�ł��B

�@�܂��AScripts.evalStorage("fontselect.tjs"); �����s����
�@���炩���߃X�N���v�g��ǂݍ���ł������A

System.doFontSelect = function {
	// �x���ǂݍ���
	Scripts.evalStorage("fontselect.tjs");
	return global.System.doFontSelect(...);
} incontextof global;

�@�ȂǂƂ��Ēx���ǂݍ��݂̍׍H�����Ă������Ƃ��K�v�ł��B

----------------------------------------------------------------


��Font.doUserSelect�Ɩ߂�l�̎d�l���኱�قȂ�܂����C
�@fontselect.tjs��ҏW���邱�ƂŌ݊��d�l�ɕύX�\�ł��B

��F
	var dialog = new FontSelectDialog(layer, layer.font.face, *);
	var result = dialog.open(layer.window);
	invalidate dialog;
+	if (result !== void) layer.font.face = result;
-	return result;
+	return result !== void;


