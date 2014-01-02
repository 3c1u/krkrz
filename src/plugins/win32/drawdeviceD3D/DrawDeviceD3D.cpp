
#include "DrawDeviceD3D.h"
#include "LayerManagerInfo.h"

/**
 * �R���X�g���N�^
 */
DrawDeviceD3D::DrawDeviceD3D(int width, int height)
	: width(width), height(height), destWidth(0), destHeight(0), defaultVisible(true),
	  direct3D(NULL), direct3DDevice(NULL)
{
	targetWindow = NULL;
	drawUpdateRectangle = false;
	backBufferDirty = true;

	direct3D = NULL;
	direct3DDevice = NULL;
	shouldShow = false;
	vsyncInterval = 16;
	ZeroMemory( &d3dPP, sizeof(d3dPP) );
	ZeroMemory( &dispMode, sizeof(dispMode) );

	TVPEnsureDirect3DObject();
}

/**
 * �f�X�g���N�^
 */
DrawDeviceD3D::~DrawDeviceD3D()
{
	detach();
}
//---------------------------------------------------------------------------
void DrawDeviceD3D::DestroyD3DDevice() {
	if(direct3DDevice) direct3DDevice->Release(), direct3DDevice = NULL;
	if(direct3D) direct3D = NULL;
}
//---------------------------------------------------------------------------
bool DrawDeviceD3D::IsTargetWindowActive() const {
	if( targetWindow == NULL ) return false;
	return ::GetForegroundWindow() == targetWindow;
}
//---------------------------------------------------------------------------
UINT DrawDeviceD3D::GetMonitorNumber( HWND window )
{
	if( direct3D == NULL || window == NULL ) return D3DADAPTER_DEFAULT;
	HMONITOR windowMonitor = ::MonitorFromWindow( window, MONITOR_DEFAULTTOPRIMARY );
	UINT iCurrentMonitor = 0;
	UINT numOfMonitor = direct3D->GetAdapterCount();
	for( ; iCurrentMonitor < numOfMonitor; ++iCurrentMonitor ) 	{
		if( direct3D->GetAdapterMonitor(iCurrentMonitor) == windowMonitor )
			break;
	}
	if( iCurrentMonitor == numOfMonitor )
		iCurrentMonitor = D3DADAPTER_DEFAULT;
	return iCurrentMonitor;
}
//---------------------------------------------------------------------------
HRESULT DrawDeviceD3D::DecideD3DPresentParameters() {
	HRESULT			hr;
	UINT iCurrentMonitor = GetMonitorNumber(targetWindow);
	if( FAILED( hr = direct3D->GetAdapterDisplayMode( iCurrentMonitor, &dispMode ) ) )
		return hr;

	ZeroMemory( &d3dPP, sizeof(d3dPP) );
	d3dPP.Windowed = TRUE;
	d3dPP.SwapEffect = D3DSWAPEFFECT_COPY;
	d3dPP.BackBufferFormat = D3DFMT_UNKNOWN;
	d3dPP.BackBufferHeight = dispMode.Height;
	d3dPP.BackBufferWidth = dispMode.Width;
	d3dPP.hDeviceWindow = targetWindow;
	d3dPP.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

	return S_OK;
}
//---------------------------------------------------------------------------
void DrawDeviceD3D::TryRecreateWhenDeviceLost()
{
	bool success = false;
	if( direct3DDevice ) {
		for (std::vector<iTVPLayerManager *>::iterator i = Managers.begin(); i != Managers.end(); i++) {
			LayerManagerInfo *info = (LayerManagerInfo*)(*i)->GetDrawDeviceData();
			if (info) {
				info->free();
			}
		}
		HRESULT hr = direct3DDevice->TestCooperativeLevel();
		if( hr == D3DERR_DEVICENOTRESET ) {
			hr = direct3DDevice->Reset(&d3dPP);
		}
		if( FAILED(hr) ) {
			attach( targetWindow );
			success = direct3DDevice != NULL;
		}
	} else {
		attach( targetWindow );
		success = direct3DDevice != NULL;
	}
	if( success ) {
		if (direct3DDevice) {
			for (std::vector<iTVPLayerManager *>::iterator i = Managers.begin(); i != Managers.end(); i++) {
				iTVPLayerManager *manager = *i;
				LayerManagerInfo *info = (LayerManagerInfo*)manager->GetDrawDeviceData();
				if (info != NULL) {
					info->alloc( manager, direct3DDevice );
				}
			}
		}
		InvalidateAll();	// �摜�̍ĕ`��(Layer Update)��v������
	}
}
//---------------------------------------------------------------------------
void DrawDeviceD3D::InvalidateAll()
{
	// ���C�����Z���ʂ����ׂă��N�G�X�g����
	// �T�[�t�F�[�X�� lost �����ۂɓ��e���č\�z����ړI�ŗp����
	RequestInvalidation(tTVPRect(0, 0, DestRect.get_width(), DestRect.get_height()));
}
/**
 * �E�C���h�E�̉���
 */
