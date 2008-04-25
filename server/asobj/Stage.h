// 
//   Copyright (C) 2005, 2006, 2007, 2008 Free Software Foundation, Inc.
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
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//

#ifndef __GNASH_ASOBJ_STAGE_H__
#define __GNASH_ASOBJ_STAGE_H__

#include "as_object.h" // for inheritance
#include "movie_root.h" // for access to scaleMode

#include <list>

namespace gnash {

/// This is the Stage ActionScript object.
//
/// Some Stage methods are implemented in movie_root, because
/// it provides the interface to the Gui and/or all the values
/// required are necessarily in movie_root:
///
/// - scaleMode
/// - width
/// - height

class Stage: public as_object
{

public:
    
    enum DisplayState {
		normal,
		fullScreen
	};

	Stage();
	
	/// Receive a resize event.
	void onResize();

	/// Notify all listeners about a resize event
	void notifyResize();

    /// Get present align mode
    const std::string& getAlignMode() const { return _alignMode; }

    /// Set align mode
    void setAlignMode(const std::string& mode);

	/// Set display state 
	void setDisplayState(DisplayState state);

	/// Set display state 
	DisplayState getDisplayState() const { return _displayState; }

private:

	std::string _alignMode;
	
	DisplayState _displayState;
};

/// Register native functions with the VM.
void registerStageNative(as_object& o);

/// Initialize the global Stage class
void stage_class_init(as_object& global);
  
} // end of gnash namespace

// __GNASH_ASOBJ_STAGE_H__
#endif

