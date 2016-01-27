/******************************************************************************/
/**
 * �A���t�@�u�����h
 * ----------------------------------------------------------------------------
 * 	Copyright (C) T.Imoto <http://www.kaede-software.com>
 * ----------------------------------------------------------------------------
 * @author		T.Imoto
 * @date		2014/02/11
 * @note		
 * Additive alpha �́Apre-multiplied alpha �ƌ����\�L��
 *****************************************************************************/

#ifndef __ALPHA_BLEND_C_H__
#define __ALPHA_BLEND_C_H__

#include "blend_util_func.h"
#include "blend_variation.h"

#define DEFINE_BLEND_VARIATION( FUNC ) \
typedef normal_op<FUNC##_func >					FUNC##_functor;			\
typedef translucent_op<FUNC##_func >			FUNC##_o_functor;		\
typedef hda_op<FUNC##_func >					FUNC##_HDA_functor;		\
typedef hda_translucent_op<FUNC##_func >		FUNC##_HDA_o_functor;	\
typedef dest_alpha_op<FUNC##_func >				FUNC##_d_functor;		\
typedef dest_alpha_translucent_op<FUNC##_func >	FUNC##_do_functor;

/** 4�p�^�[���̃o�[�W���� */
#define DEFINE_BLEND_MIN_VARIATION( FUNC ) \
typedef translucent_nsa_op<FUNC##_func>			FUNC##_o_functor;		\
typedef hda_nsa_op<FUNC##_functor>				FUNC##_HDA_functor;		\
typedef hda_translucent_nsa_op<FUNC##_func>		FUNC##_HDA_o_functor;

/** 4�p�^�[����PS�n�o�[�W���� */
#define DEFINE_BLEND_PS_VARIATION( FUNC ) \
typedef normal_op<FUNC##_func >					FUNC##_functor;			\
typedef translucent_op<FUNC##_func >			FUNC##_o_functor;		\
typedef hda_op<FUNC##_func >					FUNC##_HDA_functor;		\
typedef hda_translucent_op<FUNC##_func >		FUNC##_HDA_o_functor;

extern tjs_uint32 TVPRecipTable256[256];
//------------------------------------------------------------------------------
/**
 * alpha_blend_functor;			// dst�A���t�@����
 * alpha_blend_HDA_functor;		// dst�A���t�@����, dst �A���t�@�ی�
 * alpha_blend_o_functor;		// dst�A���t�@����, opacity �w��
 * alpha_blend_HDA_o_functor;	// dst�A���t�@����, dst �A���t�@�ی�, opacity �w��
 *
 * alpha_blend_d_functor;		// dst�A���t�@�L��
 * alpha_blend_do_functor;		// dst�A���t�@�L��, opacity �w��
 *
 * alpha_blend_a_functor;		// dst�A���t�@�L��, addalpha(pre-multiplied alpha)
 * alpha_blend_ao_functor;		// dst�A���t�@�L��, opacity �w��, addalpha(pre-multiplied alpha)
 */
//------------------------------------------------------------------------------
/** �A���t�@�u�����h */
struct alpha_blend_func {
	inline tjs_uint32 operator()( tjs_uint32 d, tjs_uint32 s, tjs_uint32 a ) const {
		tjs_uint32 d1 = d & 0xff00ff;
		d1 = (d1 + (((s & 0xff00ff) - d1) * a >> 8)) & 0xff00ff;
		d &= 0xff00;
		s &= 0xff00;
		return d1 + ((d + ((s - d) * a >> 8)) & 0xff00);
	}
};
DEFINE_BLEND_VARIATION( alpha_blend )

