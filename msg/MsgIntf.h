//---------------------------------------------------------------------------
/*
	TVP2 ( T Visual Presenter 2 )  A script authoring tool
	Copyright (C) 2000 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
// Definition of Messages and Message Related Utilities
//---------------------------------------------------------------------------
#ifndef MsgIntfH
#define MsgIntfH

#include "tjs.h"
#include "tjsMessage.h"

#ifndef TVP_MSG_DECL
	#define TVP_MSG_DECL(name, msg) extern tTJSMessageHolder name;
	#define TVP_MSG_DECL_CONST(name, msg) extern tTJSMessageHolder name;
#endif

#include "MsgImpl.h"
#include "svn_revision.h"

#define WIDEN2(x) L ## x
#define WIDEN(x) WIDEN2(x)


#ifdef TJS_TEXT_OUT_CRLF
#define RETURN_CODE TJS_W("\r\n")
#else
#define RETURN_CODE TJS_W("\n")
#endif

//---------------------------------------------------------------------------
// Message Strings ( these should be localized )
//---------------------------------------------------------------------------
// Japanese localized messages
TVP_MSG_DECL_CONST(TVPAboutString,
	
TJS_W("�g���g��[���肫��] Z ���s�R�A version %1 ( TJS version %2 )") RETURN_CODE
TJS_W("Compiled on ") WIDEN(__DATE__) TJS_W(" ") WIDEN(__TIME__) RETURN_CODE
TJS_W("SVN Revision: ") TVP_SVN_REVISION RETURN_CODE
TJS_W("Copyright (C) 1997-2012 W.Dee and contributors All rights reserved.") RETURN_CODE
TJS_W("Contributors in alphabetical order:") RETURN_CODE
TJS_W("  Go Watanabe, Kenjo, Kiyobee, Kouhei Yanagita, mey, MIK, Takenori Imoto, yun") RETURN_CODE
TJS_W("�g���g�����s�R�A�̎g�p/�z�z/���ς́A") RETURN_CODE
TJS_W("SDK �t���� license.txt �ɏ�����Ă��郉�C�Z���X�ɏ]���čs�����Ƃ��ł��܂�.") RETURN_CODE
TJS_W("------------------------------------------------------------------------------") RETURN_CODE
TJS_W("Thanks for many libraries, contributers and supporters not listible here.") RETURN_CODE
TJS_W("This software is based in part on the work of Independent JPEG Group.") RETURN_CODE
TJS_W("Regex++ Copyright (c) 1998-2003 Dr John Maddock") RETURN_CODE
TJS_W("ERINA-Library Copyright (C) 2001 Leshade Entis, Entis-soft.") RETURN_CODE
TJS_W("- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - ") RETURN_CODE
TJS_W("Using \"A C-program for MT19937\"") RETURN_CODE
RETURN_CODE
TJS_W("   Copyright (C) 1997 - 2002, Makoto Matsumoto and Takuji Nishimura,") RETURN_CODE
TJS_W("   All rights reserved.") RETURN_CODE
RETURN_CODE
TJS_W("   Redistribution and use in source and binary forms, with or without") RETURN_CODE
TJS_W("   modification, are permitted provided that the following conditions") RETURN_CODE
TJS_W("   are met:") RETURN_CODE
RETURN_CODE
TJS_W("     1. Redistributions of source code must retain the above copyright") RETURN_CODE
TJS_W("        notice, this list of conditions and the following disclaimer.") RETURN_CODE
RETURN_CODE
TJS_W("     2. Redistributions in binary form must reproduce the above copyright") RETURN_CODE
TJS_W("        notice, this list of conditions and the following disclaimer in the") RETURN_CODE
TJS_W("        documentation and/or other materials provided with the distribution.") RETURN_CODE
RETURN_CODE
TJS_W("     3. The names of its contributors may not be used to endorse or promote") RETURN_CODE
TJS_W("        products derived from this software without specific prior written") RETURN_CODE
TJS_W("        permission.") RETURN_CODE
RETURN_CODE
TJS_W("   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS") RETURN_CODE
TJS_W("   \"AS IS\" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT") RETURN_CODE
TJS_W("   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR") RETURN_CODE
TJS_W("   A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR") RETURN_CODE
TJS_W("   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,") RETURN_CODE
TJS_W("   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,") RETURN_CODE
TJS_W("   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR") RETURN_CODE
TJS_W("   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF") RETURN_CODE
TJS_W("   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING") RETURN_CODE
TJS_W("   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS") RETURN_CODE
TJS_W("   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.") RETURN_CODE
TJS_W("------------------------------------------------------------------------------") RETURN_CODE
TJS_W("�����") RETURN_CODE
  // important log (environment information, Debug.notice etc.) comes here
);
#undef RETURN_CODE

TVP_MSG_DECL_CONST(TVPVersionInformation,
	
TJS_W("�g���g��[���肫��] Z ���s�R�A/%1 ")
TJS_W("(SVN revision:") TVP_SVN_REVISION TJS_W("; Compiled on ") WIDEN(__DATE__) TJS_W(" ") WIDEN(__TIME__) TJS_W(") TJS2/%2 ")
TJS_W("Copyright (C) 1997-2012 W.Dee and contributors All rights reserved."));

TVP_MSG_DECL_CONST(TVPVersionInformation2,
	TJS_W("�o�[�W�������̏ڍׂ� Ctrl + F12 �ŉ{���ł��܂�"));

TVP_MSG_DECL_CONST(TVPDownloadPageURL,
	TJS_W("http://kikyou.info/tvp/"));

TVP_MSG_DECL(TVPInternalError,
	TJS_W("�����G���[���������܂���: at %1 line %2"));

TVP_MSG_DECL(TVPInvalidParam,
	TJS_W("�s���ȃp�����[�^�ł�"));

TVP_MSG_DECL(TVPWarnDebugOptionEnabled,
	TJS_W("-debug �I�v�V�������w�肳��Ă��邽�߁A���� �g���g���̓f�o�b�O���[�h�œ��삵�Ă��܂��B�f�o�b�O���[�h�ł͏\���Ȏ��s���x���o�Ȃ��ꍇ������̂Œ��ӂ��Ă�������"));

TVP_MSG_DECL(TVPCommandLineParamIgnoredAndDefaultUsed,
	TJS_W("�R�}���h���C���p�����[�^ %1 �Ɏw�肳�ꂽ�l %2 �͖����̂��߃f�t�H���g�̐ݒ��p���܂�"));

TVP_MSG_DECL(TVPInvalidCommandLineParam,
	TJS_W("�R�}���h���C���p�����[�^ %1 �Ɏw�肳�ꂽ�l %2 �͖����ł�"));

TVP_MSG_DECL(TVPNotImplemented,
	TJS_W("�������̋@�\���Ăяo�����Ƃ��܂���"));

TVP_MSG_DECL(TVPCannotOpenStorage,
	TJS_W("�X�g���[�W %1 ���J�����Ƃ��ł��܂���"));

TVP_MSG_DECL(TVPCannotFindStorage,
	TJS_W("�X�g���[�W %1 ��������܂���"));

TVP_MSG_DECL(TVPCannotOpenStorageForWrite,
	TJS_W("�X�g���[�W %1 ���������ݗp�ɊJ�����Ƃ��ł��܂���B�t�@�C�����������݋֎~�ɂȂ��Ă��Ȃ����A���邢�̓t�@�C���ɏ������݌��������邩�ǂ����A���邢�͂����������ꂪ�������݉\�ȃ��f�B�A��t�@�C���Ȃ̂����m�F���Ă�������"));

TVP_MSG_DECL(TVPStorageInArchiveNotFound,
	TJS_W("�X�g���[�W %1 ���A�[�J�C�u %2 �̒��Ɍ�����܂���"));

TVP_MSG_DECL(TVPInvalidPathName,
	TJS_W("�p�X�� %1 �͖����Ȍ`���ł��B�`�������������ǂ������m�F���Ă�������"));

TVP_MSG_DECL(TVPUnsupportedMediaName,
	TJS_W("\"%1\" �͑Ή����Ă��Ȃ����f�B�A�^�C�v�ł�"));

TVP_MSG_DECL(TVPCannotUnbindXP3EXE,
	TJS_W("%1 �͎��s�\�t�@�C���Ɍ����܂����A����Ɍ������ꂽ�A�[�J�C�u�𔭌��ł��܂���ł���"));

TVP_MSG_DECL(TVPCannotFindXP3Mark,
	TJS_W("%1 �� XP3 �A�[�J�C�u�ł͂Ȃ����A�Ή��ł��Ȃ��`���ł��B�A�[�J�C�u�t�@�C�����w�肷�ׂ���ʂŒʏ�̃t�@�C�����w�肵���ꍇ�A���邢�͑Ή��ł��Ȃ��A�[�J�C�u�t�@�C�����w�肵���ꍇ�Ȃǂɂ��̃G���[���������܂��̂ŁA�m�F���Ă�������"));

TVP_MSG_DECL(TVPMissingPathDelimiterAtLast,
	TJS_W("�p�X���̍Ō�ɂ� '>' �܂��� '/' ���w�肵�Ă������� (�g���g���Q 2.19 beta 14 ���A�[�J�C�u�̋�؂�L���� '#' ���� '>' �ɕς��܂���)"));

TVP_MSG_DECL(TVPFilenameContainsSharpWarn,
	TJS_W("(����) '#' ���t�@�C���� \"%1\" �Ɋ܂܂�Ă��܂��B�A�[�J�C�u�̋�؂蕶���͋g���g���Q 2.19 beta 14 ���'#' ���� '>' �ɕς��܂����B")
	TJS_W("�����A�[�J�C�u�̋�؂蕶���̂���� '#' ���g�p�����ꍇ�́A���萔�ł��� '>' �ɕς��Ă�������"));

TVP_MSG_DECL(TVPCannotGetLocalName,
	TJS_W("�X�g���[�W�� %1 �����[�J���t�@�C�����ɕϊ��ł��܂���B�A�[�J�C�u�t�@�C�����̃t�@�C����A���[�J���t�@�C���łȂ��t�@�C���̓��[�J���t�@�C�����ɕϊ��ł��܂���B"));

TVP_MSG_DECL(TVPReadError,
	TJS_W("�ǂݍ��݃G���[�ł��B�t�@�C�����j�����Ă���\����A�f�o�C�X����̓ǂݍ��݂Ɏ��s�����\��������܂�"));

TVP_MSG_DECL(TVPWriteError,
	TJS_W("�������݃G���[�ł�"));

TVP_MSG_DECL(TVPSeekError,
	TJS_W("�V�[�N�Ɏ��s���܂����B�t�@�C�����j�����Ă���\����A�f�o�C�X����̓ǂݍ��݂Ɏ��s�����\��������܂�"));

TVP_MSG_DECL(TVPTruncateError,
	TJS_W("�t�@�C���̒�����؂�l�߂�̂Ɏ��s���܂���"));

TVP_MSG_DECL(TVPInsufficientMemory,
	TJS_W("�������m�ۂɎ��s���܂����B"));

TVP_MSG_DECL(TVPUncompressionFailed,
	TJS_W("�t�@�C���̓W�J�Ɏ��s���܂����B���Ή��̈��k�`�����w�肳�ꂽ���A���邢�̓t�@�C�����j�����Ă���\��������܂�"));

TVP_MSG_DECL(TVPCompressionFailed,
	TJS_W("�t�@�C���̈��k�Ɏ��s���܂���"));

TVP_MSG_DECL(TVPCannotWriteToArchive,
	TJS_W("�A�[�J�C�u�Ƀf�[�^���������ނ��Ƃ͂ł��܂���"));

TVP_MSG_DECL(TVPUnsupportedCipherMode,
	TJS_W("%1 �͖��Ή��̈Í����`�����A�f�[�^���j�����Ă��܂�"));

TVP_MSG_DECL(TVPUnsupportedModeString,
	TJS_W("�F���ł��Ȃ����[�h������̎w��ł�(%1)"));

TVP_MSG_DECL(TVPUnknownGraphicFormat,
	TJS_W("%1 �͖��m�̉摜�`���ł�"));

TVP_MSG_DECL(TVPCannotSuggestGraphicExtension,
	TJS_W("%1 �ɂ��ēK�؂Ȋg���q���������t�@�C�����������܂���ł���"));

TVP_MSG_DECL(TVPMaskSizeMismatch,
	TJS_W("�}�X�N�摜�̃T�C�Y�����C���摜�̃T�C�Y�ƈႢ�܂�"));

TVP_MSG_DECL(TVPProvinceSizeMismatch,
	TJS_W("�̈�摜 %1 �̓��C���摜�ƃT�C�Y���Ⴂ�܂�"));

TVP_MSG_DECL(TVPImageLoadError,
	TJS_W("�摜�ǂݍ��ݒ��ɃG���[���������܂���/%1"));

TVP_MSG_DECL(TVPJPEGLoadError,
	TJS_W("JPEG �ǂݍ��ݒ��ɃG���[���������܂���/%1"));

TVP_MSG_DECL(TVPPNGLoadError,
	TJS_W("PNG �ǂݍ��ݒ��ɃG���[���������܂���/%1"));

TVP_MSG_DECL(TVPERILoadError,
	TJS_W("ERI �ǂݍ��ݒ��ɃG���[���������܂���/%1"));

TVP_MSG_DECL(TVPTLGLoadError,
	TJS_W("TLG �ǂݍ��ݒ��ɃG���[���������܂���/%1"));

TVP_MSG_DECL(TVPInvalidImageSaveType,
	TJS_W("�����ȕۑ��摜�`���ł�(%1)"));

TVP_MSG_DECL(TVPInvalidOperationFor8BPP,
	TJS_W("8bpp �摜�ɑ΂��Ă͍s���Ȃ�������s�����Ƃ��܂���"));

TVP_MSG_DECL(TVPSpecifyWindow,
	TJS_W("Window �N���X�̃I�u�W�F�N�g���w�肵�Ă�������"));

TVP_MSG_DECL(TVPSpecifyLayer,
	TJS_W("Layer �N���X�̃I�u�W�F�N�g���w�肵�Ă�������"));

TVP_MSG_DECL(TVPCannotCreateEmptyLayerImage,
	TJS_W("�摜�T�C�Y�̉������邢�͏c���� 0 �ȉ��̐��ɐݒ肷�邱�Ƃ͂ł��܂���"));

TVP_MSG_DECL(TVPCannotSetPrimaryInvisible,
	TJS_W("�v���C�}�����C���͕s���ɂł��܂���"));

TVP_MSG_DECL(TVPCannotMovePrimary,
	TJS_W("�v���C�}�����C���͈ړ��ł��܂���"));

TVP_MSG_DECL(TVPCannotSetParentSelf,
	TJS_W("�������g��e�Ƃ��邱�Ƃ͂ł��܂���"));

TVP_MSG_DECL(TVPCannotMoveNextToSelfOrNotSiblings,
	TJS_W("�������g�̑O���e�̈قȂ郌�C���̑O��Ɉړ����邱�Ƃ͂ł��܂���"));

TVP_MSG_DECL(TVPCannotMovePrimaryOrSiblingless,
	TJS_W("�v���C�}�����C����Z��̖������C���͑O��Ɉړ����邱�Ƃ͂ł��܂���"));

TVP_MSG_DECL(TVPCannotMoveToUnderOtherPrimaryLayer,
	TJS_W("�ʂ̃v���C�}�����C�����Ƀ��C�����ړ����邱�Ƃ͂ł��܂���"));

TVP_MSG_DECL(TVPInvalidImagePosition,
	TJS_W("���C���̈�ɉ摜�̖����̈悪�������܂���"));

TVP_MSG_DECL(TVPCannotSetModeToDisabledOrModal,
	TJS_W("���łɃ��[�_���ȃ��C���̐e���C���A���邢�͕s��/�����ȃ��C�������[�_���ɂ��邱�Ƃ͂ł��܂���"));

TVP_MSG_DECL(TVPNotDrawableLayerType,
	TJS_W("���� type �̃��C���ł͕`���摜�ǂݍ��݂�摜�T�C�Y/�ʒu�̕ύX/�擾�͂ł��܂���"));

TVP_MSG_DECL(TVPSourceLayerHasNoImage,
	TJS_W("�]�������C���͉摜�������Ă��܂���"));

TVP_MSG_DECL(TVPUnsupportedLayerType,
	TJS_W("%1 �͂��� type �̃��C���ł͎g�p�ł��܂���"));

TVP_MSG_DECL(TVPNotDrawableFaceType,
	TJS_W("%1 �ł͂��� face �ɕ`��ł��܂���"));

TVP_MSG_DECL(TVPCannotConvertLayerTypeUsingGivenDirection,
	TJS_W("�w�肳�ꂽ���C���^�C�v�ϊ��͂ł��܂���"));

TVP_MSG_DECL(TVPNegativeOpacityNotSupportedOnThisFace,
	TJS_W("���̕s�����x�͂��� face �ł͎w��ł��܂���"));

TVP_MSG_DECL(TVPSrcRectOutOfBitmap,
	TJS_W("�]�������r�b�g�}�b�v�O�̗̈���܂�ł��܂��B�������͈͂Ɏ��܂�悤�ɓ]�������w�肵�Ă�������"));

TVP_MSG_DECL(TVPBoxBlurAreaMustContainCenterPixel,
	TJS_W("��`�u���[�͈͕̔͂K��(0,0)�����̒��Ɋ܂ޕK�v������܂��Bleft��right�������Ƃ����̐��l�A���邢�͗����Ƃ����̐��l�Ƃ����w��͂ł��܂���(top��bottom�ɑ΂��Ă����l)"));

TVP_MSG_DECL(TVPBoxBlurAreaMustBeSmallerThan16Million,
	TJS_W("��`�u���[�͈̔͂��傫�����܂��B��`�u���[�͈̔͂�1677���ȉ��ł���K�v������܂�"));

TVP_MSG_DECL(TVPCannotChangeFocusInProcessingFocus,
	TJS_W("�t�H�[�J�X�ύX�������̓t�H�[�J�X��V���ɕύX���邱�Ƃ͂ł��܂���"));

TVP_MSG_DECL(TVPWindowHasNoLayer,
	TJS_W("�E�B���h�E�Ƀ��C��������܂���"));

TVP_MSG_DECL(TVPWindowHasAlreadyPrimaryLayer,
	TJS_W("�E�B���h�E�ɂ͂��łɃv���C�}�����C��������܂�"));

TVP_MSG_DECL(TVPSpecifiedEventNeedsParameter,
	TJS_W("�C�x���g %1 �ɂ̓p�����[�^���K�v�ł�"));

TVP_MSG_DECL(TVPSpecifiedEventNeedsParameter2,
	TJS_W("�C�x���g %1 �ɂ̓p�����[�^ %2 ���K�v�ł�"));

TVP_MSG_DECL(TVPSpecifiedEventNameIsUnknown,
	TJS_W("�C�x���g�� %1 �͖��m�̃C�x���g���ł�"));

TVP_MSG_DECL(TVPOutOfRectangle,
	TJS_W("��`�O���w�肳��܂���"));

TVP_MSG_DECL(TVPInvalidMethodInUpdating,
	TJS_W("��ʍX�V���͂��̋@�\�����s�ł��܂���"));

TVP_MSG_DECL(TVPCannotCreateInstance,
	TJS_W("���̃N���X�̓C���X�^���X���쐬�ł��܂���"));

TVP_MSG_DECL(TVPUnknownWaveFormat,
	TJS_W("%1 �͑Ή��ł��Ȃ� Wave �`���ł�"));

TVP_MSG_DECL(TVPCurrentTransitionMustBeStopping,
	TJS_W("���݂̃g�����W�V�������~�����Ă���V�����g�����W�V�������J�n���Ă��������B�������C���ɑ΂��ĕ����̃g�����W�V�����𓯎��Ɏ��s���悤�Ƃ���Ƃ��̃G���[���������܂�"));

TVP_MSG_DECL(TVPTransHandlerError,
	TJS_W("�g�����W�V�����n���h���ŃG���[���������܂��� : %1"));

TVP_MSG_DECL(TVPTransAlreadyRegistered,
	TJS_W("�g�����W�V���� %1 �͊��ɓo�^����Ă��܂�"));

TVP_MSG_DECL(TVPCannotFindTransHander,
	TJS_W("�g�����W�V�����n���h�� %1 ��������܂���"));

TVP_MSG_DECL(TVPSpecifyTransitionSource,
	TJS_W("�g�����W�V���������w�肵�Ă�������"));

TVP_MSG_DECL(TVPLayerCannotHaveImage,
	TJS_W("���̃��C���͉摜�������Ƃ͂ł��܂���"));

TVP_MSG_DECL(TVPTransitionSourceAndDestinationMustHaveImage,
	TJS_W("�g�����W�V�������ƃg�����W�V������͂Ƃ��ɉ摜�������Ă���K�v������܂�"));

TVP_MSG_DECL(TVPCannotLoadRuleGraphic,
	TJS_W("���[���摜 %1 ��ǂݍ��ނ��Ƃ��ł��܂���"));

TVP_MSG_DECL(TVPSpecifyOption,
	TJS_W("�I�v�V���� %1 ���w�肵�Ă�������"));

TVP_MSG_DECL(TVPTransitionLayerSizeMismatch,
	TJS_W("�g�����W�V������(%1)�ƃg�����W�V������(%2)�̃��C���̃T�C�Y����v���܂���"));

TVP_MSG_DECL(TVPTransitionMutualSource,
	TJS_W("�g�����W�V�������̃g�����W�V���������������g�ł�"));

TVP_MSG_DECL(TVPHoldDestinationAlphaParameterIsNowDeprecated,
	TJS_W("�x�� : ���\�b�h %1 �� %2 �Ԗڂɓn���ꂽ hda �p�����[�^�́A�g���g���Q 2.23 beta 2 ��薳�������悤�ɂȂ�܂����B����� Layer.holdAlpha �v���p�e�B��p���Ă��������B"));

TVP_MSG_DECL(TVPCannotConnectMultipleWaveSoundBufferAtOnce,
	TJS_W("������ WaveSoundBuffer ����̃t�B���^�œ����Ɏg�p���邱�Ƃ͂ł��܂���"));

TVP_MSG_DECL(TVPInvalidWindowSizeMustBeIn64to32768,
	TJS_W("window �� 64�`32768 �͈̔͂� 2 �̗ݏ�Ŗ�����΂Ȃ�܂���"));

TVP_MSG_DECL(TVPInvalidOverlapCountMustBeIn2to32,
	TJS_W("overlap �� 2�`32 �͈̔͂� 2 �̗ݏ�Ŗ�����΂Ȃ�܂���"));

//---------------------------------------------------------------------------
// ���ߍ��܂�Ă������b�Z�[�W���R�s�[

TVP_MSG_DECL(TVPFaildClipboardCopy,
	TJS_W("copying to clipboard failed."));


//---------------------------------------------------------------------------
// Utility Functions
//---------------------------------------------------------------------------
TJS_EXP_FUNC_DEF(ttstr, TVPFormatMessage, (const tjs_char *msg, const ttstr & p1));
TJS_EXP_FUNC_DEF(ttstr, TVPFormatMessage, (const tjs_char *msg, const ttstr & p1,
	const ttstr & p2));
TJS_EXP_FUNC_DEF(void, TVPThrowExceptionMessage, (const tjs_char *msg));
TJS_EXP_FUNC_DEF(void, TVPThrowExceptionMessage, (const tjs_char *msg,
	const ttstr &p1, tjs_int num));
TJS_EXP_FUNC_DEF(void, TVPThrowExceptionMessage, (const tjs_char *msg, const ttstr &p1));
TJS_EXP_FUNC_DEF(void, TVPThrowExceptionMessage, (const tjs_char *msg,
	const ttstr & p1, const ttstr & p2));

TJS_EXP_FUNC_DEF(ttstr, TVPGetAboutString, ());
TJS_EXP_FUNC_DEF(ttstr, TVPGetVersionInformation, ());
TJS_EXP_FUNC_DEF(ttstr, TVPGetVersionString, ());

#define TVPThrowInternalError \
	TVPThrowExceptionMessage(TVPInternalError, __FILE__,  __LINE__)
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
// version retrieving
//---------------------------------------------------------------------------
extern tjs_int TVPVersionMajor;
extern tjs_int TVPVersionMinor;
extern tjs_int TVPVersionRelease;
extern tjs_int TVPVersionBuild;
//---------------------------------------------------------------------------
extern void TVPGetVersion();
/*
	implement in each platforms;
	fill these four version field.
*/
//---------------------------------------------------------------------------
TJS_EXP_FUNC_DEF(void, TVPGetSystemVersion, (tjs_int &major, tjs_int &minor,
	tjs_int &release, tjs_int &build));
TJS_EXP_FUNC_DEF(void, TVPGetTJSVersion, (tjs_int &major, tjs_int &minor,
	tjs_int &release));
//---------------------------------------------------------------------------


#endif
