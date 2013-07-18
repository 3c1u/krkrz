

#ifndef __GDI_DRAWER_H__
#define __GDI_DRAWER_H__

#include "Drawer.h"
#include <windows.h>

//---------------------------------------------------------------------------
//! @brief	GDI�ɂ��`���K�v�Ƃ����{�N���X
//---------------------------------------------------------------------------
class tTVPDrawer_GDI : public tTVPDrawer
{
	typedef tTVPDrawer inherited;
protected:
	HDC TargetDC;

public:
	//! @brief	�R���X�g���N�^
	tTVPDrawer_GDI(tTVPPassThroughDrawDevice * device);
	~tTVPDrawer_GDI();
	void SetTargetWindow(HWND wnd);
};
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
//! @brief	DrawDib�ɂ��o�b�t�@�����`����s����{�N���X
//---------------------------------------------------------------------------
class tTVPDrawer_DrawDibNoBuffering : public tTVPDrawer_GDI
{
	typedef tTVPDrawer_GDI inherited;
	HPEN BluePen;
	HPEN YellowPen;

public:
	//! @brief	�R���X�g���N�^
	tTVPDrawer_DrawDibNoBuffering(tTVPPassThroughDrawDevice * device);
	~tTVPDrawer_DrawDibNoBuffering();
	virtual ttstr GetName();
	bool SetDestSize(tjs_int width, tjs_int height);
	bool NotifyLayerResize(tjs_int w, tjs_int h);
	void SetTargetWindow(HWND wnd);
	void StartBitmapCompletion();
	void NotifyBitmapCompleted(tjs_int x, tjs_int y, const void * bits, const BITMAPINFO * bitmapinfo, const tTVPRect &cliprect);
	void EndBitmapCompletion();
	int GetInterpolationCapability();

};
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
//! @brief	GDI�ɂ��_�u���o�b�t�@�����O���s���N���X
//---------------------------------------------------------------------------
class tTVPDrawer_GDIDoubleBuffering : public tTVPDrawer_GDI
{
	typedef tTVPDrawer_GDI inherited;
	HBITMAP OffScreenBitmap; //!< �I�t�X�N���[���r�b�g�}�b�v
	HDC OffScreenDC; //!< �I�t�X�N���[�� DC
	HBITMAP OldOffScreenBitmap; //!< OffScreenDC �ɈȑO�I������Ă��� �r�b�g�}�b�v
	bool ShouldShow; //!< show �Ŏ��ۂɉ�ʂɉ摜��]�����ׂ���
	bool InBenchMark; //!< �x���`�}�[�N�����ǂ���

public:
	tTVPDrawer_GDIDoubleBuffering(tTVPPassThroughDrawDevice * device);
	~tTVPDrawer_GDIDoubleBuffering();
	ttstr GetName();
	void DestroyBitmap();
	void CreateBitmap();
	bool SetDestSize(tjs_int width, tjs_int height);
	bool NotifyLayerResize(tjs_int w, tjs_int h);
	void SetTargetWindow(HWND wnd);
	void StartBitmapCompletion();
	void NotifyBitmapCompleted(tjs_int x, tjs_int y, const void * bits, const BITMAPINFO * bitmapinfo, const tTVPRect &cliprect);
	void EndBitmapCompletion();
	void Show();
	virtual int GetInterpolationCapability();
	virtual void InitTimings();
	virtual void ReportTimings();
};
//---------------------------------------------------------------------------

#endif // __GDI_DRAWER_H__