//------------------------------------------------------------------------------
struct alpha_blend_a_functor {
	premulalpha_blend_a_d_func blend_;
	alpha_to_premulalpha_func topremulalpha_;
	inline tjs_uint32 operator()( tjs_uint32 d, tjs_uint32 s ) const {
		return blend_( d, topremulalpha_(s) );
	}
};
//------------------------------------------------------------------------------
// typedef premulalpha_blend_a_d_o_func alpha_blend_ao_functor;
struct alpha_blend_ao_functor : public premulalpha_blend_a_d_o_func {
	tjs_int opa_;
	//alpha_blend_a_functor blend_;
	inline alpha_blend_ao_functor( tjs_int opa ) : opa_(opa) {}
	inline tjs_uint32 operator()( tjs_uint32 d, tjs_uint32 s ) const {
		//s = (s & 0xffffff) + ((((s >> 24) * opa_) >> 8) << 24);
		//return blend_(d,s);
		return premulalpha_blend_a_d_o_func::operator()( d, s, opa_ );
	}
};
//------------------------------------------------------------------------------
/** pre-multiplied �A���t�@���� */
struct premulalpha_blend_func {
	premulalpha_blend_n_a_func add_alpha_;
	inline tjs_uint32 operator()( tjs_uint32 d, tjs_uint32 s, tjs_uint32 a ) const {
		s = (((s & 0xff00ff)*a >> 8) & 0xff00ff) + (((s >> 8) & 0xff00ff)*a & 0xff00ff00);
		return add_alpha_(d, s);
	}
};
DEFINE_BLEND_VARIATION( premulalpha_blend )

typedef premulalpha_blend_func premulalpha_blend_a_functor;
//struct premulalpha_blend_a_functor : public premulalpha_blend_func {};
typedef premulalpha_blend_a_d_func premulalpha_blend_ao_functor;
//struct premulalpha_blend_ao_functor {};

//------------------------------------------------------------------------------
/** ���Z���� */
typedef saturated_u8_add_func add_blend_functor;
struct add_blend_func : public add_blend_functor {
	inline tjs_uint32 operator()( tjs_uint32 d, tjs_uint32 s, tjs_uint32 a ) const {
		s = ( ((s&0x00ff00)  * a >> 8)&0x00ff00) + (( (s&0xff00ff) * a >> 8)&0xff00ff);
		return add_blend_functor::operator()( d, s );
	}
};
DEFINE_BLEND_MIN_VARIATION( add_blend )

//------------------------------------------------------------------------------
/** ���Z���� */ // �I���W�i����HDA������Ƃ��������H
struct sub_blend_functor {
	inline tjs_uint32 operator()( tjs_uint32 d, tjs_uint32 s ) const {
		tjs_uint32 tmp = (  ( s & d ) + ( ((s^d)>>1) & 0x7f7f7f7f)  ) & 0x80808080;
		tmp = (tmp << 1) - (tmp >> 7);
		return (s + d - tmp) & tmp;
	}
};
struct sub_blend_func : public sub_blend_functor {
	inline tjs_uint32 operator()( tjs_uint32 d, tjs_uint32 s, tjs_uint32 a ) const {
		s = ~s;
		s = ~ (( ((s&0x00ff00)  * a >> 8)&0x00ff00) + (( (s&0xff00ff) * a >> 8)&0xff00ff) );
		return sub_blend_functor::operator()( d, s );
	}
};
DEFINE_BLEND_MIN_VARIATION( sub_blend )

//------------------------------------------------------------------------------
/** ��Z���� */
struct mul_blend_functor {
	inline tjs_uint32 operator()( tjs_uint32 d, tjs_uint32 s ) const {
		tjs_uint32 tmp  = (d & 0xff) * (s & 0xff) & 0xff00;
		tmp |= ((d & 0xff00) >> 8) * (s & 0xff00) & 0xff0000;
		tmp |= ((d & 0xff0000) >> 16) * (s & 0xff0000) & 0xff000000;
		return tmp >> 8;
	}
};
struct mul_blend_func : public mul_blend_functor {
	inline tjs_uint32 operator()( tjs_uint32 d, tjs_uint32 s, tjs_uint32 a ) const {
		s = ~s;
		s = ~( ( ((s&0x00ff00)  * a >> 8)&0x00ff00) + (( (s&0xff00ff) * a >> 8)&0xff00ff));
		return mul_blend_functor::operator()( d, s );
	}
};
DEFINE_BLEND_MIN_VARIATION( mul_blend )

