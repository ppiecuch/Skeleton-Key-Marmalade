/*
	Jonathan Dummer
	2007-07-26-10.36

	Simple OpenGL Image Library

	Public Domain
	using Sean Barret's stb_image as a base

	Thanks to:
	* Sean Barret - for the awesome stb_image
	* Dan Venkitachalam - for finding some non-compliant DDS files, and patching some explicit casts
	* everybody at gamedev.net
*/

#include <SDL_opengl.h>
#if defined(__SYMBIAN32__) || defined(SHP) || defined(__ANDROID__) || defined(__QNXNTO__)
# include <EGL/egl.h>
#endif

#if defined(DEBUG) || defined(_DEBUG)
	#define SOIL_CHECK_FOR_GL_ERRORS 1
#endif

#define ErrorLog(fmt, ...) fprintf(stderr, fmt, ##__VA_ARGS__);fprintf(stderr, "\n")

#include "SOIL.h"
#include "stb_image.h"
#include "image_helper.h"
#include "image_DXT.h"

#include <stdlib.h>
#include <string.h>

static inline uint8_t clamp_8bit(int val)
{
   if (val > 255)
      return 255;
   else if (val < 0)
      return 0;
   else
      return (uint8_t)val;
}

inline static BOOL is_pow2(int a) { return((a & -a) == a); };

#if defined(SCALER_NO_SIMD) || !defined(WIN32)
#undef __SSE2__
#endif

#if defined(__SSE2__)
#include <emmintrin.h>
#endif

// default clamp mode:
#define GL_CLAMP_MODE GL_CLAMP_TO_EDGE

/*	error reporting	*/
const char *result_string_pointer = "SOIL initialized";

/*	for loading cube maps	*/
enum{
	SOIL_CAPABILITY_UNKNOWN = -1,
	SOIL_CAPABILITY_NONE = 0,
	SOIL_CAPABILITY_PRESENT = 1
};
static int has_cubemap_capability = SOIL_CAPABILITY_UNKNOWN;
int query_cubemap_capability( void );
#define SOIL_TEXTURE_WRAP_R					0x8072
#define SOIL_CLAMP_TO_EDGE					0x812F
#define SOIL_NORMAL_MAP						0x8511
#define SOIL_REFLECTION_MAP					0x8512
#define SOIL_TEXTURE_CUBE_MAP                   0x8513
#define SOIL_TEXTURE_BINDING_CUBE_MAP		0x8514
#define SOIL_TEXTURE_CUBE_MAP_POSITIVE_X	0x8515
#define SOIL_TEXTURE_CUBE_MAP_NEGATIVE_X	0x8516
#define SOIL_TEXTURE_CUBE_MAP_POSITIVE_Y	0x8517
#define SOIL_TEXTURE_CUBE_MAP_NEGATIVE_Y	0x8518
#define SOIL_TEXTURE_CUBE_MAP_POSITIVE_Z	0x8519
#define SOIL_TEXTURE_CUBE_MAP_NEGATIVE_Z	0x851A
#define SOIL_PROXY_TEXTURE_CUBE_MAP		0x851B
#define SOIL_MAX_CUBE_MAP_TEXTURE_SIZE		0x851C
/*	for non-power-of-two texture	*/
static int has_NPOT_capability = SOIL_CAPABILITY_UNKNOWN;
int query_NPOT_capability( void );
/*	for texture rectangles	*/
static int has_tex_rectangle_capability = SOIL_CAPABILITY_UNKNOWN;
int query_tex_rectangle_capability( void );
#define SOIL_TEXTURE_RECTANGLE_ARB			0x84F5
#define SOIL_MAX_RECTANGLE_TEXTURE_SIZE_ARB		0x84F8
/*	for using DXT compression	*/
static int has_DXT_capability = SOIL_CAPABILITY_UNKNOWN;
int query_DXT_capability( void );
#define SOIL_RGB_S3TC_DXT1		0x83F0
#define SOIL_RGBA_S3TC_DXT1		0x83F1
#define SOIL_RGBA_S3TC_DXT3		0x83F2
#define SOIL_RGBA_S3TC_DXT5		0x83F3
typedef void (APIENTRY * P_SOIL_GLCOMPRESSEDTEXIMAGE2DPROC) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid * data);
P_SOIL_GLCOMPRESSEDTEXIMAGE2DPROC soilGlCompressedTexImage2D = NULL;
unsigned int SOIL_direct_load_DDS(
		const char *filename,
		unsigned int reuse_texture_ID,
		int flags,
		int loading_as_cubemap );
unsigned int SOIL_direct_load_DDS_from_memory(
		const unsigned char *const buffer,
		int buffer_length,
		unsigned int reuse_texture_ID,
		int flags,
		int loading_as_cubemap );
/*	other functions	*/
unsigned int
	SOIL_internal_create_OGL_texture
	(
		const unsigned char *const data,
		int &width, int &height, int channels,
		unsigned int reuse_texture_ID,
		unsigned int flags,
		unsigned int opengl_texture_type,
		unsigned int opengl_texture_target,
		unsigned int texture_check_size_enum
	);
/*	pixel coversion	*/
void conv_0rgb1555_argb8888(void *output, const void *input,
      int width, int height,
      int out_stride, int in_stride);

void conv_0rgb1555_rgb565(void *output, const void *input,
      int width, int height,
      int out_stride, int in_stride);

void conv_rgb565_0rgb1555(void *output, const void *input,
      int width, int height,
      int out_stride, int in_stride);

void conv_rgb565_argb8888(void *output, const void *input,
      int width, int height,
      int out_stride, int in_stride);

void conv_bgr24_argb8888(void *output, const void *input,
      int width, int height,
      int out_stride, int in_stride);

void conv_argb8888_0rgb1555(void *output, const void *input,
      int width, int height,
      int out_stride, int in_stride);

void conv_argb8888_rgb565(void *output, const void *input,
      int width, int height,
      int out_stride, int in_stride);

void conv_argb8888_bgr24(void *output, const void *input,
      int width, int height,
      int out_stride, int in_stride);

void conv_argb8888_abgr8888(void *output, const void *input,
      int width, int height,
      int out_stride, int in_stride);

void conv_0rgb1555_bgr24(void *output, const void *input,
      int width, int height,
      int out_stride, int in_stride);

void conv_rgb565_bgr24(void *output, const void *input,
      int width, int height,
      int out_stride, int in_stride);

void conv_yuyv_argb8888(void *output, const void *input,
      int width, int height,
      int out_stride, int in_stride);

void conv_copy(void *output, const void *input,
      int width, int height,
      int out_stride, int in_stride);

/*	and the code magic begins here [8^)	*/
unsigned int
	SOIL_load_OGL_texture
	(
		const char *filename,
		int force_channels,
		unsigned int reuse_texture_ID,
		unsigned int flags
	)
{
	/*	variables	*/
	unsigned char* img;
	int width, height, channels;
	unsigned int tex_id;
	/*	does the user want direct uploading of the image as a DDS file?	*/
	if( flags & SOIL_FLAG_DDS_LOAD_DIRECT )
	{
		/*	1st try direct loading of the image as a DDS file
			note: direct uploading will only load what is in the
			DDS file, no MIPmaps will be generated, the image will
			not be flipped, etc.	*/
		tex_id = SOIL_direct_load_DDS( filename, reuse_texture_ID, flags, 0 );
		if( tex_id )
		{
			/*	hey, it worked!!	*/
			return tex_id;
		}
	}
	/*	try to load the image	*/
	img = SOIL_load_image( filename, &width, &height, &channels, force_channels );
	/*	channels holds the original number of channels, which may have been forced	*/
	if( (force_channels >= 1) && (force_channels <= 4) )
	{
		channels = force_channels;
	}
	if( NULL == img )
	{
		/*	image loading failed	*/
		result_string_pointer = stbi_failure_reason();
		return 0;
	}
	/*	OK, make it a texture!	*/
	tex_id = SOIL_internal_create_OGL_texture(
			img, width, height, channels,
			reuse_texture_ID, flags,
			GL_TEXTURE_2D, GL_TEXTURE_2D,
			GL_MAX_TEXTURE_SIZE );
	/*	and nuke the image data	*/
	SOIL_free_image_data( img );
	/*	and return the handle, such as it is	*/
	return tex_id;
}

unsigned int
	SOIL_load_OGL_HDR_texture
	(
		const char *filename,
		int fake_HDR_format,
		int rescale_to_max,
		unsigned int reuse_texture_ID,
		unsigned int flags
	)
{
	/*	variables	*/
	unsigned char* img;
	int width, height, channels;
	unsigned int tex_id;
	/*	no direct uploading of the image as a DDS file	*/
	/* error check */
	if( (fake_HDR_format != SOIL_HDR_RGBE) &&
		(fake_HDR_format != SOIL_HDR_RGBdivA) &&
		(fake_HDR_format != SOIL_HDR_RGBdivA2) )
	{
		result_string_pointer = "Invalid fake HDR format specified";
		return 0;
	}
	/*	try to load the image (only the HDR type) */
	img = stbi_hdr_load_rgbe( filename, &width, &height, &channels, 4 );
	/*	channels holds the original number of channels, which may have been forced	*/
	if( NULL == img )
	{
		/*	image loading failed	*/
		result_string_pointer = stbi_failure_reason();
		return 0;
	}
	/* the load worked, do I need to convert it? */
	if( fake_HDR_format == SOIL_HDR_RGBdivA )
	{
		RGBE_to_RGBdivA( img, width, height, rescale_to_max );
	} else if( fake_HDR_format == SOIL_HDR_RGBdivA2 )
	{
		RGBE_to_RGBdivA2( img, width, height, rescale_to_max );
	}
	/*	OK, make it a texture!	*/
	tex_id = SOIL_internal_create_OGL_texture(
			img, width, height, channels,
			reuse_texture_ID, flags,
			GL_TEXTURE_2D, GL_TEXTURE_2D,
			GL_MAX_TEXTURE_SIZE );
	/*	and nuke the image data	*/
	SOIL_free_image_data( img );
	/*	and return the handle, such as it is	*/
	return tex_id;
}

unsigned int
	SOIL_load_OGL_texture_from_memory
	(
		const unsigned char *const buffer,
		int buffer_length,
		int force_channels,
		unsigned int reuse_texture_ID,
		unsigned int flags
	)
{
	/*	variables	*/
	unsigned char* img;
	int width, height, channels;
	unsigned int tex_id;
	/*	does the user want direct uploading of the image as a DDS file?	*/
	if( flags & SOIL_FLAG_DDS_LOAD_DIRECT )
	{
		/*	1st try direct loading of the image as a DDS file
			note: direct uploading will only load what is in the
			DDS file, no MIPmaps will be generated, the image will
			not be flipped, etc.	*/
		tex_id = SOIL_direct_load_DDS_from_memory(
				buffer, buffer_length,
				reuse_texture_ID, flags, 0 );
		if( tex_id )
		{
			/*	hey, it worked!!	*/
			return tex_id;
		}
	}
	/*	try to load the image	*/
	img = SOIL_load_image_from_memory(
					buffer, buffer_length,
					&width, &height, &channels,
					force_channels );
	/*	channels holds the original number of channels, which may have been forced	*/
	if( (force_channels >= 1) && (force_channels <= 4) )
	{
		channels = force_channels;
	}
	if( NULL == img )
	{
		/*	image loading failed	*/
		result_string_pointer = stbi_failure_reason();
		return 0;
	}
	/*	OK, make it a texture!	*/
	tex_id = SOIL_internal_create_OGL_texture(
			img, width, height, channels,
			reuse_texture_ID, flags,
			GL_TEXTURE_2D, GL_TEXTURE_2D,
			GL_MAX_TEXTURE_SIZE );
	/*	and nuke the image data	*/
	SOIL_free_image_data( img );
	/*	and return the handle, such as it is	*/
	return tex_id;
}

