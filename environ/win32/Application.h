
#ifndef __T_APPLICATION_H__
#define __T_APPLICATION_H__

#include <vector>
#include <map>
#include "tstring.h"

//tstring ParamStr( int index );
tstring ExePath();

// ��Ō��ʂ��̂悤���@�ɕύX����
extern int _argc;
extern char ** _argv;

enum {
	mrOk,
	mrAbort,
	mrCancel,
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

enum {
  mtWarning = MB_ICONWARNING,
  mtError = MB_ICONERROR,
  mtInformation = MB_ICONINFORMATION,
  mtConfirmation = MB_ICONQUESTION,
  mtStop = MB_ICONSTOP,
  mtCustom = 0
};
enum {
	mbOK = MB_OK,
};

class AcceleratorKey {
	HACCEL hAccel_;
	ACCEL* keys_;
	int key_count_;

public:
	AcceleratorKey();
	~AcceleratorKey();
	void AddKey( WORD id, WORD key, BYTE virt );
	void DelKey( WORD id );
	HACCEL GetHandle() { return hAccel_; }
};
class AcceleratorKeyTable {
	std::map<HWND,AcceleratorKey*> keys_;
	HACCEL hAccel_;

public:
	AcceleratorKeyTable();
	~AcceleratorKeyTable();
	void AddKey( HWND hWnd, WORD id, WORD key, BYTE virt );
	void DelKey( HWND hWnd, WORD id );
	void DelTable( HWND hWnd );
	HACCEL GetHandle(HWND hWnd) {
		std::map<HWND,AcceleratorKey*>::iterator i = keys_.find(hWnd);
		if( i != keys_.end() ) {
			return i->second->GetHandle();
		}
		return hAccel_;
	}
};
class TApplication {
	std::vector<class TTVPWindowForm*> windows_list_;
	tstring title_;

	bool is_attach_console_;
	FILE* oldstdin_;
	FILE* oldstdout_;
	tstring console_title_;
	AcceleratorKeyTable accel_key_;

public:
	TApplication() : is_attach_console_(false) {}
	void CheckConsole();
	void CloseConsole();
	bool IsAttachConsole() { return is_attach_console_; }
	void CheckDigitizer();

	HWND GetHandle();
	bool IsIconic() {
		HWND hWnd = GetHandle();
		if( hWnd != INVALID_HANDLE_VALUE ) {
			return 0 != ::IsIconic(hWnd);
		}
		return true; // ���������E�B���h�E���Ȃ�
	}
	void Minimize();
	void Restore();
	void BringToFront();

	void AddWindow( class TTVPWindowForm* win ) {
		windows_list_.push_back( win );
	}
	void RemoveWindow( class TTVPWindowForm* win );

	void ProcessMessages();
	void HandleMessage();

	tstring GetTitle() const { return title_; }
	void SetTitle( const tstring& caption );

	static inline int MessageDlg( const tstring& string, const tstring& caption, int type, int button ) {
		return ::MessageBox( NULL, string.c_str(), caption.c_str(), type|button  );
	}
	void Terminate() {
		::PostQuitMessage(0);
	}
	void SetHintHidePause( int v ) {}
	void SetShowHint( bool b ) {}
	void SetShowMainForm( bool b ) {}

	void Initialize() {}
	void ShowException( class Exception* e );
	void Run();

	HWND GetMainWindowHandle();

	int ArgC;
	char ** ArgV;
	std::vector<std::string> CommandLines;

	void PostMessageToMainWindow(UINT message, WPARAM wParam, LPARAM lParam);


	void ModalStarted() {}
	void ModalFinished() {}
	void DisableWindows();
	void EnableWindows( const std::vector<class TTVPWindowForm*>& ignores );
	void GetDisableWindowList( std::vector<class TTVPWindowForm*>& win );

	
	void RegisterAcceleratorKey(HWND hWnd, char virt, short key, short cmd);
	void UnregisterAcceleratorKey(HWND hWnd, short cmd);
	void DeleteAcceleratorKeyTable( HWND hWnd );
};
std::vector<std::string>* LoadLinesFromFile( const tstring& path );

// �X�^�u�A�������͓��삵�Ȃ��͂��B
inline HWND AllocateHWnd( LRESULT (CALLBACK *UtilWndProc)(HWND,UINT,WPARAM,LPARAM) ) {
	
	const TCHAR* classname = _T("TPUtilWindow");
	WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, UtilWndProc, 0L, 0L,
						GetModuleHandle(NULL), NULL, NULL, NULL, NULL,
						classname, NULL };
	::RegisterClassEx( &wc );
	HWND hWnd = ::CreateWindow( classname, _T(""),
						WS_OVERLAPPEDWINDOW, 0, 0, 0, 0,
						NULL, NULL, wc.hInstance, NULL );
	return hWnd;
}
inline bool DeallocateHWnd( HWND hWnd ) {
	return 0 != ::DestroyWindow( hWnd );
}


inline HINSTANCE GetHInstance() { return ((HINSTANCE)GetModuleHandle(0)); }
extern class TApplication* Application;


#endif // __T_APPLICATION_H__