void
DrawDeviceD3D::attach(HWND hWnd)
{
	DestroyD3DDevice();
	this->hWnd = hWnd;
	try {
		// Direct3D �f�o�C�X�A�e�N�X�`���Ȃǂ��쐬����
		HRESULT hr;
		// get Direct3D9 interface
		if( NULL == ( direct3D = TVPGetDirect3DObjectNoAddRef() ) )
			TVPThrowExceptionMessage( TJS_W("Direct3D9 not available") );
		// direct3D->AddRef();

		if( FAILED( hr = DecideD3DPresentParameters() ) ) {
			if( IsTargetWindowActive() ) {
				TVPThrowExceptionMessage( TJS_W("Faild to decide backbuffer format.") );
			}
		}

		UINT iCurrentMonitor = GetMonitorNumber( targetWindow );
		DWORD	BehaviorFlags = D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_MULTITHREADED;
		if( D3D_OK != ( hr = direct3D->CreateDevice( iCurrentMonitor, D3DDEVTYPE_HAL, targetWindow, BehaviorFlags, &d3dPP, &direct3DDevice ) ) ) {
			if( IsTargetWindowActive() ) {
				TVPThrowExceptionMessage( TJS_W("Faild to create Direct3D9 Device.") );
			}
		}
		currentMonitor = iCurrentMonitor;
		backBufferDirty = true;

		D3DVIEWPORT9 vp;
		vp.X  = 0;
		vp.Y  = 0;
		vp.Width = d3dPP.BackBufferWidth;
		vp.Height = d3dPP.BackBufferHeight;
		vp.MinZ  = 0.0f;
		vp.MaxZ  = 1.0f;
		if( FAILED(hr = direct3DDevice->SetViewport(&vp)) ) {
			if( IsTargetWindowActive() ) {
				TVPThrowExceptionMessage( TJS_W("Faild to set viewport.") );
			}
		}

		int refreshrate = dispMode.RefreshRate;
		if( refreshrate == 0 ) {
			HDC hdc;
			hdc = ::GetDC(targetWindow);
			refreshrate = GetDeviceCaps( hdc, VREFRESH );
			::ReleaseDC( targetWindow, hdc );
		}
		vsyncInterval = 1000 / refreshrate;
	} catch(...) {
		TVPAddImportantLog(TJS_W("D3DDrawDevice: Failed to create Direct3D devices: unknown reason"));
	}
}


/**
 * �E�C���h�E�̉���
 */
void
DrawDeviceD3D::detach()
{
	for (std::vector<iTVPLayerManager *>::iterator i = Managers.begin(); i != Managers.end(); i++) {
		LayerManagerInfo *info = (LayerManagerInfo*)(*i)->GetDrawDeviceData();
		if (info) {
			info->free();
		}
	}
	// �������
	DestroyD3DDevice();
}

/**
 * Device���v���C�}�����C���̍��W�̕ϊ����s��
 * @param		x		X�ʒu
 * @param		y		Y�ʒu
 * @note		x, y �� DestRect�� (0,0) �����_�Ƃ�����W�Ƃ��ēn�����ƌ��Ȃ�
 */
