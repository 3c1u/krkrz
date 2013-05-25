

#ifndef __MOUSE_CURSOR_H__
#define __MOUSE_CURSOR_H__

#include "stdafx.h"

class MouseCursor {
	enum {
		CURSOR_APPSTARTING,	// �W�����J�[�\������я��^�����v�J�[�\��
		CURSOR_ARROW,		// �W�����J�[�\��
		CURSOR_CROSS,		// �\���J�[�\��
		CURSOR_HAND,		// �n���h�J�[�\��
		CURSOR_IBEAM,		// �A�C�r�[�� (�c��) �J�[�\��
		CURSOR_HELP,		// ���Ƌ^�╄
		CURSOR_NO,			// �֎~�J�[�\��
		CURSOR_SIZEALL,		// 4 �������J�[�\��
		CURSOR_SIZENESW,	// �΂ߍ�������̗��������J�[�\��
		CURSOR_SIZENS,		// �㉺���������J�[�\��
		CURSOR_SIZENWSE,	// �΂߉E������̗��������J�[�\��
		CURSOR_SIZEWE,		// ���E���������J�[�\��
		CURSOR_UPARROW,		// �����̖��J�[�\��
		CURSOR_WAIT,		// �����v�J�[�\�� 
		CURSOR_EOT,
	};
	static const LPTSTR CURSORS[CURSOR_EOT];
	static HCURSOR CURSOR_HANDLES[CURSOR_EOT];
	static const int INVALID_CURSOR_INDEX = 0x7FFFFFFF;
	static bool is_cursor_hide_;

public:
	static void Initialize();
	static void SetMouseCursor( int index );

private:
	HCURSOR hCursor_;
	int cursor_index_;

public:
	MouseCursor() : hCursor_(INVALID_HANDLE_VALUE), cursor_index_(INVALID_CURSOR_INDEX) {}
	MouseCursor( int index ) : hCursor_(INVALID_HANDLE_VALUE), cursor_index_(index) {}

	void SetCursor();

	bool IsCurrentCursor( int index ) {
		return cursor_index_ == index;
	}
	void SetCursorIndex( int index );
};

#endif
