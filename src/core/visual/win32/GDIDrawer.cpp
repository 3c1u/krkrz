
#include "tjsCommHead.h"
#include "GDIDrawer.h"
#include "ComplexRect.h"
#include "MsgIntf.h"
#include "DebugIntf.h"
#include "PassThroughDrawDevice.h"


extern bool TVPZoomInterpolation;


//! @brief	�R���X�g���N�^
tTVPDrawer_GDI::tTVPDrawer_GDI(tTVPPassThroughDrawDevice * device) : tTVPDrawer(device)
{
	TargetDC = NULL;
}

//! @brief	�f�X�g���N�^
tTVPDrawer_GDI::~tTVPDrawer_GDI()
{
	if(TargetDC && TargetWindow) ReleaseDC(TargetWindow, TargetDC);
}

void tTVPDrawer_GDI::SetTargetWindow(HWND wnd)
{
	if(wnd)
	{
		// �`��p DC ���擾����
		TargetDC = GetDC(wnd);
	}
	else
	{
		// �`��p DC ���J������
		if(TargetDC) ReleaseDC(TargetWindow, TargetDC), TargetDC = NULL;
	}

	inherited::SetTargetWindow(wnd);

}



//! @brief	�R���X�g���N�^
tTVPDrawer_DrawDibNoBuffering::tTVPDrawer_DrawDibNoBuffering(tTVPPassThroughDrawDevice * device) : tTVPDrawer_GDI(device)
{
	BluePen = NULL;
	YellowPen = NULL;
}

//! @brief	�f�X�g���N�^
tTVPDrawer_DrawDibNoBuffering::~tTVPDrawer_DrawDibNoBuffering()
{
	if(BluePen)   DeleteObject(BluePen);
	if(YellowPen) DeleteObject(YellowPen);
}

ttstr tTVPDrawer_DrawDibNoBuffering::GetName() { return TJS_W("DrawDIB (no buffering)"); }

bool tTVPDrawer_DrawDibNoBuffering::SetDestSize(tjs_int width, tjs_int height)
{
	// ���̃f�o�C�X�ł͊g��k���͂ł��Ȃ��̂�
	// �g��k�����K�v�ȏꍇ�� false ��Ԃ�
	tjs_int w, h;
	Device->GetSrcSize(w, h);
	if(width != w || height != h)
		return false;

	return inherited::SetDestSize(width, height);
}

bool tTVPDrawer_DrawDibNoBuffering::NotifyLayerResize(tjs_int w, tjs_int h)
{
	return inherited::NotifyLayerResize(w, h);
}

void tTVPDrawer_DrawDibNoBuffering::SetTargetWindow(HWND wnd)
{
	inherited::SetTargetWindow(wnd);
}

void tTVPDrawer_DrawDibNoBuffering::StartBitmapCompletion()
{
	// ��邱�ƂȂ�
}

void tTVPDrawer_DrawDibNoBuffering::NotifyBitmapCompleted(tjs_int x, tjs_int y, const void * bits, const BITMAPINFO * bitmapinfo,
	const tTVPRect &cliprect)
{
	// DrawDibDraw �ɂ� TargetDC �ɕ`����s��
	if(DrawDibHandle && TargetDC)
		DrawDibDraw(DrawDibHandle,
			TargetDC,
			x + DestLeft,
			y + DestTop,
			cliprect.get_width(),
			cliprect.get_height(),
			const_cast<BITMAPINFOHEADER*>(reinterpret_cast<const BITMAPINFOHEADER*>(bitmapinfo)),
			const_cast<void*>(bits),
			cliprect.left,
			cliprect.top,
			cliprect.get_width(),
			cliprect.get_height(),
			0);

	// �X�V��`�̕\��
	if(DrawUpdateRectangle)
	{
		if(!BluePen) BluePen = CreatePen(PS_SOLID, 1, RGB(0, 0, 255));
		if(!YellowPen) YellowPen = CreatePen(PS_SOLID, 1, RGB(255, 255, 0));

		HPEN oldpen;

		int ROP_save = GetROP2(TargetDC);

		tjs_int rleft   = x + DestLeft;
		tjs_int rtop    = y + DestTop;
		tjs_int rright  = rleft + cliprect.get_width();
		tjs_int rbottom = rtop  + cliprect.get_height();

		POINT points[5];
		points[0].x = rleft;
		points[0].y = rtop;
		points[1].x = rright -1;
		points[1].y = rtop;
		points[2].x = rright -1;
		points[2].y = rbottom -1;
		points[3].x = rleft;
		points[3].y = rbottom -1;
		points[4] = points[0];

		oldpen = SelectObject(TargetDC, BluePen);
		SetROP2(TargetDC, R2_NOTMASKPEN);
		Polyline(TargetDC, points, 4);

		SelectObject(TargetDC, YellowPen);
		SetROP2(TargetDC, R2_MERGEPEN);
		Polyline(TargetDC, points, 5);

		SelectObject(TargetDC, oldpen);
	}
}

void tTVPDrawer_DrawDibNoBuffering::EndBitmapCompletion()
{
	// ��邱�ƂȂ�
}

int tTVPDrawer_DrawDibNoBuffering::GetInterpolationCapability() { return 1; }
	// bit 0 for point-on-point, bit 1 for bilinear interpolation

//---------------------------------------------------------------------------


//! @brief	�R���X�g���N�^
tTVPDrawer_GDIDoubleBuffering::tTVPDrawer_GDIDoubleBuffering(tTVPPassThroughDrawDevice * device) : tTVPDrawer_GDI(device)
{
	OffScreenBitmap = NULL;
	OffScreenDC = NULL;
	OldOffScreenBitmap = NULL;
	ShouldShow = false;
	InBenchMark = false;
}

