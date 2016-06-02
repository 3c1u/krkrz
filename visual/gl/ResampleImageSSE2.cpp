

#define _USE_MATH_DEFINES

#include "tjsCommHead.h"
#include "LayerBitmapIntf.h"
#include "LayerBitmapImpl.h"
#include "ThreadIntf.h"

#include <float.h>
#include <math.h>
#include <cmath>
#include <vector>

#include <xmmintrin.h> // SSE
#include <emmintrin.h> // SSE2

#include "x86simdutil.h"
#include "aligned_allocator.h"

#include "WeightFunctorSSE.h"
#include "ResampleImageInternal.h"

static __m128 M128_PS_STEP( _mm_set_ps(3.0f,2.0f,1.0f,0.0f) );
static __m128 M128_PS_4_0( _mm_set1_ps( 4.0f ) );
static __m128 M128_PS_FIXED15( _mm_set1_ps( (float)(1<<15) ) );
static __m128i M128_U32_FIXED_ROUND( (_mm_set1_epi32(0x00200020)) );
static __m128i M128_U32_FIXED_COLOR_MASK( (_mm_set1_epi32(0x00ff00ff)) );
static __m128i M128_U32_FIXED_COLOR_MASK8( (_mm_set1_epi32(0x000000ff)) );
static __m128 M128_EPSILON( _mm_set1_ps( FLT_EPSILON ) );
static __m128 M128_ABS_MASK( _mm_castsi128_ps(_mm_set1_epi32(0x7fffffff)) );

static bool InitializedResampleSSE2 = false;
void TVPInitializeResampleSSE2() {
	if( !InitializedResampleSSE2) {
		M128_PS_STEP = ( _mm_set_ps(3.0f,2.0f,1.0f,0.0f) );
		M128_PS_4_0 = ( _mm_set1_ps( 4.0f ) );
		M128_PS_FIXED15 = ( _mm_set1_ps( (float)(1<<15) ) );
		M128_U32_FIXED_ROUND = ( (_mm_set1_epi32(0x00200020)) );
		M128_U32_FIXED_COLOR_MASK = ( (_mm_set1_epi32(0x00ff00ff)) );
		M128_U32_FIXED_COLOR_MASK8 = ( (_mm_set1_epi32(0x000000ff)) );
		M128_EPSILON = ( _mm_set1_ps( FLT_EPSILON ) );
		M128_ABS_MASK = ( _mm_castsi128_ps(_mm_set1_epi32(0x7fffffff)) );
		InitializedResampleSSE2 = true;
	}
}

void TJS_USERENTRY ResamplerSSE2FixFunc( void* p );
void TJS_USERENTRY ResamplerSSE2Func( void* p );

template<typename TWeight>
struct AxisParamSSE2 {
	std::vector<int> start_;	// �J�n�C���f�b�N�X
	std::vector<int> length_;	// �e�v�f����
	std::vector<int> length_min_;	// �e�v�f����, �A���C�����g������Ă��Ȃ��ŏ�����
	std::vector<TWeight,aligned_allocator<TWeight> > weight_;

	/**
	 * �͂ݏo���Ă��镔�����J�b�g����
	 */
	static inline void calculateEdge( float* weight, int& len, int leftedge, int rightedge ) {
		// ���[or�E�[�̎��A�͂ݏo�����̃E�F�C�g��[�ɉ��Z����
		if( leftedge ) {
			// ���[����͂ݏo���������Z
			int i = 1;
			for( ; i <= leftedge; i++ ) {
				weight[0] += weight[i];
			}
			// ���Z���������ړ�
			for( int j = 1; i < len; i++, j++ ) {
				weight[j] = weight[i];
			}
			// �͂ݏo�������̒������J�b�g
			len -= leftedge;
		}
		if( rightedge ) {
			// �E�[����͂ݏo���������Z
			int i = len - rightedge;
			int r = i - 1;
			for( ; i < len; i++ ) {
				weight[r] += weight[i];
			}
			// �͂ݏo�������̒������J�b�g
			len -= rightedge;
		}
	}
	// ���v�l�����߂�
	static inline __m128 sumWeight( float* weight, int len4 ) {
		float* w = weight;
		__m128 sum = _mm_setzero_ps();
		for( int i = 0; i < len4; i+=4 ) {
			__m128 weight4 = _mm_load_ps( w );
			sum = _mm_add_ps( sum, weight4 );
			w += 4;
		}
		return m128_hsum_sse1_ps(sum);
	}
	static inline void normalizeAndFixed( float* weight, tjs_uint32*& output, int& len, int len4, bool strip ) {
		// ���v�l�����߂�
		__m128 sum = sumWeight( weight, len4 );

		// EPSILON ��菬�����ꍇ�� 0 ��ݒ�
		const __m128 one = M128_PS_FIXED15; // �����t�Ȃ̂ŁB���Ɛ��K������Ă��邩��A�ő�l��1�ɂȂ�
		__m128 onemask = _mm_cmpgt_ps( sum, M128_EPSILON ); // sum > FLT_EPSILON ? 0xffffffff : 0; _mm_cmpgt_ps
		__m128 rcp = m128_rcp_22bit_ps( sum );
		rcp = _mm_mul_ps( rcp, one );	// ��ɃV�t�g�����|���Ă���
		rcp = _mm_and_ps( rcp, onemask );
		float* w = weight;
		// ���K���ƌŒ菬���_��
		for( int i = 0; i < len4; i+=4 ) {
			__m128 weight4 = _mm_load_ps( w ); w += 4;
			weight4 = _mm_mul_ps( weight4, rcp );

			// �Œ菬���_��
			__m128i fix = _mm_cvtps_epi32( weight4 );
			fix = _mm_packs_epi32( fix, fix );		// 16bit�� [01 02 03 04 01 02 03 04]
			fix = _mm_unpacklo_epi16( fix, fix );	// 01 01 02 02 03 03 04 04
			_mm_storeu_si128( (__m128i*)output, fix );	// tjs_uint32 �� short*2 �œ����l���i�[����
			output += 4;
		}
		if( strip ) {
			output -= len4-len;
		}
	}
	static inline void calculateWeight( float* weight, tjs_uint32*& output, int& len, int leftedge, int rightedge, bool strip=false ) {
		// len �ɂ͂͂ݏo���������܂܂�Ă���̂ŁA�܂��͂��̕������J�b�g����
		calculateEdge( weight, len, leftedge, rightedge );

		// 4 �̔{����
		int len4 = ((len+3)>>2)<<2;

		// �_�~�[������0�ɐݒ�
		for( int i = len; i < len4; i++ ) weight[i] = 0.0f;

		// ���K���ƌŒ菬���_��
		normalizeAndFixed( weight, output, len, len4, strip );
	}
	static inline void normalize( float* weight, float*& output, int& len, int len4, bool strip ) {
		// ���v�l�����߂�
		__m128 sum = sumWeight( weight, len4 );

		// EPSILON ��菬�����ꍇ�� 0 ��ݒ�
		__m128 onemask = _mm_cmpgt_ps( sum, M128_EPSILON ); // sum > FLT_EPSILON ? 0xffffffff : 0; _mm_cmpgt_ps
		__m128 rcp = m128_rcp_22bit_ps( sum );
		rcp = _mm_and_ps( rcp, onemask );
		float* w = weight;
		// ���K��
		for( int i = 0; i < len4; i+=4 ) {
			__m128 weight4 = _mm_load_ps( w ); w += 4;
			weight4 = _mm_mul_ps( weight4, rcp );
			_mm_storeu_ps( (float*)output, weight4 );
			output += 4;
		}
		if( strip ) {
			output -= len4-len;
		}
	}
	static inline void calculateWeight( float* weight, float*& output, int& len, int leftedge, int rightedge, bool strip=false ) {
		// len �ɂ͂͂ݏo���������܂܂�Ă���̂ŁA�܂��͂��̕������J�b�g����
		calculateEdge( weight, len, leftedge, rightedge );

		// 4 �̔{����
		int len4 = ((len+3)>>2)<<2;

		// �_�~�[������0�ɐݒ�
		for( int i = len; i < len4; i++ ) weight[i] = 0.0f;

		// ���K���ƌŒ菬���_��
		normalize( weight, output, len, len4, strip );
	}