//------------------------------------------------------------------------------
/** �����Ă����� */
struct color_dodge_blend_functor {
	inline tjs_uint32 operator()( tjs_uint32 d, tjs_uint32 s ) const {
		tjs_uint32 tmp2 = ~s;
		tjs_uint32 tmp = (d& 0xff) * TVPRecipTable256[tmp2 & 0xff] >> 8;
		tjs_uint32 tmp3 = (tmp | ((tjs_int32)~(tmp - 0x100) >> 31)) & 0xff;
		tmp = ((d & 0xff00)>>8) * TVPRecipTable256[(tmp2 & 0xff00)>>8];
		tmp3 |= (tmp | ((tjs_int32)~(tmp - 0x10000) >> 31)) & 0xff00;
		tmp = ((d & 0xff0000)>>16) * TVPRecipTable256[(tmp2 & 0xff0000)>>16];
		tmp3 |= ((tmp | ((tjs_int32)~(tmp - 0x10000) >> 31)) & 0xff00 ) << 8;
		return tmp3;
	}
};
struct color_dodge_blend_func : public color_dodge_blend_functor {
	inline tjs_uint32 operator()( tjs_uint32 d, tjs_uint32 s, tjs_uint32 a ) const {
		s = (( ((s&0x00ff00)  * a >> 8)&0x00ff00) + (( (s&0xff00ff) * a >> 8)&0xff00ff) );
		return color_dodge_blend_functor::operator()( d, s );
	}
};
DEFINE_BLEND_MIN_VARIATION( color_dodge_blend )

//------------------------------------------------------------------------------
/** ��r(��)���� */
struct darken_blend_functor {
	inline tjs_uint32 operator()( tjs_uint32 d, tjs_uint32 s ) const {
		tjs_uint32 m_src = ~s;
		tjs_uint32 tmp = ((m_src & d) + (((m_src ^ d) >> 1) & 0x7f7f7f7f) ) & 0x80808080;
		tmp = (tmp << 1) - (tmp >> 7);
		d ^= (d ^ s) & tmp;
		return d;
	}
};
struct darken_blend_func {
	inline tjs_uint32 operator()( tjs_uint32 d, tjs_uint32 s, tjs_uint32 a ) const {
		tjs_uint32 m_src = ~s;
		tjs_uint32 tmp = ((m_src & d) + (((m_src ^ d) >> 1) & 0x7f7f7f7f) ) & 0x80808080;
		tmp = (tmp << 1) - (tmp >> 7);
		tmp = d ^ ((d ^ s) & tmp);
		tjs_uint32 d1 = d & 0xff00ff;
		d1 = (d1 + (((tmp & 0xff00ff) - d1) * a >> 8)) & 0xff00ff;
		m_src = d & 0xff00;
		tmp &= 0xff00;
		return d1 + ((m_src + ((tmp - m_src) * a >> 8)) & 0xff00);
	}
};
DEFINE_BLEND_MIN_VARIATION( darken_blend )

//------------------------------------------------------------------------------
/** ��r(��)���� */
struct lighten_blend_functor {
	inline tjs_uint32 operator()( tjs_uint32 d, tjs_uint32 s ) const {
		tjs_uint32 m_dest = ~d;
		tjs_uint32 tmp = ((s & m_dest) + (((s ^ m_dest) >> 1) & 0x7f7f7f7f) ) & 0x80808080;
		tmp = (tmp << 1) - (tmp >> 7);
		d ^= (d ^ s) & tmp;
		return d;
	}
};
struct lighten_blend_func {
	inline tjs_uint32 operator()( tjs_uint32 d, tjs_uint32 s, tjs_uint32 a ) const {
		tjs_uint32 m_dest = ~d;
		tjs_uint32 tmp = ((s & m_dest) + (((s ^ m_dest) >> 1) & 0x7f7f7f7f) ) & 0x80808080;
		tmp = (tmp << 1) - (tmp >> 7);
		tmp = d ^ ((d ^ s) & tmp);
		tjs_uint32 d1 = d & 0xff00ff;
		d1 = (d1 + (((tmp & 0xff00ff) - d1) * a >> 8)) & 0xff00ff;
		m_dest = d & 0xff00;
		tmp &= 0xff00;
		return d1 + ((m_dest + ((tmp - m_dest) * a >> 8)) & 0xff00);
	}
};
DEFINE_BLEND_MIN_VARIATION( lighten_blend )