unsigned int
	SOIL_load_OGL_cubemap
	(
		const char *x_pos_file,
		const char *x_neg_file,
		const char *y_pos_file,
		const char *y_neg_file,
		const char *z_pos_file,
		const char *z_neg_file,
		int force_channels,
		unsigned int reuse_texture_ID,
		unsigned int flags
	)
{
	/*	variables	*/
	unsigned char* img;
	int width, height, channels;
	unsigned int tex_id;
	/*	error checking	*/
	if( (x_pos_file == NULL) ||
		(x_neg_file == NULL) ||
		(y_pos_file == NULL) ||
		(y_neg_file == NULL) ||
		(z_pos_file == NULL) ||
		(z_neg_file == NULL) )
	{
		result_string_pointer = "Invalid cube map files list";
		return 0;
	}
	/*	capability checking	*/
	if( query_cubemap_capability() != SOIL_CAPABILITY_PRESENT )
	{
		result_string_pointer = "No cube map capability present";
		return 0;
	}
	/*	1st face: try to load the image	*/
	img = SOIL_load_image( x_pos_file, &width, &height, &channels, force_channels );
	/*	channels holds the original number of channels, which may have been forced	*/
	if( (force_channels >= 1) && (force_channels <= 4) )
	{
		channels = force_channels;
	}
	if( NULL == img )
	{
		/*	image loading failed	*/
		result_string_pointer = stbi_failure_reason();
		return 0;
	}
	/*	upload the texture, and create a texture ID if necessary	*/
	tex_id = SOIL_internal_create_OGL_texture(
			img, width, height, channels,
			reuse_texture_ID, flags,
			SOIL_TEXTURE_CUBE_MAP, SOIL_TEXTURE_CUBE_MAP_POSITIVE_X,
			SOIL_MAX_CUBE_MAP_TEXTURE_SIZE );
	/*	and nuke the image data	*/
	SOIL_free_image_data( img );
	/*	continue?	*/
	if( tex_id != 0 )
	{
		/*	1st face: try to load the image	*/
		img = SOIL_load_image( x_neg_file, &width, &height, &channels, force_channels );
		/*	channels holds the original number of channels, which may have been forced	*/
		if( (force_channels >= 1) && (force_channels <= 4) )
		{
			channels = force_channels;
		}
		if( NULL == img )
		{
			/*	image loading failed	*/
			result_string_pointer = stbi_failure_reason();
			return 0;
		}
		/*	upload the texture, but reuse the assigned texture ID	*/
		tex_id = SOIL_internal_create_OGL_texture(
				img, width, height, channels,
				tex_id, flags,
				SOIL_TEXTURE_CUBE_MAP, SOIL_TEXTURE_CUBE_MAP_NEGATIVE_X,
				SOIL_MAX_CUBE_MAP_TEXTURE_SIZE );
		/*	and nuke the image data	*/
		SOIL_free_image_data( img );
	}
	/*	continue?	*/
	if( tex_id != 0 )
	{
		/*	1st face: try to load the image	*/
		img = SOIL_load_image( y_pos_file, &width, &height, &channels, force_channels );
		/*	channels holds the original number of channels, which may have been forced	*/
		if( (force_channels >= 1) && (force_channels <= 4) )
		{
			channels = force_channels;
		}
		if( NULL == img )
		{
			/*	image loading failed	*/
			result_string_pointer = stbi_failure_reason();
			return 0;
		}
		/*	upload the texture, but reuse the assigned texture ID	*/
		tex_id = SOIL_internal_create_OGL_texture(
				img, width, height, channels,
				tex_id, flags,
				SOIL_TEXTURE_CUBE_MAP, SOIL_TEXTURE_CUBE_MAP_POSITIVE_Y,
				SOIL_MAX_CUBE_MAP_TEXTURE_SIZE );
		/*	and nuke the image data	*/
		SOIL_free_image_data( img );
	}
	/*	continue?	*/
	if( tex_id != 0 )
	{
		/*	1st face: try to load the image	*/
		img = SOIL_load_image( y_neg_file, &width, &height, &channels, force_channels );
		/*	channels holds the original number of channels, which may have been forced	*/
		if( (force_channels >= 1) && (force_channels <= 4) )
		{
			channels = force_channels;
		}
		if( NULL == img )
		{
			/*	image loading failed	*/
			result_string_pointer = stbi_failure_reason();
			return 0;
		}
		/*	upload the texture, but reuse the assigned texture ID	*/
		tex_id = SOIL_internal_create_OGL_texture(
				img, width, height, channels,
				tex_id, flags,
				SOIL_TEXTURE_CUBE_MAP, SOIL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
				SOIL_MAX_CUBE_MAP_TEXTURE_SIZE );
		/*	and nuke the image data	*/
		SOIL_free_image_data( img );
	}
	/*	continue?	*/
	if( tex_id != 0 )
	{
		/*	1st face: try to load the image	*/
		img = SOIL_load_image( z_pos_file, &width, &height, &channels, force_channels );
		/*	channels holds the original number of channels, which may have been forced	*/
		if( (force_channels >= 1) && (force_channels <= 4) )
		{
			channels = force_channels;
		}
		if( NULL == img )
		{
			/*	image loading failed	*/
			result_string_pointer = stbi_failure_reason();
			return 0;
		}
		/*	upload the texture, but reuse the assigned texture ID	*/
		tex_id = SOIL_internal_create_OGL_texture(
				img, width, height, channels,
				tex_id, flags,
				SOIL_TEXTURE_CUBE_MAP, SOIL_TEXTURE_CUBE_MAP_POSITIVE_Z,
				SOIL_MAX_CUBE_MAP_TEXTURE_SIZE );
		/*	and nuke the image data	*/
		SOIL_free_image_data( img );
	}
	/*	continue?	*/
	if( tex_id != 0 )
	{
		/*	1st face: try to load the image	*/
		img = SOIL_load_image( z_neg_file, &width, &height, &channels, force_channels );
		/*	channels holds the original number of channels, which may have been forced	*/
		if( (force_channels >= 1) && (force_channels <= 4) )
		{
			channels = force_channels;
		}
		if( NULL == img )
		{
			/*	image loading failed	*/
			result_string_pointer = stbi_failure_reason();
			return 0;
		}
		/*	upload the texture, but reuse the assigned texture ID	*/
		tex_id = SOIL_internal_create_OGL_texture(
				img, width, height, channels,
				tex_id, flags,
				SOIL_TEXTURE_CUBE_MAP, SOIL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
				SOIL_MAX_CUBE_MAP_TEXTURE_SIZE );
		/*	and nuke the image data	*/
		SOIL_free_image_data( img );
	}
	/*	and return the handle, such as it is	*/
	return tex_id;
}

unsigned int
	SOIL_load_OGL_cubemap_from_memory
	(
		const unsigned char *const x_pos_buffer,
		int x_pos_buffer_length,
		const unsigned char *const x_neg_buffer,
		int x_neg_buffer_length,
		const unsigned char *const y_pos_buffer,
		int y_pos_buffer_length,
		const unsigned char *const y_neg_buffer,
		int y_neg_buffer_length,
		const unsigned char *const z_pos_buffer,
		int z_pos_buffer_length,
		const unsigned char *const z_neg_buffer,
		int z_neg_buffer_length,
		int force_channels,
		unsigned int reuse_texture_ID,
		unsigned int flags
	)
{
	/*	variables	*/
	unsigned char* img;
	int width, height, channels;
	unsigned int tex_id;
	/*	error checking	*/
	if( (x_pos_buffer == NULL) ||
		(x_neg_buffer == NULL) ||
		(y_pos_buffer == NULL) ||
		(y_neg_buffer == NULL) ||
		(z_pos_buffer == NULL) ||
		(z_neg_buffer == NULL) )
	{
		result_string_pointer = "Invalid cube map buffers list";
		return 0;
	}
	/*	capability checking	*/
	if( query_cubemap_capability() != SOIL_CAPABILITY_PRESENT )
	{
		result_string_pointer = "No cube map capability present";
		return 0;
	}
	/*	1st face: try to load the image	*/
	img = SOIL_load_image_from_memory(
			x_pos_buffer, x_pos_buffer_length,
			&width, &height, &channels, force_channels );
	/*	channels holds the original number of channels, which may have been forced	*/
	if( (force_channels >= 1) && (force_channels <= 4) )
	{
		channels = force_channels;
	}
	if( NULL == img )
	{
		/*	image loading failed	*/
		result_string_pointer = stbi_failure_reason();
		return 0;
	}
	/*	upload the texture, and create a texture ID if necessary	*/
	tex_id = SOIL_internal_create_OGL_texture(
			img, width, height, channels,
			reuse_texture_ID, flags,
			SOIL_TEXTURE_CUBE_MAP, SOIL_TEXTURE_CUBE_MAP_POSITIVE_X,
			SOIL_MAX_CUBE_MAP_TEXTURE_SIZE );
	/*	and nuke the image data	*/
	SOIL_free_image_data( img );
	/*	continue?	*/
	if( tex_id != 0 )
	{
		/*	1st face: try to load the image	*/
		img = SOIL_load_image_from_memory(
				x_neg_buffer, x_neg_buffer_length,
				&width, &height, &channels, force_channels );
		/*	channels holds the original number of channels, which may have been forced	*/
		if( (force_channels >= 1) && (force_channels <= 4) )
		{
			channels = force_channels;
		}
		if( NULL == img )
		{
			/*	image loading failed	*/
			result_string_pointer = stbi_failure_reason();
			return 0;
		}
		/*	upload the texture, but reuse the assigned texture ID	*/
		tex_id = SOIL_internal_create_OGL_texture(
				img, width, height, channels,
				tex_id, flags,
				SOIL_TEXTURE_CUBE_MAP, SOIL_TEXTURE_CUBE_MAP_NEGATIVE_X,
				SOIL_MAX_CUBE_MAP_TEXTURE_SIZE );
		/*	and nuke the image data	*/
		SOIL_free_image_data( img );
	}
	/*	continue?	*/
	if( tex_id != 0 )
	{
		/*	1st face: try to load the image	*/
		img = SOIL_load_image_from_memory(
				y_pos_buffer, y_pos_buffer_length,
				&width, &height, &channels, force_channels );
		/*	channels holds the original number of channels, which may have been forced	*/
		if( (force_channels >= 1) && (force_channels <= 4) )
		{
			channels = force_channels;
		}
		if( NULL == img )
		{
			/*	image loading failed	*/
			result_string_pointer = stbi_failure_reason();
			return 0;
		}
		/*	upload the texture, but reuse the assigned texture ID	*/
		tex_id = SOIL_internal_create_OGL_texture(
				img, width, height, channels,
				tex_id, flags,
				SOIL_TEXTURE_CUBE_MAP, SOIL_TEXTURE_CUBE_MAP_POSITIVE_Y,
				SOIL_MAX_CUBE_MAP_TEXTURE_SIZE );
		/*	and nuke the image data	*/
		SOIL_free_image_data( img );
	}
	/*	continue?	*/
	if( tex_id != 0 )
	{
		/*	1st face: try to load the image	*/
		img = SOIL_load_image_from_memory(
				y_neg_buffer, y_neg_buffer_length,
				&width, &height, &channels, force_channels );
		/*	channels holds the original number of channels, which may have been forced	*/
		if( (force_channels >= 1) && (force_channels <= 4) )
		{
			channels = force_channels;
		}
		if( NULL == img )
		{
			/*	image loading failed	*/
			result_string_pointer = stbi_failure_reason();
			return 0;
		}
		/*	upload the texture, but reuse the assigned texture ID	*/
		tex_id = SOIL_internal_create_OGL_texture(
				img, width, height, channels,
				tex_id, flags,
				SOIL_TEXTURE_CUBE_MAP, SOIL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
				SOIL_MAX_CUBE_MAP_TEXTURE_SIZE );
		/*	and nuke the image data	*/
		SOIL_free_image_data( img );
	}
	/*	continue?	*/
	if( tex_id != 0 )
	{
		/*	1st face: try to load the image	*/
		img = SOIL_load_image_from_memory(
				z_pos_buffer, z_pos_buffer_length,
				&width, &height, &channels, force_channels );
		/*	channels holds the original number of channels, which may have been forced	*/
		if( (force_channels >= 1) && (force_channels <= 4) )
		{
			channels = force_channels;
		}
		if( NULL == img )
		{
			/*	image loading failed	*/
			result_string_pointer = stbi_failure_reason();
			return 0;
		}
		/*	upload the texture, but reuse the assigned texture ID	*/
		tex_id = SOIL_internal_create_OGL_texture(
				img, width, height, channels,
				tex_id, flags,
				SOIL_TEXTURE_CUBE_MAP, SOIL_TEXTURE_CUBE_MAP_POSITIVE_Z,
				SOIL_MAX_CUBE_MAP_TEXTURE_SIZE );
		/*	and nuke the image data	*/
		SOIL_free_image_data( img );
	}
	/*	continue?	*/
	if( tex_id != 0 )
	{
		/*	1st face: try to load the image	*/
		img = SOIL_load_image_from_memory(
				z_neg_buffer, z_neg_buffer_length,
				&width, &height, &channels, force_channels );
		/*	channels holds the original number of channels, which may have been forced	*/
		if( (force_channels >= 1) && (force_channels <= 4) )
		{
			channels = force_channels;
		}
		if( NULL == img )
		{
			/*	image loading failed	*/
			result_string_pointer = stbi_failure_reason();
			return 0;
		}
		/*	upload the texture, but reuse the assigned texture ID	*/
		tex_id = SOIL_internal_create_OGL_texture(
				img, width, height, channels,
				tex_id, flags,
				SOIL_TEXTURE_CUBE_MAP, SOIL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
				SOIL_MAX_CUBE_MAP_TEXTURE_SIZE );
		/*	and nuke the image data	*/
		SOIL_free_image_data( img );
	}
	/*	and return the handle, such as it is	*/
	return tex_id;
}

unsigned int
	SOIL_load_OGL_single_cubemap
	(
		const char *filename,
		const char face_order[6],
		int force_channels,
		unsigned int reuse_texture_ID,
		unsigned int flags
	)
{
	/*	variables	*/
	unsigned char* img;
	int width, height, channels, i;
	unsigned int tex_id = 0;
	/*	error checking	*/
	if( filename == NULL )
	{
		result_string_pointer = "Invalid single cube map file name";
		return 0;
	}
	/*	does the user want direct uploading of the image as a DDS file?	*/
	if( flags & SOIL_FLAG_DDS_LOAD_DIRECT )
	{
		/*	1st try direct loading of the image as a DDS file
			note: direct uploading will only load what is in the
			DDS file, no MIPmaps will be generated, the image will
			not be flipped, etc.	*/
		tex_id = SOIL_direct_load_DDS( filename, reuse_texture_ID, flags, 1 );
		if( tex_id )
		{
			/*	hey, it worked!!	*/
			return tex_id;
		}
	}
	/*	face order checking	*/
	for( i = 0; i < 6; ++i )
	{
		if( (face_order[i] != 'N') &&
			(face_order[i] != 'S') &&
			(face_order[i] != 'W') &&
			(face_order[i] != 'E') &&
			(face_order[i] != 'U') &&
			(face_order[i] != 'D') )
		{
			result_string_pointer = "Invalid single cube map face order";
			return 0;
		};
	}
	/*	capability checking	*/
	if( query_cubemap_capability() != SOIL_CAPABILITY_PRESENT )
	{
		result_string_pointer = "No cube map capability present";
		return 0;
	}
	/*	1st off, try to load the full image	*/
	img = SOIL_load_image( filename, &width, &height, &channels, force_channels );
	/*	channels holds the original number of channels, which may have been forced	*/
	if( (force_channels >= 1) && (force_channels <= 4) )
	{
		channels = force_channels;
	}
	if( NULL == img )
	{
		/*	image loading failed	*/
		result_string_pointer = stbi_failure_reason();
		return 0;
	}
	/*	now, does this image have the right dimensions?	*/
	if( (width != 6*height) &&
		(6*width != height) )
	{
		SOIL_free_image_data( img );
		result_string_pointer = "Single cubemap image must have a 6:1 ratio";
		return 0;
	}
	/*	try the image split and create	*/
	tex_id = SOIL_create_OGL_single_cubemap(
			img, width, height, channels,
			face_order, reuse_texture_ID, flags
			);
	/*	nuke the temporary image data and return the texture handle	*/
	SOIL_free_image_data( img );
	return tex_id;
}