	template<typename TWeightFunc>
	void calculateAxis( int srcstart, int srcend, int srclength, int dstlength, float tap, bool strip, TWeightFunc& func) {
		start_.clear();
		start_.reserve( dstlength );
		length_.clear();
		length_.reserve( dstlength );
		length_min_.clear();
		length_min_.reserve( dstlength );
		// �܂��͋������v�Z
		// left/right������O�ɏo���Ə������������Ȃ�Ƃ͎v�������x���Ȃ����Ainline������Ȃ��̂�������Ȃ�
		if( srclength <= dstlength ) { // �g��
			float rangex = tap;
			int maxrange = ((((int)rangex*2+2)+3)>>2)<<2;
			std::vector<float,aligned_allocator<float> > work( maxrange, 0.0f );
			float* weight = &work[0];
			int length = (((dstlength * maxrange + dstlength)+3)>>2)<<2;
#ifdef _DEBUG
			weight_.resize( length );
#else
			weight_.reserve( length );
#endif
			const __m128 delta4 = M128_PS_4_0;
			const __m128 deltafirst = M128_PS_STEP;
			const __m128 absmask = M128_ABS_MASK;
			TWeight* output = &weight_[0];
			for( int x = 0; x < dstlength; x++ ) {
				float cx = (x+0.5f)*(float)srclength/(float)dstlength + srcstart;
				int left = (int)std::floor(cx-rangex);
				int right = (int)std::floor(cx+rangex);
				int start = left;
				int leftedge = 0;
				if( left < srcstart ) {
					leftedge = srcstart - left;
					start = srcstart;
				}
				int rightedge = 0;
				if( right >= srcend ) {
					rightedge = right - srcend;
				}
				start_.push_back( start );
				int len = right - left;
				__m128 dist4 = _mm_set1_ps((float)left+0.5f-cx);
				int len4 = ((len+3)>>2)<<2;	// 4 �̔{����
				float* w = weight;
				// �܂��͍ŏ��̗v�f�̂ݏ�������
				dist4 = _mm_add_ps( dist4, deltafirst );
				_mm_store_ps( w, func( _mm_and_ps( dist4, absmask ) ) );	// ��Βl+weight�v�Z
				w += 4;
				for( int sx = 4; sx < len4; sx+=4 ) {
					dist4 = _mm_add_ps( dist4, delta4 );	// 4���X���C�h
					_mm_store_ps( w, func( _mm_and_ps( dist4, absmask ) ) );	// ��Βl+weight�v�Z
					w += 4;
				}
				calculateWeight( weight, output, len, leftedge, rightedge, strip );
				len4 = ((len+3)>>2)<<2;
				if( strip ) {
					length_.push_back( len );
					length_min_.push_back( len );
				} else {
					length_.push_back( len4 );
					length_min_.push_back( len );
				}
			}
		} else { // �k��
			float rangex = tap*(float)srclength/(float)dstlength;
			int maxrange = ((((int)rangex*2+2)+3)>>2)<<2;
			std::vector<float,aligned_allocator<float> > work( maxrange, 0.0f );
			float* weight = &work[0];
			int length = (((srclength * maxrange + srclength)+3)>>2)<<2;
#ifdef _DEBUG
			weight_.resize( length );
#else
			weight_.reserve( length );
#endif
			TWeight* output = &weight_[0];
			const float delta = (float)dstlength/(float)srclength; // �]������W�ł̈ʒu����

			__m128 delta4 = _mm_set1_ps(delta);
			__m128 deltafirst = M128_PS_STEP;
			const __m128 absmask = M128_ABS_MASK;
			deltafirst = _mm_mul_ps( deltafirst, delta4 );	// 0 1 2 3 �Ə��ɉ��Z�����悤�ɂ���
			// 4�{����
			delta4 = _mm_add_ps( delta4, delta4 );
			delta4 = _mm_add_ps( delta4, delta4 );
			for( int x = 0; x < dstlength; x++ ) {
				float cx = (x+0.5f)*(float)srclength/(float)dstlength + srcstart;
				int left = (int)std::floor(cx-rangex);
				int right = (int)std::floor(cx+rangex);
				int start = left;
				int leftedge = 0;
				if( left < srcstart ) {
					leftedge = srcstart - left;
					start = srcstart;
				}
				int rightedge = 0;
				if( right >= srcend ) {
					rightedge = right - srcend;
				}
				start_.push_back( start );
				// �]������W�ł̈ʒu
				int len = right-left;
				float dx = (left+0.5f-cx) * delta;
				__m128 dist4 = _mm_set1_ps(dx);
				int len4 = ((len+3)>>2)<<2;	// 4 �̔{����
				float* w = weight;
				// �܂��͍ŏ��̗v�f�̂ݏ�������
				dist4 = _mm_add_ps( dist4, deltafirst );
				_mm_store_ps( w, func( _mm_and_ps( dist4, absmask ) ) );	// ��Βl+weight�v�Z
				w += 4;
				for( int sx = 4; sx < len4; sx+=4 ) {
					dist4 = _mm_add_ps( dist4, delta4 );	// 4���X���C�h
					_mm_store_ps( w, func( _mm_and_ps( dist4, absmask ) ) );	// ��Βl+weight�v�Z
					w += 4;
				}
				calculateWeight( weight, output, len, leftedge, rightedge, strip );
				len4 = ((len+3)>>2)<<2;
				if( strip ) {
					length_.push_back( len );
					length_min_.push_back( len );
				} else {
					length_.push_back( len4 );
					length_min_.push_back( len );
				}
			}
		}
	}
	// ���v�l�����߂�
	static inline __m128 sumWeightUnalign( float* weight, int len4 ) {
		float* w = weight;
		__m128 sum = _mm_setzero_ps();
		for( int i = 0; i < len4; i+=4 ) {
			__m128 weight4 = _mm_loadu_ps( w );
			sum = _mm_add_ps( sum, weight4 );
			w += 4;
		}
		return m128_hsum_sse1_ps(sum);
	}
	// ���K��
	void normalizeAreaAvg( float* wstart, float* dweight, tjs_uint size, bool strip ) {
		const int count = (const int)length_.size();
		int dwindex = 0;
		const __m128 epsilon = M128_EPSILON;
		for( int i = 0; i < count; i++ ) {
			float* dw = dweight;
			int len = length_[i];
			float* w = wstart;
			int len4 = ((len+3)>>2)<<2;	// 4 �̔{����
			int idx = 0;
			for( ; idx < len; idx++ ) {
				*dw = *w;
				dw++;
				w++;
			}
			wstart = w;
			w = dweight;
			// �A���C�����g
			for( ; idx < len4; idx++ ) {
				*dw = 0.0f;
				dw++;
			}
			dweight = dw;

			// ���v�l�����߂�
			__m128 sum;
			if( strip ) {
				sum = sumWeightUnalign( w, len4 );
			} else {
				sum = sumWeight( w, len4 );
			}

			// EPSILON ��菬�����ꍇ�� 0 ��ݒ�
			__m128 onemask = _mm_cmpgt_ps( sum, epsilon ); // sum > FLT_EPSILON ? 0xffffffff : 0; _mm_cmpgt_ps
			__m128 rcp = m128_rcp_22bit_ps( sum );
			rcp = _mm_and_ps( rcp, onemask );
			// ���K��
			for( int j = 0; j < len4; j += 4 ) {
				__m128 weight4 = _mm_loadu_ps( w );
				weight4 = _mm_mul_ps( weight4, rcp );
				_mm_storeu_ps( (float*)w, weight4 );
				w += 4;
			}
			if( strip ) {
				dweight -= len4-len;
				length_min_.push_back( len );
			} else {
				length_[i] = len4;
				length_min_.push_back( len );
			}
		}
	}
	void normalizeAreaAvg( float* wstart, tjs_uint32* dweight, tjs_uint size,  bool strip ) {
		const int count = (const int)length_.size();
#ifdef _DEBUG
		std::vector<float,aligned_allocator<float> > work(size);
#else
		std::vector<float,aligned_allocator<float> > work;
		work.reserve( size );
#endif
		int dwindex = 0;
		const __m128 one = M128_PS_FIXED15; // �����t�Ȃ̂ŁB���Ɛ��K������Ă��邩��A�ő�l��1�ɂȂ�
		const __m128 epsilon = M128_EPSILON;
		for( int i = 0; i < count; i++ ) {
			float* dw = &work[0];
			int len = length_[i];
			float* w = wstart;
			int len4 = ((len+3)>>2)<<2;	// 4 �̔{����
			int idx = 0;
			for( ; idx < len; idx++ ) {
				*dw = *w;
				dw++;
				w++;
			}
			wstart = w;
			w = &work[0];
			// �A���C�����g
			for( ; idx < len4; idx++ ) {
				*dw = 0.0f;
				dw++;
			}

			// ���v�l�����߂�
			__m128 sum = sumWeight( w, len4 );

			// EPSILON ��菬�����ꍇ�� 0 ��ݒ�
			__m128 onemask = _mm_cmpgt_ps( sum, epsilon ); // sum > FLT_EPSILON ? 0xffffffff : 0; _mm_cmpgt_ps
			__m128 rcp = m128_rcp_22bit_ps( sum );
			rcp = _mm_mul_ps( rcp, one );	// ��ɃV�t�g�����|���Ă���
			rcp = _mm_and_ps( rcp, onemask );
			// ���K��
			for( int j = 0; j < len4; j += 4 ) {
				__m128 weight4 = _mm_load_ps( w ); w += 4;
				weight4 = _mm_mul_ps( weight4, rcp );
				// �Œ菬���_��
				__m128i fix = _mm_cvtps_epi32( weight4 );
				fix = _mm_packs_epi32( fix, fix );		// 16bit�� [01 02 03 04 01 02 03 04]
				fix = _mm_unpacklo_epi16( fix, fix );	// 01 01 02 02 03 03 04 04
				_mm_storeu_si128( (__m128i*)dweight, fix );	// tjs_uint32 �� short*2 �œ����l���i�[����
				dweight += 4;
			}
			if( strip ) {
				dweight -= len4-len;
				length_min_.push_back( len );
			} else {
				length_[i] = len4;
				length_min_.push_back( len );
			}
		}
	}
	void calculateAxisAreaAvg( int srcstart, int srcend, int srclength, int dstlength, bool strip ) {
		if( dstlength <= srclength ) { // �k���̂�
			std::vector<float> weight;
			TVPCalculateAxisAreaAvg( srcstart, srcend, srclength, dstlength, start_, length_, weight );
			// ���ۂ̃T�C�Y�����߂�
			int maxsize = 0;
			if( strip == false ) {
				int count = (int)length_.size();
				for( int i = 0; i < count; i++ ) {
					int len = length_[i];
					maxsize += ((len+3)>>2)<<2;	// 4 �̔{����
				}
			} else {
				maxsize = (int)weight.size();
			}
#ifdef _DEBUG
			weight_.resize( maxsize+3 );
#else
			weight_.reserve( maxsize+3 );
#endif
			normalizeAreaAvg( &weight[0], &weight_[0], maxsize+3, strip );
		}
	}
	
};