void
DrawDeviceD3D::transformToManager(iTVPLayerManager * manager, tjs_int &x, tjs_int &y)
{
	// �v���C�}�����C���}�l�[�W���̃v���C�}�����C���̃T�C�Y�𓾂�
	tjs_int pl_w, pl_h;
	manager->GetPrimaryLayerSize(pl_w, pl_h);
	x = destWidth  ? (x * pl_w / destWidth) : 0;
	y = destHeight ? (y * pl_h / destHeight) : 0;
}

/** �v���C�}�����C����Device�����̍��W�̕ϊ����s��
 * @param		x		X�ʒu
 * @param		y		Y�ʒu
 * @note		x, y �� ���C���� (0,0) �����_�Ƃ�����W�Ƃ��ēn�����ƌ��Ȃ�
 */
void
DrawDeviceD3D::transformFromManager(iTVPLayerManager * manager, tjs_int &x, tjs_int &y)
{
	// �v���C�}�����C���}�l�[�W���̃v���C�}�����C���̃T�C�Y�𓾂�
	tjs_int pl_w, pl_h;
	manager->GetPrimaryLayerSize(pl_w, pl_h);
	x = pl_w ? (x * destWidth  / pl_w) : 0;
	y = pl_h ? (y * destHeight / pl_h) : 0;
}

/**
 * Device���W����ʂ̍��W�̕ϊ����s��
 * @param		x		X�ʒu
 * @param		y		Y�ʒu
 * @note		x, y �� DestRect�� (0,0) �����_�Ƃ�����W�Ƃ��ēn�����ƌ��Ȃ�
 */
void
DrawDeviceD3D::transformTo(tjs_int &x, tjs_int &y)
{
	x = destWidth  ? (x * width / destWidth) : 0;
	y = destHeight ? (y * height / destHeight) : 0;
}

/** �W����ʁ�Device�����̍��W�̕ϊ����s��
 * @param		x		X�ʒu
 * @param		y		Y�ʒu
 * @note		x, y �� ���C���� (0,0) �����_�Ƃ�����W�Ƃ��ēn�����ƌ��Ȃ�
 */
void
DrawDeviceD3D::transformFrom(tjs_int &x, tjs_int &y)
{
	// �v���C�}�����C���}�l�[�W���̃v���C�}�����C���̃T�C�Y�𓾂�
	x = width ? (x * destWidth  / width) : 0;
	y = height ? (y * destHeight / height) : 0;
}

/**
 * ���C���}�l�[�W���̓o�^
 * @param manager ���C���}�l�[�W��
 */
void TJS_INTF_METHOD
DrawDeviceD3D::AddLayerManager(iTVPLayerManager * manager)
{
	int id = (int)Managers.size();
	tTVPDrawDevice::AddLayerManager(manager);
	LayerManagerInfo *info = new LayerManagerInfo(id, defaultVisible);
	manager->SetDrawDeviceData((void*)info);
	//manager->SetDesiredLayerType(ltAlpha); // ltAlpha �ȏo�͂��󂯎�肽��
}

/**
 * ���C���}�l�[�W���̍폜
 * @param manager ���C���}�l�[�W��
 */
void TJS_INTF_METHOD
DrawDeviceD3D::RemoveLayerManager(iTVPLayerManager * manager)
{
	LayerManagerInfo *info = (LayerManagerInfo*)manager->GetDrawDeviceData();
	if (info != NULL) {
		manager->SetDrawDeviceData(NULL);
		delete info;
	}
	tTVPDrawDevice::RemoveLayerManager(manager);
}

/***
 * �E�C���h�E�̎w��
 * @param wnd �E�C���h�E�n���h��
 */
void TJS_INTF_METHOD
DrawDeviceD3D::SetTargetWindow(HWND wnd, bool is_main)
{
	detach();
	targetWindow = wnd;
	isMainWindow = is_main;
	if (wnd != NULL) {
		attach(wnd);
		Window->NotifySrcResize(); // ������ĂԂ��Ƃ� GetSrcSize(), SetDestRectangle() �̌ĂѕԂ�������
		// �}�l�[�W���ɑ΂���e�N�X�`���̊��蓖��
		if (direct3DDevice) {
			for (std::vector<iTVPLayerManager *>::iterator i = Managers.begin(); i != Managers.end(); i++) {
				iTVPLayerManager *manager = *i;
				LayerManagerInfo *info = (LayerManagerInfo*)manager->GetDrawDeviceData();
				if (info != NULL) {
					info->alloc( manager, direct3DDevice );
				}
			}
		}
	}
}