unsigned int
	SOIL_load_OGL_single_cubemap_from_memory
	(
		const unsigned char *const buffer,
		int buffer_length,
		const char face_order[6],
		int force_channels,
		unsigned int reuse_texture_ID,
		unsigned int flags
	)
{
	/*	variables	*/
	unsigned char* img;
	int width, height, channels, i;
	unsigned int tex_id = 0;
	/*	error checking	*/
	if( buffer == NULL )
	{
		result_string_pointer = "Invalid single cube map buffer";
		return 0;
	}
	/*	does the user want direct uploading of the image as a DDS file?	*/
	if( flags & SOIL_FLAG_DDS_LOAD_DIRECT )
	{
		/*	1st try direct loading of the image as a DDS file
			note: direct uploading will only load what is in the
			DDS file, no MIPmaps will be generated, the image will
			not be flipped, etc.	*/
		tex_id = SOIL_direct_load_DDS_from_memory(
				buffer, buffer_length,
				reuse_texture_ID, flags, 1 );
		if( tex_id )
		{
			/*	hey, it worked!!	*/
			return tex_id;
		}
	}
	/*	face order checking	*/
	for( i = 0; i < 6; ++i )
	{
		if( (face_order[i] != 'N') &&
			(face_order[i] != 'S') &&
			(face_order[i] != 'W') &&
			(face_order[i] != 'E') &&
			(face_order[i] != 'U') &&
			(face_order[i] != 'D') )
		{
			result_string_pointer = "Invalid single cube map face order";
			return 0;
		};
	}
	/*	capability checking	*/
	if( query_cubemap_capability() != SOIL_CAPABILITY_PRESENT )
	{
		result_string_pointer = "No cube map capability present";
		return 0;
	}
	/*	1st off, try to load the full image	*/
	img = SOIL_load_image_from_memory(
			buffer, buffer_length,
			&width, &height, &channels,
			force_channels );
	/*	channels holds the original number of channels, which may have been forced	*/
	if( (force_channels >= 1) && (force_channels <= 4) )
	{
		channels = force_channels;
	}
	if( NULL == img )
	{
		/*	image loading failed	*/
		result_string_pointer = stbi_failure_reason();
		return 0;
	}
	/*	now, does this image have the right dimensions?	*/
	if( (width != 6*height) &&
		(6*width != height) )
	{
		SOIL_free_image_data( img );
		result_string_pointer = "Single cubemap image must have a 6:1 ratio";
		return 0;
	}
	/*	try the image split and create	*/
	tex_id = SOIL_create_OGL_single_cubemap(
			img, width, height, channels,
			face_order, reuse_texture_ID, flags
			);
	/*	nuke the temporary image data and return the texture handle	*/
	SOIL_free_image_data( img );
	return tex_id;
}

unsigned int
	SOIL_create_OGL_single_cubemap
	(
		const unsigned char *const data,
		int width, int height, int channels,
		const char face_order[6],
		unsigned int reuse_texture_ID,
		unsigned int flags
	)
{
	/*	variables	*/
	unsigned char* sub_img;
	int dw, dh, sz, i;
	unsigned int tex_id;
	/*	error checking	*/
	if( data == NULL )
	{
		result_string_pointer = "Invalid single cube map image data";
		return 0;
	}
	/*	face order checking	*/
	for( i = 0; i < 6; ++i )
	{
		if( (face_order[i] != 'N') &&
			(face_order[i] != 'S') &&
			(face_order[i] != 'W') &&
			(face_order[i] != 'E') &&
			(face_order[i] != 'U') &&
			(face_order[i] != 'D') )
		{
			result_string_pointer = "Invalid single cube map face order";
			return 0;
		};
	}
	/*	capability checking	*/
	if( query_cubemap_capability() != SOIL_CAPABILITY_PRESENT )
	{
		result_string_pointer = "No cube map capability present";
		return 0;
	}
	/*	now, does this image have the right dimensions?	*/
	if( (width != 6*height) &&
		(6*width != height) )
	{
		result_string_pointer = "Single cubemap image must have a 6:1 ratio";
		return 0;
	}
	/*	which way am I stepping?	*/
	if( width > height )
	{
		dw = height;
		dh = 0;
	} else
	{
		dw = 0;
		dh = width;
	}
	sz = dw+dh;
	sub_img = (unsigned char *)malloc( sz*sz*channels );
	/*	do the splitting and uploading	*/
	tex_id = reuse_texture_ID;
	for( i = 0; i < 6; ++i )
	{
		int x, y, idx = 0;
		unsigned int cubemap_target = 0;
		/*	copy in the sub-image	*/
		for( y = i*dh; y < i*dh+sz; ++y )
		{
			for( x = i*dw*channels; x < (i*dw+sz)*channels; ++x )
			{
				sub_img[idx++] = data[y*width*channels+x];
			}
		}
		/*	what is my texture target?
			remember, this coordinate system is
			LHS if viewed from inside the cube!	*/
		switch( face_order[i] )
		{
		case 'N':
			cubemap_target = SOIL_TEXTURE_CUBE_MAP_POSITIVE_Z;
			break;
		case 'S':
			cubemap_target = SOIL_TEXTURE_CUBE_MAP_NEGATIVE_Z;
			break;
		case 'W':
			cubemap_target = SOIL_TEXTURE_CUBE_MAP_NEGATIVE_X;
			break;
		case 'E':
			cubemap_target = SOIL_TEXTURE_CUBE_MAP_POSITIVE_X;
			break;
		case 'U':
			cubemap_target = SOIL_TEXTURE_CUBE_MAP_POSITIVE_Y;
			break;
		case 'D':
			cubemap_target = SOIL_TEXTURE_CUBE_MAP_NEGATIVE_Y;
			break;
		}
		/*	upload it as a texture	*/
		tex_id = SOIL_internal_create_OGL_texture(
				sub_img, sz, sz, channels,
				tex_id, flags,
				SOIL_TEXTURE_CUBE_MAP,
				cubemap_target,
				SOIL_MAX_CUBE_MAP_TEXTURE_SIZE );
	}
	/*	and nuke the image and sub-image data	*/
	SOIL_free_image_data( sub_img );
	/*	and return the handle, such as it is	*/
	return tex_id;
}

unsigned int
	SOIL_create_OGL_texture
	(
		const unsigned char *const data,
		const int width, const int height, int channels,
		unsigned int reuse_texture_ID,
		unsigned int flags
	 ) {
	int w = width, h = height;
	return SOIL_create_OGL_texture2( data, w, h, channels, reuse_texture_ID, flags );
}
unsigned int
	SOIL_create_OGL_texture2
	(
		const unsigned char *const data,
		int &width, int &height, int channels,
		unsigned int reuse_texture_ID,
		unsigned int flags
	)
{
	/*	wrapper function for 2D textures	*/
	return SOIL_internal_create_OGL_texture(
				data, width, height, channels,
				reuse_texture_ID, flags,
				GL_TEXTURE_2D, GL_TEXTURE_2D,
				GL_MAX_TEXTURE_SIZE );
}

#ifdef SOIL_CHECK_FOR_GL_ERRORS

	void check_for_GL_errors( const char *calling_location ) {
		/*	check for errors	*/
		GLenum err_code = glGetError();
		while( GL_NO_ERROR != err_code ) {
		  fprintf( stderr, "[SOIL] OpenGL error @ %s: %i\n", calling_location, err_code );
		  err_code = glGetError();
		}
	}

#else

	inline void check_for_GL_errors( const char *calling_location ) {
		/*	no check for errors	*/
	}

#endif