//------------------------------------------------------------------------------
/** �X�N���[������ */
struct screen_blend_functor {
	inline tjs_uint32 operator()( tjs_uint32 d, tjs_uint32 s ) const {
		s = ~s;
		d = ~d;
		tjs_uint32 tmp  = (d & 0xff) * (s & 0xff) & 0xff00;
		tmp |= ((d & 0xff00) >> 8) * (s & 0xff00) & 0xff0000;
		tmp |= ((d & 0xff0000) >> 16) * (s & 0xff0000) & 0xff000000;
		tmp >>= 8;
		return ~tmp;
	}
};
struct screen_blend_func : public screen_blend_functor {
	inline tjs_uint32 operator()( tjs_uint32 d, tjs_uint32 s, tjs_uint32 a ) const {
		s = ~( ( ((s&0x00ff00)  * a >> 8)&0x00ff00) + (( (s&0xff00ff) * a >> 8)&0xff00ff));
		return screen_blend_functor::operator()( d, s );
	}
};
DEFINE_BLEND_MIN_VARIATION( screen_blend )

//------------------------------------------------------------------------------
/** Photoshop�݊��̃A���t�@���� */
struct ps_alpha_blend_func {
	inline tjs_uint32 operator()( tjs_uint32 d, tjs_uint32 s, tjs_uint32 a ) const {
		tjs_uint32 d1 = d & 0x00ff00ff;
		tjs_uint32 d2 = d & 0x0000ff00;
		return ((((((s&0x00ff00ff)-d1)*a)>>8)+d1)&0x00ff00ff) | ((((((s&0x0000ff00)-d2)*a)>>8)+d2)&0x0000ff00);
	}
};
DEFINE_BLEND_PS_VARIATION( ps_alpha_blend )

/** Photoshop�݊��́u�����Ă�(���j�A)�v���� */
struct ps_add_blend_func : public ps_alpha_blend_func {
	ps_alpha_blend_func func;
	inline tjs_uint32 operator()( tjs_uint32 d, tjs_uint32 s, tjs_uint32 a ) const {
		tjs_uint32		n;
		n = (((d&s)<<1)+((d^s)&0x00fefefe))&0x01010100;
		n = ((n>>8)+0x007f7f7f)^0x007f7f7f;
		s = (d+s-n)|n;
		return ps_alpha_blend_func::operator()( d, s, a );
	}
};
DEFINE_BLEND_PS_VARIATION( ps_add_blend )

//--------------------------------------------------------------------------------------------------------
//! Photoshop�݊��́u�Ă�����(���j�A)�v����
struct ps_sub_blend_func : public ps_alpha_blend_func {
	inline tjs_uint32 operator()( tjs_uint32 d, tjs_uint32 s, tjs_uint32 a ) const {
		tjs_uint32		n;
		s = ~s;
		n = (((~d&s)<<1)+((~d^s)&0x00fefefe))&0x01010100;
		n = ((n>>8)+0x007f7f7f)^0x007f7f7f;
		s = (d|n)-(s|n);
		return ps_alpha_blend_func::operator()( d, s, a );
	}
};
DEFINE_BLEND_PS_VARIATION( ps_sub_blend )

//--------------------------------------------------------------------------------------------------------
//! Photoshop�݊��́u��Z�v����
struct ps_mul_blend_func : public ps_alpha_blend_func {
	inline tjs_uint32 operator()( tjs_uint32 d, tjs_uint32 s, tjs_uint32 a ) const {
		s = ( ((((d>>16)&0xff)*(s&0x00ff0000))&0xff000000) |
			  ((((d>>8 )&0xff)*(s&0x0000ff00))&0x00ff0000) |
			  ((((d>>0 )&0xff)*(s&0x000000ff))           ) ) >> 8;
		return ps_alpha_blend_func::operator()( d, s, a );
	}
};
DEFINE_BLEND_PS_VARIATION( ps_mul_blend )

