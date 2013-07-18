
#ifndef __D3D9_DRAWER_H__
#define __D3D9_DRAWER_H__

#include "Drawer.h"
#include "DLLLoader.h"
#include <d3d9.h>

//---------------------------------------------------------------------------
//! @brief	Direct3D7 �ɂ��_�u���o�b�t�@�����O���s���N���X
//! @note	tTVPDrawer_DDDoubleBuffering �Ƃ悭���Ă��邪�ʃN���X�ɂȂ��Ă���B
//!			�C�����s���ꍇ�́A�݂��ɂ悭����ׁA�����悤�ȂƂ��낪����΂Ƃ��ɏC�������݂邱�ƁB
//---------------------------------------------------------------------------
class tTVPDrawer_D3DDoubleBuffering : public tTVPDrawer
{
	typedef tTVPDrawer inherited;

	tTVPDLLLoader		D3DDll;
	IDirect3D9*			Direct3D;
	IDirect3DDevice9*	Direct3DDevice;
	//IDirect3DSurface9*	Surface;
	//D3DSURFACE_DESC		SurfaceDesc;
	IDirect3DTexture9*	Texture;
	//IDirect3DSurface9*	RenderTarget;

	void * TextureBuffer; //!< �e�N�X�`���̃T�[�t�F�[�X�ւ̃������|�C���^
	long TexturePitch; //!< �e�N�X�`���̃s�b�`

	tjs_uint TextureWidth; //!< �e�N�X�`���̉���
	tjs_uint TextureHeight; //!< �e�N�X�`���̏c��

	bool ShouldShow; //!< show �Ŏ��ۂɉ�ʂɉ摜��]�����ׂ���

	SIZE BackBufferSize;

private:
	HRESULT DecideD3DPresentParameters( D3DPRESENT_PARAMETERS& d3dpp );
	UINT GetMonitorNumber();
	HRESULT InitializeDirect3DState();

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
	virtual int GetInterpolationCapability();
};
//---------------------------------------------------------------------------

#endif // __D3D9_DRAWER_H__
