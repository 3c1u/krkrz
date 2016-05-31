
#ifndef __TVP_VIDEO_OVERLAY_H__
#define __TVP_VIDEO_OVERLAY_H__

extern void __stdcall GetVideoOverlayObject(
	HWND callbackwin, struct IStream *stream, const wchar_t * streamname,
	const wchar_t *type, unsigned __int64 size, class iTVPVideoOverlay **out);

extern void __stdcall GetVideoLayerObject(
	HWND callbackwin, struct IStream *stream, const wchar_t * streamname,
	const wchar_t *type, unsigned __int64 size, class iTVPVideoOverlay **out);

extern void __stdcall GetMixingVideoOverlayObject(
	HWND callbackwin, struct IStream *stream, const wchar_t * streamname,
	const wchar_t *type, unsigned __int64 size, class iTVPVideoOverlay **out);

extern void __stdcall GetMFVideoOverlayObject(
	HWND callbackwin, struct IStream *stream, const wchar_t * streamname,
	const wchar_t *type, unsigned __int64 size, class iTVPVideoOverlay **out);


/*[*/
typedef void* (*tTVPCreateDSFilter)( void* formatdata );
/*]*/

TJS_EXP_FUNC_DEF( void, TVPRegisterDSVideoCodec, ( const ttstr & name, void* guid, tTVPCreateDSFilter splitter,
	tTVPCreateDSFilter video, tTVPCreateDSFilter audio, void* formatdata ) );
TJS_EXP_FUNC_DEF( void, TVPUnregisterDSVideoCodec, (const ttstr & name, void* guid, tTVPCreateDSFilter splitter,
	tTVPCreateDSFilter video, tTVPCreateDSFilter audio, void* formatdata ) );

// ��C�ɓo�^����`�ɂȂ��Ă��邯�ǁA�g���q�Ƃ��̃t�H�[�}�b�g��GUID
// ���W���[�^�C�v�ƃT�u�^�C�v��GUID�ƃt�B���^�̃y�A��o�^����悤�ɂȂ��Ă�������ėp�I����
// �ł����������DShow�̂悤�ɂ��ׂ����o�^�ł�������������Ă��ƂɂȂ��Ă܂���ς�

struct tTVPDSFilterHandlerType
{
	ttstr Extension;
	GUID* Guid;
	tTVPCreateDSFilter SplitterHander;
	tTVPCreateDSFilter VideoHander;
	tTVPCreateDSFilter AudioHander;
	void * FormatData;

	tTVPDSFilterHandlerType(const ttstr &ext,
		void* guid,
		tTVPCreateDSFilter splitter,
		tTVPCreateDSFilter video,
		tTVPCreateDSFilter audio,
		void * data) : Extension(ext), Guid((GUID*)guid),
		SplitterHander(splitter), VideoHander(video), AudioHander(audio), FormatData(data)
	{
	}

	tTVPDSFilterHandlerType(const tTVPDSFilterHandlerType & ref)
	{
		Guid = ref.Guid;
		SplitterHander = ref.SplitterHander;
		VideoHander = ref.VideoHander;
		AudioHander = ref.AudioHander;
		Extension = ref.Extension;
		FormatData = ref.FormatData;
	}

	bool operator == (const tTVPDSFilterHandlerType & ref) const
	{
		return Guid == ref.Guid &&
				SplitterHander == ref.SplitterHander &&
				VideoHander == ref.VideoHander &&
				AudioHander == ref.AudioHander &&
				Extension == ref.Extension &&
				FormatData == ref.FormatData;
	}
};

extern tTVPDSFilterHandlerType* TVPGetDSFilterHandler( const ttstr& ext );
extern tTVPDSFilterHandlerType* TVPGetDSFilterHandler( const GUID& guid );

#endif // __TVP_VIDEO_OVERLAY_H__
