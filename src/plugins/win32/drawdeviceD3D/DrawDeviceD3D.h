#ifndef DrawDeviceD3D_H
#define DrawDeviceD3D_H

#include <windows.h>
#include <stdio.h>
#include <ddraw.h>
#include <d3d9.h>
#include "BasicDrawDevice.h"

/**
 * D3D �x�[�X�� DrawDevice
 */
class DrawDeviceD3D : public tTVPDrawDevice
{
	typedef tTVPDrawDevice inherited;

protected:
	tjs_int width;        //< ���[�U�w��̉�ʉ���
	tjs_int height;       //< ���[�U�w��̉�ʏc��
	tjs_int destTop;      //< ���`��̈�̉���
	tjs_int destLeft;     //< ���`��̈�̏c��
	tjs_int destWidth;    //< ���`��̈�̉���
	tjs_int destHeight;   //< ���`��̈�̏c��
	
	HWND targetWindow;
	bool isMainWindow;
	bool drawUpdateRectangle;
	bool backBufferDirty;

	HWND hWnd;
	
	IDirect3D9*				direct3D;
	IDirect3DDevice9*		direct3DDevice;
	D3DPRESENT_PARAMETERS	d3dPP;
	D3DDISPLAYMODE			dispMode;
	
	UINT		currentMonitor;
	bool		shouldShow; //!< show �Ŏ��ۂɉ�ʂɉ摜��]�����ׂ���
	tjs_uint	vsyncInterval;
public:
	// �R���X�g���N�^
	DrawDeviceD3D(int width, int height);
	// �f�X�g���N�^
	virtual ~DrawDeviceD3D();

protected:
	
	void InvalidateAll();
	UINT GetMonitorNumber( HWND window );

	bool IsTargetWindowActive() const;

	HRESULT DecideD3DPresentParameters();

	void DestroyD3DDevice();
	void TryRecreateWhenDeviceLost();

	// �f�o�C�X���蓖��
	void attach(HWND hwnd);

	// �f�o�C�X�������
	void detach();
	
	/**
	 * Device�����C���}�l�[�W���̍��W�̕ϊ����s��
	 * @param		x		X�ʒu
	 * @param		y		Y�ʒu
	 * @note		x, y �� DestRect�� (0,0) �����_�Ƃ�����W�Ƃ��ēn�����ƌ��Ȃ�
	 */
	void transformToManager(iTVPLayerManager * manager, tjs_int &x, tjs_int &y);

	/** ���C���}�l�[�W����Device�����̍��W�̕ϊ����s��
	 * @param		x		X�ʒu
	 * @param		y		Y�ʒu
	 * @note		x, y �� ���C���� (0,0) �����_�Ƃ�����W�Ƃ��ēn�����ƌ��Ȃ�
	 */
	void transformFromManager(iTVPLayerManager * manager, tjs_int &x, tjs_int &y);

	/**
	 * Device���W�����W�̕ϊ����s��
	 * @param		x		X�ʒu
	 * @param		y		Y�ʒu
	 * @note		x, y �� DestRect�� (0,0) �����_�Ƃ�����W�Ƃ��ēn�����ƌ��Ȃ�
	 */
	void transformTo(tjs_int &x, tjs_int &y);
	
	/** �W�����W��Device�����̍��W�̕ϊ����s��
	 * @param		x		X�ʒu
	 * @param		y		Y�ʒu
	 * @note		x, y �� ���C���� (0,0) �����_�Ƃ�����W�Ƃ��ēn�����ƌ��Ȃ�
	 */
	void transformFrom(tjs_int &x, tjs_int &y);
	
public:
	//---- LayerManager �̊Ǘ��֘A
	virtual void TJS_INTF_METHOD AddLayerManager(iTVPLayerManager * manager);
	virtual void TJS_INTF_METHOD RemoveLayerManager(iTVPLayerManager * manager);

	//---- �`��ʒu�E�T�C�Y�֘A
	virtual void TJS_INTF_METHOD SetTargetWindow(HWND wnd, bool is_main);
	virtual void TJS_INTF_METHOD SetDestRectangle(const tTVPRect & rect);
	virtual void TJS_INTF_METHOD GetSrcSize(tjs_int &w, tjs_int &h);
	virtual void TJS_INTF_METHOD NotifyLayerResize(iTVPLayerManager * manager);
	virtual void TJS_INTF_METHOD NotifyLayerImageChange(iTVPLayerManager * manager);

