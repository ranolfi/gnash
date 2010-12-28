// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010 Free Software Foundation, Inc.
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA

///
/// Author: Visor <cutevisor@gmail.com>
/// Heavily hacked by Rob <rob@welcomehome.org> to work with Gnash
/// git master.
///

#ifndef GNASH_RENDER_HANDLER_OVG_BITMAP_H
#define GNASH_RENDER_HANDLER_OVG_BITMAP_H

#include "Geometry.h"
#include "CachedBitmap.h"
#include "GnashImage.h"
#include "Renderer.h"
#include "openvg/Renderer_ovg.h"

namespace gnash {

namespace renderer {

namespace openvg {

class bitmap_info_ovg : public CachedBitmap
{
public:
    /// Set line and fill styles for mesh & line_strip rendering.
    enum bitmap_wrap_mode { WRAP_REPEAT, WRAP_CLAMP };
    
    bitmap_info_ovg(std::auto_ptr<image::GnashImage> im)
        : _image(im.release())
    {
        GNASH_REPORT_FUNCTION;
    }
  
    bitmap_info_ovg(std::auto_ptr<image::GnashImage> im,
                    VGImageFormat pixelformat, VGPaint vgpaint)
        : _image(im.release()),
          _pixel_format(pixelformat),
          _vgpaint(vgpaint)
    {
        GNASH_REPORT_FUNCTION;
        
        size_t width = _image->width();
        size_t height = _image->height();
        
        _vgimage = vgCreateImage(VG_sRGB_565, width, height,
                                 VG_IMAGE_QUALITY_FASTER);    
        
        vgImageSubData(_vgimage, _image->begin(), width * 4, VG_sRGB_565,
                       0, 0, width, height);
        
        _tex_size += width * height * 4;
        log_debug("Add Texture size:%d (%d x %d x %dbpp)", width * height * 4, 
                  width, height, 4);
        log_debug("Current Texture size: %d", _tex_size);
    } 

    ~bitmap_info_ovg()
    {
        GNASH_REPORT_FUNCTION;
        
        _tex_size -= _image->width() * _image->height() * 4;
        log_debug(_("Remove Texture size:%d (%d x %d x %dbpp)"),
                  _image->width() * _image->height() * 4,
                  _image->width(), _image->height(), 4);
        log_debug(_("Current Texture size: %d"), _tex_size);
        
        vgDestroyImage(_vgimage);
    }

    void dispose()  { _image.reset(); }
    bool disposed() const { return !_image.get(); }

    image::GnashImage& image() {
        GNASH_REPORT_FUNCTION;
        if (_image) {
            return *_image;
        }
    }    

    void apply(const gnash::SWFMatrix& bitmap_matrix,
               bitmap_wrap_mode wrap_mode) const
    {
        GNASH_REPORT_FUNCTION;
        gnash::SWFMatrix mat;
        VGfloat     vmat[9];
        
        mat = bitmap_matrix;
        
        vgSetParameteri (_vgpaint, VG_PAINT_TYPE, VG_PAINT_TYPE_PATTERN);
        vgPaintPattern (_vgpaint, _vgimage);
        
        mat.invert();
        memset(vmat, 0, sizeof(vmat));
        vmat[0] = mat.sx  / 65536.0f;
        vmat[1] = mat.shx / 65536.0f;
        vmat[3] = mat.shy / 65536.0f;
        vmat[4] = mat.sy  / 65536.0f;
        vmat[6] = mat.tx;
        vmat[7] = mat.ty;
        
        vgSeti (VG_MATRIX_MODE, VG_MATRIX_FILL_PAINT_TO_USER);
        vgLoadMatrix (vmat);
        vgSeti (VG_MATRIX_MODE, VG_MATRIX_STROKE_PAINT_TO_USER);
        vgLoadMatrix (vmat);
        vgSeti (VG_MATRIX_MODE, VG_MATRIX_PATH_USER_TO_SURFACE);
        
        if (wrap_mode == WRAP_CLAMP) {  
            vgSetParameteri (_vgpaint, VG_PAINT_PATTERN_TILING_MODE, VG_TILE_PAD);
        } else {
            vgSetParameteri (_vgpaint, VG_PAINT_PATTERN_TILING_MODE, VG_TILE_REPEAT);
        }
    }

    // Accessors for the GnashImage internal data
    int getWidth() { return _image->width(); };
    int getHeight() { return _image->height(); };
    boost::uint8_t *getData() const { return _image->begin(); };

private:    
    boost::scoped_ptr<image::GnashImage> _image;
    VGImageFormat   _pixel_format;
    mutable VGImage _vgimage;
    VGPaint         _vgpaint;
    int             _tex_size;
};

} // namespace gnash::renderer::openvg
} // namespace gnash::renderer
} // namespace gnash

#endif // __RENDER_HANDLER_OVG_BITMAP_H__

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
