//---------------------------------------------------------------------------
/*
	TVP2 ( T Visual Presenter 2 )  A script authoring tool
	Copyright (C) 2000 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
//!@file "PassThrough" �`��f�o�C�X�Ǘ�
//---------------------------------------------------------------------------
#define NOMINMAX
#include "tjsCommHead.h"
#include "DrawDevice.h"
#include "PassThroughDrawDevice.h"
#include "LayerIntf.h"
#include "MsgIntf.h"
#include "SysInitIntf.h"
#include "WindowIntf.h"
#include "DebugIntf.h"
#include "ThreadIntf.h"
#include "Drawer.h"

#include "D3D9Drawer.h"
#include "GDIDrawer.h"
//#include "DDrawDrawer.h"


/*
	PassThroughDrawDevice �N���X�ɂ́AWindow.PassThroughDrawDevice �Ƃ���
	�A�N�Z�X�ł���B�ʏ�AWindow �N���X�𐶐�����ƁA���� drawDevice �v���p
	�e�B�ɂ͎����I�ɂ��̃N���X�̃C���X�^���X���ݒ肳���̂ŁA(�ق���DrawDevice
	���g��Ȃ������) ���Ɉӎ�����K�v�͂Ȃ��B

	PassThroughDrawDevice �͈ȉ��̃��\�b�h�ƃv���p�e�B�����B

	recreate()
		Drawer (�����Ŏg�p���Ă���`�����) ��؂�ւ���BpreferredDrawer �v���p�e�B
		�� dtNone �ȊO�ł���΂���ɏ]�����A�K���w�肳�ꂽ drawer ���g�p�����ۏ؂͂Ȃ��B

	preferredDrawer
		�g�p������ drawer ��\���v���p�e�B�B�ȉ��̂����ꂩ�̒l���Ƃ�B
		�l��ݒ肷�邱�Ƃ��\�Bnew ����̒l�� �R�}���h���C���I�v�V������ dbstyle ��
		�ݒ肵���l�ɂȂ�B
		drawer�����̒l�ɂȂ�ۏ؂͂Ȃ� (���Ƃ��� dtDBD3D ���w�肵�Ă��Ă����炩��
		������ Direct3D �̏������Ɏ��s�����ꍇ�� DirectDraw ���g�p�����\��������)�B
		�E�B���h�E�쐬����A�ŏ��Ƀv���C�}�����C�����쐬��������O�ɂ��̃v���p�e�B��
		�ݒ肷�鎖�ɂ��Arecreate() ���킴�킴���s���Ȃ��Ă��w��� drawer ���g�p
		�����邱�Ƃ��ł���B
		Window.PassThroughDrawDevice.dtNone			�w�肵�Ȃ�
		Window.PassThroughDrawDevice.dtDrawDib		�g��k�����K�v�ȏꍇ��GDI�A
													�����łȂ����DB�Ȃ�
		Window.PassThroughDrawDevice.dtDBGDI		GDI�ɂ��DB
		Window.PassThroughDrawDevice.dtDBDD			DirectDraw�ɂ��DB(�폜)
		Window.PassThroughDrawDevice.dtDBD3D		Direct3D�ɂ��DB

	drawer
		���ݎg�p����Ă��� drawer ��\���v���p�e�B�B�ȉ��̂����ꂩ�̒l���Ƃ�B
		�ǂݎ���p�B
		Window.PassThroughDrawDevice.dtNone			���ʂ͂���͂Ȃ�
		Window.PassThroughDrawDevice.dtDrawDib		�_�u���o�b�t�@�����O(DB)�Ȃ�
		Window.PassThroughDrawDevice.dtDBGDI		GDI�ɂ��DB
		Window.PassThroughDrawDevice.dtDBDD			DirectDraw�ɂ��DB(�폜)
		Window.PassThroughDrawDevice.dtDBD3D		Direct3D�ɂ��DB
*/