class ResamplerSSE2Fix {
	AxisParamSSE2<tjs_uint32> paramx_;
	AxisParamSSE2<tjs_uint32> paramy_;

public:
	/**
	 * �}���`�X���b�h���p
	 */
	struct ThreadParameterHV {
		ResamplerSSE2Fix* sampler_;
		int start_;
		int end_;
		int alingnwidth_;

		const tjs_uint32* wstarty_;
		const tTVPBaseBitmap* src_;
		const tTVPRect* srcrect_;
		tTVPBaseBitmap* dest_;
		const tTVPRect* destrect_;

		const tTVPResampleClipping* clip_;
		const tTVPImageCopyFuncBase* blendfunc_;
	};
	/**
	 * �������̏�������ɂ����ꍇ�̎���
	 */
	inline void samplingHorizontal( tjs_uint32* dstbits, const int offsetx, const int dstwidth, const tjs_uint32* srcbits ) {
		const tjs_uint32* weightx = &paramx_.weight_[0];
		// �܂�offset�����X�L�b�v
		for( int x = 0; x < offsetx; x++ ) {
			weightx += paramx_.length_[x];
		}
		const tjs_uint32* src = srcbits;
		const __m128i cmask = M128_U32_FIXED_COLOR_MASK;
		const __m128i fixround = M128_U32_FIXED_ROUND;
		for( int x = offsetx; x < dstwidth; x++ ) {
			const int left = paramx_.start_[x];
			int right = left + paramx_.length_[x];
			__m128i color_lo = _mm_setzero_si128();
			__m128i color_hi = _mm_setzero_si128();
			// 4�s�N�Z������������
			for( int sx = left; sx < right; sx+=4 ) {
				__m128i col4 = _mm_loadu_si128( (const __m128i*)&src[sx] ); // 4�s�N�Z���ǂݍ���
				__m128i weight4 = _mm_loadu_si128( (const __m128i*)weightx ); // �E�F�C�g(�Œ菭��)4��(16bit��8)�ǂݍ��� 0 1 2 3
				weightx += 4;

				__m128i col = _mm_and_si128( col4, cmask );	// 00 RR 00 BB & 0x00ff00ff
				col = _mm_slli_epi16( col, 7 );	// << 7
				col = _mm_mulhi_epi16( col, weight4 );
				color_lo = _mm_adds_epi16( color_lo, col );

				col = _mm_srli_epi16( col4, 8 );	// 00 AA 00 GG
				col = _mm_slli_epi16( col, 7 );	// << 7
				col = _mm_mulhi_epi16( col, weight4 );
				color_hi = _mm_adds_epi16( color_hi, col );
			}
			{	// SSE - �������Z
				__m128i sumlo = color_lo;
				color_lo = _mm_shuffle_epi32( color_lo, _MM_SHUFFLE(1,0,3,2) ); // 0 1 2 3 + 1 0 3 2
				sumlo = _mm_adds_epi16( sumlo, color_lo );
				color_lo = _mm_shuffle_epi32( sumlo, _MM_SHUFFLE(2,3,0,1) ); // 3 2 1 0
				sumlo = _mm_adds_epi16( sumlo, color_lo );
				sumlo = _mm_adds_epi16( sumlo, fixround );
				sumlo = _mm_srai_epi16( sumlo, 6 ); // �Œ菬���_���琮���� - << 15, << 7, >> 16 = 6

				__m128i sumhi = color_hi;
				color_hi = _mm_shuffle_epi32( color_hi, _MM_SHUFFLE(1,0,3,2) ); // 0 1 2 3 + 1 0 3 2
				sumhi = _mm_adds_epi16( sumhi, color_hi );
				color_hi = _mm_shuffle_epi32( sumhi, _MM_SHUFFLE(2,3,0,1) ); // 3 2 1 0
				sumhi = _mm_adds_epi16( sumhi, color_hi );
				sumhi = _mm_adds_epi16( sumhi, fixround );
				sumhi = _mm_srai_epi16( sumhi, 6 ); // �Œ菬���_���琮����

				sumlo = _mm_unpacklo_epi16( sumlo, sumhi );
				sumlo = _mm_packus_epi16( sumlo, sumlo );
				*dstbits = _mm_cvtsi128_si32( sumlo );
			}
			dstbits++;
		}
	}
	/** */
	inline void samplingVertical( int y, tjs_uint32* dstbits, int dstheight, int srcwidth, const tTVPBaseBitmap *src, const tTVPRect &srcrect, const tjs_uint32*& wstarty ) {
		const int top = paramy_.start_[y];
		const int len = paramy_.length_min_[y];
		const int bottom = top + len;
		const tjs_uint32* weighty = wstarty;
		const __m128i cmask = M128_U32_FIXED_COLOR_MASK;
		const __m128i fixround = M128_U32_FIXED_ROUND;
		const tjs_uint32* srctop = (const tjs_uint32*)src->GetScanLine(top) + srcrect.left;
		tjs_int stride = src->GetPitchBytes()/(int)sizeof(tjs_uint32);
		for( int x = 0; x < srcwidth; x+=4 ) {
			weighty = wstarty;
			__m128i color_lo = _mm_setzero_si128();
			__m128i color_hi = _mm_setzero_si128();
			const tjs_uint32* srcbits = &srctop[x];
			for( int sy = top; sy < bottom; sy++ ) {
				__m128i col4 = _mm_loadu_si128( (const __m128i*)srcbits ); // 4��ǂݍ���
				srcbits += stride;
				__m128i weight4 = _mm_set1_epi32( (int)*weighty ); // weight �́A�����l��ݒ�
				weighty++;

				__m128i col = _mm_and_si128( col4, cmask );	// 00 RR 00 BB
				col = _mm_slli_epi16( col, 7 );	// << 7
				col = _mm_mulhi_epi16( col, weight4 );
				color_lo = _mm_adds_epi16( color_lo, col );

				col = _mm_srli_epi16( col4, 8 );	// 00 AA 00 GG
				col = _mm_slli_epi16( col, 7 );	// << 7
				col = _mm_mulhi_epi16( col, weight4 );
				color_hi = _mm_adds_epi16( color_hi, col );
			}
			{
				color_lo = _mm_adds_epi16( color_lo, fixround );
				color_hi = _mm_adds_epi16( color_hi, fixround );
				color_lo = _mm_srai_epi16( color_lo, 6 ); // �Œ菬���_���琮���� - << 15, << 7, >> 16 = 6
				color_hi = _mm_srai_epi16( color_hi, 6 ); // �Œ菬���_���琮����
				__m128i lo = _mm_unpacklo_epi16( color_lo, color_hi );
				__m128i hi = _mm_unpackhi_epi16( color_lo, color_hi );
				color_lo = _mm_packus_epi16( lo, hi );
				_mm_store_si128( (__m128i *)&dstbits[x], color_lo );
			}
		}
		wstarty = weighty;
	}