void TJS_INTF_METHOD
DrawDeviceD3D::SetDestRectangle(const tTVPRect &dest)
{
	destLeft = dest.Left;
	destTop  = dest.Top;
	destWidth = dest.get_width();
	destHeight = dest.get_height();
	
	backBufferDirty = true;
	// �ʒu�����̕ύX�̏ꍇ���ǂ������`�F�b�N����
	if(dest.get_width() == DestRect.get_width() && dest.get_height() == DestRect.get_height()) {
		// �ʒu�����̕ύX��
		inherited::SetDestRectangle(dest);
	} else {
		// �T�C�Y���Ⴄ
		inherited::SetDestRectangle(dest);
	}
}

void TJS_INTF_METHOD
DrawDeviceD3D::GetSrcSize(tjs_int &w, tjs_int &h)
{
	w = width;
	h = height;
}

void TJS_INTF_METHOD
DrawDeviceD3D::NotifyLayerResize(iTVPLayerManager * manager)
{
	LayerManagerInfo *info = (LayerManagerInfo*)manager->GetDrawDeviceData();
	if (info != NULL) {
		info->free();
		if (direct3DDevice) {
			info->alloc(manager, direct3DDevice);
		}
	}
}

void TJS_INTF_METHOD
DrawDeviceD3D::NotifyLayerImageChange(iTVPLayerManager * manager)
{
	Window->RequestUpdate();
}

// -------------------------------------------------------------------------------------
// ���̓C�x���g�����p
// -------------------------------------------------------------------------------------

void TJS_INTF_METHOD
DrawDeviceD3D::OnMouseDown(tjs_int x, tjs_int y, tTVPMouseButton mb, tjs_uint32 flags)
{
	// �g���g���̃v���C�}�����C���ɑ���
	iTVPLayerManager * manager = GetLayerManagerAt(PrimaryLayerManagerIndex);
	if (manager) {
		transformToManager(manager, x, y);
		manager->NotifyMouseDown(x, y, mb, flags);
	}
}

void TJS_INTF_METHOD
DrawDeviceD3D::OnMouseUp(tjs_int x, tjs_int y, tTVPMouseButton mb, tjs_uint32 flags)
{
	// �g���g���̃v���C�}�����C���ɑ���
	iTVPLayerManager * manager = GetLayerManagerAt(PrimaryLayerManagerIndex);
	if (manager) {
		transformToManager(manager, x, y);
		manager->NotifyMouseUp(x, y, mb, flags);
	}
}

void TJS_INTF_METHOD
DrawDeviceD3D::OnMouseMove(tjs_int x, tjs_int y, tjs_uint32 flags)
{
	// �g���g���̃v���C�}�����C���ɑ���
	iTVPLayerManager * manager = GetLayerManagerAt(PrimaryLayerManagerIndex);
	if (manager) {
		transformToManager(manager, x, y);
		manager->NotifyMouseMove(x, y, flags);
	}
}

void TJS_INTF_METHOD
DrawDeviceD3D::OnMouseWheel(tjs_uint32 shift, tjs_int delta, tjs_int x, tjs_int y)
{
	// �g���g���̃v���C�}�����C���ɑ���
	iTVPLayerManager * manager = GetLayerManagerAt(PrimaryLayerManagerIndex);
	if (manager) {
		transformToManager(manager, x, y);
		manager->NotifyMouseWheel(shift, delta, x, y);
	}
}

void TJS_INTF_METHOD
DrawDeviceD3D::GetCursorPos(iTVPLayerManager * manager, tjs_int &x, tjs_int &y)
{
	Window->GetCursorPos(x, y);
	transformToManager(manager, x, y);
}

