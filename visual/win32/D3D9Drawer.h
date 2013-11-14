
#ifndef __D3D9_DRAWER_H__
#define __D3D9_DRAWER_H__

#include "Drawer.h"
#include <d3d9.h>

//---------------------------------------------------------------------------
//! @brief	Direct3D7 �ɂ��_�u���o�b�t�@�����O���s���N���X
//! @note	tTVPDrawer_DDDoubleBuffering �Ƃ悭���Ă��邪�ʃN���X�ɂȂ��Ă���B
//!			�C�����s���ꍇ�́A�݂��ɂ悭����ׁA�����悤�ȂƂ��낪����΂Ƃ��ɏC�������݂邱�ƁB
//---------------------------------------------------------------------------
class tTVPDrawer_D3DDoubleBuffering : public tTVPDrawer
{
	typedef tTVPDrawer inherited;

	IDirect3D9*			Direct3D;
	IDirect3DDevice9*	Direct3DDevice;
	IDirect3DTexture9*	Texture;
	//IDirect3DSurface9*	RenderTarget;
	D3DPRESENT_PARAMETERS	D3dPP;
	bool	IsInitd3dpp;
	UINT	CurrentMonitor;

	void * TextureBuffer; //!< �e�N�X�`���̃T�[�t�F�[�X�ւ̃������|�C���^
	long TexturePitch; //!< �e�N�X�`���̃s�b�`

	tjs_uint TextureWidth; //!< �e�N�X�`���̉���
	tjs_uint TextureHeight; //!< �e�N�X�`���̏c��

	bool ShouldShow; //!< show �Ŏ��ۂɉ�ʂɉ摜��]�����ׂ���

	tjs_uint VsyncInterval;

private:
	HRESULT DecideD3DPresentParameters();
	HRESULT DecideD3DPresentParameters( UINT monitor, HWND window, bool iswindow=true, tjs_uint width=0, tjs_uint height=0, tjs_uint bpp=32, tjs_uint color=0, bool changeresolution=true );
	UINT GetMonitorNumber( HWND window );
	HRESULT InitializeDirect3DState();
	void ErrorToLog( HRESULT hr );
	void DestroyTexture();
	void CreateTexture();

public:
	//! @brief	�R���X�g���N�^
	tTVPDrawer_D3DDoubleBuffering(tTVPPassThroughDrawDevice * device);

	//! @brief	�f�X�g���N�^
	~tTVPDrawer_D3DDoubleBuffering();

	virtual ttstr GetName();

	void DestroyOffScreenSurface();
	void InvalidateAll();
	void GetDirect3D9Device();
	void CreateOffScreenSurface();
	bool SetDestPos(tjs_int left, tjs_int top);
	bool SetDestSize(tjs_int width, tjs_int height);
	bool NotifyLayerResize(tjs_int w, tjs_int h);
	bool SetDestSizeAndNotifyLayerResize(tjs_int width, tjs_int height, tjs_int w, tjs_int h);
	void SetTargetWindow(HWND wnd);

//#define TVPD3DTIMING
#ifdef TVPD3DTIMING
	DWORD StartTick;

	DWORD GetDCTime;
	DWORD DrawDibDrawTime;
	DWORD ReleaseDCTime;
	DWORD DrawPrimitiveTime;
	DWORD BltTime;
	void InitTimings();
	void ReportTimings();
#endif

	void StartBitmapCompletion();
	void NotifyBitmapCompleted(tjs_int x, tjs_int y, const void * bits, const BITMAPINFO * bitmapinfo, const tTVPRect &cliprect);
	void EndBitmapCompletion();
	void Show();
	bool WaitForVBlank( tjs_int* in_vblank, tjs_int* delayed );
	virtual int GetInterpolationCapability();

	bool SupportFullScreenChange() const { return true; }
	bool SwitchToFullScreen( HWND window, tjs_uint w, tjs_uint h, tjs_uint bpp, tjs_uint color, bool changeresolution );
	void RevertFromFullScreen( HWND window, tjs_uint w, tjs_uint h, tjs_uint bpp, tjs_uint color );
};
//---------------------------------------------------------------------------

#endif // __D3D9_DRAWER_H__
