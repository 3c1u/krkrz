#ifndef LAYERMANAGERINFO_H
#define LAYERMANAGERINFO_H

#include <windows.h>
#include <stdio.h>
#include <d3d9.h>

/**
 * ���C���}�l�[�W���p�t�����
 */
class LayerManagerInfo {

protected:
	// ���ʗpID
	int id;

	// �����C���T�C�Y
	int srcWidth;
	int srcHeight;

	// ���蓖�ăe�N�X�`��
	IDirect3DTexture9 *texture;
	tjs_uint textureWidth; //< �e�N�X�`���̉���
	tjs_uint textureHeight; //< �e�N�X�`���̏c��

	void *textureBuffer; //< �e�N�X�`���̃T�[�t�F�[�X�ւ̃������|�C���^
	long texturePitch; //< �e�N�X�`���̃s�b�`

	bool lastOK;     //< �O��̏����͐���������
	
public:
	// �\���Ώۂ��ǂ���
	bool visible;

public:
	/**
	 * �R���X�g���N�^
	 * @param id ���C��ID
	 * @param visible �����\�����
	 */
	LayerManagerInfo(int id, bool visible);
	virtual ~LayerManagerInfo();
	
	/**
	 * �e�N�X�`�����蓖�ď���
	 */
	void alloc(iTVPLayerManager *manager, IDirect3DDevice9 *direct3DDevice);

	/*
	 * �e�N�X�`�����
	 */
	void free();
	
	// �e�N�X�`���`�摀��p
	void lock();
	void copy(tjs_int x, tjs_int y, const void * bits, const BITMAPINFO * bitmapinfo,
			  const tTVPRect &cliprect, tTVPLayerType type, tjs_int opacity);
	void unlock();

	/**
	 * �`��
	 */
	void draw(IDirect3DDevice9 *direct3DDevice9, const tTVPRect &destrect, const tTVPRect &cliprect);
};

#endif