	//---- ���[�U�[�C���^�[�t�F�[�X�֘A
	// window �� drawdevice
	virtual void TJS_INTF_METHOD OnMouseDown(tjs_int x, tjs_int y, tTVPMouseButton mb, tjs_uint32 flags);
	virtual void TJS_INTF_METHOD OnMouseUp(tjs_int x, tjs_int y, tTVPMouseButton mb, tjs_uint32 flags);
	virtual void TJS_INTF_METHOD OnMouseMove(tjs_int x, tjs_int y, tjs_uint32 flags);
	virtual void TJS_INTF_METHOD OnMouseWheel(tjs_uint32 shift, tjs_int delta, tjs_int x, tjs_int y);

	virtual void TJS_INTF_METHOD GetCursorPos(iTVPLayerManager * manager, tjs_int &x, tjs_int &y);
	virtual void TJS_INTF_METHOD SetCursorPos(iTVPLayerManager * manager, tjs_int x, tjs_int y);
	virtual void TJS_INTF_METHOD RequestInvalidation(const tTVPRect & rect);
	
	//---- �ĕ`��֘A
	virtual void TJS_INTF_METHOD Show();
	virtual bool TJS_INTF_METHOD WaitForVBlank( tjs_int* in_vblank, tjs_int* delayed );
	
	//---- LayerManager ����̉摜�󂯓n���֘A
	virtual void TJS_INTF_METHOD StartBitmapCompletion(iTVPLayerManager * manager);
	virtual void TJS_INTF_METHOD NotifyBitmapCompleted(iTVPLayerManager * manager,
		tjs_int x, tjs_int y, const void * bits, const BITMAPINFO * bitmapinfo,
		const tTVPRect &cliprect, tTVPLayerType type, tjs_int opacity);
	virtual void TJS_INTF_METHOD EndBitmapCompletion(iTVPLayerManager * manager);
	
//---- �t���X�N���[��
	virtual bool TJS_INTF_METHOD SwitchToFullScreen( HWND window, tjs_uint w, tjs_uint h, tjs_uint bpp, tjs_uint color, bool changeresolution );
	virtual void TJS_INTF_METHOD RevertFromFullScreen( HWND window, tjs_uint w, tjs_uint h, tjs_uint bpp, tjs_uint color );
	// -----------------------------------------------------------------------
	// �ŗL���\�b�h
	// -----------------------------------------------------------------------
	
public:
	/**
	 * @return �f�o�C�X���
	 */
	tjs_int64 getDevice() {
		return reinterpret_cast<tjs_int64>((tTVPDrawDevice*)this);
	}

	tjs_int getWidth() {
		return width;
	}
	
	void setWidth(tjs_int width) {
		this->width = width;
		Window->NotifySrcResize();
	}

	tjs_int getHeight() {
		return height;
	}
	
	void setHeight(tjs_int height) {
		this->height = height;
		Window->NotifySrcResize();
	}

	void setSize(tjs_int width, tjs_int height) {
		this->width = width;
		this->height = height;
		Window->NotifySrcResize();
	}

	tjs_int getDestWidth() {
		return destWidth;
	}

	tjs_int getDestHeight() {
		return destHeight;
	}
	
protected:
	/*
	 * �v���C�}�����C���̕W���� visible
	 */
	bool defaultVisible;

public:
	void setDefaultVisible(bool visible) {
		defaultVisible = visible;
	}
	
	bool getDefaultVisible() {
		return defaultVisible;
	}

	/**
	 * �v���C�}�����C���̕\����Ԃ̎w��
	 * @param id �v���C�}�����C���̓o�^ID
	 * @param visible �\�����
	 */
	void setVisible(int id, bool visible);

	/**
	 * �v���C�}�����C���̕\����Ԃ̎w��
	 * @param id �v���C�}�����C���̓o�^ID
	 * @return visible �\�����
	 */
	bool getVisible(int id);
	
};

#endif