	void ResampleImage( const tTVPResampleClipping &clip, const tTVPImageCopyFuncBase* blendfunc, tTVPBaseBitmap *dest, const tTVPRect &destrect, const tTVPBaseBitmap *src, const tTVPRect &srcrect ) {
		const int srcwidth = srcrect.get_width();
		const int dstheight = destrect.get_height();
		const int alingnwidth = (((srcwidth+3)>>2)<<2) + 3;
#ifdef _DEBUG
		std::vector<tjs_uint32,aligned_allocator<tjs_uint32> > work( alingnwidth );
#else
		std::vector<tjs_uint32,aligned_allocator<tjs_uint32> > work;
		work.reserve( alingnwidth );
#endif
		const tjs_uint32* wstarty = &paramy_.weight_[0];
		// �N���b�s���O�����X�L�b�v
		for( int y = 0; y < clip.offsety_; y++ ) {
			wstarty += paramy_.length_[y];
		}
		tjs_uint32* workbits = &work[0];
		tjs_int dststride = dest->GetPitchBytes()/(int)sizeof(tjs_uint32);
		tjs_uint32* dstbits = (tjs_uint32*)dest->GetScanLineForWrite(clip.dst_top_) + clip.dst_left_;
		if( blendfunc == NULL ) {
			for( int y = clip.offsety_; y < clip.height_; y++ ) {
				samplingVertical( y, workbits, dstheight, srcwidth, src, srcrect, wstarty );
				samplingHorizontal( dstbits, clip.offsetx_, clip.width_, workbits );
				dstbits += dststride;
			}
		} else {	// �P���R�s�[�ȊO�́A��x�e���|�����ɏ����o���Ă��獇������
#ifdef _DEBUG
			std::vector<tjs_uint32> dstwork(clip.getDestWidth()+3);
#else
			std::vector<tjs_uint32> dstwork;
			dstwork.reserve( clip.getDestWidth()+3 );
#endif
			tjs_uint32* midbits = &dstwork[0];	// �r�������p�o�b�t�@
			for( int y = clip.offsety_; y < clip.height_; y++ ) {
				samplingVertical( y, workbits, dstheight, srcwidth, src, srcrect, wstarty );
				samplingHorizontal( midbits, clip.offsetx_, clip.width_, workbits ); // �ꎞ�o�b�t�@�ɂ܂��R�s�[, �͈͊O�͏������Ȃ�
				(*blendfunc)( dstbits, midbits, clip.getDestWidth() );
				dstbits += dststride;
			}
		}
	}
	void ResampleImageMT( const tTVPResampleClipping &clip, const tTVPImageCopyFuncBase* blendfunc, tTVPBaseBitmap *dest, const tTVPRect &destrect, const tTVPBaseBitmap *src, const tTVPRect &srcrect, tjs_int threadNum ) {
		const int srcwidth = srcrect.get_width();
		const int alingnwidth = ((srcwidth+3)>>2)<<2;
		const tjs_uint32* wstarty = &paramy_.weight_[0];
		// �N���b�s���O�����X�L�b�v
		for( int y = 0; y < clip.offsety_; y++ ) {
			wstarty += paramy_.length_[y];
		}
		int offset = clip.offsety_;
		const int height = clip.getDestHeight();

		TVPBeginThreadTask(threadNum);
		std::vector<ThreadParameterHV> params(threadNum);
		for( int i = 0; i < threadNum; i++ ) {
			ThreadParameterHV* param = &params[i];
			param->sampler_ = this;
			param->start_ = height * i / threadNum + offset;
			param->end_ = height * (i + 1) / threadNum + offset;
			param->alingnwidth_ = alingnwidth;
			param->wstarty_ = wstarty;
			param->src_ = src;
			param->srcrect_ = &srcrect;
			param->dest_ = dest;
			param->destrect_ = &destrect;
			param->clip_ = &clip;
			param->blendfunc_ = blendfunc;
			int top = param->start_;
			int bottom = param->end_;
			TVPExecThreadTask(&ResamplerSSE2FixFunc, TVP_THREAD_PARAM(param));
			if( i < (threadNum-1) ) {
				for( int y = top; y < bottom; y++ ) {
					int len = paramy_.length_[y];
					wstarty += len;
				}
			}
		}
		TVPEndThreadTask();
	}
public:
	template<typename TWeightFunc>
	void Resample( const tTVPResampleClipping &clip, const tTVPImageCopyFuncBase* blendfunc, tTVPBaseBitmap *dest, const tTVPRect &destrect, const tTVPBaseBitmap *src, const tTVPRect &srcrect, float tap, TWeightFunc& func ) {
		const int srcwidth = srcrect.get_width();
		const int srcheight = srcrect.get_height();
		const int dstwidth = destrect.get_width();
		const int dstheight = destrect.get_height();

		paramx_.calculateAxis( 0, srcwidth, srcwidth, dstwidth, tap, false, func );
		paramy_.calculateAxis( srcrect.top, srcrect.bottom, srcheight, dstheight, tap, true, func );
		ResampleImage( clip, blendfunc, dest, destrect, src, srcrect );
	}
	template<typename TWeightFunc>
	void ResampleMT( const tTVPResampleClipping &clip, const tTVPImageCopyFuncBase* blendfunc, tTVPBaseBitmap *dest, const tTVPRect &destrect, const tTVPBaseBitmap *src, const tTVPRect &srcrect, float tap, TWeightFunc& func ) {
		const int srcwidth = srcrect.get_width();
		const int srcheight = srcrect.get_height();
		const int dstwidth = destrect.get_width();
		const int dstheight = destrect.get_height();
		int maxwidth = srcwidth > dstwidth ? srcwidth : dstwidth;
		int maxheight = srcheight > dstheight ? srcheight : dstheight;
		int threadNum = 1;
		int pixelNum = maxwidth*(int)tap*maxheight + maxheight*(int)tap*maxwidth;
		if( pixelNum >= 50 * 500 ) {
			threadNum = TVPGetThreadNum();
		}
		if( threadNum == 1 ) { // �ʐς����Ȃ��X���b�h��1�̎��͂��̂܂܎��s
			Resample( clip, blendfunc, dest, destrect, src, srcrect, tap, func );
			return;
		}

		paramx_.calculateAxis( 0, srcwidth, srcwidth, dstwidth, tap, false, func );
		paramy_.calculateAxis( srcrect.top, srcrect.bottom, srcheight, dstheight, tap, true, func );
		ResampleImageMT( clip, blendfunc, dest, destrect, src, srcrect, threadNum );
	}
	void ResampleAreaAvg( const tTVPResampleClipping &clip, const tTVPImageCopyFuncBase* blendfunc, tTVPBaseBitmap *dest, const tTVPRect &destrect, const tTVPBaseBitmap *src, const tTVPRect &srcrect ) {
		const int srcwidth = srcrect.get_width();
		const int srcheight = srcrect.get_height();
		const int dstwidth = destrect.get_width();
		const int dstheight = destrect.get_height();
		if( dstwidth > srcwidth || dstheight > srcheight ) return;

		paramx_.calculateAxisAreaAvg( 0, srcwidth, srcwidth, dstwidth, false );
		paramy_.calculateAxisAreaAvg( srcrect.top, srcrect.bottom, srcheight, dstheight, true );
		ResampleImage( clip, blendfunc, dest, destrect, src, srcrect );
	}
	void ResampleAreaAvgMT( const tTVPResampleClipping &clip, const tTVPImageCopyFuncBase* blendfunc, tTVPBaseBitmap *dest, const tTVPRect &destrect, const tTVPBaseBitmap *src, const tTVPRect &srcrect ) {
		const int srcwidth = srcrect.get_width();
		const int srcheight = srcrect.get_height();
		const int dstwidth = destrect.get_width();
		const int dstheight = destrect.get_height();
		if( dstwidth > srcwidth || dstheight > srcheight ) return;

		int maxwidth = srcwidth > dstwidth ? srcwidth : dstwidth;
		int maxheight = srcheight > dstheight ? srcheight : dstheight;
		int threadNum = 1;
		int pixelNum = maxwidth*maxheight;
		if( pixelNum >= 50 * 500 ) {
			threadNum = TVPGetThreadNum();
		}
		if( threadNum == 1 ) { // �ʐς����Ȃ��X���b�h��1�̎��͂��̂܂܎��s
			ResampleAreaAvg( clip, blendfunc, dest, destrect, src, srcrect );
			return;
		}

		paramx_.calculateAxisAreaAvg( 0, srcwidth, srcwidth, dstwidth, false );
		paramy_.calculateAxisAreaAvg( srcrect.top, srcrect.bottom, srcheight, dstheight, true );
		ResampleImageMT( clip, blendfunc, dest, destrect, src, srcrect, threadNum );
	}
};

