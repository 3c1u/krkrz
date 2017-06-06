
#ifndef __GL_TEXTURE_H__
#define __GL_TEXTURE_H__

#include "OpenGLHeader.h"

#ifdef ANDROID
extern bool TVPIsSupportGLES3();
#endif

class GLTexture {
protected:
	GLuint texture_id_;
	GLint format_;
	GLuint width_;
	GLuint height_;

public:
	GLTexture() : texture_id_(0), width_(0), height_(0), format_(0) {}
	GLTexture( GLuint w, GLuint h, const GLvoid* bits, GLint format=GL_RGBA ) : width_(w), height_(h) {
		create( w, h, bits, format );
	}
	~GLTexture() {
		destory();
	}

	void create( GLuint w, GLuint h, const GLvoid* bits, GLint format=GL_RGBA ) {
		glGenTextures( 1, &texture_id_ );
		glBindTexture( GL_TEXTURE_2D, texture_id_ );
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexImage2D( GL_TEXTURE_2D, 0, format, w, h, 0, format, GL_UNSIGNED_BYTE, bits );
		glBindTexture( GL_TEXTURE_2D, 0 );
		format_ = format;
		width_ = w;
		height_ = h;
	}
	void destory() {
		if( texture_id_ != 0 ) {
			glDeleteTextures( 1, &texture_id_ );
			texture_id_ = 0;
		}
	}
	void copyImage( GLint x, GLint y, GLint w, GLint h, const GLvoid* bits ) {
		 glTexSubImage2D( GL_TEXTURE_2D, 0, x, y, w, h, format_, GL_UNSIGNED_BYTE, bits );
	}
	static int getMaxTextureSize() {
		GLint maxTex;
		glGetIntegerv( GL_MAX_TEXTURE_SIZE, &maxTex );
		return maxTex;
	}
	GLuint width() const { return width_; }
	GLuint height() const { return height_; }
	GLuint id() const { return texture_id_; }
	GLint format() const { return format_; }
};


#endif // __GL_TEXTURE_H__