//--------------------------------------------------------------------------------------------------------
//! Photoshop�݊��́u�X�N���[���v����
struct ps_screen_blend_func {
	inline tjs_uint32 operator()( tjs_uint32 d, tjs_uint32 s, tjs_uint32 a ) const {
		/* c = ((s+d-(s*d)/255)-d)*a + d = (s-(s*d)/255)*a + d */
		tjs_uint32 sd1, sd2;
		sd1 = ( ((((d>>16)&0xff)*(s&0x00ff0000))&0xff000000) |
				((((d>>0 )&0xff)*(s&0x000000ff))           ) ) >> 8;
		sd2 = ( ((((d>>8 )&0xff)*(s&0x0000ff00))&0x00ff0000) ) >> 8;
		return ((((((s&0x00ff00ff)-sd1)*a)>>8)+(d&0x00ff00ff))&0x00ff00ff) |
				((((((s&0x0000ff00)-sd2)*a)>>8)+(d&0x0000ff00))&0x0000ff00);
	}
};
DEFINE_BLEND_PS_VARIATION( ps_screen_blend )

//--------------------------------------------------------------------------------------------------------
//! �e�[�u���g�p����
template<typename TTable>
struct ps_table_blend_func : public ps_alpha_blend_func {
	inline tjs_uint32 operator()( tjs_uint32 d, tjs_uint32 s, tjs_uint32 a ) const {
		s = (TTable::TABLE[(s>>16)&0xff][(d>>16)&0xff]<<16) |
			(TTable::TABLE[(s>>8 )&0xff][(d>>8 )&0xff]<<8 ) |
			(TTable::TABLE[(s>>0 )&0xff][(d>>0 )&0xff]<<0 );
		return ps_alpha_blend_func::operator()( d, s, a );
	}
};
//--------------------------------------------------------------------------------------------------------
//! Photoshop�݊��́u�\�t�g���C�g�v�����e�[�u��
struct ps_soft_light_table { static unsigned char TABLE[256][256]; };
//--------------------------------------------------------------------------------------------------------
//! Photoshop�݊��́u�����Ă��J���[�v�����e�[�u��
struct ps_color_dodge_table { static unsigned char TABLE[256][256]; };
//--------------------------------------------------------------------------------------------------------
//! Photoshop�݊��́u�Ă����݃J���[�v�����e�[�u��
struct ps_color_burn_table { static unsigned char TABLE[256][256]; };
//--------------------------------------------------------------------------------------------------------
#ifdef TVPPS_USE_OVERLAY_TABLE
//! Photoshop�݊��́u�I�[�o�[���C�v�����e�[�u��
struct ps_overlay_table { static unsigned char TABLE[256][256]; };
#endif

//--------------------------------------------------------------------------------------------------------
//! Photoshop�݊��́u�\�t�g���C�g�v����
//typedef ps_table_blend_func<ps_soft_light_table> ps_soft_light_blend_func
struct ps_soft_light_blend_func : public ps_table_blend_func<ps_soft_light_table> {};
DEFINE_BLEND_PS_VARIATION( ps_soft_light_blend )

//--------------------------------------------------------------------------------------------------------
//! Photoshop�݊��́u�����Ă��J���[�v����
//typedef ps_table_blend_func<ps_color_dodge_table> ps_color_dodge_blend_func
struct ps_color_dodge_blend_func : public ps_table_blend_func<ps_color_dodge_table> {};
DEFINE_BLEND_PS_VARIATION( ps_color_dodge_blend )

//--------------------------------------------------------------------------------------------------------
//! Photoshop�݊��́u�Ă����݃J���[�v����
//typedef ps_table_blend_func<ps_color_burn_table> ps_color_burn_blend_func
struct ps_color_burn_blend_func : public ps_table_blend_func<ps_color_burn_table> {};
DEFINE_BLEND_PS_VARIATION( ps_color_burn_blend )