void TJS_USERENTRY ResamplerSSE2FixFunc( void* p ) {
	ResamplerSSE2Fix::ThreadParameterHV* param = (ResamplerSSE2Fix::ThreadParameterHV*)p;
	const int alingnwidth = param->alingnwidth_;
#ifdef _DEBUG
	std::vector<tjs_uint32,aligned_allocator<tjs_uint32> > work(alingnwidth);
#else
	std::vector<tjs_uint32,aligned_allocator<tjs_uint32> > work;
	work.reserve( alingnwidth );
#endif

	tTVPBaseBitmap* dest = param->dest_;
	const tTVPRect& destrect = *param->destrect_;
	const tTVPBaseBitmap* src = param->src_;
	const tTVPRect& srcrect = *param->srcrect_;

	const int srcwidth = srcrect.get_width();
	const int dstwidth = destrect.get_width();
	const int dstheight = destrect.get_height();
	const tjs_uint32* wstarty = param->wstarty_;
	tjs_uint32* workbits = &work[0];
	tjs_int dststride = dest->GetPitchBytes()/(int)sizeof(tjs_uint32);
	tjs_uint32* dstbits = (tjs_uint32*)dest->GetScanLineForWrite(param->start_+destrect.top) + param->clip_->dst_left_;
	if( param->blendfunc_ == NULL ) {
		for( int y = param->start_; y < param->end_; y++ ) {
			param->sampler_->samplingVertical( y, workbits, dstheight, srcwidth, src, srcrect, wstarty );
			param->sampler_->samplingHorizontal( dstbits, param->clip_->offsetx_, param->clip_->width_, workbits );
			dstbits += dststride;
		}
	} else {	// �P���R�s�[�ȊO
#ifdef _DEBUG
		std::vector<tjs_uint32> dstwork(param->clip_->getDestWidth()+3);
#else
		std::vector<tjs_uint32> dstwork;
		dstwork.reserve( param->clip_->getDestWidth()+3 );
#endif
		tjs_uint32* midbits = &dstwork[0];	// �r�������p�o�b�t�@
		for( int y = param->start_; y < param->end_; y++ ) {
			param->sampler_->samplingVertical( y, workbits, dstheight, srcwidth, src, srcrect, wstarty );
			param->sampler_->samplingHorizontal( midbits, param->clip_->offsetx_, param->clip_->width_, workbits ); // �ꎞ�o�b�t�@�ɂ܂��R�s�[, �͈͊O�͏������Ȃ�
			(*param->blendfunc_)( dstbits, midbits, param->clip_->getDestWidth() );
			dstbits += dststride;
		}
	}
}

class ResamplerSSE2 {
	AxisParamSSE2<float> paramx_;
	AxisParamSSE2<float> paramy_;

public:
	/** �}���`�X���b�h���p */
	struct ThreadParameterHV {
		ResamplerSSE2* sampler_;
		int start_;
		int end_;
		int alingnwidth_;
		
		const float* wstarty_;
		const tTVPBaseBitmap* src_;
		const tTVPRect* srcrect_;
		tTVPBaseBitmap* dest_;
		const tTVPRect* destrect_;