//! @brief	�f�X�g���N�^
tTVPDrawer_GDIDoubleBuffering::~tTVPDrawer_GDIDoubleBuffering()
{
	DestroyBitmap();
}

ttstr tTVPDrawer_GDIDoubleBuffering::GetName() { return TJS_W("GDI double buffering"); }

void tTVPDrawer_GDIDoubleBuffering::DestroyBitmap()
{
	if(OffScreenBitmap && OffScreenDC)
	{
		SelectObject(OffScreenDC, OldOffScreenBitmap), OldOffScreenBitmap = NULL;
		if(OffScreenBitmap) DeleteObject(OffScreenBitmap), OffScreenBitmap = NULL;
		if(OffScreenDC)     DeleteDC(OffScreenDC), OffScreenDC = NULL;
	}
}

void tTVPDrawer_GDIDoubleBuffering::CreateBitmap()
{
	// �X�N���[���݊��� DDB ���쐬����B
	// ����͂����Ă��̏ꍇ�A�r�f�I��������ɍ쐬�����B
	DestroyBitmap();
	if(TargetWindow && SrcWidth > 0 && SrcHeight > 0)
	{
		try
		{
			HDC screendc = GetDC(TargetWindow);
			if(!screendc) TVPThrowExceptionMessage(TJS_W("Failed to create screen DC"));
			OffScreenBitmap = CreateCompatibleBitmap(screendc, SrcWidth, SrcHeight);
			if(!OffScreenBitmap) TVPThrowExceptionMessage(TJS_W("Failed to create offscreen bitmap"));
			OffScreenDC     = CreateCompatibleDC(screendc);
			if(!OffScreenDC) TVPThrowExceptionMessage(TJS_W("Failed to create offscreen DC"));
			ReleaseDC(TargetWindow, screendc);
			OldOffScreenBitmap = SelectObject(OffScreenDC, OffScreenBitmap);
		}
		catch(...)
		{
			DestroyBitmap();
			throw;
		}
	}
}

bool tTVPDrawer_GDIDoubleBuffering::SetDestSize(tjs_int width, tjs_int height)
{
	return inherited::SetDestSize(width, height);
}

bool tTVPDrawer_GDIDoubleBuffering::NotifyLayerResize(tjs_int w, tjs_int h)
{
	if(inherited::NotifyLayerResize(w, h))
	{
		CreateBitmap();
		return true;
	}
	return false;
}

void tTVPDrawer_GDIDoubleBuffering::SetTargetWindow(HWND wnd)
{
	inherited::SetTargetWindow(wnd);
	CreateBitmap();
}

void tTVPDrawer_GDIDoubleBuffering::StartBitmapCompletion()
{
	// ��邱�ƂȂ�
}

void tTVPDrawer_GDIDoubleBuffering::NotifyBitmapCompleted(tjs_int x, tjs_int y, const void * bits, const BITMAPINFO * bitmapinfo,
	const tTVPRect &cliprect)
{
	// DrawDibDraw �ɂ� OffScreenDC �ɕ`����s��
	if(DrawDibHandle && OffScreenDC)
	{
		ShouldShow = true;
		DrawDibDraw(DrawDibHandle,
			OffScreenDC,
			x,
			y,
			cliprect.get_width(),
			cliprect.get_height(),
			const_cast<BITMAPINFOHEADER*>(reinterpret_cast<const BITMAPINFOHEADER*>(bitmapinfo)),
			const_cast<void*>(bits),
			cliprect.left,
			cliprect.top,
			cliprect.get_width(),
			cliprect.get_height(),
			0);
	}
}

void tTVPDrawer_GDIDoubleBuffering::EndBitmapCompletion()
{
}

void tTVPDrawer_GDIDoubleBuffering::Show()
{
	if(TargetDC && OffScreenDC && ShouldShow)
	{
		// �I�t�X�N���[���r�b�g�}�b�v�� TargetDC �ɓ]������
		if(DestWidth == SrcWidth && DestHeight == SrcHeight)
		{
			// �g��E�k���͕K�v�Ȃ�
			BitBlt(TargetDC,
				DestLeft,
				DestTop,
				DestWidth,
				DestHeight,
				OffScreenDC,
				0,
				0,
				SRCCOPY);
		}
		else
		{
			// �g��E�k�����K�v
			if(TVPZoomInterpolation)
				SetStretchBltMode(TargetDC, HALFTONE);
			else
				SetStretchBltMode(TargetDC, COLORONCOLOR);
			SetBrushOrgEx(TargetDC, 0, 0, NULL);

			StretchBlt(TargetDC,
				DestLeft,
				DestTop,
				DestWidth,
				DestHeight,
				OffScreenDC,
				0,
				0,
				SrcWidth,
				SrcHeight,
				SRCCOPY);
		}

		if(InBenchMark)
		{
			// ��ʂ���̓ǂݏo�����s���֐������s����
			// �������Ȃ��� StrechBlt �Ȃǂ̓R�}���h�L���[�ɂ��������܂��
			// �����ŁA���ۂ̕`���҂����ɋA��\��������B
			(void)GetPixel(TargetDC, DestLeft + DestWidth / 2, DestTop + DestHeight / 2);
		}

		ShouldShow = false;
	}
}

int tTVPDrawer_GDIDoubleBuffering::GetInterpolationCapability() { return 1+2; }
	// bit 0 for point-on-point, bit 1 for bilinear interpolation

void tTVPDrawer_GDIDoubleBuffering::InitTimings() { InBenchMark = true; }
void tTVPDrawer_GDIDoubleBuffering::ReportTimings() { InBenchMark = false; }


//---------------------------------------------------------------------------