#ifdef TVPPS_USE_OVERLAY_TABLE
//--------------------------------------------------------------------------------------------------------
//! Photoshop�݊��́u�I�[�o�[���C�v����
//typedef ps_table_blend_func<ps_overlay_table> ps_overlay_blend_func
struct ps_overlay_blend_func : public ps_table_blend_func<ps_overlay_table> {};
#else
struct ps_overlay_blend_func : public ps_alpha_blend_func {
	inline tjs_uint32 operator()( tjs_uint32 d, tjs_uint32 s, tjs_uint32 a ) const {
		tjs_uint32 n = (((d&0x00808080)>>7)+0x007f7f7f)^0x007f7f7f;
		tjs_uint32 sa1, sa2, d1 = d&n, s1 = s&n;
		/* some tricks to avoid overflow (error between /255 and >>8) */
		s |= 0x00010101;
		sa1 = ( ((((d>>16)&0xff)*(s&0x00ff0000))&0xff800000) |
				((((d>>0 )&0xff)*(s&0x000000ff))           ) ) >> 7;
		sa2 = ( ((((d>>8 )&0xff)*(s&0x0000ff00))&0x00ff8000) ) >> 7;
		s = ((sa1&~n)|(sa2&~n));
		s |= (((s1&0x00fe00fe)+(d1&0x00ff00ff))<<1)-(n&0x00ff00ff)-(sa1&n);
		s |= (((s1&0x0000fe00)+(d1&0x0000ff00))<<1)-(n&0x0000ff00)-(sa2&n);
		return ps_alpha_blend_func::operator()( d, s, a );
	}
};
#endif
DEFINE_BLEND_PS_VARIATION( ps_overlay_blend )

//--------------------------------------------------------------------------------------------------------
//! Photoshop�݊��́u�n�[�h���C�g�v����
struct ps_hard_light_blend_func : public ps_alpha_blend_func {
	inline tjs_uint32 operator()( tjs_uint32 d, tjs_uint32 s, tjs_uint32 a ) const {
#ifdef TVPPS_USE_OVERLAY_TABLE
		s = (ps_overlay_table::TABLE[(d>>16)&0xff][(s>>16)&0xff]<<16) |
			(ps_overlay_table::TABLE[(d>>8 )&0xff][(s>>8 )&0xff]<<8 ) |
			(ps_overlay_table::TABLE[(d>>0 )&0xff][(s>>0 )&0xff]<<0 );
#else
		tjs_uint32 n = (((s&0x00808080)>>7)+0x007f7f7f)^0x007f7f7f;
		tjs_uint32 sa1, sa2, d1 = d&n, s1 = s&n;
		/* some tricks to avoid overflow (error between /255 and >>8) */
		d |= 0x00010101;
		sa1 = ( ((((d>>16)&0xff)*(s&0x00ff0000))&0xff800000) |
                ((((d>>0 )&0xff)*(s&0x000000ff))           ) ) >> 7;
		sa2 = ( ((((d>>8 )&0xff)*(s&0x0000ff00))&0x00ff8000) ) >> 7;
		s = ((sa1&~n)|(sa2&~n));
		s |= (((s1&0x00ff00ff)+(d1&0x00fe00fe))<<1)-(n&0x00ff00ff)-(sa1&n);
		s |= (((s1&0x0000ff00)+(d1&0x0000fe00))<<1)-(n&0x0000ff00)-(sa2&n);
#endif
		return ps_alpha_blend_func::operator()( d, s, a );
	}
};
DEFINE_BLEND_PS_VARIATION( ps_hard_light_blend )

//--------------------------------------------------------------------------------------------------------
//! Photoshop�݊��́u�����Ă��J���[�v����(Photoshop 5.x �ȉ��ƌ݊�)
struct ps_color_dodge5_blend_func {
	inline tjs_uint32 operator()( tjs_uint32 d, tjs_uint32 s, tjs_uint32 a ) const {
		s = ((((s&0x00ff00ff)*a)>>8)&0x00ff00ff)|((((s&0x0000ff00)*a)>>8)&0x0000ff00);
		return	(ps_color_dodge_table::TABLE[(s>>16)&0xff][(d>>16)&0xff]<<16) |
				(ps_color_dodge_table::TABLE[(s>>8 )&0xff][(d>>8 )&0xff]<<8 ) |
				(ps_color_dodge_table::TABLE[(s>>0 )&0xff][(d>>0 )&0xff]<<0 );
	}
};
DEFINE_BLEND_PS_VARIATION( ps_color_dodge5_blend )