		const tTVPResampleClipping* clip_;
		const tTVPImageCopyFuncBase* blendfunc_;
	};
	/**
	 * �������̏��� (��ɏ���)
	 */
	inline void samplingHorizontal( tjs_uint32* dstbits, const int offsetx, const int dstwidth, const tjs_uint32* srcbits ) {
		const __m128i cmask = M128_U32_FIXED_COLOR_MASK8;	// 8bit�����邽�߂̃}�X�N
		const float* weightx = &paramx_.weight_[0];
		// �܂�offset�����X�L�b�v
		for( int x = 0; x < offsetx; x++ ) {
			weightx += paramx_.length_[x];
		}
		const tjs_uint32* src = srcbits;
		const __m128i zero = _mm_setzero_si128();
		for( int x = offsetx; x < dstwidth; x++ ) {
			const int left = paramx_.start_[x];
			int right = left + paramx_.length_[x];
			__m128 color_elm = _mm_setzero_ps();
			// 4�s�N�Z������������
			for( int sx = left; sx < right; sx+=4 ) {
				__m128i col4 = _mm_loadu_si128( (const __m128i*)&src[sx] ); // 4�s�N�Z���ǂݍ���
				__m128 weight4 = _mm_loadu_ps( (const float*)weightx ); // �E�F�C�g4��
				weightx += 4;

				// a r g b | a r g b �� 2���������邩��Aweight �����̌`�ɃC���^�[���[�u
				__m128i collo = _mm_unpacklo_epi8( col4, zero );		// 00 01 00 02 00 03 0 04 00 05 00 06...
				__m128i col = _mm_unpacklo_epi16( collo, zero );		// 00 00 00 01 00 00 00 02...
				__m128 colf = _mm_cvtepi32_ps( col );
				__m128 wlo = _mm_unpacklo_ps( weight4, weight4 );
				__m128 w = _mm_unpacklo_ps( wlo, wlo );	// 00 00 00 00 04 04 04 04
				colf = _mm_mul_ps( colf, w );
				color_elm = _mm_add_ps( color_elm, colf );
				
				col = _mm_unpackhi_epi16( collo, zero );		// 00 00 00 01 00 00 00 02...
				colf = _mm_cvtepi32_ps( col );				// int to float
				w = _mm_unpackhi_ps( wlo, wlo );
				colf = _mm_mul_ps( colf, w );
				color_elm = _mm_add_ps( color_elm, colf );
				
				__m128i colhi = _mm_unpackhi_epi8( col4, zero );	// 00 01 00 02 00 03 0 04 00 05 00 06...
				col = _mm_unpacklo_epi16( colhi, zero );			// 00 00 00 01 00 00 00 02...
				colf = _mm_cvtepi32_ps( col );					// int to float
				__m128 whi = _mm_unpackhi_ps( weight4, weight4 );
				w = _mm_unpacklo_ps( whi, whi );
				colf = _mm_mul_ps( colf, w );
				color_elm = _mm_add_ps( color_elm, colf );
				
				col = _mm_unpackhi_epi16( colhi, zero );		// 00 00 00 01 00 00 00 02...
				colf = _mm_cvtepi32_ps( col );				// int to float
				w = _mm_unpackhi_ps( whi, whi );
				colf = _mm_mul_ps( colf, w );
				color_elm = _mm_add_ps( color_elm, colf );
			}
			{	// SSE
				__m128i color = _mm_cvtps_epi32( color_elm );
				color = _mm_packus_epi32( color, color );
				color = _mm_packus_epi16( color, color );
				*dstbits = _mm_cvtsi128_si32( color );
			}
			dstbits++;
		}
	}
	/**
	 * �c��������
	 */
	inline void samplingVertical( int y, tjs_uint32* dstbits, int dstheight, int srcwidth, const tTVPBaseBitmap *src, const tTVPRect &srcrect, const float*& wstarty ) {
		const int top = paramy_.start_[y];
		const int len = paramy_.length_min_[y];
		const int bottom = top + len;
		const float* weighty = wstarty;
		const __m128i cmask = M128_U32_FIXED_COLOR_MASK8;
		const tjs_uint32* srctop = (const tjs_uint32*)src->GetScanLine(top) + srcrect.left;
		tjs_int stride = src->GetPitchBytes()/(int)sizeof(tjs_uint32);
		for( int x = 0; x < srcwidth; x+=4 ) {
			weighty = wstarty;
			__m128 color_a = _mm_setzero_ps();
			__m128 color_r = _mm_setzero_ps();
			__m128 color_g = _mm_setzero_ps();
			__m128 color_b = _mm_setzero_ps();
			const tjs_uint32* srcbits = &srctop[x];
			for( int sy = top; sy < bottom; sy++ ) {
				__m128i col4 = _mm_loadu_si128( (const __m128i*)srcbits ); // 8��ǂݍ���
				srcbits += stride;
				__m128 weight4 = _mm_set1_ps( *weighty ); // weight �́A�����l��ݒ�
				weighty++;
				
				__m128i c = _mm_srli_epi32( col4, 24 );
				__m128 cf = _mm_cvtepi32_ps(c);
				cf = _mm_mul_ps( cf, weight4 );
				color_a = _mm_add_ps( color_a, cf );

				c = _mm_srli_epi32( col4, 16 );
				c = _mm_and_si128( c, cmask );
				cf = _mm_cvtepi32_ps(c);
				cf = _mm_mul_ps( cf, weight4 );
				color_r = _mm_add_ps( color_r, cf );
				
				c = _mm_srli_epi32( col4, 8 );
				c = _mm_and_si128( c, cmask );
				cf = _mm_cvtepi32_ps(c);
				cf = _mm_mul_ps( cf, weight4 );
				color_g = _mm_add_ps( color_g, cf );

				c = _mm_and_si128( col4, cmask );
				cf = _mm_cvtepi32_ps(c);
				cf = _mm_mul_ps( cf, weight4 );
				color_b = _mm_add_ps( color_b, cf );
			}
			{
				__m128i a = _mm_cvtps_epi32( color_a );
				__m128i r = _mm_cvtps_epi32( color_r );
				__m128i g = _mm_cvtps_epi32( color_g );
				__m128i b = _mm_cvtps_epi32( color_b );
				// �C���^�[���[�u
				__m128i arl = _mm_unpacklo_epi32( r, a );
				__m128i arh = _mm_unpackhi_epi32( r, a );
				arl = _mm_packs_epi32( arl, arh );	// a r a r a r ar
				__m128i gbl = _mm_unpacklo_epi32( b, g );
				__m128i gbh = _mm_unpackhi_epi32( b, g );
				gbl = _mm_packs_epi32( gbl, gbh );	// g b g b g b g b
				__m128i l = _mm_unpacklo_epi32( gbl, arl );
				__m128i h = _mm_unpackhi_epi32( gbl, arl );
				l = _mm_packus_epi16( l, h );
				_mm_store_si128( (__m128i *)&dstbits[x], l );
			}
		}
		wstarty = weighty;
	}
	