unsigned int
	SOIL_internal_create_OGL_texture
	(
		const unsigned char *const data,
		int &width, int &height, int channels,
		unsigned int reuse_texture_ID,
		unsigned int flags,
		unsigned int opengl_texture_type,
		unsigned int opengl_texture_target,
		unsigned int texture_check_size_enum
	)
{
	/*	variables	*/
	unsigned char* img;
	unsigned int tex_id;
	unsigned int internal_texture_format = 0, original_texture_format = 0;
	int DXT_mode = SOIL_CAPABILITY_UNKNOWN;
	int max_supported_size;
	/*	If the user wants to use the texture rectangle I kill a few flags	*/
	if( flags & SOIL_FLAG_TEXTURE_RECTANGLE )
	{
		/*	well, the user asked for it, can we do that?	*/
		if( query_tex_rectangle_capability() == SOIL_CAPABILITY_PRESENT )
		{
			/*	only allow this if the user in _NOT_ trying to do a cubemap!	*/
			if( opengl_texture_type == GL_TEXTURE_2D )
			{
				/*	clean out the flags that cannot be used with texture rectangles	*/
				flags &= ~(
						SOIL_FLAG_POWER_OF_TWO | SOIL_FLAG_MIPMAPS |
						SOIL_FLAG_TEXTURE_REPEATS
					);
				/*	and change my target	*/
				opengl_texture_target = SOIL_TEXTURE_RECTANGLE_ARB;
				opengl_texture_type = SOIL_TEXTURE_RECTANGLE_ARB;
			} else
			{
				/*	not allowed for any other uses (yes, I'm looking at you, cubemaps!)	*/
				flags &= ~SOIL_FLAG_TEXTURE_RECTANGLE;
			}

		} else
		{
			/*	can't do it, and that is a breakable offense (uv coords use pixels instead of [0,1]!)	*/
			result_string_pointer = "Texture Rectangle extension unsupported";
			return 0;
		}
	}
	/*	create a copy the image data	*/
	img = (unsigned char*)malloc( width*height*channels );
	memcpy( img, data, width*height*channels );
	/*	does the user want me to invert the image?	*/
	if( flags & SOIL_FLAG_INVERT_Y )
	{
		int i, j;
		for( j = 0; j*2 < height; ++j )
		{
			int index1 = j * width * channels;
			int index2 = (height - 1 - j) * width * channels;
			for( i = width * channels; i > 0; --i )
			{
				unsigned char temp = img[index1];
				img[index1] = img[index2];
				img[index2] = temp;
				++index1;
				++index2;
			}
		}
	}
	/*	does the user want me to scale the colors into the NTSC safe RGB range?	*/
	if( flags & SOIL_FLAG_NTSC_SAFE_RGB )
	{
		scale_image_RGB_to_NTSC_safe( img, width, height, channels );
	}
	/*	does the user want me to convert from straight to pre-multiplied alpha?
		(and do we even _have_ alpha?)	*/
	if( flags & SOIL_FLAG_MULTIPLY_ALPHA )
	{
		int i;
		switch( channels )
		{
		case 2:
			for( i = 0; i < 2*width*height; i += 2 )
			{
				img[i] = (img[i] * img[i+1] + 128) >> 8;
			}
			break;
		case 4:
			for( i = 0; i < 4*width*height; i += 4 )
			{
				img[i+0] = (img[i+0] * img[i+3] + 128) >> 8;
				img[i+1] = (img[i+1] * img[i+3] + 128) >> 8;
				img[i+2] = (img[i+2] * img[i+3] + 128) >> 8;
			}
			break;
		default:
			/*	no other number of channels contains alpha data	*/
			break;
		}
	}
	/*	if the user can't support NPOT textures, make sure we force the POT option	*/
	if( (query_NPOT_capability() == SOIL_CAPABILITY_NONE) &&
		!(flags & SOIL_FLAG_TEXTURE_RECTANGLE) )
	{
		/*	add in the POT flag */
		flags |= SOIL_FLAG_POWER_OF_TWO;
	}
	/*	how large of a texture can this OpenGL implementation handle?	*/
	/*	texture_check_size_enum will be GL_MAX_TEXTURE_SIZE or SOIL_MAX_CUBE_MAP_TEXTURE_SIZE	*/
	glGetIntegerv( texture_check_size_enum, &max_supported_size );
	/*	do I need to make it a power of 2?	*/
	if(
		(flags & SOIL_FLAG_POWER_OF_TWO) ||	/*	user asked for it	*/
		(flags & SOIL_FLAG_MIPMAPS) ||		/*	need it for the MIP-maps	*/
		(width > max_supported_size) ||		/*	it's too big, (make sure it's	*/
		(height > max_supported_size) )		/*	2^n for later down-sampling)	*/
	{
		int new_width = 1;
		int new_height = 1;
		while( new_width < width )
		{
			new_width *= 2;
		}
		while( new_height < height )
		{
			new_height *= 2;
		}
		/*	still?	*/
		if( (new_width != width) || (new_height != height) )
		{
			/*	yep, resize	*/
			unsigned char *resampled = (unsigned char*)malloc( channels*new_width*new_height );
			up_scale_image(
					img, width, height, channels,
					resampled, new_width, new_height );
			/*	OJO	this is for debug only!	*/
			/*
			SOIL_save_image( "\\showme.bmp", SOIL_SAVE_TYPE_BMP,
							new_width, new_height, channels,
							resampled );
			*/
			/*	nuke the old guy, then point it at the new guy	*/
			SOIL_free_image_data( img );
			img = resampled;
			width = new_width;
			height = new_height;
			
			printf("[SOIL] resampled image to: %dx%d.\n", width, height);
		}
	}
	/*	now, if it is too large...	*/
	if( (width > max_supported_size) || (height > max_supported_size) )
	{
		/*	I've already made it a power of two, so simply use the MIPmapping
			code to reduce its size to the allowable maximum.	*/
		unsigned char *resampled;
		int reduce_block_x = 1, reduce_block_y = 1;
		int new_width, new_height;
		if( width > max_supported_size )
		{
			reduce_block_x = width / max_supported_size;
		}
		if( height > max_supported_size )
		{
			reduce_block_y = height / max_supported_size;
		}
		new_width = width / reduce_block_x;
		new_height = height / reduce_block_y;
		resampled = (unsigned char*)malloc( channels*new_width*new_height );
		/*	perform the actual reduction	*/
		mipmap_image(	img, width, height, channels,
						resampled, reduce_block_x, reduce_block_y );
		/*	nuke the old guy, then point it at the new guy	*/
		SOIL_free_image_data( img );
		img = resampled;
		width = new_width;
		height = new_height;
	}
	/*	does the user want us to use YCoCg color space?	*/
	if( flags & SOIL_FLAG_CoCg_Y )
	{
		/*	this will only work with RGB and RGBA images */
		convert_RGB_to_YCoCg( img, width, height, channels );
		//save_image_as_DDS( "CoCg_Y.dds", width, height, channels, img );
	}
	/*	create the OpenGL texture ID handle
    	(note: allowing a forced texture ID lets me reload a texture)	*/
    tex_id = reuse_texture_ID;
    if( tex_id == 0 )
    {
		glGenTextures( 1, &tex_id );
    }
	check_for_GL_errors( "glGenTextures" );
	/* Note: sometimes glGenTextures fails (usually no OpenGL context)	*/
	if( tex_id )
	{
		/*	and what type am I using as the internal texture format?	*/
		switch( channels )
		{
		case 1:
			original_texture_format = GL_LUMINANCE;
			if (flags & SOIL_FLAG_INVERT_ONECHANNEL) {
				// yes - revert it (pawelp):
				for( int i = 0; i < width*height; i++ ) {
					img[i] = ~img[i]; 
				}
			}

			break;
		case 2:
			original_texture_format = GL_LUMINANCE_ALPHA;
			break;
		case 3:
			original_texture_format = GL_RGB;
			break;
		case 4:
			original_texture_format = GL_RGBA;
			break;
		}
		// pawelp - mess a bit with flags and internal format
		if (channels == 1 && (flags & SOIL_FLAG_ONECHANNEL_AS_ALPHA))
			internal_texture_format = GL_ALPHA;
		else if (channels == 1 && (flags & SOIL_FLAG_ONECHANNEL_AS_INTENSITY))
			internal_texture_format = GL_LUMINANCE;
		else
			internal_texture_format = original_texture_format;

		/*	does the user want me to, and can I, save as DXT?	*/
		if( flags & SOIL_FLAG_COMPRESS_TO_DXT )
		{
			DXT_mode = query_DXT_capability();
			if( DXT_mode == SOIL_CAPABILITY_PRESENT )
			{
				/*	I can use DXT, whether I compress it or OpenGL does	*/
				if( (channels & 1) == 1 )
				{
					/*	1 or 3 channels = DXT1	*/
					internal_texture_format = SOIL_RGB_S3TC_DXT1;
				} else
				{
					/*	2 or 4 channels = DXT5	*/
					internal_texture_format = SOIL_RGBA_S3TC_DXT5;
				}
			}
		}
		/*  bind an OpenGL texture ID	*/
		glBindTexture( opengl_texture_type, tex_id );
		check_for_GL_errors( "glBindTexture" );
		/*  upload the main image	*/
		if( DXT_mode == SOIL_CAPABILITY_PRESENT )
		{
			/*	user wants me to do the DXT conversion!	*/
			int DDS_size;
			unsigned char *DDS_data = NULL;
			if( (channels & 1) == 1 )
			{
				/*	RGB, use DXT1	*/
				DDS_data = convert_image_to_DXT1( img, width, height, channels, &DDS_size );
			} else {
				/*	RGBA, use DXT5	*/
				DDS_data = convert_image_to_DXT5( img, width, height, channels, &DDS_size );
			}
			if( DDS_data )
			{
				soilGlCompressedTexImage2D(
					opengl_texture_target, 0,
					internal_texture_format, width, height, 0,
					DDS_size, DDS_data );
				check_for_GL_errors( "glCompressedTexImage2D" );
				SOIL_free_image_data( DDS_data );
			} else {
				/*	my compression failed, try the OpenGL driver's version	*/
				glTexImage2D(
					opengl_texture_target, 0,
					internal_texture_format, width, height, 0,
					original_texture_format, GL_UNSIGNED_BYTE, img );
				check_for_GL_errors( "glTexImage2D" );
				/*	printf( "OpenGL DXT compressor\n" );	*/
			}
		} else {
			/*	user want OpenGL to do all the work!	*/
			glTexImage2D(
				opengl_texture_target, 0,
				internal_texture_format, width, height, 0,
				original_texture_format, GL_UNSIGNED_BYTE, img );
			check_for_GL_errors( "glTexImage2D" );
			/*printf( "OpenGL DXT compressor\n" );	*/
		}
		/*	are any MIPmaps desired?	*/
		if( flags & SOIL_FLAG_MIPMAPS )
		{
			int MIPlevel = 1;
			int MIPwidth = (width+1) / 2;
			int MIPheight = (height+1) / 2;
			unsigned char *resampled = (unsigned char*)malloc( channels*MIPwidth*MIPheight );
			while( ((1<<MIPlevel) <= width) || ((1<<MIPlevel) <= height) )
			{
				/*	do this MIPmap level	*/
				mipmap_image(
						img, width, height, channels,
						resampled,
						(1 << MIPlevel), (1 << MIPlevel) );
				/*  upload the MIPmaps	*/
				if( DXT_mode == SOIL_CAPABILITY_PRESENT )
				{
					/*	user wants me to do the DXT conversion!	*/
					int DDS_size;
					unsigned char *DDS_data = NULL;
					if( (channels & 1) == 1 )
					{
						/*	RGB, use DXT1	*/
						DDS_data = convert_image_to_DXT1(
								resampled, MIPwidth, MIPheight, channels, &DDS_size );
					} else
					{
						/*	RGBA, use DXT5	*/
						DDS_data = convert_image_to_DXT5(
								resampled, MIPwidth, MIPheight, channels, &DDS_size );
					}
					if( DDS_data )
					{
						soilGlCompressedTexImage2D(
							opengl_texture_target, MIPlevel,
							internal_texture_format, MIPwidth, MIPheight, 0,
							DDS_size, DDS_data );
						check_for_GL_errors( "glCompressedTexImage2D" );
						SOIL_free_image_data( DDS_data );
					} else
					{
						/*	my compression failed, try the OpenGL driver's version	*/
						glTexImage2D(
							opengl_texture_target, MIPlevel,
							internal_texture_format, MIPwidth, MIPheight, 0,
							original_texture_format, GL_UNSIGNED_BYTE, resampled );
						check_for_GL_errors( "glTexImage2D" );
					}
				} else {
					/*	user want OpenGL to do all the work!	*/
					glTexImage2D(
						opengl_texture_target, MIPlevel,
						internal_texture_format, MIPwidth, MIPheight, 0,
						original_texture_format, GL_UNSIGNED_BYTE, resampled );
					check_for_GL_errors( "glTexImage2D" );
				}
				/*	prep for the next level	*/
				++MIPlevel;
				MIPwidth = (MIPwidth + 1) / 2;
				MIPheight = (MIPheight + 1) / 2;
			}
			SOIL_free_image_data( resampled );
			/*	instruct OpenGL to use the MIPmaps	*/
			glTexParameterf( opengl_texture_type, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR );
			glTexParameterf( opengl_texture_type, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
			check_for_GL_errors( "GL_TEXTURE_MIN/MAG_FILTER WITH MIPMAP" );
		} else {
			/*	instruct OpenGL _NOT_ to use the MIPmaps	*/
			glTexParameterf( opengl_texture_type, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
			glTexParameterf( opengl_texture_type, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
			check_for_GL_errors( "GL_TEXTURE_MIN/MAG_FILTER" );
		}

		#ifdef GL_TEXTURE_MAX_ANISOTROPY_EXT
			float degree;
			glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &degree);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, degree);
			check_for_GL_errors( "GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT" );
		#endif

		/*	does the user want clamping, or wrapping?	*/
			if( flags & SOIL_FLAG_TEXTURE_REPEATS 
			    // force clamp for npot textures (gl es):
			    && is_pow2(width) && is_pow2(height)
			    )
		{
			glTexParameterf( opengl_texture_type, GL_TEXTURE_WRAP_S, GL_REPEAT );
			glTexParameterf( opengl_texture_type, GL_TEXTURE_WRAP_T, GL_REPEAT );
			if( opengl_texture_type == SOIL_TEXTURE_CUBE_MAP ) {
				/*	SOIL_TEXTURE_WRAP_R is invalid if cubemaps aren't supported	*/
				glTexParameterf( opengl_texture_type, SOIL_TEXTURE_WRAP_R, GL_REPEAT );
			}
			check_for_GL_errors( "GL_TEXTURE_WRAP_*_REPEAT" );
		} else {
			if( flags & SOIL_FLAG_TEXTURE_REPEATS )
			  fprintf(stderr, "** Forcing CLAMP mode for npot texture size (%dx%d).\n", width, height);
			unsigned int clamp_mode = GL_CLAMP_MODE;
			glTexParameterf( opengl_texture_type, GL_TEXTURE_WRAP_S, clamp_mode );
			glTexParameterf( opengl_texture_type, GL_TEXTURE_WRAP_T, clamp_mode );
			if( opengl_texture_type == SOIL_TEXTURE_CUBE_MAP ) {
				/*	SOIL_TEXTURE_WRAP_R is invalid if cubemaps aren't supported	*/
				glTexParameterf( opengl_texture_type, SOIL_TEXTURE_WRAP_R, clamp_mode );
			}
			check_for_GL_errors( "GL_TEXTURE_WRAP_*_CLAMP" );
		}
		/*	done	*/
		result_string_pointer = "Image loaded as an OpenGL texture";
	} else
	{
		/*	failed	*/
		result_string_pointer = "Failed to generate an OpenGL texture name; missing OpenGL context?";
	}
	SOIL_free_image_data( img );
	return tex_id;
}

int
	SOIL_save_screenshot
	(
		const char *filename,
		int image_type,
		int x, int y,
		int width, int height
	)
{
	unsigned char *pixel_data;
	int i, j;
	int save_result;

	/*	error checks	*/
	if( (width < 1) || (height < 1) ) {
		result_string_pointer = "Invalid screenshot dimensions";
		return 0;
	}
	if( (x < 0) || (y < 0) ) {
		result_string_pointer = "Invalid screenshot location";
		return 0;
	}
	if( filename == NULL ) {
		result_string_pointer = "Invalid screenshot filename";
		return 0;
	}

    /*  Get the data from OpenGL	*/
    pixel_data = (unsigned char*)malloc( 3*width*height );
    glReadPixels (x, y, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixel_data);

    /*	invert the image	*/
    for( j = 0; j*2 < height; ++j ) {
		int index1 = j * width * 3;
		int index2 = (height - 1 - j) * width * 3;
		for( i = width * 3; i > 0; --i ) {
			unsigned char temp = pixel_data[index1];
			pixel_data[index1] = pixel_data[index2];
			pixel_data[index2] = temp;
			++index1;
			++index2;
		}
	}

    /*	save the image	*/
    save_result = SOIL_save_image( filename, image_type, width, height, 3, pixel_data);

    /*  And free the memory	*/
    SOIL_free_image_data( pixel_data );
	return save_result;
}

unsigned char*
	SOIL_load_image
	(
		const char *filename,
		int *width, int *height, int *channels,
		int force_channels
	)
{
	unsigned char *result = stbi_load( filename,
			width, height, channels, force_channels );
	if( result == NULL ) {
		result_string_pointer = stbi_failure_reason();
	} else {
		result_string_pointer = "Image loaded";
	}
	return result;
}

unsigned char*
	SOIL_load_image_from_memory
	(
		const unsigned char *const buffer,
		int buffer_length,
		int *width, int *height, int *channels,
		int force_channels
	)
{
	unsigned char *result = stbi_load_from_memory(
				buffer, buffer_length,
				width, height, channels,
				force_channels );
	if( result == NULL ) {
		result_string_pointer = stbi_failure_reason();
	} else {
		result_string_pointer = "Image loaded from memory";
	}
	return result;
}

int
	SOIL_save_image
	(
		const char *filename,
		int image_type,
		int width, int height, int channels,
		const unsigned char *const data
	)
{
	int save_result;

	/*	error check	*/
	if( (width < 1) || (height < 1) ||
		(channels < 1) || (channels > 4) ||
		(data == NULL) ||
		(filename == NULL) )
	{
		return 0;
	}
	if( image_type == SOIL_SAVE_TYPE_BMP ) {
		save_result = stbi_write_bmp( filename,
				width, height, channels, (void*)data );
	} else if( image_type == SOIL_SAVE_TYPE_TGA ) {
		save_result = stbi_write_tga( filename,
				width, height, channels, (void*)data );
	} else if( image_type == SOIL_SAVE_TYPE_DDS ) {
		save_result = save_image_as_DDS( filename,
				width, height, channels, (const unsigned char *const)data );
	} else {
		save_result = 0;
	}
	if( save_result == 0 ) {
		result_string_pointer = "Saving the image failed";
	} else {
		result_string_pointer = "Image saved";
	}
	return save_result;
}

void
	SOIL_free_image_data
	(
		unsigned char *img_data
	)
{
	free( (void*)img_data );
}

const char*
	SOIL_last_result
	(
		void
	)
{
	return result_string_pointer;
}

unsigned int SOIL_direct_load_DDS_from_memory(
		const unsigned char *const buffer,
		int buffer_length,
		unsigned int reuse_texture_ID,
		int flags,
		int loading_as_cubemap )
{
	/*	variables	*/
	DDS_header header;
	unsigned int buffer_index = 0;
	unsigned int tex_ID = 0;
	/*	file reading variables	*/
	unsigned int S3TC_type = 0;
	unsigned char *DDS_data;
	unsigned int DDS_main_size;
	unsigned int DDS_full_size;
	unsigned int width, height;
	int mipmaps, cubemap, uncompressed, block_size = 16;
	unsigned int flag;
	unsigned int cf_target, ogl_target_start, ogl_target_end;
	unsigned int opengl_texture_type;
	int i;
	/*	1st off, does the filename even exist?	*/
	if( NULL == buffer )
	{
		/*	we can't do it!	*/
		result_string_pointer = "NULL buffer";
		return 0;
	}
	if( buffer_length < sizeof( DDS_header ) )
	{
		/*	we can't do it!	*/
		result_string_pointer = "DDS file was too small to contain the DDS header";
		return 0;
	}
	/*	try reading in the header	*/
	memcpy ( (void*)(&header), (const void *)buffer, sizeof( DDS_header ) );
	buffer_index = sizeof( DDS_header );
	/*	guilty until proven innocent	*/
	result_string_pointer = "Failed to read a known DDS header";
	/*	validate the header (warning, "goto"'s ahead, shield your eyes!!)	*/
	flag = ('D'<<0)|('D'<<8)|('S'<<16)|(' '<<24);
	if( header.dwMagic != flag ) {goto quick_exit;}
	if( header.dwSize != 124 ) {goto quick_exit;}
	/*	I need all of these	*/
	flag = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
	if( (header.dwFlags & flag) != flag ) {goto quick_exit;}
	/*	According to the MSDN spec, the dwFlags should contain
		DDSD_LINEARSIZE if it's compressed, or DDSD_PITCH if
		uncompressed.  Some DDS writers do not conform to the
		spec, so I need to make my reader more tolerant	*/
	/*	I need one of these	*/
	flag = DDPF_FOURCC | DDPF_RGB;
	if( (header.sPixelFormat.dwFlags & flag) == 0 ) {goto quick_exit;}
	if( header.sPixelFormat.dwSize != 32 ) {goto quick_exit;}
	if( (header.sCaps.dwCaps1 & DDSCAPS_TEXTURE) == 0 ) {goto quick_exit;}
	/*	make sure it is a type we can upload	*/
	if( (header.sPixelFormat.dwFlags & DDPF_FOURCC) &&
		!(
		(header.sPixelFormat.dwFourCC == (('D'<<0)|('X'<<8)|('T'<<16)|('1'<<24))) ||
		(header.sPixelFormat.dwFourCC == (('D'<<0)|('X'<<8)|('T'<<16)|('3'<<24))) ||
		(header.sPixelFormat.dwFourCC == (('D'<<0)|('X'<<8)|('T'<<16)|('5'<<24)))
		) )
	{
		goto quick_exit;
	}
	/*	OK, validated the header, let's load the image data	*/
	result_string_pointer = "DDS header loaded and validated";
	width = header.dwWidth;
	height = header.dwHeight;
	uncompressed = 1 - (header.sPixelFormat.dwFlags & DDPF_FOURCC) / DDPF_FOURCC;
	cubemap = (header.sCaps.dwCaps2 & DDSCAPS2_CUBEMAP) / DDSCAPS2_CUBEMAP;
	if( uncompressed )
	{
		S3TC_type = GL_RGB;
		block_size = 3;
		if( header.sPixelFormat.dwFlags & DDPF_ALPHAPIXELS )
		{
			S3TC_type = GL_RGBA;
			block_size = 4;
		}
		DDS_main_size = width * height * block_size;
	} else
	{
		/*	can we even handle direct uploading to OpenGL DXT compressed images?	*/
		if( query_DXT_capability() != SOIL_CAPABILITY_PRESENT )
		{
			/*	we can't do it!	*/
			result_string_pointer = "Direct upload of S3TC images not supported by the OpenGL driver";
			return 0;
		}
		/*	well, we know it is DXT1/3/5, because we checked above	*/
		switch( (header.sPixelFormat.dwFourCC >> 24) - '0' )
		{
		case 1:
			S3TC_type = SOIL_RGBA_S3TC_DXT1;
			block_size = 8;
			break;
		case 3:
			S3TC_type = SOIL_RGBA_S3TC_DXT3;
			block_size = 16;
			break;
		case 5:
			S3TC_type = SOIL_RGBA_S3TC_DXT5;
			block_size = 16;
			break;
		}
		DDS_main_size = ((width+3)>>2)*((height+3)>>2)*block_size;
	}
	if( cubemap )
	{
		/* does the user want a cubemap?	*/
		if( !loading_as_cubemap )
		{
			/*	we can't do it!	*/
			result_string_pointer = "DDS image was a cubemap";
			return 0;
		}
		/*	can we even handle cubemaps with the OpenGL driver?	*/
		if( query_cubemap_capability() != SOIL_CAPABILITY_PRESENT )
		{
			/*	we can't do it!	*/
			result_string_pointer = "Direct upload of cubemap images not supported by the OpenGL driver";
			return 0;
		}
		ogl_target_start = SOIL_TEXTURE_CUBE_MAP_POSITIVE_X;
		ogl_target_end =   SOIL_TEXTURE_CUBE_MAP_NEGATIVE_Z;
		opengl_texture_type = SOIL_TEXTURE_CUBE_MAP;
	} else
	{
		/* does the user want a non-cubemap?	*/
		if( loading_as_cubemap )
		{
			/*	we can't do it!	*/
			result_string_pointer = "DDS image was not a cubemap";
			return 0;
		}
		ogl_target_start = GL_TEXTURE_2D;
		ogl_target_end =   GL_TEXTURE_2D;
		opengl_texture_type = GL_TEXTURE_2D;
	}
	if( (header.sCaps.dwCaps1 & DDSCAPS_MIPMAP) && (header.dwMipMapCount > 1) )
	{
		int shift_offset;
		mipmaps = header.dwMipMapCount - 1;
		DDS_full_size = DDS_main_size;
		if( uncompressed )
		{
			/*	uncompressed DDS, simple MIPmap size calculation	*/
			shift_offset = 0;
		} else
		{
			/*	compressed DDS, MIPmap size calculation is block based	*/
			shift_offset = 2;
		}
		for( i = 1; i <= mipmaps; ++ i )
		{
			int w, h;
			w = width >> (shift_offset + i);
			h = height >> (shift_offset + i);
			if( w < 1 )
			{
				w = 1;
			}
			if( h < 1 )
			{
				h = 1;
			}
			DDS_full_size += w*h*block_size;
		}
	} else
	{
		mipmaps = 0;
		DDS_full_size = DDS_main_size;
	}
	DDS_data = (unsigned char*)malloc( DDS_full_size );
	/*	got the image data RAM, create or use an existing OpenGL texture handle	*/
	tex_ID = reuse_texture_ID;
	if( tex_ID == 0 )
	{
		glGenTextures( 1, &tex_ID );
	}
	/*  bind an OpenGL texture ID	*/
	glBindTexture( opengl_texture_type, tex_ID );
	/*	do this for each face of the cubemap!	*/
	for( cf_target = ogl_target_start; cf_target <= ogl_target_end; ++cf_target )
	{
		if( buffer_index + DDS_full_size <= buffer_length )
		{
			unsigned int byte_offset = DDS_main_size;
			memcpy( (void*)DDS_data, (const void*)(&buffer[buffer_index]), DDS_full_size );
			buffer_index += DDS_full_size;
			/*	upload the main chunk	*/
			if( uncompressed )
			{
				/*	and remember, DXT uncompressed uses BGR(A),
					so swap to RGB(A) for ALL MIPmap levels	*/
				for( i = 0; i < DDS_full_size; i += block_size )
				{
					unsigned char temp = DDS_data[i];
					DDS_data[i] = DDS_data[i+2];
					DDS_data[i+2] = temp;
				}
				glTexImage2D(
					cf_target, 0,
					S3TC_type, width, height, 0,
					S3TC_type, GL_UNSIGNED_BYTE, DDS_data );
			} else
			{
				soilGlCompressedTexImage2D(
					cf_target, 0,
					S3TC_type, width, height, 0,
					DDS_main_size, DDS_data );
			}
			/*	upload the mipmaps, if we have them	*/
			for( i = 1; i <= mipmaps; ++i )
			{
				int w, h, mip_size;
				w = width >> i;
				h = height >> i;
				if( w < 1 )
				{
					w = 1;
				}
				if( h < 1 )
				{
					h = 1;
				}
				/*	upload this mipmap	*/
				if( uncompressed )
				{
					mip_size = w*h*block_size;
					glTexImage2D(
						cf_target, i,
						S3TC_type, w, h, 0,
						S3TC_type, GL_UNSIGNED_BYTE, &DDS_data[byte_offset] );
				} else
				{
					mip_size = ((w+3)/4)*((h+3)/4)*block_size;
					soilGlCompressedTexImage2D(
						cf_target, i,
						S3TC_type, w, h, 0,
						mip_size, &DDS_data[byte_offset] );
				}
				/*	and move to the next mipmap	*/
				byte_offset += mip_size;
			}
			/*	it worked!	*/
			result_string_pointer = "DDS file loaded";
		} else
		{
			::glDeleteTextures( 1, & tex_ID );
			tex_ID = 0;
			cf_target = ogl_target_end + 1;
			result_string_pointer = "DDS file was too small for expected image data";
		}
	}/* end reading each face */
	SOIL_free_image_data( DDS_data );
	if( tex_ID )
	{
		/*	did I have MIPmaps?	*/
		if( mipmaps > 0 )
		{
			/*	instruct OpenGL to use the MIPmaps	*/
			glTexParameterf( opengl_texture_type, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
			glTexParameterf( opengl_texture_type, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
		} else
		{
			/*	instruct OpenGL _NOT_ to use the MIPmaps	*/
			glTexParameterf( opengl_texture_type, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
			glTexParameterf( opengl_texture_type, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		}
		/*	does the user want clamping, or wrapping?	*/
		if( flags & SOIL_FLAG_TEXTURE_REPEATS )
		{
			glTexParameterf( opengl_texture_type, GL_TEXTURE_WRAP_S, GL_REPEAT );
			glTexParameterf( opengl_texture_type, GL_TEXTURE_WRAP_T, GL_REPEAT );
			glTexParameterf( opengl_texture_type, SOIL_TEXTURE_WRAP_R, GL_REPEAT );
		} else {
			unsigned int clamp_mode = GL_CLAMP_MODE;
			glTexParameterf( opengl_texture_type, GL_TEXTURE_WRAP_S, clamp_mode );
			glTexParameterf( opengl_texture_type, GL_TEXTURE_WRAP_T, clamp_mode );
			glTexParameterf( opengl_texture_type, SOIL_TEXTURE_WRAP_R, clamp_mode );
		}
	}

quick_exit:
	/*	report success or failure	*/
	return tex_ID;
}

unsigned int SOIL_direct_load_DDS(
		const char *filename,
		unsigned int reuse_texture_ID,
		int flags,
		int loading_as_cubemap )
{
	FILE *f;
	unsigned char *buffer;
	size_t buffer_length, bytes_read;
	unsigned int tex_ID = 0;
	/*	error checks	*/
	if( NULL == filename )
	{
		result_string_pointer = "NULL filename";
		return 0;
	}
	f = fopen( filename, "rb" );
	if( NULL == f )
	{
		/*	the file doesn't seem to exist (or be open-able)	*/
		result_string_pointer = "Can not find DDS file";
		return 0;
	}
	fseek( f, 0, SEEK_END );
	buffer_length = ftell( f );
	fseek( f, 0, SEEK_SET );
	buffer = (unsigned char *) malloc( buffer_length );
	if( NULL == buffer )
	{
		result_string_pointer = "malloc failed";
		fclose( f );
		return 0;
	}
	bytes_read = fread( (void*)buffer, 1, buffer_length, f );
	fclose( f );
	if( bytes_read < buffer_length )
	{
		/*	huh?	*/
		buffer_length = bytes_read;
	}
	/*	now try to do the loading	*/
	tex_ID = SOIL_direct_load_DDS_from_memory(
		(const unsigned char *const)buffer, buffer_length,
		reuse_texture_ID, flags, loading_as_cubemap );
	SOIL_free_image_data( buffer );
	return tex_ID;
}

int query_NPOT_capability( void )
{
	/*	check for the capability	*/
	if( has_NPOT_capability == SOIL_CAPABILITY_UNKNOWN )
	{
		/*	we haven't yet checked for the capability, do so	*/
		if(
			(NULL == strstr( (char const*)glGetString( GL_EXTENSIONS ),
				"GL_ARB_texture_non_power_of_two" ) )
			)
		{
			/*	not there, flag the failure	*/
			has_NPOT_capability = SOIL_CAPABILITY_NONE;
		} else {
			/*	it's there!	*/
			has_NPOT_capability = SOIL_CAPABILITY_PRESENT;
		}
	}
	/*	let the user know if we can do non-power-of-two textures or not	*/
	return has_NPOT_capability;
}

int query_tex_rectangle_capability( void )
{
	/*	check for the capability	*/
	if( has_tex_rectangle_capability == SOIL_CAPABILITY_UNKNOWN )
	{
		/*	we haven't yet checked for the capability, do so	*/
		if(
			(NULL == strstr( (char const*)glGetString( GL_EXTENSIONS ),
				"GL_ARB_texture_rectangle" ) )
		&&
			(NULL == strstr( (char const*)glGetString( GL_EXTENSIONS ),
				"GL_EXT_texture_rectangle" ) )
		&&
			(NULL == strstr( (char const*)glGetString( GL_EXTENSIONS ),
				"GL_NV_texture_rectangle" ) )
			)
		{
			/*	not there, flag the failure	*/
			has_tex_rectangle_capability = SOIL_CAPABILITY_NONE;
		} else
		{
			/*	it's there!	*/
			has_tex_rectangle_capability = SOIL_CAPABILITY_PRESENT;
		}
	}
	/*	let the user know if we can do texture rectangles or not	*/
	return has_tex_rectangle_capability;
}

int query_cubemap_capability( void )
{
	/*	check for the capability	*/
	if( has_cubemap_capability == SOIL_CAPABILITY_UNKNOWN )
	{
		/*	we haven't yet checked for the capability, do so	*/
		if(
			(NULL == strstr( (char const*)glGetString( GL_EXTENSIONS ),
				"GL_ARB_texture_cube_map" ) )
		&&
			(NULL == strstr( (char const*)glGetString( GL_EXTENSIONS ),
				"GL_EXT_texture_cube_map" ) )
			)
		{
			/*	not there, flag the failure	*/
			has_cubemap_capability = SOIL_CAPABILITY_NONE;
		} else
		{
			/*	it's there!	*/
			has_cubemap_capability = SOIL_CAPABILITY_PRESENT;
		}
	}
	/*	let the user know if we can do cubemaps or not	*/
	return has_cubemap_capability;
}

int query_DXT_capability( void )
{
	/*	check for the capability	*/
	if( has_DXT_capability == SOIL_CAPABILITY_UNKNOWN )
	{
		/*	we haven't yet checked for the capability, do so	*/
		if( NULL == strstr(
				(char const*)glGetString( GL_EXTENSIONS ),
				"GL_EXT_texture_compression_s3tc" ) )
		{
			/*	not there, flag the failure	*/
			has_DXT_capability = SOIL_CAPABILITY_NONE;
		} else {
			/*	and find the address of the extension function	*/
			P_SOIL_GLCOMPRESSEDTEXIMAGE2DPROC ext_addr = NULL;
			#if defined(__APPLE__) || defined(__APPLE_CC__)
				/*	I can't test this Apple stuff!	*/
				CFBundleRef bundle;
				CFURLRef bundleURL =
					CFURLCreateWithFileSystemPath(
						kCFAllocatorDefault,
						CFSTR("/System/Library/Frameworks/OpenGL.framework"),
						kCFURLPOSIXPathStyle,
						true );
				CFStringRef extensionName =
					CFStringCreateWithCString(
						kCFAllocatorDefault,
						"glCompressedTexImage2DARB",
						kCFStringEncodingASCII );
				bundle = CFBundleCreate( kCFAllocatorDefault, bundleURL );
				assert( bundle != NULL );
				ext_addr = (P_SOIL_GLCOMPRESSEDTEXIMAGE2DPROC)
						CFBundleGetFunctionPointerForName
						(
							bundle, extensionName
						);
				CFRelease( bundleURL );
				CFRelease( extensionName );
				CFRelease( bundle );
			#elif defined(__SYMBIAN32__) || defined(SHP) || defined(__ANDROID__) || defined(__QNXNTO__)
				ext_addr = (P_SOIL_GLCOMPRESSEDTEXIMAGE2DPROC)
					eglGetProcAddress
					(
							"glCompressedTexImage2DARB"
					);
			#elif defined(WIN32)
				ext_addr = (P_SOIL_GLCOMPRESSEDTEXIMAGE2DPROC)
					wglGetProcAddress
					(
							"glCompressedTexImage2DARB"
					);
			#else
				ext_addr = (P_SOIL_GLCOMPRESSEDTEXIMAGE2DPROC)
						glXGetProcAddressARB
						(
							(const GLubyte *)"glCompressedTexImage2DARB"
						);
			#endif
			/*	Flag it so no checks needed later	*/
			if( NULL == ext_addr )
			{
				/*	hmm, not good!!  This should not happen, but does on my
					laptop's VIA chipset.  The GL_EXT_texture_compression_s3tc
					spec requires that ARB_texture_compression be present too.
					this means I can upload and have the OpenGL drive do the
					conversion, but I can't use my own routines or load DDS files
					from disk and upload them directly [8^(	*/
				has_DXT_capability = SOIL_CAPABILITY_NONE;
			} else {
				/*	all's well!	*/
				soilGlCompressedTexImage2D = ext_addr;
				has_DXT_capability = SOIL_CAPABILITY_PRESENT;
			}
		}
	}
	/*	let the user know if we can do DXT or not	*/
	return has_DXT_capability;
}

#if defined(__SSE2_)
void conv_rgb565_0rgb1555(void *output_, const void *input_,
      int width, int height,
      int out_stride, int in_stride)
{
   int h, w;
   const uint16_t *input = (const uint16_t*)input_;
   uint16_t *output = (uint16_t*)output_;

   int max_width = width - 7;

   const __m128i hi_mask   = _mm_set1_epi16(0x7fe0);
   const __m128i lo_mask   = _mm_set1_epi16(0x1f);

   for (h = 0; h < height; h++, output += out_stride >> 1, input += in_stride >> 1)
   {
      for (w = 0; w < max_width; w += 8)
      {
         const __m128i in = _mm_loadu_si128((const __m128i*)(input + w));
         __m128i hi = _mm_and_si128(_mm_slli_epi16(in, 1), hi_mask);
         __m128i lo = _mm_and_si128(in, lo_mask);
         _mm_storeu_si128((__m128i*)(output + w), _mm_or_si128(hi, lo));
      }

      for (; w < width; w++)
      {
         uint16_t col = input[w];
         uint16_t hi = (col >> 1) & 0x7fe0;
         uint16_t lo = col & 0x1f;
         output[w] = hi | lo;
      }
   }
}
#else
void conv_rgb565_0rgb1555(void *output_, const void *input_,
      int width, int height,
      int out_stride, int in_stride)
{
   int h, w;
   const uint16_t *input = (const uint16_t*)input_;
   uint16_t *output = (uint16_t*)output_;

   for (h = 0; h < height; h++, output += out_stride >> 1, input += in_stride >> 1)
   {
      for (w = 0; w < width; w++)
      {
         uint16_t col = input[w];
         uint16_t hi = (col >> 1) & 0x7fe0;
         uint16_t lo = col & 0x1f;
         output[w] = hi | lo;
      }
   }
}

#endif

#if defined(__SSE2__)
void conv_0rgb1555_rgb565(void *output_, const void *input_,
      int width, int height,
      int out_stride, int in_stride)
{
   int h, w;
   const uint16_t *input = (const uint16_t*)input_;
   uint16_t *output = (uint16_t*)output_;

   int max_width = width - 7;

   const __m128i hi_mask   = _mm_set1_epi16((int16_t)((0x1f << 11) | (0x1f << 6)));
   const __m128i lo_mask   = _mm_set1_epi16(0x1f);
   const __m128i glow_mask = _mm_set1_epi16(1 << 5);

   for (h = 0; h < height; h++, output += out_stride >> 1, input += in_stride >> 1)
   {
      for (w = 0; w < max_width; w += 8)
      {
         const __m128i in = _mm_loadu_si128((const __m128i*)(input + w));
         __m128i rg   = _mm_and_si128(_mm_slli_epi16(in, 1), hi_mask);
         __m128i b    = _mm_and_si128(in, lo_mask);
         __m128i glow = _mm_and_si128(_mm_srli_epi16(in, 4), glow_mask);
         _mm_storeu_si128((__m128i*)(output + w), _mm_or_si128(rg, _mm_or_si128(b, glow)));
      }

      for (; w < width; w++)
      {
         uint16_t col = input[w];
         uint16_t rg = (col << 1) & ((0x1f << 11) | (0x1f << 6));
         uint16_t b = col & 0x1f;
         uint16_t glow = (col >> 4) & (1 << 5);
         output[w] = rg | b | glow;
      }
   }
}
#else
void conv_0rgb1555_rgb565(void *output_, const void *input_,
      int width, int height,
      int out_stride, int in_stride)
{
   int h, w;
   const uint16_t *input = (const uint16_t*)input_;
   uint16_t *output = (uint16_t*)output_;

   for (h = 0; h < height; h++, output += out_stride >> 1, input += in_stride >> 1)
   {
      for (w = 0; w < width; w++)
      {
         uint16_t col = input[w];
         uint16_t rg = (col << 1) & ((0x1f << 11) | (0x1f << 6));
         uint16_t b = col & 0x1f;
         uint16_t glow = (col >> 4) & (1 << 5);
         output[w] = rg | b | glow;
      }
   }
}
#endif

#if defined(__SSE2__)
void conv_0rgb1555_argb8888(void *output_, const void *input_,
      int width, int height,
      int out_stride, int in_stride)
{
   int h, w;
   const uint16_t *input = (const uint16_t*)input_;
   uint32_t *output      = (uint32_t*)output_;

   const __m128i pix_mask_r  = _mm_set1_epi16(0x1f << 10);
   const __m128i pix_mask_gb = _mm_set1_epi16(0x1f <<  5);
   const __m128i mul15_mid   = _mm_set1_epi16(0x4200);
   const __m128i mul15_hi    = _mm_set1_epi16(0x0210);
   const __m128i a           = _mm_set1_epi16(0x00ff);

   int max_width = width - 7;

   for (h = 0; h < height; h++, output += out_stride >> 2, input += in_stride >> 1)
   {
      for (w = 0; w < max_width; w += 8)
      {
         const __m128i in = _mm_loadu_si128((const __m128i*)(input + w));
         __m128i r = _mm_and_si128(in, pix_mask_r);
         __m128i g = _mm_and_si128(in, pix_mask_gb);
         __m128i b = _mm_and_si128(_mm_slli_epi16(in, 5), pix_mask_gb);

         r = _mm_mulhi_epi16(r, mul15_hi);
         g = _mm_mulhi_epi16(g, mul15_mid);
         b = _mm_mulhi_epi16(b, mul15_mid);

         __m128i res_lo_bg = _mm_unpacklo_epi8(b, g);
         __m128i res_hi_bg = _mm_unpackhi_epi8(b, g);
         __m128i res_lo_ra = _mm_unpacklo_epi8(r, a);
         __m128i res_hi_ra = _mm_unpackhi_epi8(r, a);

         __m128i res_lo = _mm_or_si128(res_lo_bg, _mm_slli_si128(res_lo_ra, 2));
         __m128i res_hi = _mm_or_si128(res_hi_bg, _mm_slli_si128(res_hi_ra, 2));

         _mm_storeu_si128((__m128i*)(output + w + 0), res_lo);
         _mm_storeu_si128((__m128i*)(output + w + 4), res_hi);
      }

      for (; w < width; w++)
      {
         uint32_t col = input[w];
         uint32_t r = (col >> 10) & 0x1f;
         uint32_t g = (col >>  5) & 0x1f;
         uint32_t b = (col >>  0) & 0x1f;
         r = (r << 3) | (r >> 2);
         g = (g << 3) | (g >> 2);
         b = (b << 3) | (b >> 2);

         output[w] = (0xff << 24) | (r << 16) | (g << 8) | (b << 0);
      }
   }
}
#else
void conv_0rgb1555_argb8888(void *output_, const void *input_,
      int width, int height,
      int out_stride, int in_stride)
{
   int h, w;
   const uint16_t *input = (const uint16_t*)input_;
   uint32_t *output      = (uint32_t*)output_;

   for (h = 0; h < height; h++, output += out_stride >> 2, input += in_stride >> 1)
   {
      for (w = 0; w < width; w++)
      {
         uint32_t col = input[w];
         uint32_t r = (col >> 10) & 0x1f;
         uint32_t g = (col >>  5) & 0x1f;
         uint32_t b = (col >>  0) & 0x1f;
         r = (r << 3) | (r >> 2);
         g = (g << 3) | (g >> 2);
         b = (b << 3) | (b >> 2);

         output[w] = (0xffu << 24) | (r << 16) | (g << 8) | (b << 0);
      }
   }
}
#endif

#if defined(__SSE2__)
void conv_rgb565_argb8888(void *output_, const void *input_,
      int width, int height,
      int out_stride, int in_stride)
{
   int h, w;
   const uint16_t *input = (const uint16_t*)input_;
   uint32_t *output      = (uint32_t*)output_;

   const __m128i pix_mask_r = _mm_set1_epi16(0x1f << 10);
   const __m128i pix_mask_g = _mm_set1_epi16(0x3f <<  5);
   const __m128i pix_mask_b = _mm_set1_epi16(0x1f <<  5);
   const __m128i mul16_r    = _mm_set1_epi16(0x0210);
   const __m128i mul16_g    = _mm_set1_epi16(0x2080);
   const __m128i mul16_b    = _mm_set1_epi16(0x4200);
   const __m128i a          = _mm_set1_epi16(0x00ff);

   int max_width = width - 7;

   for (h = 0; h < height; h++, output += out_stride >> 2, input += in_stride >> 1)
   {
      for (w = 0; w < max_width; w += 8)
      {
         const __m128i in = _mm_loadu_si128((const __m128i*)(input + w));
         __m128i r = _mm_and_si128(_mm_srli_epi16(in, 1), pix_mask_r);
         __m128i g = _mm_and_si128(in, pix_mask_g);
         __m128i b = _mm_and_si128(_mm_slli_epi16(in, 5), pix_mask_b);

         r = _mm_mulhi_epi16(r, mul16_r);
         g = _mm_mulhi_epi16(g, mul16_g);
         b = _mm_mulhi_epi16(b, mul16_b);

         __m128i res_lo_bg = _mm_unpacklo_epi8(b, g);
         __m128i res_hi_bg = _mm_unpackhi_epi8(b, g);
         __m128i res_lo_ra = _mm_unpacklo_epi8(r, a);
         __m128i res_hi_ra = _mm_unpackhi_epi8(r, a);

         __m128i res_lo = _mm_or_si128(res_lo_bg, _mm_slli_si128(res_lo_ra, 2));
         __m128i res_hi = _mm_or_si128(res_hi_bg, _mm_slli_si128(res_hi_ra, 2));

         _mm_storeu_si128((__m128i*)(output + w + 0), res_lo);
         _mm_storeu_si128((__m128i*)(output + w + 4), res_hi);
      }

      for (; w < width; w++)
      {
         uint32_t col = input[w];
         uint32_t r = (col >> 11) & 0x1f;
         uint32_t g = (col >>  5) & 0x3f;
         uint32_t b = (col >>  0) & 0x1f;
         r = (r << 3) | (r >> 2);
         g = (g << 2) | (g >> 4);
         b = (b << 3) | (b >> 2);

         output[w] = (0xff << 24) | (r << 16) | (g << 8) | (b << 0);
      }
   }
}
#else
void conv_rgb565_argb8888(void *output_, const void *input_,
      int width, int height,
      int out_stride, int in_stride)
{
   int h, w;
   const uint16_t *input = (const uint16_t*)input_;
   uint32_t *output      = (uint32_t*)output_;

   for (h = 0; h < height; h++, output += out_stride >> 2, input += in_stride >> 1)
   {
      for (w = 0; w < width; w++)
      {
         uint32_t col = input[w];
         uint32_t r = (col >> 11) & 0x1f;
         uint32_t g = (col >>  5) & 0x3f;
         uint32_t b = (col >>  0) & 0x1f;
         r = (r << 3) | (r >> 2);
         g = (g << 2) | (g >> 4);
         b = (b << 3) | (b >> 2);

         output[w] = (0xffu << 24) | (r << 16) | (g << 8) | (b << 0);
      }
   }
}
#endif

#if defined(__SSE2__)
// :( TODO: Make this saner.
static inline void store_bgr24_sse2(void *output, __m128i a, __m128i b, __m128i c, __m128i d)
{
   const __m128i mask_0 = _mm_set_epi32(0, 0, 0, 0x00ffffff);
   const __m128i mask_1 = _mm_set_epi32(0, 0, 0x00ffffff, 0);
   const __m128i mask_2 = _mm_set_epi32(0, 0x00ffffff, 0, 0);
   const __m128i mask_3 = _mm_set_epi32(0x00ffffff, 0, 0, 0);

   __m128i a0 = _mm_and_si128(a, mask_0);
   __m128i a1 = _mm_srli_si128(_mm_and_si128(a, mask_1),  1);
   __m128i a2 = _mm_srli_si128(_mm_and_si128(a, mask_2),  2);
   __m128i a3 = _mm_srli_si128(_mm_and_si128(a, mask_3),  3);
   __m128i a4 = _mm_slli_si128(_mm_and_si128(b, mask_0), 12);
   __m128i a5 = _mm_slli_si128(_mm_and_si128(b, mask_1), 11);

   __m128i b0 = _mm_srli_si128(_mm_and_si128(b, mask_1), 5);
   __m128i b1 = _mm_srli_si128(_mm_and_si128(b, mask_2), 6);
   __m128i b2 = _mm_srli_si128(_mm_and_si128(b, mask_3), 7);
   __m128i b3 = _mm_slli_si128(_mm_and_si128(c, mask_0), 8);
   __m128i b4 = _mm_slli_si128(_mm_and_si128(c, mask_1), 7);
   __m128i b5 = _mm_slli_si128(_mm_and_si128(c, mask_2), 6);

   __m128i c0 = _mm_srli_si128(_mm_and_si128(c, mask_2), 10);
   __m128i c1 = _mm_srli_si128(_mm_and_si128(c, mask_3), 11);
   __m128i c2 = _mm_slli_si128(_mm_and_si128(d, mask_0),  4);
   __m128i c3 = _mm_slli_si128(_mm_and_si128(d, mask_1),  3);
   __m128i c4 = _mm_slli_si128(_mm_and_si128(d, mask_2),  2);
   __m128i c5 = _mm_slli_si128(_mm_and_si128(d, mask_3),  1);

   __m128i *out = (__m128i*)output;

   _mm_storeu_si128(out + 0,
         _mm_or_si128(a0, _mm_or_si128(a1, _mm_or_si128(a2, _mm_or_si128(a3, _mm_or_si128(a4, a5))))));

   _mm_storeu_si128(out + 1,
         _mm_or_si128(b0, _mm_or_si128(b1, _mm_or_si128(b2, _mm_or_si128(b3, _mm_or_si128(b4, b5))))));

   _mm_storeu_si128(out + 2,
         _mm_or_si128(c0, _mm_or_si128(c1, _mm_or_si128(c2, _mm_or_si128(c3, _mm_or_si128(c4, c5))))));
}

void conv_0rgb1555_bgr24(void *output_, const void *input_,
      int width, int height,
      int out_stride, int in_stride)
{
   int h, w;
   const uint16_t *input = (const uint16_t*)input_;
   uint8_t *output       = (uint8_t*)output_;

   const __m128i pix_mask_r  = _mm_set1_epi16(0x1f << 10);
   const __m128i pix_mask_gb = _mm_set1_epi16(0x1f <<  5);
   const __m128i mul15_mid   = _mm_set1_epi16(0x4200);
   const __m128i mul15_hi    = _mm_set1_epi16(0x0210);
   const __m128i a           = _mm_set1_epi16(0x00ff);

   int max_width = width - 15;

   for (h = 0; h < height; h++, output += out_stride, input += in_stride >> 1)
   {
      uint8_t *out = output;

      for (w = 0; w < max_width; w += 16, out += 48)
      {
         const __m128i in0 = _mm_loadu_si128((const __m128i*)(input + w + 0));
         const __m128i in1 = _mm_loadu_si128((const __m128i*)(input + w + 8));
         __m128i r0 = _mm_and_si128(in0, pix_mask_r);
         __m128i r1 = _mm_and_si128(in1, pix_mask_r);
         __m128i g0 = _mm_and_si128(in0, pix_mask_gb);
         __m128i g1 = _mm_and_si128(in1, pix_mask_gb);
         __m128i b0 = _mm_and_si128(_mm_slli_epi16(in0, 5), pix_mask_gb);
         __m128i b1 = _mm_and_si128(_mm_slli_epi16(in1, 5), pix_mask_gb);

         r0 = _mm_mulhi_epi16(r0, mul15_hi);
         r1 = _mm_mulhi_epi16(r1, mul15_hi);
         g0 = _mm_mulhi_epi16(g0, mul15_mid);
         g1 = _mm_mulhi_epi16(g1, mul15_mid);
         b0 = _mm_mulhi_epi16(b0, mul15_mid);
         b1 = _mm_mulhi_epi16(b1, mul15_mid);

         __m128i res_lo_bg0 = _mm_unpacklo_epi8(b0, g0);
         __m128i res_lo_bg1 = _mm_unpacklo_epi8(b1, g1);
         __m128i res_hi_bg0 = _mm_unpackhi_epi8(b0, g0);
         __m128i res_hi_bg1 = _mm_unpackhi_epi8(b1, g1);
         __m128i res_lo_ra0 = _mm_unpacklo_epi8(r0, a);
         __m128i res_lo_ra1 = _mm_unpacklo_epi8(r1, a);
         __m128i res_hi_ra0 = _mm_unpackhi_epi8(r0, a);
         __m128i res_hi_ra1 = _mm_unpackhi_epi8(r1, a);

         __m128i res_lo0 = _mm_or_si128(res_lo_bg0, _mm_slli_si128(res_lo_ra0, 2));
         __m128i res_lo1 = _mm_or_si128(res_lo_bg1, _mm_slli_si128(res_lo_ra1, 2));
         __m128i res_hi0 = _mm_or_si128(res_hi_bg0, _mm_slli_si128(res_hi_ra0, 2));
         __m128i res_hi1 = _mm_or_si128(res_hi_bg1, _mm_slli_si128(res_hi_ra1, 2));

         // Non-POT pixel sizes ftl :(
         store_bgr24_sse2(out, res_lo0, res_hi0, res_lo1, res_hi1);
      }

      for (; w < width; w++)
      {
         uint32_t col = input[w];
         uint32_t b = (col >>  0) & 0x1f;
         uint32_t g = (col >>  5) & 0x1f;
         uint32_t r = (col >> 10) & 0x1f;
         b = (b << 3) | (b >> 2);
         g = (g << 3) | (g >> 2);
         r = (r << 3) | (r >> 2);

         *out++ = b;
         *out++ = g;
         *out++ = r;
      }
   }
}

void conv_rgb565_bgr24(void *output_, const void *input_,
      int width, int height,
      int out_stride, int in_stride)
{
   int h, w;
   const uint16_t *input = (const uint16_t*)input_;
   uint8_t *output      = (uint8_t*)output_;

   const __m128i pix_mask_r = _mm_set1_epi16(0x1f << 10);
   const __m128i pix_mask_g = _mm_set1_epi16(0x3f <<  5);
   const __m128i pix_mask_b = _mm_set1_epi16(0x1f <<  5);
   const __m128i mul16_r    = _mm_set1_epi16(0x0210);
   const __m128i mul16_g    = _mm_set1_epi16(0x2080);
   const __m128i mul16_b    = _mm_set1_epi16(0x4200);
   const __m128i a          = _mm_set1_epi16(0x00ff);

   int max_width = width - 15;

   for (h = 0; h < height; h++, output += out_stride, input += in_stride >> 1)
   {
      uint8_t *out = output;

      for (w = 0; w < max_width; w += 16, out += 48)
      {
         const __m128i in0 = _mm_loadu_si128((const __m128i*)(input + w));
         const __m128i in1 = _mm_loadu_si128((const __m128i*)(input + w + 8));
         __m128i r0 = _mm_and_si128(_mm_srli_epi16(in0, 1), pix_mask_r);
         __m128i g0 = _mm_and_si128(in0, pix_mask_g);
         __m128i b0 = _mm_and_si128(_mm_slli_epi16(in0, 5), pix_mask_b);
         __m128i r1 = _mm_and_si128(_mm_srli_epi16(in1, 1), pix_mask_r);
         __m128i g1 = _mm_and_si128(in1, pix_mask_g);
         __m128i b1 = _mm_and_si128(_mm_slli_epi16(in1, 5), pix_mask_b);

         r0 = _mm_mulhi_epi16(r0, mul16_r);
         g0 = _mm_mulhi_epi16(g0, mul16_g);
         b0 = _mm_mulhi_epi16(b0, mul16_b);
         r1 = _mm_mulhi_epi16(r1, mul16_r);
         g1 = _mm_mulhi_epi16(g1, mul16_g);
         b1 = _mm_mulhi_epi16(b1, mul16_b);

         __m128i res_lo_bg0 = _mm_unpacklo_epi8(b0, g0);
         __m128i res_hi_bg0 = _mm_unpackhi_epi8(b0, g0);
         __m128i res_lo_ra0 = _mm_unpacklo_epi8(r0, a);
         __m128i res_hi_ra0 = _mm_unpackhi_epi8(r0, a);
         __m128i res_lo_bg1 = _mm_unpacklo_epi8(b1, g1);
         __m128i res_hi_bg1 = _mm_unpackhi_epi8(b1, g1);
         __m128i res_lo_ra1 = _mm_unpacklo_epi8(r1, a);
         __m128i res_hi_ra1 = _mm_unpackhi_epi8(r1, a);

         __m128i res_lo0 = _mm_or_si128(res_lo_bg0, _mm_slli_si128(res_lo_ra0, 2));
         __m128i res_hi0 = _mm_or_si128(res_hi_bg0, _mm_slli_si128(res_hi_ra0, 2));
         __m128i res_lo1 = _mm_or_si128(res_lo_bg1, _mm_slli_si128(res_lo_ra1, 2));
         __m128i res_hi1 = _mm_or_si128(res_hi_bg1, _mm_slli_si128(res_hi_ra1, 2));

         store_bgr24_sse2(out, res_lo0, res_hi0, res_lo1, res_hi1);
      }

      for (; w < width; w++)
      {
         uint32_t col = input[w];
         uint32_t r = (col >> 11) & 0x1f;
         uint32_t g = (col >>  5) & 0x3f;
         uint32_t b = (col >>  0) & 0x1f;
         r = (r << 3) | (r >> 2);
         g = (g << 2) | (g >> 4);
         b = (b << 3) | (b >> 2);

         *out++ = b;
         *out++ = g;
         *out++ = r;
      }
   }
}
#else
void conv_0rgb1555_bgr24(void *output_, const void *input_,
      int width, int height,
      int out_stride, int in_stride)
{
   int h, w;
   const uint16_t *input = (const uint16_t*)input_;
   uint8_t *output       = (uint8_t*)output_;

   for (h = 0; h < height; h++, output += out_stride, input += in_stride >> 1)
   {
      uint8_t *out = output;
      for (w = 0; w < width; w++)
      {
         uint32_t col = input[w];
         uint32_t b = (col >>  0) & 0x1f;
         uint32_t g = (col >>  5) & 0x1f;
         uint32_t r = (col >> 10) & 0x1f;
         b = (b << 3) | (b >> 2);
         g = (g << 3) | (g >> 2);
         r = (r << 3) | (r >> 2);

         *out++ = b;
         *out++ = g;
         *out++ = r;
      }
   }
}

void conv_rgb565_bgr24(void *output_, const void *input_,
      int width, int height,
      int out_stride, int in_stride)
{
   int h, w;
   const uint16_t *input = (const uint16_t*)input_;
   uint8_t *output       = (uint8_t*)output_;

   for (h = 0; h < height; h++, output += out_stride, input += in_stride >> 1)
   {
      uint8_t *out = output;
      for (w = 0; w < width; w++)
      {
         uint32_t col = input[w];
         uint32_t b = (col >>  0) & 0x1f;
         uint32_t g = (col >>  5) & 0x3f;
         uint32_t r = (col >> 11) & 0x1f;
         b = (b << 3) | (b >> 2);
         g = (g << 2) | (g >> 4);
         r = (r << 3) | (r >> 2);

         *out++ = b;
         *out++ = g;
         *out++ = r;
      }
   }
}
#endif

void conv_bgr24_argb8888(void *output_, const void *input_,
      int width, int height,
      int out_stride, int in_stride)
{
   int h, w;
   const uint8_t *input = (const uint8_t*)input_;
   uint32_t *output     = (uint32_t*)output_;

   for (h = 0; h < height; h++, output += out_stride >> 2, input += in_stride)
   {
      const uint8_t *inp = input;
      for (w = 0; w < width; w++)
      {
         uint32_t b = *inp++;
         uint32_t g = *inp++;
         uint32_t r = *inp++;
         output[w] = (0xffu << 24) | (r << 16) | (g << 8) | (b << 0);
      }
   }
}

void conv_argb8888_0rgb1555(void *output_, const void *input_,
      int width, int height,
      int out_stride, int in_stride)
{
   int h, w;
   const uint32_t *input = (const uint32_t*)input_;
   uint16_t *output      = (uint16_t*)output_;

   for (h = 0; h < height; h++, output += out_stride >> 1, input += in_stride >> 2)
   {
      for (w = 0; w < width; w++)
      {
         uint32_t col = input[w];
         uint16_t r = (col >> 19) & 0x1f;
         uint16_t g = (col >> 11) & 0x1f;
         uint16_t b = (col >>  3) & 0x1f;
         output[w] = (r << 10) | (g << 5) | (b << 0);
      }
   }
}

#if defined(__SSE2__)
void conv_argb8888_bgr24(void *output_, const void *input_,
      int width, int height,
      int out_stride, int in_stride)
{
   int h, w;
   const uint32_t *input = (const uint32_t*)input_;
   uint8_t *output       = (uint8_t*)output_;

   int max_width = width - 15;

   for (h = 0; h < height; h++, output += out_stride, input += in_stride >> 2)
   {
      uint8_t *out = output;

      for (w = 0; w < max_width; w += 16, out += 48)
      {
         store_bgr24_sse2(out,
               _mm_loadu_si128((const __m128i*)(input + w +  0)),
               _mm_loadu_si128((const __m128i*)(input + w +  4)),
               _mm_loadu_si128((const __m128i*)(input + w +  8)),
               _mm_loadu_si128((const __m128i*)(input + w + 12)));
      }

      for (; w < width; w++)
      {
         uint32_t col = input[w];
         *out++ = (uint8_t)(col >>  0);
         *out++ = (uint8_t)(col >>  8);
         *out++ = (uint8_t)(col >> 16);
      }
   }
}
#else
void conv_argb8888_bgr24(void *output_, const void *input_,
      int width, int height,
      int out_stride, int in_stride)
{
   int h, w;
   const uint32_t *input = (const uint32_t*)input_;
   uint8_t *output       = (uint8_t*)output_;

   for (h = 0; h < height; h++, output += out_stride, input += in_stride >> 2)
   {
      uint8_t *out = output;
      for (w = 0; w < width; w++)
      {
         uint32_t col = input[w];
         *out++ = (uint8_t)(col >>  0);
         *out++ = (uint8_t)(col >>  8);
         *out++ = (uint8_t)(col >> 16);
      }
   }
}
#endif

void conv_argb8888_abgr8888(void *output_, const void *input_,
      int width, int height,
      int out_stride, int in_stride)
{
   int h, w;
   const uint32_t *input = (const uint32_t*)input_;
   uint32_t *output      = (uint32_t*)output_;

   for (h = 0; h < height; h++, output += out_stride >> 2, input += in_stride >> 2)
   {
      for (w = 0; w < width; w++)
      {
         uint32_t col = input[w];
         output[w] = ((col << 16) & 0xff0000) | ((col >> 16) & 0xff) | (col & 0xff00ff00);
      }
   }
}

#define YUV_SHIFT 6
#define YUV_OFFSET (1 << (YUV_SHIFT - 1))
#define YUV_MAT_Y (1 << 6)
#define YUV_MAT_U_G (-22)
#define YUV_MAT_U_B (113)
#define YUV_MAT_V_R (90)
#define YUV_MAT_V_G (-46)
#if defined(__SSE2__)
void conv_yuyv_argb8888(void *output_, const void *input_,
      int width, int height,
      int out_stride, int in_stride)
{
   int h, w;
   const uint8_t *input = (const uint8_t*)input_;
   uint32_t *output     = (uint32_t*)output_;

   const __m128i mask_y = _mm_set1_epi16(0xffu);
   const __m128i mask_u = _mm_set1_epi32(0xffu << 8);
   const __m128i mask_v = _mm_set1_epi32(0xffu << 24);
   const __m128i chroma_offset = _mm_set1_epi16(128);
   const __m128i round_offset = _mm_set1_epi16(YUV_OFFSET);

   const __m128i yuv_mul = _mm_set1_epi16(YUV_MAT_Y);
   const __m128i u_g_mul = _mm_set1_epi16(YUV_MAT_U_G);
   const __m128i u_b_mul = _mm_set1_epi16(YUV_MAT_U_B);
   const __m128i v_r_mul = _mm_set1_epi16(YUV_MAT_V_R);
   const __m128i v_g_mul = _mm_set1_epi16(YUV_MAT_V_G);
   const __m128i a       = _mm_cmpeq_epi16(_mm_setzero_si128(), _mm_setzero_si128());

   for (h = 0; h < height; h++, output += out_stride >> 2, input += in_stride)
   {
      const uint8_t *src = input;
      uint32_t *dst = output;

      // Each loop processes 16 pixels.
      for (w = 0; w + 16 <= width; w += 16, src += 32, dst += 16)
      {
         __m128i yuv0 = _mm_loadu_si128((const __m128i*)(src +  0)); // [Y0, U0, Y1, V0, Y2, U1, Y3, V1, ...]
         __m128i yuv1 = _mm_loadu_si128((const __m128i*)(src + 16)); // [Y0, U0, Y1, V0, Y2, U1, Y3, V1, ...]

         __m128i y0 = _mm_and_si128(yuv0, mask_y); // [Y0, Y1, Y2, ...] (16-bit)
         __m128i u0 = _mm_and_si128(yuv0, mask_u); // [0, U0, 0, 0, 0, U1, 0, 0, ...]
         __m128i v0 = _mm_and_si128(yuv0, mask_v); // [0, 0, 0, V1, 0, , 0, V1, ...]
         __m128i y1 = _mm_and_si128(yuv1, mask_y); // [Y0, Y1, Y2, ...] (16-bit)
         __m128i u1 = _mm_and_si128(yuv1, mask_u); // [0, U0, 0, 0, 0, U1, 0, 0, ...]
         __m128i v1 = _mm_and_si128(yuv1, mask_v); // [0, 0, 0, V1, 0, , 0, V1, ...]

         // Juggle around to get U and V in the same 16-bit format as Y.
         u0 = _mm_srli_si128(u0, 1);
         v0 = _mm_srli_si128(v0, 3);
         u1 = _mm_srli_si128(u1, 1);
         v1 = _mm_srli_si128(v1, 3);
         __m128i u = _mm_packs_epi32(u0, u1);
         __m128i v = _mm_packs_epi32(v0, v1);

         // Apply YUV offsets (U, V) -= (-128, -128)
         u = _mm_sub_epi16(u, chroma_offset);
         v = _mm_sub_epi16(v, chroma_offset);

         // Upscale chroma horizontally (nearest)
         u0 = _mm_unpacklo_epi16(u, u);
         u1 = _mm_unpackhi_epi16(u, u);
         v0 = _mm_unpacklo_epi16(v, v);
         v1 = _mm_unpackhi_epi16(v, v);

         // Apply transformations
         y0 = _mm_mullo_epi16(y0, yuv_mul);
         y1 = _mm_mullo_epi16(y1, yuv_mul);
         __m128i u0_g   = _mm_mullo_epi16(u0, u_g_mul);
         __m128i u1_g   = _mm_mullo_epi16(u1, u_g_mul);
         __m128i u0_b   = _mm_mullo_epi16(u0, u_b_mul);
         __m128i u1_b   = _mm_mullo_epi16(u1, u_b_mul);
         __m128i v0_r   = _mm_mullo_epi16(v0, v_r_mul);
         __m128i v1_r   = _mm_mullo_epi16(v1, v_r_mul);
         __m128i v0_g   = _mm_mullo_epi16(v0, v_g_mul);
         __m128i v1_g   = _mm_mullo_epi16(v1, v_g_mul);

         // Add contibutions from the transformed components.
         __m128i r0 = _mm_srai_epi16(_mm_adds_epi16(_mm_adds_epi16(y0, v0_r), round_offset), YUV_SHIFT);
         __m128i g0 = _mm_srai_epi16(_mm_adds_epi16(_mm_adds_epi16(_mm_adds_epi16(y0, v0_g), u0_g), round_offset), YUV_SHIFT);
         __m128i b0 = _mm_srai_epi16(_mm_adds_epi16(_mm_adds_epi16(y0, u0_b), round_offset), YUV_SHIFT);

         __m128i r1 = _mm_srai_epi16(_mm_adds_epi16(_mm_adds_epi16(y1, v1_r), round_offset), YUV_SHIFT);
         __m128i g1 = _mm_srai_epi16(_mm_adds_epi16(_mm_adds_epi16(_mm_adds_epi16(y1, v1_g), u1_g), round_offset), YUV_SHIFT);
         __m128i b1 = _mm_srai_epi16(_mm_adds_epi16(_mm_adds_epi16(y1, u1_b), round_offset), YUV_SHIFT);

         // Saturate into 8-bit.
         r0 = _mm_packus_epi16(r0, r1);
         g0 = _mm_packus_epi16(g0, g1);
         b0 = _mm_packus_epi16(b0, b1);

         // Interleave into ARGB.
         __m128i res_lo_bg = _mm_unpacklo_epi8(b0, g0);
         __m128i res_hi_bg = _mm_unpackhi_epi8(b0, g0);
         __m128i res_lo_ra = _mm_unpacklo_epi8(r0, a);
         __m128i res_hi_ra = _mm_unpackhi_epi8(r0, a);
         __m128i res0 = _mm_unpacklo_epi16(res_lo_bg, res_lo_ra);
         __m128i res1 = _mm_unpackhi_epi16(res_lo_bg, res_lo_ra);
         __m128i res2 = _mm_unpacklo_epi16(res_hi_bg, res_hi_ra);
         __m128i res3 = _mm_unpackhi_epi16(res_hi_bg, res_hi_ra);

         _mm_storeu_si128((__m128i*)(dst +  0), res0);
         _mm_storeu_si128((__m128i*)(dst +  4), res1);
         _mm_storeu_si128((__m128i*)(dst +  8), res2);
         _mm_storeu_si128((__m128i*)(dst + 12), res3);
      }

      // Finish off the rest (if any) in C.
      for (; w < width; w += 2, src += 4, dst += 2)
      {
         int y0 = src[0];
         int  u = src[1] - 128;
         int y1 = src[2];
         int  v = src[3] - 128;

         uint8_t r0 = clamp_8bit((YUV_MAT_Y * y0 +                   YUV_MAT_V_R * v + YUV_OFFSET) >> YUV_SHIFT);
         uint8_t g0 = clamp_8bit((YUV_MAT_Y * y0 + YUV_MAT_U_G * u + YUV_MAT_V_G * v + YUV_OFFSET) >> YUV_SHIFT);
         uint8_t b0 = clamp_8bit((YUV_MAT_Y * y0 + YUV_MAT_U_B * u                   + YUV_OFFSET) >> YUV_SHIFT);

         uint8_t r1 = clamp_8bit((YUV_MAT_Y * y1 +                   YUV_MAT_V_R * v + YUV_OFFSET) >> YUV_SHIFT);
         uint8_t g1 = clamp_8bit((YUV_MAT_Y * y1 + YUV_MAT_U_G * u + YUV_MAT_V_G * v + YUV_OFFSET) >> YUV_SHIFT);
         uint8_t b1 = clamp_8bit((YUV_MAT_Y * y1 + YUV_MAT_U_B * u                   + YUV_OFFSET) >> YUV_SHIFT);

         dst[0] = 0xff000000u | (r0 << 16) | (g0 << 8) | (b0 << 0);
         dst[1] = 0xff000000u | (r1 << 16) | (g1 << 8) | (b1 << 0);
      }
   }
}
#else
void conv_yuyv_argb8888(void *output_, const void *input_,
      int width, int height,
      int out_stride, int in_stride)
{
   int h, w;
   const uint8_t *input = (const uint8_t*)input_;
   uint32_t *output     = (uint32_t*)output_;

   for (h = 0; h < height; h++, output += out_stride >> 2, input += in_stride)
   {
      const uint8_t *src = input;
      uint32_t *dst = output;

      for (w = 0; w < width; w += 2, src += 4, dst += 2)
      {
         int y0 = src[0];
         int  u = src[1] - 128;
         int y1 = src[2];
         int  v = src[3] - 128;

         uint8_t r0 = clamp_8bit((YUV_MAT_Y * y0 +                   YUV_MAT_V_R * v + YUV_OFFSET) >> YUV_SHIFT);
         uint8_t g0 = clamp_8bit((YUV_MAT_Y * y0 + YUV_MAT_U_G * u + YUV_MAT_V_G * v + YUV_OFFSET) >> YUV_SHIFT);
         uint8_t b0 = clamp_8bit((YUV_MAT_Y * y0 + YUV_MAT_U_B * u                   + YUV_OFFSET) >> YUV_SHIFT);

         uint8_t r1 = clamp_8bit((YUV_MAT_Y * y1 +                   YUV_MAT_V_R * v + YUV_OFFSET) >> YUV_SHIFT);
         uint8_t g1 = clamp_8bit((YUV_MAT_Y * y1 + YUV_MAT_U_G * u + YUV_MAT_V_G * v + YUV_OFFSET) >> YUV_SHIFT);
         uint8_t b1 = clamp_8bit((YUV_MAT_Y * y1 + YUV_MAT_U_B * u                   + YUV_OFFSET) >> YUV_SHIFT);

         dst[0] = 0xff000000u | (r0 << 16) | (g0 << 8) | (b0 << 0);
         dst[1] = 0xff000000u | (r1 << 16) | (g1 << 8) | (b1 << 0);
      }
   }
}
#endif

void conv_copy(void *output_, const void *input_,
      int width, int height,
      int out_stride, int in_stride)
{
   int h;
   int copy_len = abs(out_stride);
   if (abs(in_stride) < copy_len)
      copy_len = abs(in_stride);

   const uint8_t *input = (const uint8_t*)input_;
   uint8_t *output      = (uint8_t*)output_;

   for (h = 0; h < height; h++, output += out_stride, input += in_stride)
      memcpy(output, input, copy_len);
}