void TJS_INTF_METHOD
DrawDeviceD3D::SetCursorPos(iTVPLayerManager * manager, tjs_int x, tjs_int y)
{
	transformFromManager(manager, x, y);
	Window->SetCursorPos(x, y);
}

void TJS_INTF_METHOD
DrawDeviceD3D::RequestInvalidation(const tTVPRect & rect)
{
	for (std::vector<iTVPLayerManager *>::iterator i = Managers.begin(); i != Managers.end(); i++) {
		iTVPLayerManager *manager = *i;
		LayerManagerInfo *info = (LayerManagerInfo*)manager->GetDrawDeviceData();
		if (info && info->visible) {
			tjs_int l = rect.left, t = rect.top, r = rect.right, b = rect.bottom;
			transformToManager(manager, l, t);
			transformToManager(manager, r, b);
			r ++; // �덷�̋z��(�{���͂���������ƌ����ɂ��Ȃ��ƂȂ�Ȃ������ꂪ���ɂȂ邱�Ƃ͂Ȃ�)
			b ++;
			manager->RequestInvalidation(tTVPRect(l, t, r, b));
		}
	}
}


// -------------------------------------------------------------------------------------
// �ĕ`�揈���p
// -------------------------------------------------------------------------------------

void
DrawDeviceD3D::Show()
{
	if(!targetWindow) return;
	if(!direct3DDevice) return;
	if(!shouldShow) return;

	shouldShow = false;

	// ��ʏ���
	direct3DDevice->Clear( 0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0,0,0), 1.0, 0 );
	// �ʃ��C���}�l�[�W���̕`��
	for (std::vector<iTVPLayerManager *>::iterator i = Managers.begin(); i != Managers.end(); i++) {
		LayerManagerInfo *info = (LayerManagerInfo*)(*i)->GetDrawDeviceData();
		if (info) {
			info->draw(direct3DDevice, destWidth, destHeight);
		}
	}

	HRESULT hr = D3D_OK;
	RECT client;
	if( ::GetClientRect( targetWindow, &client ) ) {
		RECT drect;
		drect.left   = 0;
		drect.top    = 0;
		drect.right  = client.right - client.left;
		drect.bottom = client.bottom - client.top;

		RECT srect = drect;
		hr = direct3DDevice->Present( &srect, &drect, targetWindow, NULL );
	} else {
		shouldShow = true;
	}

	if(hr == D3DERR_DEVICELOST) {
		TryRecreateWhenDeviceLost();
	} else if(hr != D3D_OK) {
		TVPAddImportantLog( TVPFormatMessage( TJS_W("D3DDrawDevice: (inf) IDirect3DDevice::Present failed/HR=%1"),TJSInt32ToHex(hr, 8)) );
	}
}
//---------------------------------------------------------------------------
bool TJS_INTF_METHOD DrawDeviceD3D::WaitForVBlank( tjs_int* in_vblank, tjs_int* delayed )
{
	if( direct3DDevice == NULL ) return false;

	bool inVsync = false;
	D3DRASTER_STATUS rs;
	if( D3D_OK == direct3DDevice->GetRasterStatus(0,&rs) ) {
		inVsync = rs.InVBlank == TRUE;
	}

	// VSync �҂����s��
	bool isdelayed = false;
	if(!inVsync) {
		// vblank ���甲����܂ő҂�
		DWORD timeout_target_tick = ::timeGetTime() + 1;
		rs.InVBlank = FALSE;
		HRESULT hr = D3D_OK;
		do {
			hr = direct3DDevice->GetRasterStatus(0,&rs);
		} while( D3D_OK == hr && rs.InVBlank == TRUE && (long)(::timeGetTime() - timeout_target_tick) <= 0);

		// vblank �ɓ���܂ő҂�
		rs.InVBlank = TRUE;
		do {
			hr = direct3DDevice->GetRasterStatus(0,&rs);
		} while( D3D_OK == hr && rs.InVBlank == FALSE && (long)(::timeGetTime() - timeout_target_tick) <= 0);

		if((int)(::timeGetTime() - timeout_target_tick) > 0) {
			// �t���[���X�L�b�v�����������ƍl���Ă悢
			isdelayed  = true;
		}
		inVsync = rs.InVBlank == TRUE;
	}
	*delayed = isdelayed ? 1 : 0;
	*in_vblank = inVsync ? 1 : 0;
	return true;
}
//---------------------------------------------------------------------------
bool TJS_INTF_METHOD DrawDeviceD3D::SwitchToFullScreen( HWND window, tjs_uint w, tjs_uint h, tjs_uint bpp, tjs_uint color, bool changeresolution )
{
	// �t���X�N���[�����̏����͂Ȃɂ��s��Ȃ��A�݊����̂��߂ɃE�B���h�E��S��ʉ�����݂̂ŏ�������
	// Direct3D9 �Ńt���X�N���[��������ƃt�H�[�J�X�������ƃf�o�C�X�����X�g����̂ŁA���̂��тɃ��Z�b�gor��蒼�����K�v�ɂȂ�B
	// ���[�_���E�B���h�E���g�p����V�X�e���ł́A����͍���̂ŏ�ɃE�B���h�E���[�h�ōs���B
	// ���[�_���E�B���h�E���g�p���Ȃ��V�X�e���ɂ���̂Ȃ�A�t���X�N���[�����g�p����DrawDevice�����Ɨǂ��B
	backBufferDirty = true;
	shouldShow = true;
	//CheckMonitorMoved();
	return true;
}
//---------------------------------------------------------------------------
void TJS_INTF_METHOD DrawDeviceD3D::RevertFromFullScreen( HWND window, tjs_uint w, tjs_uint h, tjs_uint bpp, tjs_uint color )
{
	backBufferDirty = true;
	shouldShow = true;
	//CheckMonitorMoved();
}
//---------------------------------------------------------------------------