	void ResampleImage( const tTVPResampleClipping &clip, const tTVPImageCopyFuncBase* blendfunc, tTVPBaseBitmap *dest, const tTVPRect &destrect, const tTVPBaseBitmap *src, const tTVPRect &srcrect ) {
		const int srcwidth = srcrect.get_width();
		const int dstheight = destrect.get_height();
		const int alingnwidth = (((srcwidth+3)>>2)<<2) + 3;
#ifdef _DEBUG
		std::vector<tjs_uint32,aligned_allocator<tjs_uint32> > work( alingnwidth );
#else
		std::vector<tjs_uint32,aligned_allocator<tjs_uint32> > work;
		work.reserve( alingnwidth );
#endif
		const float* wstarty = &paramy_.weight_[0];
		// �N���b�s���O�����X�L�b�v
		for( int y = 0; y < clip.offsety_; y++ ) {
			wstarty += paramy_.length_[y];
		}
		tjs_uint32* workbits = &work[0];
		tjs_int dststride = dest->GetPitchBytes()/(int)sizeof(tjs_uint32);
		tjs_uint32* dstbits = (tjs_uint32*)dest->GetScanLineForWrite(clip.dst_top_) + clip.dst_left_;
		if( blendfunc == NULL ) {
			for( int y = clip.offsety_; y < clip.height_; y++ ) {
				samplingVertical( y, workbits, dstheight, srcwidth, src, srcrect, wstarty );
				samplingHorizontal( dstbits, clip.offsetx_, clip.width_, workbits );
				dstbits += dststride;
			}
		} else {	// �P���R�s�[�ȊO�́A��x�e���|�����ɏ����o���Ă��獇������
#ifdef _DEBUG
			std::vector<tjs_uint32> dstwork(clip.getDestWidth()+3);
#else
			std::vector<tjs_uint32> dstwork;
			dstwork.reserve( clip.getDestWidth()+3 );
#endif
			tjs_uint32* midbits = &dstwork[0];	// �r�������p�o�b�t�@
			for( int y = clip.offsety_; y < clip.height_; y++ ) {
				samplingVertical( y, workbits, dstheight, srcwidth, src, srcrect, wstarty );
				samplingHorizontal( midbits, clip.offsetx_, clip.width_, workbits ); // �ꎞ�o�b�t�@�ɂ܂��R�s�[, �͈͊O�͏������Ȃ�
				(*blendfunc)( dstbits, midbits, clip.getDestWidth() );
				dstbits += dststride;
			}
		}
	}
	void ResampleImageMT( const tTVPResampleClipping &clip, const tTVPImageCopyFuncBase* blendfunc, tTVPBaseBitmap *dest, const tTVPRect &destrect, const tTVPBaseBitmap *src, const tTVPRect &srcrect, tjs_int threadNum ) {
		const int srcwidth = srcrect.get_width();
		const int alingnwidth = ((srcwidth+3)>>2)<<2;
		const float* wstarty = &paramy_.weight_[0];
		// �N���b�s���O�����X�L�b�v
		for( int y = 0; y < clip.offsety_; y++ ) {
			wstarty += paramy_.length_[y];
		}
		int offset = clip.offsety_;
		const int height = clip.getDestHeight();

		TVPBeginThreadTask(threadNum);
		std::vector<ThreadParameterHV> params(threadNum);
		for( int i = 0; i < threadNum; i++ ) {
			ThreadParameterHV* param = &params[i];
			param->sampler_ = this;
			param->start_ = height * i / threadNum + offset;
			param->end_ = height * (i + 1) / threadNum + offset;
			param->alingnwidth_ = alingnwidth;
			param->wstarty_ = wstarty;
			param->src_ = src;
			param->srcrect_ = &srcrect;
			param->dest_ = dest;
			param->destrect_ = &destrect;
			param->clip_ = &clip;
			param->blendfunc_ = blendfunc;
			int top = param->start_;
			int bottom = param->end_;
			TVPExecThreadTask(&ResamplerSSE2Func, TVP_THREAD_PARAM(param));
			if( i < (threadNum-1) ) {
				for( int y = top; y < bottom; y++ ) {
					int len = paramy_.length_[y];
					wstarty += len;
				}
			}
		}
		TVPEndThreadTask();
	}
public:
	/** 4���C������������ */
	template<typename TWeightFunc>
	void Resample( const tTVPResampleClipping &clip, const tTVPImageCopyFuncBase* blendfunc, tTVPBaseBitmap *dest, const tTVPRect &destrect, const tTVPBaseBitmap *src, const tTVPRect &srcrect, float tap, TWeightFunc& func ) {
		const int srcwidth = srcrect.get_width();
		const int srcheight = srcrect.get_height();
		const int dstwidth = destrect.get_width();
		const int dstheight = destrect.get_height();
		paramx_.calculateAxis( 0, srcwidth, srcwidth, dstwidth, tap, false, func );
		paramy_.calculateAxis( srcrect.top, srcrect.bottom, srcheight, dstheight, tap, true, func );
		ResampleImage( clip, blendfunc, dest, destrect, src, srcrect );
	}
	template<typename TWeightFunc>
	void ResampleMT( const tTVPResampleClipping &clip, const tTVPImageCopyFuncBase* blendfunc, tTVPBaseBitmap *dest, const tTVPRect &destrect, const tTVPBaseBitmap *src, const tTVPRect &srcrect, float tap, TWeightFunc& func ) {
		const int srcwidth = srcrect.get_width();
		const int srcheight = srcrect.get_height();
		const int dstwidth = destrect.get_width();
		const int dstheight = destrect.get_height();
		int maxwidth = srcwidth > dstwidth ? srcwidth : dstwidth;
		int maxheight = srcheight > dstheight ? srcheight : dstheight;
		int threadNum = 1;
		int pixelNum = maxwidth*(int)tap*maxheight + maxheight*(int)tap*maxwidth;
		if( pixelNum >= 50 * 500 ) {
			threadNum = TVPGetThreadNum();
		}
		if( threadNum == 1 ) { // �ʐς����Ȃ��X���b�h��1�̎��͂��̂܂܎��s
			Resample( clip, blendfunc, dest, destrect, src, srcrect, tap, func );
			return;
		}
		paramx_.calculateAxis( 0, srcwidth, srcwidth, dstwidth, tap, false, func );
		paramy_.calculateAxis( srcrect.top, srcrect.bottom, srcheight, dstheight, tap, true, func );
		ResampleImageMT( clip, blendfunc, dest, destrect, src, srcrect, threadNum );
	}
	
	void ResampleAreaAvg( const tTVPResampleClipping &clip, const tTVPImageCopyFuncBase* blendfunc, tTVPBaseBitmap *dest, const tTVPRect &destrect, const tTVPBaseBitmap *src, const tTVPRect &srcrect ) {
		const int srcwidth = srcrect.get_width();
		const int srcheight = srcrect.get_height();
		const int dstwidth = destrect.get_width();
		const int dstheight = destrect.get_height();
		if( dstwidth > srcwidth || dstheight > srcheight ) return;
		paramx_.calculateAxisAreaAvg( 0, srcwidth, srcwidth, dstwidth, false );
		paramy_.calculateAxisAreaAvg( srcrect.top, srcrect.bottom, srcheight, dstheight, true );
		ResampleImage( clip, blendfunc, dest, destrect, src, srcrect );
	}
	void ResampleAreaAvgMT( const tTVPResampleClipping &clip, const tTVPImageCopyFuncBase* blendfunc, tTVPBaseBitmap *dest, const tTVPRect &destrect, const tTVPBaseBitmap *src, const tTVPRect &srcrect ) {
		const int srcwidth = srcrect.get_width();
		const int srcheight = srcrect.get_height();
		const int dstwidth = destrect.get_width();
		const int dstheight = destrect.get_height();
		if( dstwidth > srcwidth || dstheight > srcheight ) return;
		int maxwidth = srcwidth > dstwidth ? srcwidth : dstwidth;
		int maxheight = srcheight > dstheight ? srcheight : dstheight;
		int threadNum = 1;
		int pixelNum = maxwidth*maxheight;
		if( pixelNum >= 50 * 500 ) {
			threadNum = TVPGetThreadNum();
		}
		if( threadNum == 1 ) { // �ʐς����Ȃ��X���b�h��1�̎��͂��̂܂܎��s
			ResampleAreaAvg( clip, blendfunc, dest, destrect, src, srcrect );
			return;
		}
		paramx_.calculateAxisAreaAvg( 0, srcwidth, srcwidth, dstwidth, false );
		paramy_.calculateAxisAreaAvg( srcrect.top, srcrect.bottom, srcheight, dstheight, true );
		ResampleImageMT( clip, blendfunc, dest, destrect, src, srcrect, threadNum );
	}
};

void TJS_USERENTRY ResamplerSSE2Func( void* p ) {
	ResamplerSSE2::ThreadParameterHV* param = (ResamplerSSE2::ThreadParameterHV*)p;
	const int alingnwidth = param->alingnwidth_;
#ifdef _DEBUG
	std::vector<tjs_uint32,aligned_allocator<tjs_uint32> > work(alingnwidth);
#else
	std::vector<tjs_uint32,aligned_allocator<tjs_uint32> > work;
	work.reserve( alingnwidth );
#endif

	tTVPBaseBitmap* dest = param->dest_;
	const tTVPRect& destrect = *param->destrect_;
	const tTVPBaseBitmap* src = param->src_;
	const tTVPRect& srcrect = *param->srcrect_;

	const int srcwidth = srcrect.get_width();
	const int dstwidth = destrect.get_width();
	const int dstheight = destrect.get_height();
	const float* wstarty = param->wstarty_;
	tjs_uint32* workbits = &work[0];
	tjs_int dststride = dest->GetPitchBytes()/(int)sizeof(tjs_uint32);
	tjs_uint32* dstbits = (tjs_uint32*)dest->GetScanLineForWrite(param->start_+destrect.top) + param->clip_->dst_left_;
	if( param->blendfunc_ == NULL ) {
		for( int y = param->start_; y < param->end_; y++ ) {
			param->sampler_->samplingVertical( y, workbits, dstheight, srcwidth, src, srcrect, wstarty );
			param->sampler_->samplingHorizontal( dstbits, param->clip_->offsetx_, param->clip_->width_, workbits );
			dstbits += dststride;
		}
	} else {	// �P���R�s�[�ȊO
#ifdef _DEBUG
		std::vector<tjs_uint32> dstwork(param->clip_->getDestWidth()+3);
#else
		std::vector<tjs_uint32> dstwork;
		dstwork.reserve( param->clip_->getDestWidth()+3 );
#endif
		tjs_uint32* midbits = &dstwork[0];	// �r�������p�o�b�t�@
		for( int y = param->start_; y < param->end_; y++ ) {
			param->sampler_->samplingVertical( y, workbits, dstheight, srcwidth, src, srcrect, wstarty );
			param->sampler_->samplingHorizontal( midbits, param->clip_->offsetx_, param->clip_->width_, workbits ); // �ꎞ�o�b�t�@�ɂ܂��R�s�[, �͈͊O�͏������Ȃ�
			(*param->blendfunc_)( dstbits, midbits, param->clip_->getDestWidth() );
			dstbits += dststride;
		}
	}
}