//--------------------------------------------------------------------------------------------------------
//! Photoshop�݊��́u��r(��)�v����
struct ps_lighten_blend_func : public ps_alpha_blend_func {
	inline tjs_uint32 operator()( tjs_uint32 d, tjs_uint32 s, tjs_uint32 a ) const {
		tjs_uint32	n;
		n = (((~d&s)<<1)+((~d^s)&0x00fefefe))&0x01010100;
		n = ((n>>8)+0x007f7f7f)^0x007f7f7f;
		/* n=mask (d<s:0xff, d>=s:0x00) */
		s = (s&n)|(d&~n);
		return ps_alpha_blend_func::operator()( d, s, a );
	}
};
DEFINE_BLEND_PS_VARIATION( ps_lighten_blend )

//--------------------------------------------------------------------------------------------------------
//! Photoshop�݊��́u��r(��)�v����
struct ps_darken_blend_func : public ps_alpha_blend_func {
	inline tjs_uint32 operator()( tjs_uint32 d, tjs_uint32 s, tjs_uint32 a ) const {
		tjs_uint32	n;
		n = (((~d&s)<<1)+((~d^s)&0x00fefefe))&0x01010100;
		n = ((n>>8)+0x007f7f7f)^0x007f7f7f;
		/* n=mask (d<s:0xff, d>=s:0x00) */
		s = (d&n)|(s&~n);
		return ps_alpha_blend_func::operator()( d, s, a );
	}
};
DEFINE_BLEND_PS_VARIATION( ps_darken_blend )

//--------------------------------------------------------------------------------------------------------
//! Photoshop�݊��́u���̐�Βl�v����
struct ps_diff_blend_func : public ps_alpha_blend_func {
	inline tjs_uint32 operator()( tjs_uint32 d, tjs_uint32 s, tjs_uint32 a ) const {
		tjs_uint32	n;
		n = (((~d&s)<<1)+((~d^s)&0x00fefefe))&0x01010100;
		n = ((n>>8)+0x007f7f7f)^0x007f7f7f;
		/* n=mask (d<s:0xff, d>=s:0x00) */
		s = ((s&n)-(d&n))|((d&~n)-(s&~n));
		return ps_alpha_blend_func::operator()( d, s, a );
	}
};
DEFINE_BLEND_PS_VARIATION( ps_diff_blend )

//--------------------------------------------------------------------------------------------------------
//! Photoshop�݊��́u���̐�Βl�v����(Photoshop 5.x �ȉ��ƌ݊�)
struct ps_diff5_blend_func {
	inline tjs_uint32 operator()( tjs_uint32 d, tjs_uint32 s, tjs_uint32 a ) const {
		s = ((((s&0x00ff00ff)*a)>>8)&0x00ff00ff)|((((s&0x0000ff00)*a)>>8)&0x0000ff00);	/* Fade src first */
		tjs_uint32	n;
		n = (((~d&s)<<1)+((~d^s)&0x00fefefe))&0x01010100;
		n = ((n>>8)+0x007f7f7f)^0x007f7f7f;
		/* n=mask (d<s:0xff, d>=s:0x00) */
		return ((s&n)-(d&n))|((d&~n)-(s&~n));
	}
};
DEFINE_BLEND_PS_VARIATION( ps_diff5_blend )

//--------------------------------------------------------------------------------------------------------
//! Photoshop�݊��́u���O�v����
struct ps_exclusion_blend_func {
	inline tjs_uint32 operator()( tjs_uint32 d, tjs_uint32 s, tjs_uint32 a ) const {
		/* c = ((s+d-(s*d*2)/255)-d)*a + d = (s-(s*d*2)/255)*a + d */
		tjs_uint32	sd1, sd2;
		sd1 = ( ((((d>>16)&0xff)*((s&0x00ff0000)>>7))&0x01ff0000) |
			((((d>>0 )&0xff)*( s&0x000000ff    ))>>7        ) );
		sd2 = ( ((((d>>8 )&0xff)*(s&0x0000ff00))&0x00ff8000) ) >> 7;
		return	((((((s&0x00ff00ff)-sd1)*a)>>8)+(d&0x00ff00ff))&0x00ff00ff) |
				((((((s&0x0000ff00)-sd2)*a)>>8)+(d&0x0000ff00))&0x0000ff00);
	}
};
DEFINE_BLEND_PS_VARIATION( ps_exclusion_blend )


#endif // ___ALPHA_BLEND_C_H__