// -------------------------------------------------------------------------------------
// LayerManager����̉摜�����킽��
// -------------------------------------------------------------------------------------

/**
 * �r�b�g�}�b�v�R�s�[�����J�n
 */
void TJS_INTF_METHOD
DrawDeviceD3D::StartBitmapCompletion(iTVPLayerManager * manager)
{
	LayerManagerInfo *info = (LayerManagerInfo*)manager->GetDrawDeviceData();
	if (info) {
		info->lock();
	}
}

/**
 * �r�b�g�}�b�v�R�s�[����
 */
void TJS_INTF_METHOD
DrawDeviceD3D::NotifyBitmapCompleted(iTVPLayerManager * manager,
	tjs_int x, tjs_int y, const void * bits, const BITMAPINFO * bitmapinfo,
	const tTVPRect &cliprect, tTVPLayerType type, tjs_int opacity)
{
	LayerManagerInfo *info = (LayerManagerInfo*)manager->GetDrawDeviceData();
	if (info) {
		info->copy(x, y, bits, bitmapinfo, cliprect, type, opacity);
	}
	shouldShow = true;
}

/**
 * �r�b�g�}�b�v�R�s�[�����I��
 */
void TJS_INTF_METHOD
DrawDeviceD3D::EndBitmapCompletion(iTVPLayerManager * manager)
{
	LayerManagerInfo *info = (LayerManagerInfo*)manager->GetDrawDeviceData();
	if (info) {
		info->unlock();
	}
}

/**
 * �v���C�}�����C���̕\����Ԃ̎w��
 * @param id �v���C�}�����C���̓o�^ID
 * @param visible �\�����
 */
void
DrawDeviceD3D::setVisible(int id, bool visible)
{
	if (id >= 0 && id < (int)Managers.size()) {
		LayerManagerInfo *info = (LayerManagerInfo*)Managers[id]->GetDrawDeviceData();
		if (info) {
			info->visible = visible;
			Window->RequestUpdate();
		}
	}
}

/**
 * �v���C�}�����C���̕\����Ԃ̎w��
 * @param id �v���C�}�����C���̓o�^ID
 * @return visible �\�����
 */
bool
DrawDeviceD3D::getVisible(int id)
{
	if (id >= 0 && id < (int)Managers.size()) {
		LayerManagerInfo *info = (LayerManagerInfo*)Managers[id]->GetDrawDeviceData();
		if (info) {
			return info->visible;
		}
	}
	return false;
}