void TVPBicubicResampleSSE2Fix( const tTVPResampleClipping &clip, const tTVPImageCopyFuncBase* blendfunc, tTVPBaseBitmap *dest, const tTVPRect &destrect, const tTVPBaseBitmap *src, const tTVPRect &srcrect, float sharpness ) {
	BicubicWeightSSE weightfunc(sharpness);
	ResamplerSSE2Fix sampler;
	sampler.ResampleMT( clip, blendfunc, dest, destrect, src, srcrect, BicubicWeightSSE::RANGE, weightfunc );
}
void TVPBicubicResampleSSE2( const tTVPResampleClipping &clip, const tTVPImageCopyFuncBase* blendfunc, tTVPBaseBitmap *dest, const tTVPRect &destrect, const tTVPBaseBitmap *src, const tTVPRect &srcrect, float sharpness ) {
	BicubicWeightSSE weightfunc(sharpness);
	ResamplerSSE2 sampler;
	sampler.ResampleMT( clip, blendfunc, dest, destrect, src, srcrect, BicubicWeightSSE::RANGE, weightfunc );
}

void TVPAreaAvgResampleSSE2Fix( const tTVPResampleClipping &clip, const tTVPImageCopyFuncBase* blendfunc, tTVPBaseBitmap *dest, const tTVPRect &destrect, const tTVPBaseBitmap *src, const tTVPRect &srcrect ) {
	ResamplerSSE2Fix sampler;
	sampler.ResampleAreaAvgMT( clip, blendfunc, dest, destrect, src, srcrect );
}
void TVPAreaAvgResampleSSE2( const tTVPResampleClipping &clip, const tTVPImageCopyFuncBase* blendfunc, tTVPBaseBitmap *dest, const tTVPRect &destrect, const tTVPBaseBitmap *src, const tTVPRect &srcrect ) {
	ResamplerSSE2 sampler;
	sampler.ResampleAreaAvgMT( clip, blendfunc, dest, destrect, src, srcrect );
}

template<typename TWeightFunc>
void TVPWeightResampleSSE2Fix( const tTVPResampleClipping &clip, const tTVPImageCopyFuncBase* blendfunc, tTVPBaseBitmap *dest, const tTVPRect &destrect, const tTVPBaseBitmap *src, const tTVPRect &srcrect ) {
	TWeightFunc weightfunc;
	ResamplerSSE2Fix sampler;
	sampler.ResampleMT( clip, blendfunc, dest, destrect, src, srcrect, TWeightFunc::RANGE, weightfunc );
}

template<typename TWeightFunc>
void TVPWeightResampleSSE2( const tTVPResampleClipping &clip, const tTVPImageCopyFuncBase* blendfunc, tTVPBaseBitmap *dest, const tTVPRect &destrect, const tTVPBaseBitmap *src, const tTVPRect &srcrect ) {
	TWeightFunc weightfunc;
	ResamplerSSE2 sampler;
	sampler.ResampleMT( clip, blendfunc, dest, destrect, src, srcrect, TWeightFunc::RANGE, weightfunc );
}
/**
 * �g��k������ SSE2 ��
 * @param dest : �������ݐ�摜
 * @param destrect : �������ݐ��`
 * @param src : �ǂݍ��݌��摜
 * @param srcrect : �ǂݍ��݌���`
 * @param type : �g��k���t�B���^�^�C�v
 * @param typeopt : �g��k���t�B���^�^�C�v�I�v�V����
 * @param method : �u�����h���@
 * @param opa : �s�����x
 * @param hda : �������ݐ�A���t�@�ێ�
 */
void TVPResampleImageSSE2( const tTVPResampleClipping &clip, const tTVPImageCopyFuncBase* blendfunc,
	tTVPBaseBitmap *dest, const tTVPRect &destrect, const tTVPBaseBitmap *src, const tTVPRect &srcrect,
	tTVPBBStretchType type, tjs_real typeopt ) {
	switch( type ) {
	case stLinear:
		TVPWeightResampleSSE2<BilinearWeightSSE>(clip, blendfunc, dest, destrect, src, srcrect );
		break;
	case stCubic:
		TVPBicubicResampleSSE2(clip, blendfunc, dest, destrect, src, srcrect, (float)typeopt );
		break;
	case stSemiFastLinear:
		TVPWeightResampleSSE2Fix<BilinearWeightSSE>(clip, blendfunc, dest, destrect, src, srcrect );
		break;
	case stFastCubic:
		TVPBicubicResampleSSE2Fix(clip, blendfunc, dest, destrect, src, srcrect, (float)typeopt );
		break;
	case stLanczos2:
		TVPWeightResampleSSE2<LanczosWeightSSE<2> >(clip, blendfunc, dest, destrect, src, srcrect );
		break;
	case stFastLanczos2:
		TVPWeightResampleSSE2Fix<LanczosWeightSSE<2> >(clip, blendfunc, dest, destrect, src, srcrect );
		break;
	case stLanczos3:
		TVPWeightResampleSSE2<LanczosWeightSSE<3> >(clip, blendfunc, dest, destrect, src, srcrect );
		break;
	case stFastLanczos3:
		TVPWeightResampleSSE2Fix<LanczosWeightSSE<3> >(clip, blendfunc, dest, destrect, src, srcrect );
		break;
	case stSpline16:
		TVPWeightResampleSSE2<Spline16WeightSSE>(clip, blendfunc, dest, destrect, src, srcrect );
		break;
	case stFastSpline16:
		TVPWeightResampleSSE2Fix<Spline16WeightSSE>(clip, blendfunc, dest, destrect, src, srcrect );
		break;
	case stSpline36:
		TVPWeightResampleSSE2<Spline36WeightSSE>(clip, blendfunc, dest, destrect, src, srcrect );
		break;
	case stFastSpline36:
		TVPWeightResampleSSE2Fix<Spline36WeightSSE>(clip, blendfunc, dest, destrect, src, srcrect );
		break;
	case stAreaAvg:
		TVPAreaAvgResampleSSE2(clip, blendfunc, dest, destrect, src, srcrect );
		break;
	case stFastAreaAvg:
		TVPAreaAvgResampleSSE2Fix(clip, blendfunc, dest, destrect, src, srcrect );
		break;
	case stGaussian:
		TVPWeightResampleSSE2<GaussianWeightSSE>(clip, blendfunc, dest, destrect, src, srcrect );
		break;
	case stFastGaussian:
		TVPWeightResampleSSE2Fix<GaussianWeightSSE>(clip, blendfunc, dest, destrect, src, srcrect );
		break;
	case stBlackmanSinc:
		TVPWeightResampleSSE2<BlackmanSincWeightSSE>(clip, blendfunc, dest, destrect, src, srcrect );
		break;
	case stFastBlackmanSinc:
		TVPWeightResampleSSE2Fix<BlackmanSincWeightSSE>(clip, blendfunc, dest, destrect, src, srcrect );
		break;
	default:
		throw L"Not supported yet.";
		break;
	}
}

