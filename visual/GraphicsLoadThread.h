


struct ImageLoadCommand {
	iTJSDispatch2*		owner_;	// send to event
	tTJSNI_Bitmap*		bmp_;	// set bitmap image
	ttstr				path_;
//	tTJSBinaryStream*	stream_;
	tjs_uint32			colorkey_;
	tTVPBaseBitmap*		dest_;
	ttstr				result_;
};
// BaseBitmap ���g���ƃ��G���g�����g�ł͂Ȃ��̂ŁA�ʂ̍\���̂ɓƎ��Ƀ��[�h����K�v������
struct TmpBitmapImage {
	tjs_uint32 w;
	tjs_uint32 h;
	tjs_uint32 bpp;
	tjs_uint32 imglen;
	tjs_uint32 pallen;
	tjs_uint32* img;
	tjs_uint32* pal;
	// �p���b�g���[�h�����邩��
}
typedef void (*tTVPGraphicSizeCallback)(void *callbackdata, tjs_uint w, tjs_uint h);
typedef void * (*tTVPGraphicScanLineCallback)(void *callbackdata, tjs_int y);
typedef void (*tTVPMetaInfoPushCallback)(void *callbackdata, const ttstr & name, const ttstr & value);

static void TVPLoadGraphicAsync_SizeCallback(void *callbackdata, tjs_uint w, tjs_uint h)
{
	TmpBitmapImage* img = (TmpBitmapImage*)callbackdata;
	img->h = h;
	img->w = w;
}

// onLoaded( dic, is_async, is_error, error_mes ); �G���[��
// sync ( main thead )
void loadRequest( iTJSDispatch2 *owner, tTJSNI_Bitmap* bmp, const ttstr &name, tjs_uint32 colorkey ) {
	tTVPBaseBitmap* dest;
	iTJSDispatch2* metainfo = NULL;
	ttstr nname = TVPNormalizeStorageName(name);
	if( TVPCheckImageCache(nname,dest,glmNormal,0,0,colorkey,metainfo) ) {
		// �L���b�V�����ɔ����A�����ɓǍ��݂���������
		bmp->CopyFrom( dest );
		dest->Release();
		bmp->SetLoading( false );

		tTJSVariant param[4];
		param[0] = tTJSVariant(metainfo,metainfo);
		param[1] = 0; // false
		param[2] = 0; // false
		param[3] = TJS_W(""); // error_mes
		static ttstr eventname(TJS_W("onLoaded"));
		TVPPostEvent(owner, owner, eventname, 0, TVP_EPT_IMMEDIATE, 4, param);
		return;
	}
	if( TVPIsExistentStorage(name) == false ) {
		TVPThrowExceptionMessage(TVPCannotFindStorage, name);
	}

	PushLoadQueue( owner, bmp, nname, colorkey );
}

void PushLoadQueue( iTJSDispatch2 *owner, tTJSNI_Bitmap *bmp, const ttstr &nname, tjs_uint32 colorkey ) {
		// tTJSCriticalSectionHolder cs_holder(TVPCreateStreamCS);
//	tTJSBinaryStream* stream = TVPCreateStream(nname, TJS_BS_READ);
	// TVPCreateStream �̓��b�N����Ă���̂ŁA�񓯊��Ŏ��s�\
	ImageLoadCommand* cmd = new ImageLoadCommand();
	cmd->owner_ = owner;
	owner->AddRef();
	cmd->bmp_ = bmp;
	cmd->path_ = nname;
	cmd->colorkey_ = colorkey;
	cmd->dest_ = NULL; // �{�̂Ɉˑ����Ȃ����߂ɕʂ̏�񂾂����������̂ɒu��������ׂ���
}