//---------------------------------------------------------------------------
// �I�v�V����
//---------------------------------------------------------------------------
static tjs_int TVPPassThroughOptionsGeneration = 0;
bool TVPZoomInterpolation = true;
static bool TVPForceDoublebuffer = false;
static tTVPPassThroughDrawDevice::tDrawerType TVPPreferredDrawType = tTVPPassThroughDrawDevice::dtNone;
//---------------------------------------------------------------------------
static void TVPInitPassThroughOptions()
{
	if(TVPPassThroughOptionsGeneration == TVPGetCommandLineArgumentGeneration()) return;
	TVPPassThroughOptionsGeneration = TVPGetCommandLineArgumentGeneration();

	bool initddraw = false;
	tTJSVariant val;

	TVPForceDoublebuffer = false;
	if(TVPGetCommandLine(TJS_W("-usedb"), &val) )
	{
		ttstr str(val);
		if(str == TJS_W("yes")) TVPForceDoublebuffer = true;
	}

	TVPPreferredDrawType = tTVPPassThroughDrawDevice::dtNone;
	if(TVPGetCommandLine(TJS_W("-dbstyle"), &val) )
	{
		ttstr str(val);
		if(str == TJS_W("none") || str == TJS_W("no") || str == TJS_W("auto"))
			TVPPreferredDrawType = tTVPPassThroughDrawDevice::dtNone;
		if(str == TJS_W("gdi"))
			TVPPreferredDrawType = tTVPPassThroughDrawDevice::dtDBGDI;
		//if(str == TJS_W("ddraw"))
		//	TVPPreferredDrawType = tTVPPassThroughDrawDevice::dtDBDD;
		if(str == TJS_W("d3d"))
			TVPPreferredDrawType = tTVPPassThroughDrawDevice::dtDBD3D;
	}

	TVPZoomInterpolation = true;
	if(TVPGetCommandLine(TJS_W("-smoothzoom"), &val))
	{
		ttstr str(val);
		if(str == TJS_W("no"))
			TVPZoomInterpolation = false;
		else
			TVPZoomInterpolation = true;
	}

	if(initddraw) TVPEnsureDirectDrawObject();
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
tTVPPassThroughDrawDevice::tTVPPassThroughDrawDevice()
{
	TVPInitPassThroughOptions(); // read and initialize options
	PreferredDrawerType = TVPPreferredDrawType;
	TargetWindow = NULL;
	Drawer = NULL;
	DrawerType = dtNone;
	DestSizeChanged = false;
	SrcSizeChanged = false;
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
tTVPPassThroughDrawDevice::~tTVPPassThroughDrawDevice()
{
	DestroyDrawer();
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tTVPPassThroughDrawDevice::DestroyDrawer()
{
	if(Drawer) delete Drawer, Drawer = NULL;
	DrawerType = dtNone;
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tTVPPassThroughDrawDevice::CreateDrawer(tDrawerType type)
{
	if(Drawer) delete Drawer, Drawer = NULL;

	switch(type)
	{
	case dtNone:
		break;
	case dtDrawDib:
		Drawer = new tTVPDrawer_DrawDibNoBuffering(this);
		break;
	case dtDBGDI:
		Drawer = new tTVPDrawer_GDIDoubleBuffering(this);
		break;
//	case dtDBDD:
//		Drawer = new tTVPDrawer_DDDoubleBuffering(this);
//		break;
	case dtDBD3D:
		Drawer = new tTVPDrawer_D3DDoubleBuffering(this);
		break;
	}

	try
	{

		if(Drawer)
			Drawer->SetTargetWindow(TargetWindow);

		if(Drawer)
		{
			if(!Drawer->SetDestPos(DestRect.left, DestRect.top))
			{
				TVPAddImportantLog(
					TJS_W("Passthrough: Failed to set destination position to draw device drawer"));
				delete Drawer, Drawer = NULL;
			}
		}

		if(Drawer)
		{
			tjs_int srcw, srch;
			GetSrcSize(srcw, srch);
			if(!Drawer->SetDestSizeAndNotifyLayerResize(DestRect.get_width(), DestRect.get_height(), srcw, srch))
			{
				TVPAddImportantLog(
					TJS_W("Passthrough: Failed to set destination size and source layer size to draw device drawer"));
				delete Drawer, Drawer = NULL;
			}
		}

		if(Drawer) DrawerType = type; else DrawerType = dtNone;

		RequestInvalidation(tTVPRect(0, 0, DestRect.get_width(), DestRect.get_height()));
	}
	catch(const eTJS & e)
	{
		TVPAddImportantLog(TJS_W("Passthrough: Failed to create drawer: ") + e.GetMessage());
		DestroyDrawer();
	}
	catch(...)
	{
		TVPAddImportantLog(TJS_W("Passthrough: Failed to create drawer: unknown reason"));
		DestroyDrawer();
	}
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tTVPPassThroughDrawDevice::CreateDrawer(bool zoom_required, bool should_benchmark)
{
	// �v���C�}�����C���̃T�C�Y���擾
	tjs_int srcw, srch;
	GetSrcSize(srcw, srch);

	// �������� Drawer ���폜
	tDrawerType last_type = DrawerType;
	DestroyDrawer();

	// �v���C�}�����C�����Ȃ��Ȃ�� DrawDevice �͍쐬���Ȃ�
	if(srcw == 0 || srch == 0) return;

	// should_benchmark ���U�ŁA�O�� Drawer ���쐬���Ă���΁A����Ɠ����^�C�v��
	// Drawer ��p����
	if(!Drawer && !should_benchmark && last_type != dtNone)
		CreateDrawer(last_type);

	// PreferredDrawerType ���w�肳��Ă���΂�����g��
	if(!Drawer)
	{
		// PreferredDrawerType �� dtDrawDib �̏ꍇ�́A�Y�[�����K�v�ȏꍇ��
		// dtGDI ��p����
		if (PreferredDrawerType == dtDrawDib)
			CreateDrawer(zoom_required ? dtDBGDI : dtDrawDib);
		else if(PreferredDrawerType != dtNone)
			CreateDrawer(PreferredDrawerType);
	}

	// �����Y�[�����K�v�Ȃ��A�_�u���o�b�t�@�����O���K�v�Ȃ��Ȃ��
	// ��Ԋ�{�I�� DrawDib �̂���g��
	if(!Drawer && !zoom_required && !TVPForceDoublebuffer)
		CreateDrawer(dtDrawDib);

	if(!Drawer)
	{
		// ���C���E�B���h�E�ȊO�̏ꍇ�̓Y�[�����K�v�Ȃ���Ί�{�I�ȃ��\�b�h���g��
		if(!IsMainWindow && !zoom_required)
			CreateDrawer(dtDrawDib);
	}

	if(!Drawer)
	{
		// �܂� Drawer ���쐬����ĂȂ���

		// �x���`�}�[�N���܂�����
		static tDrawerType bench_types[] = { /*dtDBDD,*/ dtDBGDI, dtDBD3D };
		const static tjs_char * type_names[] = { /*TJS_W("DirectDraw"),*/ TJS_W("GDI"), TJS_W("Direct3D") };
		static const int num_types = sizeof(bench_types) / sizeof(bench_types[0]);
		struct tBenchmarkResult
		{
			float score;
			tDrawerType type;
		} results[num_types];

		// �x���`�}�[�N�p�̌��摜���m��
		BITMAPINFOHEADER bmi;
		bmi.biSize = sizeof(BITMAPINFOHEADER);
		bmi.biWidth = srcw;
		bmi.biHeight = srch;
		bmi.biPlanes = 1;
		bmi.biBitCount = 32;
		bmi.biCompression = BI_RGB;
		bmi.biSizeImage = srcw * 4 * srch; // 32bpp �̏ꍇ�͂���ł���
		bmi.biXPelsPerMeter = 0;
		bmi.biYPelsPerMeter = 0;
		bmi.biClrUsed = 0;
		bmi.biClrImportant = 0;

		void * memblk = GlobalAlloc(GMEM_FIXED, bmi.biSizeImage + 64); // 64 = �]�T(�����Ă�������������Ȃ�)
		ZeroMemory(memblk, bmi.biSizeImage);

		tTVPRect cliprect;
		cliprect.left = 0;
		cliprect.top = 0;
		cliprect.right = srcw;
		cliprect.bottom = srch;

		// �x���`�}�[�N���s��
		for(int i = 0; i < num_types; i++)
		{
			results[i].type = bench_types[i];
			results[i].score = -1.0f;

			try
			{
				// drawer ���쐬
				CreateDrawer(results[i].type);
				if(!Drawer)
				{
					TVPAddImportantLog(TJS_W("Passthrough: Could not create drawer object ") + ttstr(type_names[i]));
					continue;
				}

				// �Y�[����Ԃ̐ݒ�͎󂯓�����邩�H
				int caps = Drawer->GetInterpolationCapability();
				if(TVPZoomInterpolation && !(caps & 2))
				{
					TVPAddImportantLog(TJS_W("Passthrough: Drawer object ") + ttstr(type_names[i]) +
						TJS_W(" does not have smooth zooming functionality"));
					continue;
				}
				else if(!TVPZoomInterpolation && !(caps & 1))
				{
					TVPAddImportantLog(TJS_W("Passthrough: Drawer object ") + ttstr(type_names[i]) +
						TJS_W(" does not have point-on-point zooming functionality"));
					continue;
				}

				// �x���`�}�[�N���s��
				// �������Ԗ�333ms�ŁA���̊Ԃɉ���]�����s���邩������
				Drawer->InitTimings();
				static const DWORD timeout = 333;
				DWORD start_tick = timeGetTime();
				int count = 0;
				while(timeGetTime() - start_tick < timeout)
				{
					Drawer->StartBitmapCompletion();
					Drawer->NotifyBitmapCompleted(0, 0, memblk, (const BITMAPINFO *)&bmi, cliprect);
					Drawer->EndBitmapCompletion();
					Drawer->Show();
					count ++;
				}
				DWORD end_tick = timeGetTime();
				Drawer->ReportTimings();

				// ���ʂ��i�[�A����ƃf�o�b�O�p�ɕ\��
				results[i].score = count * 1000 / (float)(end_tick - start_tick);
				char msg[80];
				sprintf(msg, "%.2f fps", (float)results[i].score);
				TVPAddImportantLog(TJS_W("Passthrough: benchmark result: ") + ttstr(type_names[i]) + TJS_W(" : ") +
					msg);

				// GDI �͍Ō�̎�i
				// ���ʂ����͌v���Ă������A���ꂪ���ɂȂ�̂�
				// �ق���drawer�Ɏ��s�����Ƃ��̂�
				if(results[i].type == dtDBGDI)
					results[i].score = 0.0f;
/*
				// DirectDraw + Vista �`�F�b�N
				// ��������ʂ����͑����Ă������A���ꂪ���ɂȂ�̂�
				// �ق��� drawer �Ɏ��s�����Ƃ��̂�
				if(results[i].type == dtDBDD)
				{
					OSVERSIONINFO osinfo;
					osinfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
					GetVersionEx(&osinfo);
					if(osinfo.dwMajorVersion >= 6) // vista or later
					{
						results[i].score = 0.0f;
						TVPAddImportantLog(TJS_W("Passthrough: Windows Vista (or later) detected, DirectDraw is not preferred"));
					}
				}
*/
			}
			catch(...)
			{
				DestroyDrawer();
			}
			DestroyDrawer();
		}

		// �x���`�}�[�N�Ɏg�����摜�����
		GlobalFree((HGLOBAL)memblk);


		// ���ʂ��X�R�A���Ƀ\�[�g
		// ����Ȃɐ��͑����Ȃ��̂Ō��n�I�Ƀo�u���\�[�g
		while(true)
		{
			bool swapped = false;
			for(int i = 0; i < num_types - 1; i++)
			{
				if(results[i].score < results[i+1].score)
				{
					tBenchmarkResult tmp = results[i];
					results[i] = results[i+1];
					results[i+1] = tmp;
					swapped = true;
				}
			}
			if(!swapped) break;
		}
	
		// �X�R�A�̍���������쐬�����݂�
		for(int i = 0; i < num_types; i++)
		{
			CreateDrawer(results[i].type);
			if(Drawer) break;
		}

	}

	if(!Drawer)
	{
		// Drawer ��S���쐬�ł��Ȃ�����
		// ����̓��o��
		// �܂����蓾�Ȃ����v���I�B
		TVPThrowExceptionMessage(TJS_W("Fatal: Could not create any drawer objects."));
	}

	if(Drawer)
	{
		if(IsMainWindow)
			TVPAddImportantLog(TJS_W("Passthrough: Using passthrough draw device: ") + Drawer->GetName());
	}
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tTVPPassThroughDrawDevice::EnsureDrawer()
{
	// ���̃��\�b�h�ł́A�ȉ��̏����̍ۂ� drawer �����(��蒼��)�B
	// 1. Drawer �� NULL �̏ꍇ
	// 2. ���݂� Drawer �̃^�C�v���K�؂łȂ��Ȃ����Ƃ�
	// 3. ���̃��C���̃T�C�Y���ύX���ꂽ�Ƃ�
	TVPInitPassThroughOptions();

	if(TargetWindow)
	{
		// �Y�[���͕K�v���������H
		bool zoom_was_required = false;
		if(Drawer)
		{
			tjs_int srcw, srch;
			Drawer->GetSrcSize(srcw, srch);
			tjs_int destw, desth;
			Drawer->GetDestSize(destw, desth);
			if(destw != srcw || desth != srch)
				zoom_was_required = true;
		}

		// �Y�[����(�����)�K�v���H
		bool zoom_is_required = false;
		tjs_int srcw, srch;
		GetSrcSize(srcw, srch);
		if(DestRect.get_width() != srcw || DestRect.get_height() != srch)
			zoom_is_required = true;


		bool need_recreate = false;
		bool should_benchmark = false;
		if(!Drawer) need_recreate = true;
		if(zoom_was_required != zoom_is_required) need_recreate = true;
		if(need_recreate) should_benchmark = true;
		if(SrcSizeChanged) { SrcSizeChanged = false; need_recreate = true; }
			// SrcSizeChanged �Ƃ������R�����ł� should_benchmark �͐^�ɂ�
			// �ݒ肵�Ȃ�

		if(need_recreate)
		{
			// Drawer �̍č쐬���K�v
			CreateDrawer(zoom_is_required, should_benchmark);
		}
	}
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void TJS_INTF_METHOD tTVPPassThroughDrawDevice::AddLayerManager(iTVPLayerManager * manager)
{
	if(inherited::Managers.size() > 0)
	{
		// "Pass Through" �f�o�C�X�ł͂Q�ȏ��Layer Manager��o�^�ł��Ȃ�
		TVPThrowExceptionMessage(TJS_W("\"passthrough\" device does not support layer manager more than 1"));
			// TODO: i18n
	}
	inherited::AddLayerManager(manager);

	manager->SetDesiredLayerType(ltOpaque); // ltOpaque �ȏo�͂��󂯎�肽��
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void TJS_INTF_METHOD tTVPPassThroughDrawDevice::SetTargetWindow(HWND wnd, bool is_main)
{
	TVPInitPassThroughOptions();
	DestroyDrawer();
	TargetWindow = wnd;
	IsMainWindow = is_main;
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void TJS_INTF_METHOD tTVPPassThroughDrawDevice::SetDestRectangle(const tTVPRect & rect)
{
	// �ʒu�����̕ύX�̏ꍇ���ǂ������`�F�b�N����
	if(rect.get_width() == DestRect.get_width() && rect.get_height() == DestRect.get_height())
	{
		// �ʒu�����̕ύX��
		if(Drawer) Drawer->SetDestPos(rect.left, rect.top);
		inherited::SetDestRectangle(rect);
	}
	else
	{
		// �T�C�Y���Ⴄ
		DestSizeChanged = true;
		inherited::SetDestRectangle(rect);
		EnsureDrawer();
		if(Drawer)
		{
			if(!Drawer->SetDestSize(rect.get_width(), rect.get_height()))
				DestroyDrawer(); // �G���[���N�������̂ł��� drawer ��j������
		}
	}
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void TJS_INTF_METHOD tTVPPassThroughDrawDevice::NotifyLayerResize(iTVPLayerManager * manager)
{
	inherited::NotifyLayerResize(manager);
	SrcSizeChanged = true;
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void TJS_INTF_METHOD tTVPPassThroughDrawDevice::Show()
{
	if(Drawer) Drawer->Show();
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void TJS_INTF_METHOD tTVPPassThroughDrawDevice::StartBitmapCompletion(iTVPLayerManager * manager)
{
	EnsureDrawer();

	// ���̒��� DestroyDrawer ���Ă΂��\���ɒ��ӂ��邱��
	if(Drawer) Drawer->StartBitmapCompletion();

	if(!Drawer)
	{
		// ���g���C����
		EnsureDrawer();
		if(Drawer) Drawer->StartBitmapCompletion();
	}
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void TJS_INTF_METHOD tTVPPassThroughDrawDevice::NotifyBitmapCompleted(iTVPLayerManager * manager,
	tjs_int x, tjs_int y, const void * bits, const BITMAPINFO * bitmapinfo,
	const tTVPRect &cliprect, tTVPLayerType type, tjs_int opacity)
{
	// bits, bitmapinfo �ŕ\�����r�b�g�}�b�v�� cliprect �̗̈���Ax, y �ɕ`��
	// ����B
	// opacity �� type �͖������邵���Ȃ��̂Ŗ�������
	if(Drawer)
	{
		TVPInitPassThroughOptions();
		Drawer->NotifyBitmapCompleted(x, y, bits, bitmapinfo, cliprect);
	}
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void TJS_INTF_METHOD tTVPPassThroughDrawDevice::EndBitmapCompletion(iTVPLayerManager * manager)
{
	if(Drawer) Drawer->EndBitmapCompletion();
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void TJS_INTF_METHOD tTVPPassThroughDrawDevice::SetShowUpdateRect(bool b)
{
	if(Drawer) Drawer->SetShowUpdateRect(b);
}
//---------------------------------------------------------------------------








//---------------------------------------------------------------------------
// tTJSNC_PassThroughDrawDevice : PassThroughDrawDevice TJS native class
//---------------------------------------------------------------------------
tjs_uint32 tTJSNC_PassThroughDrawDevice::ClassID = (tjs_uint32)-1;
tTJSNC_PassThroughDrawDevice::tTJSNC_PassThroughDrawDevice() :
	tTJSNativeClass(TJS_W("PassThroughDrawDevice"))
{
	// register native methods/properties

	TJS_BEGIN_NATIVE_MEMBERS(PassThroughDrawDevice)
	TJS_DECL_EMPTY_FINALIZE_METHOD
//----------------------------------------------------------------------
// constructor/methods
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_CONSTRUCTOR_DECL(/*var.name*/_this, /*var.type*/tTJSNI_PassThroughDrawDevice,
	/*TJS class name*/PassThroughDrawDevice)
{
	return TJS_S_OK;
}
TJS_END_NATIVE_CONSTRUCTOR_DECL(/*TJS class name*/PassThroughDrawDevice)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/recreate)
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_PassThroughDrawDevice);
	_this->GetDevice()->SetToRecreateDrawer();
	_this->GetDevice()->EnsureDrawer();
	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/recreate)
//----------------------------------------------------------------------


//---------------------------------------------------------------------------
//----------------------------------------------------------------------
// properties
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(interface)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_PassThroughDrawDevice);
		*result = reinterpret_cast<tjs_int64>(_this->GetDevice());
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_DENY_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_PROP_DECL(interface)
//----------------------------------------------------------------------
#define TVP_REGISTER_PTDD_ENUM(name) \
	TJS_BEGIN_NATIVE_PROP_DECL(name) \
	{ \
		TJS_BEGIN_NATIVE_PROP_GETTER \
		{ \
			*result = (tjs_int64)tTVPPassThroughDrawDevice::name; \
			return TJS_S_OK; \
		} \
		TJS_END_NATIVE_PROP_GETTER \
		TJS_DENY_NATIVE_PROP_SETTER \
	} \
	TJS_END_NATIVE_PROP_DECL(name)

TVP_REGISTER_PTDD_ENUM(dtNone)
TVP_REGISTER_PTDD_ENUM(dtDrawDib)
TVP_REGISTER_PTDD_ENUM(dtDBGDI)
//TVP_REGISTER_PTDD_ENUM(dtDBDD)
TVP_REGISTER_PTDD_ENUM(dtDBD3D)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(preferredDrawer)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_PassThroughDrawDevice);
		*result = (tjs_int64)(_this->GetDevice()->GetPreferredDrawerType());
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_BEGIN_NATIVE_PROP_SETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_PassThroughDrawDevice);
		_this->GetDevice()->SetPreferredDrawerType((tTVPPassThroughDrawDevice::tDrawerType)(tjs_int)*param);
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_PROP_DECL(preferredDrawer)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(drawer)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_PassThroughDrawDevice);
		*result = (tjs_int64)(_this->GetDevice()->GetDrawerType());
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_DENY_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_PROP_DECL(drawer)
//----------------------------------------------------------------------
	TJS_END_NATIVE_MEMBERS
}
//---------------------------------------------------------------------------
iTJSNativeInstance *tTJSNC_PassThroughDrawDevice::CreateNativeInstance()
{
	return new tTJSNI_PassThroughDrawDevice();
}
//---------------------------------------------------------------------------










//---------------------------------------------------------------------------
tTJSNI_PassThroughDrawDevice::tTJSNI_PassThroughDrawDevice()
{
	Device = new tTVPPassThroughDrawDevice();
}
//---------------------------------------------------------------------------
tTJSNI_PassThroughDrawDevice::~tTJSNI_PassThroughDrawDevice()
{
	if(Device) Device->Destruct(), Device = NULL;
}
//---------------------------------------------------------------------------
tjs_error TJS_INTF_METHOD
	tTJSNI_PassThroughDrawDevice::Construct(tjs_int numparams, tTJSVariant **param,
		iTJSDispatch2 *tjs_obj)
{
	return TJS_S_OK;
}
//---------------------------------------------------------------------------
void TJS_INTF_METHOD tTJSNI_PassThroughDrawDevice::Invalidate()
{
	if(Device) Device->Destruct(), Device = NULL;
}
//---------------------------------------------------------------------------

