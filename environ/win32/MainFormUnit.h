//---------------------------------------------------------------------------
/*
	TVP2 ( T Visual Presenter 2 )  A script authoring tool
	Copyright (C) 2000 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
// System Main Window (Controller)
//---------------------------------------------------------------------------
#ifndef MainFormUnitH
#define MainFormUnitH
//---------------------------------------------------------------------------
#include <windows.h>
#include <string>
#include "tstring.h"
//---------------------------------------------------------------------------
class TTVPMainForm
{
private:	// ���[�U�[�錾
	bool ContinuousEventCalling;
	bool AutoShowConsoleOnError;
	bool ApplicationStayOnTop;
	bool ApplicationActivating;
	bool ApplicationNotMinimizing;

	bool EventEnable;

	DWORD LastCompactedTick;
	DWORD LastContinuousTick;
	DWORD LastCloseClickedTick;
	DWORD LastShowModalWindowSentTick;
	DWORD LastRehashedTick;

	DWORD MixedIdleTick;
public:
	TTVPMainForm();

	void InvokeEvents();
	void CallDeliverAllEventsOnIdle();

	void BeginContinuousEvent();
	void EndContinuousEvent();

	void NotifyEventDelivered();

	void SetVisible( bool b );
	bool GetVisible() const;

	void SetEventButtonDown( bool b ) {
		EventEnable = b;
	}
	bool GetEventButtonDown() const { return EventEnable; }

	bool GetApplicationStayOnTop();
	void SetApplicationStayOnTop( bool );

	void NotifySystemError();

	bool GetConsoleVisible();
	void SetConsoleVisible( bool );
	
	bool GetApplicationActivating() const { return ApplicationActivating; }
	bool GetApplicationNotMinimizing() const { return ApplicationNotMinimizing; }

	HWND GetHandle() { return NULL; }

	bool ApplicationIdel();
private:
	void DeliverEvents();
};
enum {
  mtWarning = MB_ICONWARNING,
  mtError = MB_ICONERROR,
  mtInformation = MB_ICONINFORMATION,
  mtConfirmation = MB_ICONQUESTION,
  mtCustom = 0
};
/*
MB_ABORTRETRYIGNORE	���b�Z�[�W�{�b�N�X�Ɂm���~�n�A�m�Ď��s�n�A�m�����n�̊e�v�b�V���{�^����\�����܂��B
MB_CANCELTRYCONTINUE	Windows 2000�F���b�Z�[�W�{�b�N�X�Ɂm�L�����Z���n�A�m�Ď��s�n�A�m���s�n�̊e�v�b�V���{�^����\�����܂��BMB_ABORTRETRYIGNORE �̑���ɁA���̃��b�Z�[�W�{�b�N�X�^�C�v���g���Ă��������B
MB_HELP	Windows 95/98�AWindows NT 4.0 �ȍ~�F���b�Z�[�W�{�b�N�X�Ɂm�w���v�n�{�^����ǉ����܂��B���[�U�[���m�w���v�n�{�^�����N���b�N���邩 F1 �L�[�������ƁA�V�X�e���̓I�[�i�[�� ���b�Z�[�W�𑗐M���܂��B
MB_OK	���b�Z�[�W�{�b�N�X�ɁmOK�n�v�b�V���{�^��������\�����܂��B����͊���̃��b�Z�[�W�{�b�N�X�^�C�v�ł��B
MB_OKCANCEL	���b�Z�[�W�{�b�N�X�ɁmOK�n�A�m�L�����Z���n�̊e�v�b�V���{�^����\�����܂��B
MB_RETRYCANCEL	���b�Z�[�W�{�b�N�X�Ɂm�Ď��s�n�A�m�L�����Z���n�̊e�v�b�V���{�^����\�����܂��B
MB_YESNO	���b�Z�[�W�{�b�N�X�Ɂm�͂��n�A�m�������n�̊e�v�b�V���{�^����\�����܂��B
MB_YESNOCANCEL	���b�Z�[�W�{�b�N�X�Ɂm�͂��n�A�m�������n�A�m�L�����Z���n�̊e�v�b�V���{�^����\�����܂��B
*/
inline int MessageDlg( const tstring& string, int type, int buttons, void* helpCtx ) {
	return ::MessageBox( NULL, string.c_str(), _T(""), type | buttons );
}

extern TTVPMainForm *TVPMainForm;

extern bool TVPMainFormAlive;

#endif
